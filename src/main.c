#include "context.h"
#include "input/input.h"
#include "parser/parser.h"
#include "execute/execute.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

Arena *arena;

int main() {
    while (true) {
        arena = arena_create(ARENA_BLOCK_SIZE);
        if (arena == NULL) {
            fprintf(stderr, "ash: out of memory\n");
            continue;
        }

        String *line = read_input();
        if (line == NULL) {
            arena_destroy(arena);
            break;
        }

        SyntaxTreeNode *tree = parse(line);
        if (tree != NULL)
            exec_tree(tree);
        else
            fprintf(stderr, "Parsing error: %s\n", parse_error_message());

        arena_destroy(arena);
    }

    return EXIT_SUCCESS;
}
