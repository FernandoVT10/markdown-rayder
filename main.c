#include "raylib.h"
#include "raymath.h"
#include "lexer.h"
#include "image.h"

#define MD_BLACK CLITERAL(Color){9, 9, 17, 255}
#define MD_BLACK_LIGHT CLITERAL(Color){20, 21, 31, 255}
#define MD_WHITE CLITERAL(Color){221, 221, 244, 255}
#define MD_BLUE_BG CLITERAL(Color){133, 170, 249, 10}
#define MD_TRANSPARENT CLITERAL(Color){0}
#define MD_BLUE CLITERAL(Color){133, 170, 249, 255}

#define LINE_HEIGHT 1.5
#define DEFAULT_FONT_SIZE 20

// HEADERS
#define HEADER_1_FONT_SIZE 45
#define HEADER_2_FONT_SIZE 40
#define HEADER_3_FONT_SIZE 35
#define HEADER_4_FONT_SIZE 30
#define HEADER_5_FONT_SIZE 25
#define HEADER_6_FONT_SIZE 20

// LISTS
#define LIST_MARGIN_LEFT 20
#define LIST_IND_MARGIN_RIGHT 5 // the margin after the list indicator and before the text
#define LIST_DOT_RADIUS 3
#define LIST_DOT_COLOR MD_BLUE
#define LIST_NUM_COLOR MD_BLUE

#define TAB_SIZE 20

enum MDNodeType {
    TEXT_NODE,
    ULIST_INDICATOR_NODE,
    OLIST_INDICATOR_NODE,
    NEWLINE_NODE,
    TAB_NODE,
    LINK_NODE,
    IMAGE_NODE,
    CODE_BLOCK_NODE,
};

typedef struct TextNode {
    int font_size;
    char *text;
    bool italic;
    bool bold;
    Color color;
} TextNode;

typedef struct OListIndicatorNode {
    char *indicator;
} OListIndicatorNode;

typedef struct NewLineNode {
    int line_height; // this refers to the height of the line where the new line appeared
} NewLineNode;

typedef struct LinkNode {
    char *text;
    char *dest;
    bool hover;
} LinkNode;

typedef struct CodeBlockNode {
    char *contents;
} CodeBlockNode;

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

typedef struct Fonts {
    Font regular;
    Font bold;
    Font italic;
    Font bold_italic;
} Fonts;

typedef struct State {
    Fonts fonts;
} State;

State state = {0};

void insert_end_list_item(MDList *list, enum MDNodeType type, void *data)
{
    MDNode *node = calloc(sizeof(MDNode), 1);

    if(node == NULL) {
        TraceLog(LOG_ERROR, "Trying to allocate memory for a MDNode");
        return;
    }

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
        case TKN_HEADER_1:
            return HEADER_1_FONT_SIZE;
        case TKN_HEADER_2:
            return HEADER_2_FONT_SIZE;
        case TKN_HEADER_3:
            return HEADER_3_FONT_SIZE;
        case TKN_HEADER_4:
            return HEADER_4_FONT_SIZE;
        case TKN_HEADER_5:
            return HEADER_5_FONT_SIZE;
        case TKN_HEADER_6:
            return HEADER_6_FONT_SIZE;
        default:
            return DEFAULT_FONT_SIZE;
    }
}

