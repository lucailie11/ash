#include "input.h"
#include "../context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define LINE_PREFIX "$ "

void print_line_prefix() {
    printf("%s", LINE_PREFIX);
}

String *read_eof(int fd, size_t bufsize) {
    char *buffer = arena_push(arena, bufsize);
    if (buffer == NULL)
        return NULL;

    size_t size = 0;
    ssize_t n;
    while (size < bufsize && (n = read(fd, buffer + size, bufsize - size)) > 0)
        size += (size_t)n;

    if (n == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    return string_create(arena, buffer, size);
}

String *read_line(FILE *file, size_t bufsize) {
    if (file == NULL)  {
        errno = EINVAL;
        return NULL;
    }

    char *buffer = arena_push(arena, bufsize);
    if (buffer == NULL) 
        return NULL;

    if (fgets(buffer, bufsize, file) == NULL) {
        if (ferror(file)) {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        return NULL;
    }

    size_t size = strlen(buffer);
    if (size > 0 && buffer[size - 1] == '\n')
        size--;

    String *input = string_create(arena, buffer, size);
    return input;
}

String *read_input() {
    print_line_prefix();
    return read_line(stdin, MAX_INPUT_SIZE);
}
