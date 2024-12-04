#include "lexer.h"

Lexer lexer = {0};

char *load_file_contents(const char *path)
{
    FILE *file = fopen(path, "r");

    if(file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *text = calloc(sizeof(char), file_size + 1);
    fread(text, file_size, sizeof(char), file);
    text[file_size] = '\0';

    fclose(file);

    return text;
}

bool lexer_init(const char *file_path)
{
    lexer.buf = load_file_contents(file_path);
    lexer.cursor = 0;

    if(!lexer.buf) {
        TraceLog(LOG_ERROR, "Couldn't open file %s: %s\n", file_path, strerror(errno));
        return false;
    }

    return true;
}

char lexer_get_char(int pos)
{
    if(pos >= strlen(lexer.buf)) {
        return '\0';
    }

    return lexer.buf[pos];
}

char lexer_get_and_advance()
{
    return lexer_get_char(lexer.cursor++);
}

char lexer_peek_n_char(int n)
{
    return lexer_get_char(lexer.cursor + n);
}

void lexer_advance()
{
    lexer.cursor++;
}

void lexer_rewind(int n) {
    lexer.cursor -= n;
    if(lexer.cursor < 0) {
        lexer.cursor = 0;
    }
}

enum TokenType get_header_type(int level)
{
    switch(level) {
        case 1:
            return HEADER_1;
        case 2:
            return HEADER_2;
        case 3:
            return HEADER_3;
        case 4:
            return HEADER_4;
        case 5:
            return HEADER_5;
        case 6:
            return HEADER_6;
        default:
            UNREACHABLE("tried to get a header level greater than 6");
    }
}

bool is_special_char(char c)
{
    return c == '#' || c == '\n' || c == '\0' || c == '*';
}

Token lexer_next_token()
{
    char c = lexer_get_and_advance();

    if(c == '#') {
        int level = 1;

        while(lexer_peek_n_char(level - 1) == '#') level++;

        if(lexer_peek_n_char(level - 1) == ' ') {
            lexer.cursor += level;

            return ((Token) {
                .type = get_header_type(level),
                .lexeme = NULL,
            });
        }
    }

    if(c == '\n') {
        return ((Token) {
            .type = NEWLINE,
            .lexeme = NULL,
        });
    }

    if(c == '\0') {
        return ((Token) {
            .type = END_OF_FILE,
            .lexeme = NULL,
        });
    }

    if(c == '*') {
        if(lexer_peek_n_char(0) == '*') {
            lexer_advance();

            return ((Token) {
                .type = BOLD,
                .lexeme = NULL,
            });
        }

        return ((Token) {
            .type = ITALIC,
            .lexeme = NULL,
        });
    }

    int start_pos = lexer.cursor - 1;

    while(!is_special_char(c)) {
        c = lexer_get_and_advance();
    }

    lexer_rewind(1);

    return ((Token) {
        .type = TEXT,
        .lexeme = strndup(lexer.buf + start_pos, lexer.cursor - start_pos),
    });
}

void lexer_free_token(Token token)
{
    if(token.lexeme == NULL) return;
    free(token.lexeme);
}
