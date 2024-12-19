#ifndef PARSER_H_
#define PARSER_H_

#include "linklist.h"

typedef struct AST_NODE AST_NODE;

typedef struct BodyNode {
    LinkList *children;
} BodyNode;

typedef struct TextNode {
    char *text;
} TextNode;

enum ASTNodeType {
    AST_TEXT_NODE,
    AST_BODY_NODE,
    AST_NEWLINE_NODE,
};

typedef struct ASTNode {
    enum ASTNodeType type;
    void *data;
} ASTNode;

ASTNode *parse_file(const char *file_path);

#endif
