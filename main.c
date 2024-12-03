#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "raylib.h"

#define DA_INIT_CAP 256

#define MD_BLACK CLITERAL(Color){9, 9, 17, 255}
#define MD_WHITE CLITERAL(Color){221, 221, 244, 255}

#define HEADER_1_FONT_SIZE 35
#define HEADER_2_FONT_SIZE 32
#define HEADER_3_FONT_SIZE 29
#define HEADER_4_FONT_SIZE 26
#define HEADER_5_FONT_SIZE 23
#define HEADER_6_FONT_SIZE 20

#define da_append(da, item)                                                          \
    do {                                                                             \
        if((da)->count >= (da)->capacity) {                                          \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "No enough ram");                          \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while(0)

#define da_free(da) do { free((da)->items); } while(0)

#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

typedef struct Strings {
    char **items;
    size_t count;
    size_t capacity;
} Strings;

enum TokenType {
    HEADER_1,
    HEADER_2,
    HEADER_3,
    HEADER_4,
    HEADER_5,
    HEADER_6,
    NEWLINE
};

typedef struct Token {
    enum TokenType type;
    char *lexeme;
} Token;

typedef struct Tokens {
    Token *items;
    size_t count;
    size_t capacity;
} Tokens;

typedef struct Lexer {
    char *buf;
    int cursor;
} Lexer;

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

            int start_pos = lexer.cursor;

            while(lexer_get_and_advance() != '\n');

            Token token = {
                .type = get_header_type(level),
                .lexeme = strndup(lexer.buf + start_pos, lexer.cursor - start_pos - 1),
            };

            da_append(&tokens, token);
        }
    }

    return tokens;
}

int main(void)
{
    const char *file_path = "./examples/header.md";

    if(!lexer_init(file_path)) {
        return 1;
    }

    Tokens tokens = lexer_scan_tokens();
    TraceLog(LOG_INFO, "Number of lexed tokens: %lu", tokens.count);

    InitWindow(1280, 720, "Markdown RayDer");

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(MD_BLACK);

        int y = 0;

        for(size_t i = 0; i < tokens.count; i++) {
            Token token = tokens.items[i];

            int font_size = 0;

            switch(token.type) {
                case HEADER_1:
                    font_size = HEADER_1_FONT_SIZE;
                    break;
                case HEADER_2:
                    font_size = HEADER_2_FONT_SIZE;
                    break;
                case HEADER_3:
                    font_size = HEADER_3_FONT_SIZE;
                    break;
                case HEADER_4:
                    font_size = HEADER_4_FONT_SIZE;
                    break;
                case HEADER_5:
                    font_size = HEADER_5_FONT_SIZE;
                    break;
                case HEADER_6:
                    font_size = HEADER_6_FONT_SIZE;
                    break;
                default: continue;
            }

            DrawText(token.lexeme, 0, y, font_size, MD_WHITE);
            y += font_size;
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