MDList get_parsed_markdown()
{
    MDList list = {0};

    Token *token = lexer_next_token();

    int font_size = DEFAULT_FONT_SIZE;
    bool bold = false;
    bool italic = false;
    Color color = MD_WHITE;

    while(token->type != TKN_EOF) {
        switch(token->type) {
            case TKN_HEADER_1:
            case TKN_HEADER_2:
            case TKN_HEADER_3:
            case TKN_HEADER_4:
            case TKN_HEADER_5:
            case TKN_HEADER_6: {
                font_size = get_header_font_size(token->type);
            } break;
            case TKN_TEXT: {
                TextNode *text = calloc(sizeof(TextNode), 1);

                if(text == NULL) {
                    TraceLog(LOG_ERROR, "Trying to allocate memory for a TextNode");
                    break;
                }

                *text = (TextNode) {
                    .font_size = font_size,
                    .text = strdup(token->lexeme.items),
                    .italic = italic,
                    .bold = bold,
                    .color = color,
                };
                insert_end_list_item(&list, TEXT_NODE, text);
            } break;
            case TKN_NEWLINE: {
                // consecutive new lines should be ignored
                if(lexer_is_prev_token(TKN_NEWLINE)) break;

                NewLineNode *node = calloc(sizeof(NewLineNode), 1);

                if(node == NULL) {
                    TraceLog(LOG_ERROR, "Trying to allocate memory for a NewLineNode");
                    break;
                }

                node->line_height = font_size;
                insert_end_list_item(&list, NEWLINE_NODE, node);

                font_size = DEFAULT_FONT_SIZE;
                italic = false;
                bold = false;
                color = MD_WHITE;
            } break;
            case TKN_ITALIC: {
                italic = !italic;
            } break;
            case TKN_BOLD: {
                bold = !bold;
                color = bold ? MD_BLUE : MD_WHITE;
            } break;
            case TKN_CODE: {
                TextNode *text = calloc(sizeof(TextNode), 1);

                if(text == NULL) {
                    TraceLog(LOG_ERROR, "Trying to allocate memory for a TextNode");
                    break;
                }

                *text = (TextNode) {
                    .font_size = font_size,
                    .text = strdup(token->lexeme.items),
                    .italic = italic,
                    .bold = bold,
                    .color = SKYBLUE,
                };
                insert_end_list_item(&list, TEXT_NODE, text);
            } break;
            case TKN_ULIST_INDICATOR: {
                insert_end_list_item(&list, ULIST_INDICATOR_NODE, NULL);
            } break;
            case TKN_OLIST_INDICATOR: {
                OListIndicatorNode *node = calloc(sizeof(OListIndicatorNode), 1);

                if(node == NULL) {
                    TraceLog(LOG_ERROR, "Trying to allocate memory for a OListIndicatorNode");
                    break;
                }

                node->indicator = strdup(token->lexeme.items);
                insert_end_list_item(&list, OLIST_INDICATOR_NODE, node);
            } break;
            case TKN_TAB: {
                insert_end_list_item(&list, TAB_NODE, NULL);
            } break;
            case TKN_LINK_TEXT: {
                LinkNode *node = calloc(sizeof(LinkNode), 1);
                node->text = strdup(token->lexeme.items);
                insert_end_list_item(&list, LINK_NODE, node);
            } break;
            case TKN_LINK_DEST: {
                MDNode *node = list.tail;
                // TKN_LINK_TEXT should always appear before this token
                // and therefore should create the necessary node
                assert(node->type == LINK_NODE);

                LinkNode *l_node = (LinkNode*)node->data;
                l_node->dest = strdup(token->lexeme.items);
            } break;
            case TKN_IMAGE_ALT: {
                ImageNode *node = calloc(sizeof(ImageNode), 1);
                node->alt = strdup(token->lexeme.items);
                insert_end_list_item(&list, IMAGE_NODE, node);
            } break;
            case TKN_IMAGE_URL: {
                MDNode *node = list.tail;
                // TKN_IMAGE_ALT should always appear before this token
                // and therefore should create the necessary node
                assert(node->type == IMAGE_NODE);
                ImageNode *i_node = (ImageNode*)node->data;

                i_node->url = strdup(token->lexeme.items);
                i_node->loading_image = true;

                image_loader_async_load(i_node);
            } break;
            case TKN_CODE_BLOCK: {
                CodeBlockNode *c_node = calloc(sizeof(CodeBlockNode), 1);

                if(c_node == NULL) {
                    TraceLog(LOG_ERROR, "Trying to allocate memory for a CodeBlockNode");
                    break;
                }

                c_node->contents = strdup(token->lexeme.items);
                insert_end_list_item(&list, CODE_BLOCK_NODE, c_node);
            } break;
            case TKN_EOF: UNREACHABLE("END_OF_FILE reached");
        }

        token = lexer_next_token();
    }

    return list;
}

