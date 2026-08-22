#include "vector.h"
#include <string.h>

#define DEFAULT_VECTOR_CAPACITY 2

Vector *vector_create(Arena *arena) {
    Vector *vector = (Vector*)arena_push(arena, sizeof(Vector));
    if (vector == NULL)
        return NULL;

    vector->data = arena_push(arena, DEFAULT_VECTOR_CAPACITY * sizeof(void*));
    if (vector->data == NULL)
        return NULL;
    vector->size = 0;
    vector->capacity = DEFAULT_VECTOR_CAPACITY;

    return vector;
}

bool vector_push(Arena *arena, Vector *vector, void *value) {
    if (vector->size >= vector->capacity) {
        size_t new_capacity = vector->capacity * 2;
        void **new_data = (void**)arena_push(arena, new_capacity * sizeof(void*));
        if (new_data == NULL)
            return false;
        memcpy(new_data, vector->data, vector->size * sizeof(void*));
        vector->data = new_data;
        vector->capacity = new_capacity;
    }
    *((char**)vector->data + (vector->size++)) = (char*)value;
    return true;
}
