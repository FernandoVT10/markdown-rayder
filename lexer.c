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
        return EOF;
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
    return c == '#' || c == '\n' || c == EOF || c == '*' || c == '`';
}

void copy_buf_to_string(String *str, char *buf, size_t buf_size)
{
    str->count = 0;
    for(size_t i = 0; i < buf_size; i++) {
        da_append(str, buf[i]);
    }
    da_append(str, '\0');
}

void lexer_set_only_token_type(enum TokenType type)
{
    lexer.token.type = type;
    lexer.token.lexeme.count = 0;
}

void lexer_process_next_token()
{
    char c = lexer_get_and_advance();

    if(c == '#') {
        int level = 1;

        while(lexer_peek_n_char(level - 1) == '#') level++;

        if(lexer_peek_n_char(level - 1) == ' ') {
            lexer.cursor += level;
            lexer_set_only_token_type(get_header_type(level));
            return;
        }
    }

    if(c == '\n') {
        lexer_set_only_token_type(NEWLINE);
        return;
    }

    if(c == EOF) {
        lexer_set_only_token_type(END_OF_FILE);
        return;
    }

    if(c == '*') {
        if(lexer_peek_n_char(0) == '*') {
            lexer_advance();

            lexer_set_only_token_type(BOLD);
            return;
        }

        lexer_set_only_token_type(ITALIC);
        return;
    }

    if(c == '`') {
        lexer.token.type = CODE;

        int start_pos = lexer.cursor;

        c = lexer_get_and_advance();

        while(c != '`' && c != '\n' && c != EOF) {
            c = lexer_get_and_advance();
        }

        copy_buf_to_string(&lexer.token.lexeme, lexer.buf + start_pos, lexer.cursor - start_pos - 1);

        if(c != '`') {
            // we rewind either the \n or EOF
            lexer_rewind(1);
        }
        return;
    }

    int start_pos = lexer.cursor - 1;

    while(!is_special_char(c)) {
        c = lexer_get_and_advance();
    }

    lexer_rewind(1);

    lexer.token.type = TEXT;
    copy_buf_to_string(&lexer.token.lexeme, lexer.buf + start_pos, lexer.cursor - start_pos);
}

Token *lexer_next_token()
{
    lexer_process_next_token();
    return &lexer.token;
}

void lexer_destroy()
{
    assert(lexer.buf != NULL);
    assert(lexer.token.lexeme.items != NULL);

    free(lexer.buf);
    free(lexer.token.lexeme.items);
}
