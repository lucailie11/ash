#include "arena_block.h"
#include <errno.h>
#include <stdint.h>

#define ALIGN_POW2(n, p) (((size_t)(n) + ((size_t)(p) - 1)) & (~((size_t)(p) - 1)))
#define ARENA_ALIGN (sizeof(void*))

ArenaBlock *arena_block_create(size_t capacity) {
    ArenaBlock *arena_block = malloc(sizeof(ArenaBlock));
    if (arena_block == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    arena_block->data = (unsigned char*)malloc(capacity);
    if (arena_block->data == NULL) {
        free(arena_block);
        errno = ENOMEM;
        return NULL;
    }

    arena_block->pos = 0;
    arena_block->capacity = capacity;
    arena_block->next = NULL;

    return arena_block;
}

void arena_block_destroy(ArenaBlock *arena_block) {
    free(arena_block->data);
    free(arena_block);
}

void *arena_block_push(ArenaBlock *arena_block, size_t size) {
    if (size > SIZE_MAX - (ARENA_ALIGN - 1) - arena_block->pos) {
        errno = EOVERFLOW;
        return NULL;
    }

    size_t pos_aligned = ALIGN_POW2(arena_block->pos, ARENA_ALIGN);
    size_t new_pos = pos_aligned + size;
    if (new_pos > arena_block->capacity)
        return NULL;

    void *ptr = arena_block->data + pos_aligned;
    arena_block->pos = new_pos;

    return ptr;
}

void arena_block_reset(ArenaBlock *arena_block) {
    arena_block->pos = 0;
}
