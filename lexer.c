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

enum BlockTokenType get_header_type(int level)
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

Tokens lexer_scan_inline_tokens()
{
    Tokens tokens = {0};

    char c;
    int start_pos = lexer.cursor;

    while((c = lexer_get_and_advance())) {
        if(c == '\n') {
            lexer_rewind(1);
            break;
        }
    }

    da_append(&tokens, ((Token) {
        .type = BASIC,
        .value = {
            .basic_token = {
                .type = TEXT,
                .lexeme = strndup(lexer.buf + start_pos, lexer.cursor - start_pos),
            },
        },
    }));

    return tokens;
}

Tokens lexer_scan_tokens()
{
    Tokens tokens = {0};

    char c;
    while((c = lexer_get_and_advance())) {
        if(c == '#') {
            int level = 1;

            while(lexer_peek_n_char(level - 1) == '#') level++;

            if(lexer_peek_n_char(level - 1) != ' ') {
                while(lexer_get_and_advance() != '\n');
                continue;
            }

            lexer.cursor += level;

            Tokens inline_tokens = lexer_scan_inline_tokens();

            da_append(&tokens, ((Token) {
                .type = BLOCK,
                .value = {
                    .block_token = {
                        .type = get_header_type(level),
                        .tokens = &inline_tokens,
                    },
                },
            }));
        } else if(c == '\n') {
            da_append(&tokens, ((Token) {
                .type = BASIC,
                .value = {
                    .basic_token = {
                        .type = NEWLINE,
                        .lexeme = NULL,
                    },
                },
            }));
        }
    }

    return tokens;
}

void lexer_free_tokens(Tokens tokens)
{
    printf("Tokens count: %lu\n", tokens.count);

    for(int i = 0; i < tokens.count; i++) {
        Token token = tokens.items[i];

        if(token.type == BASIC) {
            printf("Freeing basic token: %s\n", token.value.basic_token.lexeme);
            free(token.value.basic_token.lexeme);
            continue;
        }

        printf("Freeing block token with pointer: %p\n", token.value.block_token.tokens->items);

        // lexer_free_tokens(token.value.block_token.tokens);
    }

    printf("Freeing tokens\n");
    free(tokens.items);
}
