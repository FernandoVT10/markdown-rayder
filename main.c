#include "raylib.h"
#include "lexer.h"

#define MD_BLACK CLITERAL(Color){9, 9, 17, 255}
#define MD_WHITE CLITERAL(Color){221, 221, 244, 255}
#define MD_BLUE_BG CLITERAL(Color){133, 170, 249, 50}
#define MD_TRANSPARENT CLITERAL(Color){0}

#define DEFAULT_FONT_SIZE 20
#define HEADER_1_FONT_SIZE 45
#define HEADER_2_FONT_SIZE 40
#define HEADER_3_FONT_SIZE 35
#define HEADER_4_FONT_SIZE 30
#define HEADER_5_FONT_SIZE 25
#define HEADER_6_FONT_SIZE 20

typedef struct TextNode {
    int line;
    int font_size;
    char *text;
    bool italic;
    bool bold;
    Color color;
} TextNode;

typedef struct ListIndicatorNode {
    int line;
} ListIndicatorNode;

typedef struct Fonts {
    Font regular;
    Font bold;
    Font italic;
    Font bold_italic;
} Fonts;

enum MDNodeType {
    TEXT_NODE,
    LIST_INDICATOR_NODE,
};

typedef struct MDNode MDNode;

typedef struct MDNode {
    enum MDNodeType type;
    void *data;
    MDNode *next;
} MDNode;

typedef struct MDList {
    MDNode *head;
    MDNode *tail;
    size_t count;
} MDList;

typedef struct State {
    Fonts fonts;
} State;

State state = {0};

void insert_end_list_item(MDList *list, enum MDNodeType type, void *data)
{
    MDNode *node = malloc(sizeof(MDNode));
    node->type = type;
    node->data = data;
    node->next = NULL;

    if(list->count > 0) {
        MDNode *last_node = list->tail;

        last_node->next = node;
        list->tail = node;
    } else {
        list->head = node;
        list->tail = node;
    }

    list->count++;
}

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

MDList get_parsed_markdown()
{
    MDList list = {0};

    Token *token = lexer_next_token();

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
            case HEADER_6: {
                font_size = get_header_font_size(token->type);
            } break;
            case TEXT: {
                TextNode *text = malloc(sizeof(TextNode));
                *text = (TextNode) {
                    .line = line,
                    .font_size = font_size,
                    .text = strdup(token->lexeme.items),
                    .italic = italic,
                    .bold = bold,
                    .color = color,
                };
                insert_end_list_item(&list, TEXT_NODE, text);
            } break;
            case NEWLINE: {
                if(lexer_is_prev_token(NEWLINE)) break;

                line++;
                font_size = DEFAULT_FONT_SIZE;
                italic = false;
                bold = false;
                color = MD_WHITE;
            } break;
            case ITALIC: {
                italic = !italic;
            } break;
            case BOLD: {
                bold = !bold;
                color = SKYBLUE;
            } break;
            case CODE: {
                TextNode *text = malloc(sizeof(TextNode));
                *text = (TextNode) {
                    .line = line,
                    .font_size = font_size,
                    .text = strdup(token->lexeme.items),
                    .italic = italic,
                    .bold = bold,
                    .color = SKYBLUE,
                };
                insert_end_list_item(&list, TEXT_NODE, text);
            } break;
            case LIST_ITEM: {
                ListIndicatorNode *node = malloc(sizeof(ListIndicatorNode));
                node->line = line;
                insert_end_list_item(&list, TEXT_NODE, node);
            } break;
            case END_OF_FILE: UNREACHABLE("END_OF_FILE reached");
        }

        token = lexer_next_token();
    }

    return list;
}

void free_md_list(MDList list)
{
    // TODO
}

void load_fonts()
{
    int load_font_size = 72;
    state.fonts.regular = LoadFontEx("./fonts/Poppins-Regular.ttf", load_font_size, NULL, 0);
    state.fonts.bold = LoadFontEx("./fonts/Poppins-Bold.ttf", load_font_size, NULL, 0);
    state.fonts.italic = LoadFontEx("./fonts/Poppins-Italic.ttf", load_font_size, NULL, 0);
    state.fonts.bold_italic = LoadFontEx("./fonts/Poppins-BoldItalic.ttf", load_font_size, NULL, 0);
}

void unload_fonts()
{
    UnloadFont(state.fonts.regular);
    UnloadFont(state.fonts.bold);
    UnloadFont(state.fonts.italic);
    UnloadFont(state.fonts.bold_italic);
}

Font get_font_from_text_node(TextNode *text)
{
    if(text->bold && text->italic) {
        return state.fonts.bold_italic;
    } else if(text->italic) {
        return state.fonts.italic;
    } else if(text->bold) {
        return state.fonts.bold;
    }

    return state.fonts.regular;
}

Vector2 draw_text_node(Vector2 pos, int start_bound, int end_bound, TextNode *node)
{
    Font font = get_font_from_text_node(node);

    char *str = strdup(node->text);
    char *word;

    int spacing = 2;

    int space_size = node->font_size * 0.3;

    while((word = strsep(&str, " "))) {
        Vector2 size = MeasureTextEx(font, word, node->font_size, spacing);

        if(pos.x + size.x > end_bound) {
            pos.x = start_bound;
            pos.y += size.y;
        }

        DrawTextEx(font, word, pos, node->font_size, spacing, node->color);
        pos.x += size.x + space_size;
    }

    // Remove the last "margin" to the right
    pos.x -= space_size;

    free(str);

    return pos;
}

int main(void)
{
    const char *file_path = "./examples/partial-example.md";

    assert(lexer_init(file_path));
    MDList list = get_parsed_markdown();
    lexer_destroy();

    InitWindow(1280, 720, "Markdown RayDer");

    load_fonts();

    Vector2 camera = {0};

    while(!WindowShouldClose()) {
        int screen_width = GetScreenWidth();
        int scroll_speed = 1000;

        float dt = GetFrameTime();

        if(IsKeyDown(KEY_DOWN)) {
            camera.y -= scroll_speed * dt;
        } else if(IsKeyDown(KEY_UP)) {
            camera.y += scroll_speed * dt;

            if(camera.y > 0) {
                camera.y = 0;
            }
        }

        if(IsKeyPressed(KEY_F)) {
            ToggleFullscreen();
        }


        BeginDrawing();
        ClearBackground(MD_BLACK);

        int cur_line = 0;
        int line_height = 10;

        Vector2 draw_pos = {0};

        int prev_height = -1;
        MDNode *node = list.head;

        while(node != NULL) {
            switch(node->type) {
                case TEXT_NODE: {
                    TextNode *text_node = (TextNode*)node->data;

                    if(cur_line != text_node->line && prev_height > 0) {
                        pos.x = 0;
                        pos.y += prev_height + line_height;
                        cur_line = text_node->line;
                    }

                    pos = draw_text_node(pos, 20, screen_width - 20, text_node);
                    prev_height = text_node->font_size;
                } break;
                default: continue;
            }

            node = node->next;
        }


        EndDrawing();
    }

    unload_fonts();

    CloseWindow();

    free_md_list(list);

    return 0;
}
