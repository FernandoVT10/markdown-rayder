#include <stdio.h>
#include "raylib.h"

int main(void)
{
    InitWindow(1280, 720, "Markdown RayDer");

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color) {9, 9, 17, 255});
        EndDrawing();
    }

    CloseWindow();
}
