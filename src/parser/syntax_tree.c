#include "syntax_tree.h"

SyntaxTreeNode *syntax_tree_node_create(Arena *arena, SyntaxTreeNodeType type, Vector *children) {
    SyntaxTreeNode *node = arena_push(arena, sizeof(SyntaxTreeNode));
    if (node == NULL)
        return NULL;
    node->type = type;
    node->separator = NO_SEPARATOR;
    node->data.children = children;
    return node;
}

SyntaxTreeNode *syntax_tree_leaf_create(Arena *arena, SyntaxTreeNodeType type, String *text) {
    SyntaxTreeNode *node = arena_push(arena, sizeof(SyntaxTreeNode));
    if (node == NULL)
        return NULL;
    node->type = type;
    node->separator = NO_SEPARATOR;
    node->data.text = text;
    return node;
}
