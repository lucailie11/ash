#ifndef ARENA_BLOCK_H
#define ARENA_BLOCK_H

#include <stdlib.h>

typedef struct ArenaBlock {
    struct ArenaBlock *next;
    unsigned char *data;
    size_t pos;
    size_t capacity;
} ArenaBlock;

ArenaBlock *arena_block_create(size_t capacity);
void arena_block_destroy(ArenaBlock *arena_block);
void *arena_block_push(ArenaBlock *arena_block, size_t size);
void arena_block_reset(ArenaBlock *arena_block);

#endif
