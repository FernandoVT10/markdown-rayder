#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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

typedef struct String {
    char *items;
    size_t count;
    size_t capacity;
} String;

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
    const char *file_path = "./a.txt";
    char *file_content = load_file_contents(file_path);

    if(file_content == NULL) {
        printf("Error: %s couldn't be opened", file_path);
        return 1;
    }

    String str = {0};
    da_append(&str, 'h');
    da_append(&str, 'e');
    da_append(&str, '\0');

    // InitWindow(1280, 720, "Markdown RayDer");
    //
    // while(!WindowShouldClose()) {
    //     BeginDrawing();
    //     ClearBackground((Color) {9, 9, 17, 255});
    //
    //     DrawText(file_content, 0, 0, 20, (Color) { 255, 255, 255, 255 });
    //     EndDrawing();
    // }
    //
    // CloseWindow();

    unload_file_contents(file_content);
    return 0;
}
