#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <stdbool.h>
#include "../arena/arena.h"

typedef struct Vector {
    void *data;
    size_t size;
    size_t capacity;
} Vector;

Vector *vector_create(Arena *arena);
bool vector_push(Arena *arena, Vector *vector, void *value);

#endif
