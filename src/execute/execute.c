#include "execute.h"
#include "process.h"
#include "shell_state.h"
#include "builtins.h"
#include "expand.h"
#include "../context.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int exec_argv(char **argv) {
    if (argv == NULL || argv[0] == NULL)
        return EXIT_SUCCESS;

    char *cmd = argv[0];
    if (STR_EQ(cmd, "exit"))
        return builtin_exit();

    if (STR_EQ(cmd, "cd"))
        return builtin_cd(argv);

    return process_spawn(argv);
}

int exec_cmd(SyntaxTreeNode *node) {
    Vector *args = vector_create(arena);
    if (args == NULL) {
        perror("ash");
        return EXEC_INTERNAL_ERROR;
    }
    for (size_t i = 0; i < node->data.children->size; i++) {
        SyntaxTreeNode *child = ((SyntaxTreeNode**)node->data.children->data)[i];
        String *arg = expand_arg(arena, child);
        if (arg == NULL || !vector_push(arena, args, arg)) {
            perror("ash");
            return EXEC_INTERNAL_ERROR;
        }
    }

    char **argv = argv_build(arena, args);
    if (argv == NULL) {
        perror("ash");
        return EXEC_INTERNAL_ERROR;
    }
    return exec_argv(argv);
}

int exec_subshell(SyntaxTreeNode *node) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXEC_INTERNAL_ERROR;
    }
    if (pid == CHILD_PID) {
        SyntaxTreeNode *child = *((SyntaxTreeNode**)node->data.children->data);
        int exit_code = exec_tree(child);
        exit(exit_code);
    }
    int status;
    if (wait_for_pid(pid, &status) == -1) {
        perror("waitpid");
        return EXEC_INTERNAL_ERROR;
    }
    return exit_code_from_status(status);
}

int exec_subgroup(SyntaxTreeNode *node) {
    SyntaxTreeNode *child = *((SyntaxTreeNode**)node->data.children->data);
    return exec_tree(child);
}

int exec_pipeline(SyntaxTreeNode *node) {
    size_t n = node->data.children->size;
    SyntaxTreeNode **children = node->data.children->data;

    pid_t pids[n];
    int fd_in = -1, fd_out = -1;
    for (size_t i = 0; i < n; i++) {
        int fd[2];
        if (i < n - 1) {
            if (pipe(fd) == -1) {
                perror("pipe");
                return EXEC_INTERNAL_ERROR;
            }
            fd_out = fd[1];
        }

        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return EXEC_INTERNAL_ERROR;
        }
        if (pids[i] == CHILD_PID) {
            if (i > 0) {
                if (dup2(fd_in, STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                if (close(fd_in) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }
            }
            if (i < n - 1) {
                if (close(fd[0]) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd_out, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                if (close(fd_out) == -1) {
                    perror("close");
                    exit(EXIT_FAILURE);
                }
            }
            int exit_code = exec_tree(children[i]);
            exit(exit_code);
        }

        if (i > 0 && close(fd_in) == -1) {
            perror("close");
            return EXEC_INTERNAL_ERROR;
        }
        if (i < n - 1) {
            fd_in = fd[0];
            if (close(fd_out) == -1) {
                perror("close");
                return EXEC_INTERNAL_ERROR;
            }
        }
    }

    int last_exit_code = 0;
    for (size_t i = 0; i < n; i++) {
        int status;
        if (wait_for_pid(pids[i], &status) == -1) {
            perror("waitpid");
            last_exit_code = EXEC_INTERNAL_ERROR;
            continue;
        }
        last_exit_code = exit_code_from_status(status);
    }

    return last_exit_code;
}

int exec_and_or(SyntaxTreeNode *node) {
    size_t n = node->data.children->size;
    if (n == 0)
        return EXIT_SUCCESS;
    SyntaxTreeNode **children = node->data.children->data;
    int last_exit_code = exec_tree(children[0]);
    for (size_t i = 1; i < n; i++) {
        switch (children[i - 1]->separator) {
            case AND:
                if (get_last_exit_code() == 0)
                    last_exit_code = exec_tree(children[i]);
                break;
            case OR:
                if (get_last_exit_code() != 0)
                    last_exit_code = exec_tree(children[i]);
                break;
            default:
                return EXIT_FAILURE;
        }
    }
    return last_exit_code;
}

int exec_cmd_list(SyntaxTreeNode *node) {
    size_t n = node->data.children->size;
    SyntaxTreeNode **children = node->data.children->data;
    int last_exit_code = EXIT_SUCCESS;
    for (size_t i = 0; i < n; i++) {
        int exit_code = EXIT_SUCCESS;
        switch (children[i]->separator)  {
            case WAIT:
                exit_code = exec_tree(children[i]);
                break;
            case BACKGROUND: {
                pid_t pid = fork();
                if (pid == -1) {
                    perror("fork");
                    return EXEC_INTERNAL_ERROR;
                }
                if (pid == CHILD_PID) {
                    exit_code = exec_tree(children[i]);
                    exit(exit_code);
                }
                exit_code = EXIT_SUCCESS;
                break;
            }
            default:
                return EXIT_FAILURE;
        }
        last_exit_code = exit_code;
    }
    return last_exit_code;
}

int exec_tree(SyntaxTreeNode *root) {
    int exit_code;
    switch (root->type) {
        case CMD:      exit_code = exec_cmd(root);      break;
        case SUBSHELL: exit_code = exec_subshell(root); break;
        case SUBGROUP: exit_code = exec_subgroup(root); break;
        case PIPELINE: exit_code = exec_pipeline(root); break;
        case AND_OR:   exit_code = exec_and_or(root);   break;
        case CMD_LIST: exit_code = exec_cmd_list(root); break;
        default:       exit_code = EXIT_FAILURE;
    }
    set_last_exit_code(exit_code);
    return exit_code;
}
