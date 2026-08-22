#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>
#include "arena_block.h"

typedef struct Arena {
    ArenaBlock *first;
    ArenaBlock *current;
    size_t block_capacity;
} Arena;

Arena *arena_create(size_t block_capacity);
void arena_destroy(Arena *arena);
void *arena_push(Arena *arena, size_t size);
void *arena_push_zero(Arena *arena, size_t size);
void arena_reset(Arena *arena);

#endif
