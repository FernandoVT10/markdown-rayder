#ifndef LEXER_H_
#define LEXER_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "raylib.h"

#define DA_INIT_CAP 256

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

typedef struct String {
    char *items;
    size_t count;
    size_t capacity;
} String;

enum TokenType {
    HEADER_1,
    HEADER_2,
    HEADER_3,
    HEADER_4,
    HEADER_5,
    HEADER_6,
    NEWLINE,
    TEXT,
    END_OF_FILE,
    ITALIC,
    BOLD,
    CODE,
};

typedef struct Token {
  enum TokenType type;
  String lexeme;
} Token;

typedef struct Lexer {
    char *buf;
    int cursor;
    Token token;
} Lexer;

bool lexer_init(const char *file_path);
Token *lexer_next_token();
void lexer_destroy();

#endif
