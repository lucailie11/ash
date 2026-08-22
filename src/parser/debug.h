#ifndef DEBUG_H
#define DEBUG_H

#include "syntax_tree.h"
#include <stddef.h>

void syntax_tree_print(SyntaxTreeNode *root, size_t depth);

#endif
