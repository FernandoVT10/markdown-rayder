#include "raylib.h"
#include "lexer.h"

#define MD_BLACK CLITERAL(Color){9, 9, 17, 255}
#define MD_WHITE CLITERAL(Color){221, 221, 244, 255}
#define MD_BLUE_BG CLITERAL(Color){133, 170, 249, 50}
#define MD_TRANSPARENT CLITERAL(Color){0}

#define DEFAULT_FONT_SIZE 20
#define HEADER_1_FONT_SIZE 40
#define HEADER_2_FONT_SIZE 37
#define HEADER_3_FONT_SIZE 34
#define HEADER_4_FONT_SIZE 33
#define HEADER_5_FONT_SIZE 30
#define HEADER_6_FONT_SIZE 27

typedef struct MDText {
    int line;
    int font_size;
    char *text;
    bool italic;
    bool bold;
    Color color;
    Color bg;
} MDText;

typedef struct MDTexts {
    MDText *items;
    size_t count;
    size_t capacity;
} MDTexts;

typedef struct Fonts {
    Font regular;
    Font bold;
    Font italic;
    Font bold_italic;
} Fonts;

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
            return DEFAULT_FONT_SIZE;
    }
}

MDTexts get_md_texts()
{
    MDTexts texts = {0};

    Token *token = lexer_next_token();
    enum TokenType prev_token_type;

    int line = 0;
    int font_size = DEFAULT_FONT_SIZE;
    bool bold = false;
    bool italic = false;
    Color color = MD_WHITE;

    while(token->type != END_OF_FILE) {
        switch(token->type) {
            case HEADER_1:
            case HEADER_2:
            case HEADER_3:
            case HEADER_4:
            case HEADER_5:
            case HEADER_6:
                font_size = get_header_font_size(token->type);
                break;
            case TEXT: {
                da_append(&texts, ((MDText) {
                    .line = line,
                    .font_size = font_size,
                    .text = strdup(token->lexeme.items),
                    .italic = italic,
                    .bold = bold,
                    .color = color,
                    .bg = MD_TRANSPARENT,
                }));
            } break;
            case NEWLINE: {
                if(prev_token_type == NEWLINE) break;

                line++;
                font_size = DEFAULT_FONT_SIZE;
                italic = false;
                bold = false;
                color = MD_WHITE;
            } break;
            case ITALIC:
                italic = !italic;
                break;
            case BOLD: {
                bold = !bold;
            } break;
            case CODE:
                da_append(&texts, ((MDText) {
                    .line = line,
                    .font_size = font_size,
                    .text = strdup(token->lexeme.items),
                    .italic = italic,
                    .bold = bold,
                    .color = SKYBLUE,
                    .bg = MD_BLUE_BG,
                }));
                break;
            case END_OF_FILE: UNREACHABLE("END_OF_FILE reached");
        }

        prev_token_type = token->type;
        token = lexer_next_token();
    }

    return texts;
}

void free_md_texts(MDTexts texts)
{
    for(size_t i = 0; i < texts.count; i++) {
        free(texts.items[i].text);
    }

    free(texts.items);
}

Fonts load_fonts()
{
    int load_font_size = 72;
    Fonts fonts = {
        .regular = LoadFontEx("./fonts/Poppins-Regular.ttf", load_font_size, NULL, 0),
        .bold = LoadFontEx("./fonts/Poppins-Bold.ttf", load_font_size, NULL, 0),
        .italic = LoadFontEx("./fonts/Poppins-Italic.ttf", load_font_size, NULL, 0),
        .bold_italic = LoadFontEx("./fonts/Poppins-BoldItalic.ttf", load_font_size, NULL, 0),
    };
    return fonts;
}

void unload_fonts(Fonts fonts)
{
    UnloadFont(fonts.regular);
    UnloadFont(fonts.bold);
    UnloadFont(fonts.italic);
    UnloadFont(fonts.bold_italic);
}

int main(void)
{
    const char *file_path = "./examples/emphasis.md";

    assert(lexer_init(file_path));
    MDTexts texts = get_md_texts();
    lexer_destroy();

    InitWindow(1280, 720, "Markdown RayDer");

    Fonts fonts = load_fonts();

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(MD_BLACK);

        Vector2 pos = {0};
        int cur_line = 0;

        int margin_top = 10;

        for(size_t i = 0; i < texts.count; i++) {
            MDText mdText = texts.items[i];

            if(cur_line < mdText.line && i > 0) {
                pos.x = 0;
                pos.y += texts.items[i - 1].font_size + margin_top;
                cur_line = mdText.line;
            }

            Font font = fonts.regular;

            if(mdText.bold && mdText.italic) {
                font = fonts.bold_italic;
            } else if(mdText.italic) {
                font = fonts.italic;
            } else if(mdText.bold) {
                font = fonts.bold;
            }

            float spacing = 2.0f;

            char *str = strdup(mdText.text);
            char *token;
            int screen_width = GetScreenWidth();

            Rectangle bg_rect = {
                .x = pos.x,
                .y = pos.y,
                .height = mdText.font_size,
            };

            while((token = strsep(&str, " "))) {
                Vector2 size = MeasureTextEx(font, token, mdText.font_size, spacing);

                if(pos.x + size.x > screen_width) {
                    pos.x = 0;
                    pos.y += size.y;
                }

                DrawTextEx(font, token, pos, mdText.font_size, spacing, mdText.color);
                pos.x += size.x + 10;
                bg_rect.width += size.x + 10;
            }

            bg_rect.width -= 10;
            pos.x -= 10;
            DrawRectangle(bg_rect.x, bg_rect.y, bg_rect.width, bg_rect.height, mdText.bg);
            free(str);
        }


        EndDrawing();
    }

    CloseWindow();

    free_md_texts(texts);

    return 0;
}
