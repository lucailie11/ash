#ifndef EXPAND_H
#define EXPAND_H

#include "../parser/syntax_tree.h"
#include "../utils/vector/vector.h"
#include "../utils/arena/arena.h"

String *expand_fragment(SyntaxTreeNode *node);
String *expand_arg(Arena *arena, SyntaxTreeNode *node);
char **argv_build(Arena *arena, Vector *args);

#endif
