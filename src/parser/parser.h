#ifndef PARSER_H
#define PARSER_H

#include "syntax_tree.h"
#include "../utils/str/str.h"

typedef enum {
    NONE,
    BUG,
    OUT_OF_MEMORY,
    UNTERMINATED_SINGLE_QUOTE,
    UNTERMINATED_DOUBLE_QUOTE,
    UNMATCHED_PARENTHESIS,
    UNMATCHED_CURLY_BRACE,
} ParseError;

SyntaxTreeNode *parse(String *line);
const char *parse_error_message();

#endif
