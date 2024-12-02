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

typedef struct Strings {
    char **items;
    size_t count;
    size_t capacity;
} Strings;

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

void unload_file_contents(char *contents)
{
    free(contents);
}

int main(void)
{
    const char *file_path = "./ascii.txt";
    char *file_content = load_file_contents(file_path);

    if(file_content == NULL) {
        TraceLog(LOG_ERROR, "Couldn't open file %s: %s\n", file_path, strerror(errno));
        return 1;
    }

    Strings words = {0};

    char *token = strtok(file_content, " ");

    while(token != NULL) {
        da_append(&words, strdup(token));
        token = strtok(0, " ");
    }

    InitWindow(1280, 720, "Markdown RayDer");

    int font_size = 20;
    int line_height = 10;

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(MD_BLACK);

        int x = 0;
        int y = 0;
        int max_width = 600;
        for(size_t i = 0; i < words.count; i++) {
            int word_width = MeasureText(words.items[i], font_size);

            if(x + word_width > max_width) {
                y += font_size + line_height;
                x = 0;
            }

            DrawText(words.items[i], x, y, font_size, MD_WHITE);

            x += word_width + font_size;
        }

        EndDrawing();
    }

    CloseWindow();

    for(int i = 0; i < words.count; i++) {
        free(words.items[i]);
    }

    da_free(&words);

    unload_file_contents(file_content);
    return 0;
}
