#include <string.h>
#include "lexer.h"
#include "parser.h"

void add_child_node(LinkList *children, enum ASTNodeType type, void *data)
{
    ASTNode *child = malloc(sizeof(ASTNode));
    child->type = type;
    child->data = data;

    list_insert_item_at_end(children, child);
}

ASTNode *parse_file(const char *file_path)
{
    if(!lexer_init(file_path)) {
        return NULL;
    }

    ASTNode *ast_node = malloc(sizeof(ASTNode));
    ast_node->type = AST_BODY_NODE;

    BodyNode *body_node = malloc(sizeof(BodyNode));
    body_node->children = list_create();
    ast_node->data = body_node;

    Token *token;

    while((token = lexer_next_token()) != NULL && token->type != TKN_EOF) {
        switch(token->type) {
            case TKN_TEXT: {
                TextNode *text_node = malloc(sizeof(TextNode));
                text_node->text = strdup(token->lexeme);
                add_child_node(body_node->children, AST_TEXT_NODE, text_node);
            } break;
            case TKN_NEWLINE:
                add_child_node(body_node->children, AST_NEWLINE_NODE, NULL);
                break;
            case TKN_EOF:
                UNREACHABLE("END_OF_FILE reached");
                break;
        }
    }

    return ast_node;
}
