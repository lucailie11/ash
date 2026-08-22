#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include "../utils/str/str.h"
#include "../utils/vector/vector.h"
#include "../utils/arena/arena.h"

typedef enum {
    TEXT,
    CMD_SUBST,
    VAR,
    ARG,
    CMD,
    SUBSHELL,
    SUBGROUP,
    PIPELINE,
    AND_OR,
    CMD_LIST,
} SyntaxTreeNodeType;

typedef enum {
    NO_SEPARATOR,
    AND,
    OR,
    WAIT,
    BACKGROUND,
} SeparatorType;

typedef struct SyntaxTreeNode {
    SyntaxTreeNodeType type;
    SeparatorType separator;
    union {
        String *text;
        Vector *children;
    } data;
} SyntaxTreeNode;

SyntaxTreeNode *syntax_tree_node_create(Arena *arena, SyntaxTreeNodeType type, Vector *children);
SyntaxTreeNode *syntax_tree_leaf_create(Arena *arena, SyntaxTreeNodeType type, String *text);

#endif
