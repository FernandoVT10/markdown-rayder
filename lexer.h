#ifndef LEXER_H_
#define LEXER_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "linklist.h"

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
    TKN_TEXT,
    TKN_EOF,
    TKN_NEWLINE,
    TKN_HASH,
    TKN_SPACE,
    TKN_ATERISK,
    TKN_UNDERSCORE,
    // TKN_BACKTICK,
    // TKN_DASH,
    // TKN_DOT,
    // TKN_NUMBER,
    // TKN_TAB,
    // TKN_OPEN_SQR_BRACKET, TKN_CLOSE_SQR_BRACKET,
    // TKN_OPEN_PARENTHESIS, TKN_CLOSE_PARENTHESIS,
    // TKN_EXCLAMATION,
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

    size_t tkn_cursor;
    Tokens tokens;
} Lexer;

bool lexer_init(const char *file_path);
Token *lexer_next_tkn();
bool lexer_is_n_tkn(enum TokenType type, int n);
bool lexer_is_prev_tkn(enum TokenType type);
bool lexer_is_first_tkn();
void lexer_adv_tkn_cursor(int n);
void lexer_rewind_tkn_cursor(int n);
void lexer_destroy();

#endif
