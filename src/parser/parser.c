#include "parser.h"
#include "../context.h"
#include <ctype.h>

#define IS_AND(ptr) ((ptr)[0] == '&' && (ptr)[1] == '&')
#define IS_OR(ptr) ((ptr)[0] == '|' && (ptr)[1] == '|')
#define IS_SEMICOLON(ptr) ((ptr)[0] == ';')
#define IS_AMPERSAND(ptr) ((ptr)[0] == '&' && (ptr)[1] != '&')
#define IS_PIPE(ptr) ((ptr)[0] == '|' && (ptr)[1] != '|')
#define IS_SINGLE_QUOTION(ptr) ((ptr)[0] == '\'')
#define IS_DOUBLE_QUOTION(ptr) ((ptr)[0] == '"')
#define IS_COMMAND_SUBSTITUTION(ptr) ((ptr)[0] == '$' && (ptr)[1] == '(')
#define IS_VAR(ptr) ((ptr)[0] == '$' && (ptr)[1] != '(')
#define IS_VAR_SPECIAL_CHAR(ptr) (isdigit((ptr)[0]) || (ptr)[0] == '?' || (ptr)[0] == '$' || (ptr)[0] == '@' || (ptr)[0] == '!' || (ptr)[0] == '#' || (ptr)[0] == '*')
#define IS_VAR_CHAR(ptr) (isalpha((ptr)[0]) || isdigit((ptr)[0]) || (ptr)[0] == '_')
#define IS_SPECIAL_CHAR(ptr) ((ptr)[0] == '(' || (ptr)[0] == '{' || (ptr)[0] == ')' || (ptr)[0] == '}' || (ptr)[0] == ';' || (ptr)[0] == '&' || (ptr)[0] == '|')
#define IS_ARG_SEPARATOR(ptr) (isspace((ptr)[0]) || IS_SPECIAL_CHAR(ptr))

#define VECTOR_CREATE(v) \
    Vector *v = vector_create(arena); \
    if ((v) == NULL) { \
        parse_error = OUT_OF_MEMORY; \
        return NULL; \
    } \

#define PUSH_CHILD(expr) \
    do { \
        SyntaxTreeNode *_node = expr; \
        if (_node == NULL) \
            return NULL; \
        if (!vector_push(arena, children, (_node))) { \
            parse_error = OUT_OF_MEMORY; \
            return NULL; \
        } \
    } while(0)

#define RETURN_NODE(expr) \
    do { \
        SyntaxTreeNode *_node = (expr); \
        if (_node == NULL) { \
            parse_error = OUT_OF_MEMORY; \
            return NULL; \
        } \
        return _node; \
    } while(0)

#define SKIP_SPACES() \
    do { \
        while (ind < line->size && isspace(line->data[ind])) \
            ind++; \
    } while(0)

static String *line;
static size_t ind;
static ParseError parse_error;

static const char *const ParseErrorMessages[] = {
    [NONE]                      = "No parsing error",
    [BUG]                       = "Bug in parser",
    [OUT_OF_MEMORY]             = "Out of memory",
    [UNTERMINATED_SINGLE_QUOTE] = "Unterminated single quote",
    [UNTERMINATED_DOUBLE_QUOTE] = "Unterminated double quote",
    [UNMATCHED_PARENTHESIS]     = "Unmatched parenthesis",
    [UNMATCHED_CURLY_BRACE]     = "Unmatched curly brace",
};

const char *parse_error_message() {
    if (parse_error < 0 || (size_t)parse_error >= sizeof(ParseErrorMessages) / sizeof(ParseErrorMessages[0]))
        return "Unknown error";
    return ParseErrorMessages[parse_error];
}

SyntaxTreeNode *node_from_interval(SyntaxTreeNodeType type, size_t l, size_t r) {
    if (l > r || r >= line->size) {
        parse_error = BUG;
        return NULL;
    }

    String *text = string_from_range(arena, line, l, r);
    if (text == NULL) {
        parse_error = OUT_OF_MEMORY; // TODO: maybe other error too
        return NULL;
    }
    SyntaxTreeNode *node = syntax_tree_leaf_create(arena, type, text);
    if (node == NULL) {
        parse_error = OUT_OF_MEMORY; // TODO: may be other error too
        return NULL;
    }
    return node;
}

