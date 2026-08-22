#include "expand.h"
#include "../utils/str/str.h"
#include "../input/input.h"
#include "../context.h"
#include "execute.h"
#include "process.h"
#include "shell_state.h"
#include <stdio.h>
#include <unistd.h>

char **argv_build(Arena *arena, Vector *args) {
    char **argv = arena_push(arena, (args->size + 1) * sizeof(char*));
    if (argv == NULL)
        return NULL;
    for (size_t i = 0; i < args->size; i++) {
        String *arg = ((String**)args->data)[i];
        argv[i] = string_to_c_str(arena, arg);
        if (argv[i] == NULL)
            return NULL;
    }
    argv[args->size] = NULL;
    return argv;
}

String *expand_cmd_subst(SyntaxTreeNode *node) {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        return NULL;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return NULL;
    }

    if (pid == CHILD_PID) {
        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        if (close(fd[0]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
        if (close(fd[1]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
        SyntaxTreeNode *child = *(SyntaxTreeNode**)node->data.children->data;
        int exit_code = exec_tree(child);
        exit(exit_code);
    }
    if (close(fd[1]) == -1) {
        perror("close");
        return NULL;
    }
    String *out = read_eof(fd[0], MAX_CMD_SUBST_INPUT);
    if (close(fd[0]) == -1)
        perror("close");

    int status;
    if (wait_for_pid(pid, &status) == -1) {
        perror("waitpid");
        set_last_exit_code(EXEC_INTERNAL_ERROR);
    } else
        set_last_exit_code(exit_code_from_status(status));

    return out;
}

String *expand_var(String *name) {
    char *c_str_name = string_to_c_str(arena, name);
    if (c_str_name == NULL)
        return NULL;

    if (STR_EQ(c_str_name, "?"))
        return string_from_int(arena, get_last_exit_code());
    if (STR_EQ(c_str_name, "$")) {
        pid_t pid = getpid();
        return string_from_int(arena, pid);
    }

    char *c_str_text = getenv(c_str_name);
    if (c_str_text == NULL)
        c_str_text = "";
    return string_from_cstr(arena, c_str_text);
}

String *expand_fragment(SyntaxTreeNode *node) {
    switch (node->type) {
        case TEXT:      return node->data.text;
        case CMD_SUBST: return expand_cmd_subst(node);
        case VAR:       return expand_var(node->data.text);
        default:        return NULL;
    }
}

String *expand_arg(Arena *arena, SyntaxTreeNode *node) {
    String *arg = string_from_empty(arena);
    if (arg == NULL)
        return NULL;
    for (size_t i = 0; i < node->data.children->size; i++) {
        SyntaxTreeNode *child = ((SyntaxTreeNode**)node->data.children->data)[i];
        String *fragment = expand_fragment(child);
        if (fragment == NULL)
            return NULL;
        arg = string_concat(arena, arg, fragment);
        if (arg == NULL)
            return NULL;
    }
    return arg;
}

