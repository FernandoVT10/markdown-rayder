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
    Token token = {0};
    token.type = type;

    if(lexeme) {
        token.lexeme = strdup(lexeme);
    } else {
        token.lexeme = NULL;
    }

    da_append(&lexer.tokens, token);
    return &lexer.tokens.items[lexer.tokens.count - 1];
}

bool lexer_init(const char *file_path)
{
    lexer.buf = load_file_contents(file_path);
    lexer.cursor = 0;

    if(!lexer.buf) {
        return false;
    }

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

bool is_special_char(char c)
{
    return c == EOF
        || c == '\n'
        || c == '#'
        || c == ' ';
    // return c == '\n'
    //     || c == '#';
    //     || c == EOF
    //     || c == '*'
    //     || c == '`'
    //     || c == '_'
    //     || c == '[';
}

void process_next_tkn()
{
    char c = lexer_get_and_adv();

    switch(c) {
        case EOF: lexer_add_tkn(TKN_EOF, NULL); return;
        case '\n': lexer_add_tkn(TKN_NEWLINE, NULL); return;
        case '#': lexer_add_tkn(TKN_HASH, NULL); return;
        case ' ': lexer_add_tkn(TKN_SPACE, NULL); return;
    }

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
    lexer_add_tkn(TKN_TEXT, lexeme);
}

Token *lexer_get_tkn(int pos)
{
    if(pos < 0) pos = 0;

    if(pos >= lexer.tokens.count) {
        process_next_tkn();
        return &lexer.tokens.items[lexer.tokens.count - 1];
    }

    return &lexer.tokens.items[pos];
}

Token *lexer_next_tkn()
{
    return lexer_get_tkn(lexer.tkn_cursor++);
}

bool lexer_is_n_tkn(enum TokenType type, int n)
{
    Token *token = lexer_get_tkn(lexer.tkn_cursor + n);
    return token->type == type;
}

void lexer_adv_tkn_cursor(int n)
{
    lexer.tkn_cursor += n;
}

void lexer_destroy()
{
    if(lexer.buf != NULL)
        free(lexer.buf);

    // TODO: Free the tokens link list
}
