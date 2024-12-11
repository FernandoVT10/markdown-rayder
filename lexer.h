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
    TKN_HEADER_1,
    TKN_HEADER_2,
    TKN_HEADER_3,
    TKN_HEADER_4,
    TKN_HEADER_5,
    TKN_HEADER_6,
    TKN_NEWLINE,
    TKN_TEXT,
    TKN_EOF,
    TKN_ITALIC,
    TKN_BOLD,
    TKN_CODE,
    TKN_ULIST_INDICATOR, // unordered list
    TKN_OLIST_INDICATOR, // ordered list
    TKN_TAB,
    TKN_LINK_TEXT,
    TKN_LINK_DEST,
};

typedef struct Token {
  enum TokenType type;
  String lexeme;
} Token;

typedef struct Lexer {
    char *buf;
    int cursor;
    size_t token_count;
    Token prev_token;
    Token token;
} Lexer;

bool lexer_init(const char *file_path);
bool lexer_is_prev_token(enum TokenType type);
Token *lexer_next_token();
void lexer_destroy();

#endif
