#ifndef PARSER_H_
#define PARSER_H_

#include "linklist.h"

typedef struct AST_NODE AST_NODE;

typedef struct BodyNode {
    LinkList *children;
} BodyNode;

typedef struct TextNode {
    char *items;
    size_t count;
    size_t capacity;
} TextNode;

typedef struct HeaderNode {
  LinkList *children;
  int level;
} HeaderNode;

typedef struct ItalicNode {
  LinkList *children;
} ItalicNode;

typedef struct BoldNode {
  LinkList *children;
} BoldNode;

enum ASTNodeType {
    AST_TEXT_NODE,
    AST_BODY_NODE,
    AST_NEWLINE_NODE,
    AST_HEADER_NODE,
    AST_ITALIC_NODE,
    AST_BOLD_NODE,
};

typedef struct ASTNode {
    enum ASTNodeType type;
    void *data;
} ASTNode;

ASTNode *parse_file(const char *file_path);

#endif