void free_md_list(MDList list)
{
    MDNode *node = list.head;
    MDNode *old_node = NULL;

    while(node != NULL) {
        switch(node->type) {
            case TEXT_NODE: {
                TextNode *t_node = (TextNode*)node->data;
                free(t_node->text);
            } break;
            case OLIST_INDICATOR_NODE: {
                OListIndicatorNode *l_node = (OListIndicatorNode*)node->data;
                free(l_node->indicator);
            } break;
            case LINK_NODE: {
                LinkNode *l_node = (LinkNode*)node->data;
                free(l_node->text);
                free(l_node->dest);
            } break;
            case IMAGE_NODE: {
                free_image_node((ImageNode *)node->data);
            } break;
            case CODE_BLOCK_NODE: {
                CodeBlockNode *c_node = (CodeBlockNode*)node->data;
                free(c_node->contents);
            } break;
            case NEWLINE_NODE:
            case TAB_NODE:
            case ULIST_INDICATOR_NODE:
                break;
        }

        free(node->data);

        old_node = node;
        node = node->next;

        free(old_node);
    }
}

void load_fonts()
{
    int load_font_size = DEFAULT_FONT_SIZE;
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
            pos.y += size.y + LINE_HEIGHT * node->font_size;
        }

        DrawTextEx(font, word, pos, node->font_size, spacing, node->color);
        pos.x += size.x + space_size;
    }

    // Remove the last "margin" to the right
    pos.x -= space_size;

    free(str);

    return pos;
}

void draw_list_dot(Vector2 *pos)
{
    int radius = LIST_DOT_RADIUS;
    pos->x += LIST_MARGIN_LEFT;

    // NOTE: to center the dot we assume that the font size of the text is the default one
    int center_y = pos->y + DEFAULT_FONT_SIZE / 2;

    DrawCircle(pos->x, center_y, radius, LIST_DOT_COLOR);

    pos->x += radius * 2 + LIST_IND_MARGIN_RIGHT;
}

void draw_list_indicator(Vector2 *pos, OListIndicatorNode *node)
{
    pos->x += LIST_MARGIN_LEFT;

    int spacing = 2;
    Font font = state.fonts.bold;

    DrawTextEx(font, node->indicator, *pos, DEFAULT_FONT_SIZE, spacing, LIST_NUM_COLOR);

    Vector2 size = MeasureTextEx(font, node->indicator, DEFAULT_FONT_SIZE, spacing);
    pos->x += size.x;
}

void open_link(char *dest)
{
    const char *cmd = "open ";
    char *full_cmd = calloc(strlen(cmd) + strlen(dest) + 1, 1);

    if(full_cmd == NULL) {
        TraceLog(LOG_ERROR, "Couldn't allocate memory because of for the cmd command");
        return;
    }

    strcpy(full_cmd, cmd);
    strcat(full_cmd, dest);

    // TODO: log error when this fails
    system(full_cmd);
}