SyntaxTreeNode *parse_single_quotion() {
    ind++; // parse starting ' char
    size_t l = ind;
    while (ind < line->size && !IS_SINGLE_QUOTION(line->data + ind))
        ind++;
    size_t r = ind - 1;
    if (ind >= line->size) {
        parse_error = UNTERMINATED_SINGLE_QUOTE;
        return NULL;
    }
    ind++; // parse terminating ' char

    return node_from_interval(TEXT, l, r);
}

SyntaxTreeNode *parse_double_quotion() { // TODO: needs double quotion rules
    ind++; // parse starting " char
    size_t l = ind;
    while (ind < line->size && !IS_DOUBLE_QUOTION(line->data + ind))
        ind++;
    size_t r = ind - 1;
    if (ind >= line->size) {
        parse_error = UNTERMINATED_DOUBLE_QUOTE;
        return NULL;
    }
    ind++; // parse terminating " char

    return node_from_interval(TEXT, l, r);
}

SyntaxTreeNode *parse_cmd_list();

SyntaxTreeNode *parse_cmd_subst() {
    ind += 2; // parse $ and starting ( char
    SyntaxTreeNode *node = parse_cmd_list();
    if (node == NULL)
        return NULL;
    if (line->data[ind] != ')') {
        parse_error = UNMATCHED_PARENTHESIS;
        return NULL;
    }
    ind++; // parse terminating ) char

    VECTOR_CREATE(children);
    PUSH_CHILD(node);
    RETURN_NODE(syntax_tree_node_create(arena, CMD_SUBST, children));
}

SyntaxTreeNode *parse_var() {
    ind++; // parse $
    if (IS_VAR_SPECIAL_CHAR(line->data + ind)) { // special var
        ind++; // parse special char
        return node_from_interval(VAR, ind - 1, ind - 1);
    }
    if (line->data[ind] == '{') { // curly-braced var
        ind++; // parse opening curly brace {
        size_t l = ind;
        while (ind < line->size && line->data[ind] != '}') ind++;
        size_t r = ind - 1;
        if (ind >= line->size) {
            parse_error = UNMATCHED_CURLY_BRACE;
            return NULL;
        }
        ind++; // parse closing curly brace }
        return node_from_interval(VAR, l, r);

    }
    if (IS_VAR_CHAR(line->data + ind)) { // standard var
        size_t l = ind;
        while (ind < line->size && IS_VAR_CHAR(line->data + ind)) ind++;
        size_t r = ind - 1;
        return node_from_interval(VAR, l, r);
    }
    return node_from_interval(VAR, ind - 1, ind - 1); // single $, treat as standard text
}

SyntaxTreeNode *parse_arg() {
    if (ind >= line->size) {
        parse_error = BUG;
        return NULL;
    }

    size_t raw_text_start = ind;
    VECTOR_CREATE(children);
    while (ind < line->size && !IS_ARG_SEPARATOR(line->data + ind)) {
        SyntaxTreeNode *node = NULL;
        bool in_raw_text = true;
        size_t raw_text_end = ind;
        if (IS_SINGLE_QUOTION(line->data + ind)) {
            node = parse_single_quotion();
            in_raw_text = false;
        } else if (IS_DOUBLE_QUOTION(line->data + ind)) {
            node = parse_double_quotion();
            in_raw_text = false;
        } else if (IS_COMMAND_SUBSTITUTION(line->data + ind)) {
            node = parse_cmd_subst();
            in_raw_text = false;
        } else if (IS_VAR(line->data + ind)) {
            node = parse_var();
            in_raw_text = false;
        }

        if (!in_raw_text) {
            if (raw_text_start < raw_text_end)
                PUSH_CHILD(node_from_interval(TEXT, raw_text_start, raw_text_end - 1));
            PUSH_CHILD(node);
            raw_text_start = ind + 1;
        } else
            ind++;
    }

    if (raw_text_start < ind)
        PUSH_CHILD(node_from_interval(TEXT, raw_text_start, ind - 1));

    RETURN_NODE(syntax_tree_node_create(arena, ARG, children));
}

