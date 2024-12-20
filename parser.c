#include <string.h>
#include "lexer.h"
#include "parser.h"

#define MAX_HEADER_LEVEL 6

void parse_children_nodes(LinkList *parent_list, enum ASTNodeType parent_type);

void text_append(TextNode *text, const char *src)
{
    if(text->count > 0 && text->items[text->count - 1] == '\0') {
        text->count--;
    }

    for(size_t i = 0; i < strlen(src); i++) {
        da_append(text, src[i]);
    }

    da_append(text, '\0');
}

void add_child_node(LinkList *children, enum ASTNodeType type, void *data)
{
    ASTNode *child = malloc(sizeof(ASTNode));
    child->type = type;
    child->data = data;

    list_insert_item_at_end(children, child);
}

void add_text_child(LinkList *list, const char *text)
{
    if(list->count > 0) {
        ASTNode *node = (ASTNode*)list->tail->data;

        if(node->type == AST_TEXT_NODE) {
            TextNode *text_node = (TextNode*)node->data;
            text_append(text_node, text);
            return;
        }
    }

    TextNode *text_node = malloc(sizeof(TextNode));
    memset(text_node, 0, sizeof(TextNode));

    text_append(text_node, text);
    add_child_node(list, AST_TEXT_NODE, text_node);
}

void parse_header(LinkList *parent_list)
{
    if(!lexer_is_prev_tkn(TKN_NEWLINE) && !lexer_is_first_tkn()) {
        add_text_child(parent_list, "#");
        return;
    }

    int level = 1;
    while(lexer_is_n_tkn(TKN_HASH, level - 1)) level++;

    if(level > MAX_HEADER_LEVEL || !lexer_is_n_tkn(TKN_SPACE, level - 1)) {
        char hashes[level + 1];
        for(size_t i = 0; i < level; i++) {
            hashes[i] = '#';
        }
        hashes[level] = '\0';

        add_text_child(parent_list, hashes);
        lexer_adv_tkn_cursor(level - 1);
        return;
    }

    lexer_adv_tkn_cursor(level);

    HeaderNode *header = malloc(sizeof(HeaderNode));
    header->level = level;
    header->children = list_create();
    parse_children_nodes(header->children, AST_HEADER_NODE);

    // rewind the new line token
    lexer_rewind_tkn_cursor(1);

    add_child_node(parent_list, AST_HEADER_NODE, header);
}

// this function will help us to know when to stop adding nodes to the children
// list of any node that can contain children
bool should_parse_stop(enum ASTNodeType type, Token *cur_tkn)
{
    if(cur_tkn->type == TKN_EOF) {
        return true;
    }

    switch(type) {
        case AST_HEADER_NODE: {
            return cur_tkn->type == TKN_NEWLINE;
        }
        case AST_ITALIC_NODE: {
            enum TokenType t = cur_tkn->type;
            return t == TKN_NEWLINE || t == TKN_UNDERSCORE || t == TKN_ATERISK;
        }
        case AST_BOLD_NODE: {
            enum TokenType t = cur_tkn->type;
            return t == TKN_NEWLINE;
        }
        default: return false;
    }
}

// parent_list refers to the list where all the parsed nodes will be stored
// parent_type is used to know, when we should stop adding nodes to the list
void parse_children_nodes(LinkList *parent_list, enum ASTNodeType parent_type)
{
    Token *token;
    while((token = lexer_next_tkn()) != NULL && !should_parse_stop(parent_type, token)) {
        switch(token->type) {
            case TKN_TEXT:
                add_text_child(parent_list, token->lexeme);
                break;
            case TKN_NEWLINE:
                add_child_node(parent_list, AST_NEWLINE_NODE, NULL);
                break;
            case TKN_HASH: parse_header(parent_list); break;
            case TKN_SPACE: add_text_child(parent_list, " "); break;
            case TKN_ATERISK:
            case TKN_UNDERSCORE: {
                if(!lexer_is_n_tkn(TKN_ATERISK, 0) && !lexer_is_n_tkn(TKN_UNDERSCORE, 0)) {
                    // ITALIC

                    ItalicNode *italic = malloc(sizeof(ItalicNode));
                    italic->children = list_create();
                    parse_children_nodes(italic->children, AST_ITALIC_NODE);
                    add_child_node(parent_list, AST_ITALIC_NODE, italic);
                    break;
                }

                lexer_adv_tkn_cursor(1);

                if(parent_type == AST_BOLD_NODE) {
                    return;
                }

                // BOLD
                BoldNode *bold = malloc(sizeof(BoldNode));
                bold->children = list_create();
                parse_children_nodes(bold->children, AST_BOLD_NODE);
                add_child_node(parent_list, AST_BOLD_NODE, bold);
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

    parse_children_nodes(body_node->children, AST_BODY_NODE);

    return ast_node;
}
