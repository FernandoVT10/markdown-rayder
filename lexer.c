#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "raylib.h"
#include "lexer.h"

Lexer lexer = {0};

char *load_file_contents(const char *path)
{
    struct stat buf_stat;
    if(stat(path, &buf_stat) == -1) {
        TraceLog(LOG_ERROR, "Couldn't open file %s: %s", path, strerror(errno));
        return NULL;
    }

    if((buf_stat.st_mode & S_IFMT) != S_IFREG) {
        TraceLog(LOG_ERROR, "%s is not a valid file path", path);
        return NULL;
    }

    FILE *file = fopen(path, "r");

    if(file == NULL) {
        TraceLog(LOG_ERROR, "Couldn't open file %s: %s\n", path, strerror(errno));
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *text = calloc(sizeof(char), file_size + 1);

    if(text == NULL) {
        TraceLog(LOG_ERROR, "Couldn't allocate memory to contain the file");
        return NULL;
    }

    fread(text, file_size, sizeof(char), file);
    text[file_size] = '\0';

    fclose(file);

    return text;
}

Token *lexer_add_tkn(enum TokenType type, const char *lexeme)
{
    Token *token = malloc(sizeof(Token));
    token->type = type;

    if(lexeme) {
        token->lexeme = strdup(lexeme);
    } else {
        token->lexeme = NULL;
    }

    list_insert_item_at_end(lexer.tokens, token);
    return token;
}

bool lexer_init(const char *file_path)
{
    lexer.buf = load_file_contents(file_path);
    lexer.cursor = 0;
    lexer.tokens = list_create();

    if(!lexer.buf || !lexer.tokens) {
        return false;
    }

    lexer_add_tkn(TKN_EOF, NULL);
    lexer_add_tkn(TKN_NEWLINE, NULL);

    return true;
}

char lexer_get_char(int pos)
{
    if(pos < 0) pos = 0;

    if(pos >= strlen(lexer.buf)) {
        return EOF;
    }

    return lexer.buf[pos];
}

char lexer_get_and_adv()
{
    return lexer_get_char(lexer.cursor++);
}

void lexer_rewind(int n)
{
    lexer.cursor -= n;
    if(lexer.cursor < 0) {
        lexer.cursor = 0;
    }
}

Token *lexer_get_tkn(enum TokenType type)
{
    ListNode *node = lexer.tokens->head;
    while(node != NULL) {
        Token *token = (Token *)node->data;

        if(token->type == type) {
            return token;
        }

        node = node->next;
    }

    TraceLog(LOG_ERROR, "Token '%u' not found", type);

    return NULL;
}

bool is_special_char(char c)
{
    return c == EOF
        || c == '\n';
    // return c == '\n'
    //     || c == '#';
    //     || c == EOF
    //     || c == '*'
    //     || c == '`'
    //     || c == '_'
    //     || c == '[';
}

Token *lexer_next_token()
{
    char c = lexer_get_and_adv();

    if(c == '\n') return lexer_get_tkn(TKN_NEWLINE);
    if(c == EOF) return lexer_get_tkn(TKN_EOF);

    // TEXT
    int start_pos = lexer.cursor - 1;

    c = lexer_get_and_adv();
    while(!is_special_char(c)) {
        c = lexer_get_and_adv();
    }

    // we rewind the special character encountered
    lexer_rewind(1);

    int length = lexer.cursor - start_pos;

    const char *lexeme = TextSubtext(lexer.buf, start_pos, length);
    return lexer_add_tkn(TKN_TEXT, lexeme);
}

void lexer_destroy()
{
    if(lexer.buf != NULL)
        free(lexer.buf);

    // TODO: Free the tokens link list
}