SyntaxTreeNode *parse_cmd() {
    if (ind >= line->size) {
        parse_error = BUG;
        return NULL;
    }

    VECTOR_CREATE(children);
    do {
        SKIP_SPACES();
        PUSH_CHILD(parse_arg());
        SKIP_SPACES();
    } while (ind < line->size && !IS_SPECIAL_CHAR(line->data + ind));

    RETURN_NODE(syntax_tree_node_create(arena, CMD, children));
}

SyntaxTreeNode *parse_subgroup() {
    ind++; // parse opening {
    VECTOR_CREATE(children);
    SyntaxTreeNode *node = parse_cmd_list();
    PUSH_CHILD(node);
    if (line->data[ind] != '}') {
        parse_error = UNMATCHED_CURLY_BRACE;
        return NULL;
    }
    ind++; // parse closing }
    RETURN_NODE(syntax_tree_node_create(arena, SUBGROUP, children));
}

SyntaxTreeNode *parse_subshell() {
    ind++; // parse opening (
    VECTOR_CREATE(children);
    SyntaxTreeNode *node = parse_cmd_list();
    PUSH_CHILD(node);
    if (line->data[ind] != ')') {
        parse_error = UNMATCHED_PARENTHESIS;
        return NULL;
    }
    ind++; // parse closing (
    RETURN_NODE(syntax_tree_node_create(arena, SUBSHELL, children));
}

SyntaxTreeNode *parse_pipe() {
    if (ind >= line->size) {
        parse_error = BUG;
        return NULL;
    }

    VECTOR_CREATE(children);
    do {
        SyntaxTreeNode *node = NULL;
        if (line->data[ind] == '(')
            node = parse_subshell();
        else if (line->data[ind] == '{')
            node = parse_subgroup();
        else
            node = parse_cmd();
        PUSH_CHILD(node);
        SKIP_SPACES();
    } while (ind < line->size && IS_PIPE(line->data + ind) && (ind++));

    if (children->size == 1)
        return ((SyntaxTreeNode**)children->data)[0];

    RETURN_NODE(syntax_tree_node_create(arena, PIPELINE, children));
}

SyntaxTreeNode *parse_and_or() {
    if (ind >= line->size) {
       parse_error = BUG;
       return NULL;
    }

    VECTOR_CREATE(children);
    do {
        SyntaxTreeNode *node = parse_pipe();
        if (node == NULL)
            return NULL;
        SKIP_SPACES();
        SKIP_SPACES();
        node->separator = (ind < line->size && IS_OR(line->data + ind) ? OR : AND);
        PUSH_CHILD(node);
    } while (ind < line->size && (IS_OR(line->data + ind) || IS_AND(line->data + ind)) && (ind += 2));

    if (children->size == 1)
        return ((SyntaxTreeNode**)children->data)[0];

    RETURN_NODE(syntax_tree_node_create(arena, AND_OR, children));
}

SyntaxTreeNode *parse_cmd_list() {
    if (ind >= line->size) {
        parse_error = BUG;
        return NULL;
    }

    VECTOR_CREATE(children);
    do {
        SyntaxTreeNode *node = parse_and_or();
        if (node == NULL)
            return NULL;
        SKIP_SPACES();
        node->separator = (ind < line->size && IS_AMPERSAND(line->data + ind) ? BACKGROUND : WAIT);
        PUSH_CHILD(node);
    } while (ind < line->size && (IS_SEMICOLON(line->data + ind) || IS_AMPERSAND(line->data + ind)) && (ind++));

    if (children->size == 1)
        return ((SyntaxTreeNode**)children->data)[0];

    RETURN_NODE(syntax_tree_node_create(arena, CMD_LIST, children));
}

SyntaxTreeNode *parse(String *_line) {
    line = _line;
    ind = 0;
    parse_error = NONE;
    return parse_cmd_list();
}

