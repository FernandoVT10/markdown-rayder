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

enum TokenType {
    BASIC,
    BLOCK,
};

enum BasicTokenType {
    TEXT,
};

enum BlockTokenType {
    HEADER_1,
    HEADER_2,
    HEADER_3,
    HEADER_4,
    HEADER_5,
    HEADER_6,
    NEWLINE,
};

typedef struct Tokens Tokens;

typedef struct BasicToken {
    enum BasicTokenType type;
    char *lexeme;
} BasicToken;

typedef struct BlockToken {
    enum BlockTokenType type;
    Tokens *tokens;
} BlockToken;

typedef struct Token {
    enum TokenType type;
    union {
      BlockToken block_token;
      BasicToken basic_token;
    } value;
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

bool lexer_init(const char *file_path);
Tokens lexer_scan_tokens();
void lexer_free_tokens(Tokens tokens);

#endif
