#include "str.h"
#include <errno.h>
#include <stdio.h>

String *string_create(Arena *arena, char *data, size_t size) {
    String *string = arena_push(arena, sizeof(String));
    if (string == NULL)
        return NULL;
    string->data = data;
    string->size = size;
    return string;
}

String *string_from_empty(Arena *arena) {
    String *string = arena_push(arena, sizeof(String));
    if (string == NULL)
        return NULL;
    string->data = arena_push(arena, sizeof(char));
    if (string->data == NULL)
        return NULL;
    string->size = 0;
    return string;
}

String *string_from_cstr(Arena *arena, char *data) {
    return string_create(arena, data, strlen(data));
}

String *string_from_range(Arena *arena, String *string, size_t l, size_t r) {
    if (l > r || r >= string->size) {
        errno = EINVAL;
        return NULL;
    }
    return string_create(arena, string->data + l, r - l + 1);
}

String *string_concat(Arena *arena, String *s, String *t) {
    char *data = arena_push(arena, (s->size + t->size) * sizeof(char));
    if (data == NULL)
        return NULL;
    memcpy(data, s->data, s->size * sizeof(char));
    memcpy(data + s->size * sizeof(char), t->data, t->size * sizeof(char));
    return string_create(arena, data, s->size + t->size);
}

String *string_from_int(Arena *arena, int x) {
    size_t n = (x < 0 ? 1 : 0);
    int y = (x > 0 ? x : -x);
    do {
        n++;
        y /= 10;
    } while (y > 0);

    char *data = arena_push(arena, n * sizeof(char));
    if (data == NULL)
        return NULL;
    size_t size = n;

    y = x;
    do {
        data[--n] = '0' + (y % 10);
        y /= 10;
    } while (y > 0);

    if (x < 0)
        data[0] = '-';

    return string_create(arena, data, size);
}

char *string_to_c_str(Arena *arena, String *string) {
    char *c_str = arena_push(arena, (string->size + 1) * sizeof(char));
    if (c_str == NULL)
        return NULL;
    memcpy(c_str, string->data, string->size * sizeof(char));
    c_str[string->size] = '\0';
    return c_str;
}

void string_printf(String *string) {
    if (fwrite(string->data, 1, string->size, stdout) != string->size) {
        perror("ash: fwrite");
        exit(EXIT_FAILURE);
    }
}

