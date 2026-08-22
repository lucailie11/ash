#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int builtin_exit(void) {
    exit(EXIT_SUCCESS);
    return EXIT_FAILURE;
}

int builtin_cd(char **argv) {
    char *dir = argv[1];
    if (dir == NULL) {
        dir = getenv("HOME");
        if (dir == NULL) {
            perror("cd");
            return EXIT_FAILURE;
        }
    }

    if (chdir(dir) == -1) {
        perror("cd");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
