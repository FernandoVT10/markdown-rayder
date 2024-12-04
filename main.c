#include "raylib.h"
#include "lexer.h"

#define MD_BLACK CLITERAL(Color){9, 9, 17, 255}
#define MD_WHITE CLITERAL(Color){221, 221, 244, 255}

#define DEFAULT_FONT_SIZE 20
#define HEADER_1_FONT_SIZE 40
#define HEADER_2_FONT_SIZE 37
#define HEADER_3_FONT_SIZE 34
#define HEADER_4_FONT_SIZE 33
#define HEADER_5_FONT_SIZE 30
#define HEADER_6_FONT_SIZE 27

typedef struct String {
    char *items;
    size_t count;
    size_t capacity;
} String;

typedef struct MDText {
    int line;
    int start_col;
    int end_col;
    int font_size;
    char *text;
    bool italic;
    bool bold;
} MDText;

typedef struct MDTexts {
    MDText *items;
    size_t count;
    size_t capacity;
} MDTexts;

int get_header_font_size(enum TokenType type)
{
    switch(type) {
        case HEADER_1:
            return HEADER_1_FONT_SIZE;
        case HEADER_2:
            return HEADER_2_FONT_SIZE;
        case HEADER_3:
            return HEADER_3_FONT_SIZE;
        case HEADER_4:
            return HEADER_4_FONT_SIZE;
        case HEADER_5:
            return HEADER_5_FONT_SIZE;
        case HEADER_6:
            return HEADER_6_FONT_SIZE;
        default:
            return 16;
    }
}

MDTexts get_md_texts()
{
    MDTexts texts = {0};

    Token token = lexer_next_token();
    int line = 0;
    int col = 0;
    int font_size = DEFAULT_FONT_SIZE;
    bool bold = false;
    bool italic = false;
    while(token.type != END_OF_FILE) {
        switch(token.type) {
            case HEADER_1:
            case HEADER_2:
            case HEADER_3:
            case HEADER_4:
            case HEADER_5:
            case HEADER_6:
                font_size = get_header_font_size(token.type);
                break;
            case TEXT: {
                int text_len = strlen(token.lexeme);
                da_append(&texts, ((MDText) {
                    .line = line,
                    .start_col = col,
                    .end_col = col + text_len,
                    .font_size = font_size,
                    .text = strdup(token.lexeme),
                    .italic = italic,
                    .bold = bold,
                }));

                col += text_len;
            } break;
            case NEWLINE: {
                line++;
                col = 0;
                font_size = DEFAULT_FONT_SIZE;
                italic = false;
                bold = false;
            } break;
            case ITALIC:
                italic = !italic;
                break;
            case BOLD:
                bold = !bold;
                break;
            case END_OF_FILE: continue;
        }

        lexer_free_token(token);
        token = lexer_next_token();
    }

    lexer_free_token(token);

    return texts;
}

void free_md_texts(MDTexts texts)
{
    for(size_t i = 0; i < texts.count; i++) {
        free(texts.items[i].text);
    }

    free(texts.items);
}

int main(void)
{
    const char *file_path = "./examples/emphasis.md";

    if(!lexer_init(file_path)) {
        return 1;
    }

    MDTexts texts = get_md_texts();

    for(size_t i = 0; i < texts.count; i++) {
        MDText mdText = texts.items[i];
        printf("Draw Text \"%s\" with font size %u at line %u and col %u with italic: %u bold: %u\n", mdText.text, mdText.font_size, mdText.line, mdText.start_col, mdText.italic, mdText.bold);
    }

    InitWindow(1280, 720, "Markdown RayDer");

    int load_font_size = 72;

    Font font_regular = LoadFontEx("./fonts/Poppins-Regular.ttf", load_font_size, NULL, 0);
    Font font_bold = LoadFontEx("./fonts/Poppins-Bold.ttf", load_font_size, NULL, 0);
    Font font_italic = LoadFontEx("./fonts/Poppins-Italic.ttf", load_font_size, NULL, 0);
    Font font_bold_italic = LoadFontEx("./fonts/Poppins-BoldItalic.ttf", load_font_size, NULL, 0);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(MD_BLACK);

        Vector2 pos = {0};
        int cur_line = 0;

        for(size_t i = 0; i < texts.count; i++) {
            MDText mdText = texts.items[i];

            if(cur_line < mdText.line && i > 0) {
                pos.x = 0;
                pos.y += texts.items[i - 1].font_size * (mdText.line - cur_line);
                cur_line = mdText.line;
            }

            Font font = font_regular;
            Color color = MD_WHITE;

            if(mdText.bold && mdText.italic) {
                font = font_bold_italic;
                color = SKYBLUE;
            } else if(mdText.italic) {
                font = font_italic;
            } else if(mdText.bold) {
                font = font_bold;
                color = SKYBLUE;
            }

            float spacing = 2.0f;

            DrawTextEx(font, mdText.text, pos, mdText.font_size, spacing, color);

            Vector2 size = MeasureTextEx(font, mdText.text, mdText.font_size, spacing);
            pos.x += size.x;
        }


        EndDrawing();
    }

    UnloadFont(font_regular);
    UnloadFont(font_bold);
    UnloadFont(font_italic);
    UnloadFont(font_bold_italic);

    CloseWindow();

    free_md_texts(texts);

    return 0;
}
