#ifndef CONTEXT_H
#define CONTEXT_H

#include "utils/arena/arena.h"

extern Arena *arena;

#define ARENA_BLOCK_SIZE (1 << 12)
#define MAX_INPUT_SIZE ARENA_BLOCK_SIZE
#define MAX_CMD_SUBST_INPUT ARENA_BLOCK_SIZE

#endif
