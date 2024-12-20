#include <string.h>
#include "lexer.h"
#include "parser.h"

#define MAX_HEADER_LEVEL 6

void parse_children_nodes(LinkList *parent_list, bool (*predicate)(Token *cur_tkn));

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

bool predicate_header(Token *cur_tkn)
{
    return cur_tkn->type != TKN_NEWLINE;
}

bool predicate_italic(Token *cur_tkn)
{
    enum TokenType t = cur_tkn->type;
    return t != TKN_NEWLINE && t != TKN_ATERISK && t != TKN_UNDERSCORE;
}

bool predicate_bold(Token *cur_tkn)
{
    enum TokenType t = cur_tkn->type;
    bool stop = t == TKN_NEWLINE
        || (t == TKN_ATERISK  && lexer_is_n_tkn(TKN_ATERISK, 0))
        || (t == TKN_UNDERSCORE && lexer_is_n_tkn(TKN_UNDERSCORE, 0));

    if(stop && t != TKN_NEWLINE) {
        // skip the next aterisk or underscore
        lexer_adv_tkn_cursor(1);
    }

    return !stop;
}

bool predicate_body(Token *cur_tkn)
{
    return true;
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
    parse_children_nodes(header->children, &predicate_header);

    // rewind the new line token
    lexer_rewind_tkn_cursor(1);

    add_child_node(parent_list, AST_HEADER_NODE, header);
}

void parse_italic(LinkList *parent_list, enum TokenType cur_tkn_type)
{
    bool closing_found = false;
    int count = 0;
    while(!lexer_is_n_tkn(TKN_NEWLINE, count) && !lexer_is_n_tkn(TKN_EOF, count)) {
        if(lexer_is_n_tkn(cur_tkn_type, count)) {
            closing_found = true;
            break;
        }

        count++;
    }

    if(!closing_found) {
        if(cur_tkn_type == TKN_ATERISK) {
            add_text_child(parent_list, "*");
        } else {
            add_text_child(parent_list, "_");
        }
        return;
    }

    ItalicNode *italic = malloc(sizeof(ItalicNode));
    italic->children = list_create();
    parse_children_nodes(italic->children, &predicate_italic);
    add_child_node(parent_list, AST_ITALIC_NODE, italic);
}

void parse_bold(LinkList *parent_list, enum TokenType cur_tkn_type)
{
    bool closing_found = false;
    int count = 0;
    while(!lexer_is_n_tkn(TKN_NEWLINE, count) && !lexer_is_n_tkn(TKN_EOF, count)) {
        if(lexer_is_n_tkn(cur_tkn_type, count) && lexer_is_n_tkn(cur_tkn_type, count + 1)) {
            closing_found = true;
            break;
        }

        count++;
    }

    if(count == 0 || !closing_found) {
        if(cur_tkn_type == TKN_ATERISK) {
            add_text_child(parent_list, "**");
        } else {
            add_text_child(parent_list, "__");
        }
        return;
    }

    BoldNode *bold = malloc(sizeof(BoldNode));
    bold->children = list_create();
    parse_children_nodes(bold->children, &predicate_bold);
    add_child_node(parent_list, AST_BOLD_NODE, bold);
}

void parse_children_nodes(LinkList *parent_list, bool (*predicate)(Token *cur_tkn))
{
    Token *token;
    while((token = lexer_next_tkn()) != NULL && token->type != TKN_EOF && predicate(token)) {
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
                if(lexer_is_n_tkn(token->type, 0)) {
                    // consume the second aterisk or underscore
                    lexer_adv_tkn_cursor(1);
                    parse_bold(parent_list, token->type);
                } else {
                    parse_italic(parent_list, token->type);
                }
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

    parse_children_nodes(body_node->children, &predicate_body);

    return ast_node;
}
