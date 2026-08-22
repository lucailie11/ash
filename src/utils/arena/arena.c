#include "arena.h"
#include <errno.h>
#include <string.h>

Arena *arena_create(size_t block_capacity) {
    Arena *arena = malloc(sizeof(Arena));
    if (arena == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    arena->first = arena->current = arena_block_create(block_capacity);
    if (arena->first == NULL) {
        free(arena);
        return NULL; // errno already set by arena_block_create
    }
    arena->block_capacity = block_capacity;

    return arena;
}

void arena_destroy(Arena *arena) {
    ArenaBlock *block = arena->first;
    while (block != NULL) {
        ArenaBlock *tmp = block->next;
        arena_block_destroy(block);
        block = tmp;
    }
    free(arena);
}

void *arena_push(Arena *arena, size_t size) {
    if (size > arena->block_capacity) {
        errno = EINVAL;
        return NULL;
    }

    errno = 0;
    void *ptr = arena_block_push(arena->current, size);
    if (ptr != NULL)
        return ptr;
    if (errno == EOVERFLOW)
        return NULL;

    if (arena->current->next == NULL) {
        ArenaBlock *next = arena_block_create(arena->block_capacity);
        if (next == NULL)
            return NULL;

        arena->current->next = next;
        arena->current = next;
    } else
        arena->current = arena->current->next;

    return arena_block_push(arena->current, size);
}

void *arena_push_zero(Arena *arena, size_t size) {
    void *ptr = arena_push(arena, size);
    if (ptr == NULL)
        return NULL;
    memset(ptr, 0, size);
    return ptr;
}

void arena_reset(Arena *arena) {
    ArenaBlock *block = arena->first;
    while (block != NULL) {
        ArenaBlock *tmp = block->next;
        arena_block_reset(block);
        block = tmp;
    }
    arena->current = arena->first;
}
