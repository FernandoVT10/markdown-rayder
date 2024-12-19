#include <string.h>
#include "lexer.h"
#include "parser.h"

#define MAX_HEADER_LEVEL 6

void add_child_node(LinkList *children, enum ASTNodeType type, void *data)
{
    ASTNode *child = malloc(sizeof(ASTNode));
    child->type = type;
    child->data = data;

    list_insert_item_at_end(children, child);
}

void parse_children_nodes(LinkList *parent_list, enum TokenType stop_type)
{
    Token *token;

    while((token = lexer_next_tkn()) != NULL && token->type != TKN_EOF && token->type != stop_type) {
        switch(token->type) {
            case TKN_TEXT: {
                TextNode *text = malloc(sizeof(TextNode));
                text->contents = strdup(token->lexeme);
                add_child_node(parent_list, AST_TEXT_NODE, text);
            } break;
            case TKN_NEWLINE:
                add_child_node(parent_list, AST_NEWLINE_NODE, NULL);
                break;
            case TKN_HASH: {
                int level = 1;
                while(lexer_is_n_tkn(TKN_HASH, level - 1)) level++;

                if(level > MAX_HEADER_LEVEL || !lexer_is_n_tkn(TKN_SPACE, level - 1)) {
                    // if(parent_list->tail) {
                    //     ASTNode *node = (ASTNode*)parent_list->tail->data;
                    //
                    //     if(node->type == AST_TEXT_NODE) {
                    //         TextNode *text = (TextNode*)node->data;
                    //         size_t size = strlen(text->contents) + strlen() + 1;
                    //         text->contents = realloc(text->contents, sizeof(char), size);
                    //         break;
                    //     }
                    // }

                    TextNode *text = malloc(sizeof(TextNode));
                    text->contents = malloc(level + 1);

                    for(size_t i = 0; i < level; i++) {
                        text->contents[i] = '#';
                    }
                    text->contents[level] = '\0';

                    lexer_adv_tkn_cursor(level - 1);
                    add_child_node(parent_list, AST_TEXT_NODE, text);
                    break;
                }

                lexer_adv_tkn_cursor(level);

                HeaderNode *header = malloc(sizeof(HeaderNode));
                header->level = level;
                header->children = list_create();
                parse_children_nodes(header->children, TKN_NEWLINE);
                add_child_node(parent_list, AST_HEADER_NODE, header);
            } break;
            case TKN_SPACE: {
            } break;
            case TKN_EOF:
                UNREACHABLE("END_OF_FILE reached");
                break;
        }
    }
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

    parse_children_nodes(body_node->children, TKN_EOF);

    return ast_node;
}
