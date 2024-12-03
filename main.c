#include "raylib.h"
#include "lexer.h"

#define MD_BLACK CLITERAL(Color){9, 9, 17, 255}
#define MD_WHITE CLITERAL(Color){221, 221, 244, 255}

#define HEADER_1_FONT_SIZE 35
#define HEADER_2_FONT_SIZE 32
#define HEADER_3_FONT_SIZE 29
#define HEADER_4_FONT_SIZE 26
#define HEADER_5_FONT_SIZE 23
#define HEADER_6_FONT_SIZE 20

typedef struct Strings {
    char **items;
    size_t count;
    size_t capacity;
} Strings;

void draw_header(BlockToken header)
{

}

int main(void)
{
    const char *file_path = "./examples/header.md";

    if(!lexer_init(file_path)) {
        return 1;
    }

    Tokens tokens = lexer_scan_tokens();
    TraceLog(LOG_INFO, "Number of lexed tokens: %lu", tokens.count);

    for(size_t i = 0; i < tokens.count; i++) {
        int type = 0;
        Token token = tokens.items[i];
        if(token.type == BASIC) {
            type = token.value.basic_token.type;
        } else {
            type = token.value.block_token.type;
        }
        // TraceLog(LOG_INFO, "Token Type: %u", type);
    }

    lexer_free_tokens(tokens);

    // InitWindow(1280, 720, "Markdown RayDer");
    //
    // while(!WindowShouldClose()) {
    //     BeginDrawing();
    //     ClearBackground(MD_BLACK);
    //
    //     int y = 0;
    //
    //     for(size_t i = 0; i < tokens.count; i++) {
    //         BlockToken token = tokens.items[i];
    //
    //         switch(token.type) {
    //             case HEADER_1:
    //             case HEADER_2:
    //             case HEADER_3:
    //             case HEADER_4:
    //             case HEADER_5:
    //             case HEADER_6:
    //                 draw_header(token);
    //             default: continue;
    //         }
    //         // DrawText(token.lexeme, 0, y, font_size, MD_WHITE);
    //     }
    //
    //     EndDrawing();
    // }
    //
    // CloseWindow();

    return 0;
}