void handle_link(Vector2 *pos, LinkNode *node)
{
    int spacing = 2;
    Font font = state.fonts.regular;
    Vector2 size = MeasureTextEx(font, node->text, DEFAULT_FONT_SIZE, spacing);

    Vector2 mouse_pos = GetMousePosition();
    Rectangle link_boundary = {
        .x = pos->x,
        .y = pos->y,
        .width = size.x,
        .height = size.y,
    };

    // we use hover like this just to call SetMouseCursor once
    if(CheckCollisionPointRec(mouse_pos, link_boundary)) {
        if(!node->hover) {
            node->hover = true;
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }
    } else {
        if(node->hover) {
            node->hover = false;
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
    }

    if(node->hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && node->dest) {
        open_link(node->dest);
    }

    Color color = node->hover ? MD_BLUE : MD_WHITE;
    // draw link text
    DrawTextEx(font, node->text, *pos, DEFAULT_FONT_SIZE, spacing, color);

    // draw line below text
    float line_pos_y = pos->y + size.y;
    Vector2 start_line = {
        .x = pos->x,
        .y = line_pos_y
    };
    Vector2 end_line = {
        .x = pos->x + size.x,
        .y = line_pos_y
    };
    float thickness = 1;
    DrawLineEx(start_line, end_line, thickness, color);

    pos->x += size.x;
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        TraceLog(LOG_ERROR, "file path is required");
        return -1;
    } else if(argc > 2) {
        TraceLog(LOG_ERROR, "too many arguments");
        return -1;
    }

    const char *file_path = argv[1];
    if(!lexer_init(file_path)) {
        return -1;
    }

    InitWindow(1280, 720, "Markdown RayDer");
    SetTargetFPS(60);

    image_loader_init();
    MDList list = get_parsed_markdown();
    lexer_destroy();
    load_fonts();

    Vector2 camera_pos = {0};

    while(!WindowShouldClose()) {
        int screen_width = GetScreenWidth();
        int scroll_speed = 1000;

        float dt = GetFrameTime();

        if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J)) {
            camera_pos.y -= scroll_speed * dt;
        } else if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_K)) {
            camera_pos.y += scroll_speed * dt;

            if(camera_pos.y > 0) {
                camera_pos.y = 0;
            }
        }

        if(IsKeyPressed(KEY_F)) {
            if(IsWindowFullscreen()) {
                SetWindowSize(1280, 720);
            } else {
                int monitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            }
            ToggleFullscreen();
        }

        BeginDrawing();
        ClearBackground(MD_BLACK);

        int line_height = 10;
        Vector2 draw_pos = camera_pos;
        MDNode *node = list.head;

        while(node != NULL) {
            switch(node->type) {
                case TEXT_NODE: {
                    TextNode *text_node = (TextNode*)node->data;
                    draw_pos = draw_text_node(draw_pos, 0, screen_width, text_node);
                } break;
                case NEWLINE_NODE: {
                    NewLineNode *n_node = (NewLineNode*)node->data;
                    draw_pos.y += n_node->line_height + line_height;
                    draw_pos.x = 0;
                } break;
                case ULIST_INDICATOR_NODE: {
                    draw_list_dot(&draw_pos);
                } break;
                case OLIST_INDICATOR_NODE: {
                    draw_list_indicator(&draw_pos, (OListIndicatorNode*)node->data);
                } break;
                case TAB_NODE: {
                    draw_pos.x += TAB_SIZE;
                } break;
                case LINK_NODE: {
                    handle_link(&draw_pos, (LinkNode*)node->data);
                } break;
                case IMAGE_NODE: {
                    ImageNode *i_node = (ImageNode*)node->data;
                    Vector2 image_size = draw_image_node(draw_pos, screen_width, i_node);
                    draw_pos = Vector2Add(draw_pos, image_size);
                } break;
                case CODE_BLOCK_NODE: {
                    CodeBlockNode *c_node = (CodeBlockNode*)node->data;

                    int spacing = 2;
                    Font font = state.fonts.regular;
                    int padding = 20;

                    Vector2 text_size = MeasureTextEx(font, c_node->contents, DEFAULT_FONT_SIZE, spacing);
                    DrawRectangle(0, draw_pos.y, screen_width, text_size.y + padding * 2, MD_BLACK_LIGHT);

                    Vector2 text_pos = {
                        draw_pos.x + padding,
                        draw_pos.y + padding,
                    };

                    DrawTextEx(font, c_node->contents, text_pos, DEFAULT_FONT_SIZE, spacing, MD_WHITE);

                    draw_pos.x += screen_width;
                    draw_pos.y += text_size.y + padding * 2 - DEFAULT_FONT_SIZE;
                } break;
            }

            node = node->next;
        }

        EndDrawing();
    }

    unload_fonts();
    free_md_list(list);
    CloseWindow();

    image_loader_destroy();

    return 0;
}
