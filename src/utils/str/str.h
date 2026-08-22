#ifndef STR_H
#define STR_H

#include "../arena/arena.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct String {
    char *data;
    size_t size;
} String;

#define STR_EQ(a, b) (strcmp((a), (b)) == 0)

String *string_create(Arena *arena, char *data, size_t size);
String *string_from_empty(Arena *arena);
String *string_from_cstr(Arena *arena, char *data);
String *string_from_range(Arena *arena, String *string, size_t l, size_t r);
String *string_concat(Arena *arena, String *s, String *t);
String *string_from_int(Arena *arena, int x);
char *string_to_c_str(Arena *arena, String *string);
void string_printf(String *string);

#endif
