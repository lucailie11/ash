#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

pid_t wait_for_pid(pid_t pid, int *status) {
    pid_t result;
    while ((result = waitpid(pid, status, 0)) == -1 && errno == EINTR);
    return result;
}

int process_spawn(char **argv) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXEC_INTERNAL_ERROR;
    }
    if (pid == CHILD_PID) {
        execvp(argv[0], argv);
        perror("ash");
        exit(EXIT_FAILURE);
    }
    int status;
    if (wait_for_pid(pid, &status) == -1) {
        perror("waitpid");
        return EXEC_INTERNAL_ERROR;
    }
    return exit_code_from_status(status);
}

int exit_code_from_status(int status) {
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);
    return EXEC_INTERNAL_ERROR;
}
