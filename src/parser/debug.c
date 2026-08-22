#include "debug.h"
#include "../utils/str/str.h"
#include <stdio.h>

static const char *const SyntaxTreeNodeTypeNames[] = {
    [TEXT]      = "TEXT",
    [CMD_SUBST] = "CMD_SUBST",
    [VAR]       = "VAR",
    [ARG]       = "ARG",
    [CMD]       = "CMD",
    [SUBSHELL]  = "SUBSHELL",
    [SUBGROUP]  = "SUBGROUP",
    [PIPELINE]  = "PIPELINE",
    [AND_OR]    = "AND_OR",
    [CMD_LIST]  = "CMD_LIST",
};

void syntax_tree_print(SyntaxTreeNode *root, size_t depth) {
    if (depth > 0)
        printf("%*s", 4 * (int)depth, "");

    if (root == NULL) {
        printf("NULL\n");
        return;
    }

    SyntaxTreeNodeType type = root->type;
    if (type < 0 || type >= sizeof(SyntaxTreeNodeTypeNames) / sizeof(SyntaxTreeNodeTypeNames[0])) {
        printf("UNKNOWN(%d)", type);
        return;
    }
    printf("%s", SyntaxTreeNodeTypeNames[type]);

    if (root->type == TEXT || root->type == VAR) {
        printf(" ---> ");
        string_printf(root->data.text);
    }
    printf("\n");

    if (root->type != TEXT && root->type != VAR) {
        size_t n = root->data.children->size;
        SyntaxTreeNode **children = root->data.children->data;
        for (size_t i = 0; i < n; i++)
            syntax_tree_print(children[i], depth + 1);
    }
}
