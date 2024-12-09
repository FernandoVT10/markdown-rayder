#include <ctype.h>
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
    if(pos < 0) pos = 0;

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

bool lexer_is_next_char(char c) {
    return lexer_peek_n_char(0) == c;
}

void lexer_advance()
{
    lexer.cursor++;
}

void lexer_advance_n(int n)
{
    lexer.cursor += n;
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
        case 1: return TKN_HEADER_1;
        case 2: return TKN_HEADER_2;
        case 3: return TKN_HEADER_3;
        case 4: return TKN_HEADER_4;
        case 5: return TKN_HEADER_5;
        case 6: return TKN_HEADER_6;
        default:
            UNREACHABLE("tried to get a header level greater than 6");
    }
}

// returns true for any char that belong to an inline token
bool is_special_char(char c)
{
    return c == '\n' || c == EOF || c == '*' || c == '`' || c == '_';
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

bool lexer_is_first_token()
{
    return lexer.token_count == 0;
}

bool lexer_is_prev_token(enum TokenType type)
{
    return lexer.prev_token.type == type;
}

bool lexer_is_prev_token_whitespace()
{
    return lexer_is_prev_token(TKN_NEWLINE)
            || lexer_is_first_token()
            || lexer_is_prev_token(TKN_TAB);
}

void lexer_process_next_token()
{
    char c = lexer_get_and_advance();

    // TABS
    if(c == ' ' && lexer_is_prev_token_whitespace()) {
        int spaces_count = 1;

        while(lexer_is_next_char(' ') && spaces_count < 4) {
            spaces_count++;
            c = lexer_get_and_advance();
        }

        if(spaces_count > 1) {
            lexer_set_only_token_type(TKN_TAB);
            return;
        }

        // if there's only 1 space, we ignore it, and continue lexing the next char
        c = lexer_get_and_advance();
    }

    // HEADERS
    if(c == '#' && lexer_is_prev_token_whitespace()) {
        int level = 1;

        while(lexer_peek_n_char(level - 1) == '#') level++;

        if(lexer_peek_n_char(level - 1) == ' ') {
            lexer.cursor += level;
            lexer_set_only_token_type(get_header_type(level));
            return;
        }
    }

    // NEWLINE
    if(c == '\n') {
        lexer_set_only_token_type(TKN_NEWLINE);
        return;
    }

    // END OF FILE
    if(c == EOF) {
        lexer_set_only_token_type(TKN_EOF);
        return;
    }

    // UNORDERED LISTS
    // NOTE: It's important for this to be before of the bold & italic check
    if(c == '*' && lexer_is_prev_token_whitespace()) {
        if(lexer_is_next_char(' ')) {
            lexer_advance(1);
            lexer_set_only_token_type(TKN_ULIST_INDICATOR);
            return;
        }
    }

    // ORDERED LISTS
    if(isdigit(c) && lexer_is_prev_token_whitespace()) {
        int start_pos = lexer.cursor - 1;
        int digit_count = 1;

        while(isdigit(lexer_peek_n_char(digit_count - 1))) digit_count++;

        if(lexer_peek_n_char(digit_count - 1) == '.' && lexer_peek_n_char(digit_count) == ' ') {
            lexer_advance_n(digit_count + 1);

            lexer.token.type = TKN_OLIST_INDICATOR;
            copy_buf_to_string(&lexer.token.lexeme, lexer.buf + start_pos, lexer.cursor - start_pos);
            return;
        }
    }

    // BOLD AND ITALIC
    if(c == '*' || c == '_') {
        if((c == '*' && lexer_is_next_char('*')) || (c == '_' && lexer_is_next_char('_'))) {
            lexer_advance();

            lexer_set_only_token_type(TKN_BOLD);
            return;
        }

        lexer_set_only_token_type(TKN_ITALIC);
        return;
    }

    // INLINE CODE
    if(c == '`') {
        lexer.token.type = TKN_CODE;

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

    // TEXT
    int start_pos = lexer.cursor - 1;

    c = lexer_get_and_advance();
    while(!is_special_char(c)) {
        c = lexer_get_and_advance();
    }

    // we rewind the special character encountered
    lexer_rewind(1);

    lexer.token.type = TKN_TEXT;
    copy_buf_to_string(&lexer.token.lexeme, lexer.buf + start_pos, lexer.cursor - start_pos);
}

Token *lexer_next_token()
{
    if(lexer.token_count > 0) {
        lexer.prev_token.type = lexer.token.type;
    }
    lexer_process_next_token();
    lexer.token_count++;
    return &lexer.token;
}

void lexer_destroy()
{
    if(lexer.buf != NULL)
        free(lexer.buf);

    da_free(&lexer.token.lexeme);
    da_free(&lexer.prev_token.lexeme);
}
