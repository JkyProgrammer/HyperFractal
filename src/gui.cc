
#include "../lib/raylib.h"
#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "../lib/raygui.h"
#include "gui.h"
#include <math.h>
#include <algorithm>

#define CONTROL_MIN_WIDTH 200
#define WINDOW_INIT_WIDTH 800
#define WINDOW_INIT_HEIGHT 450

int gui_main () {

    int imageDimension = WINDOW_INIT_HEIGHT;
    int controlPanelWidth = WINDOW_INIT_WIDTH - imageDimension;

    InitWindow(WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT, "HyperFractal Mathematical Visualiser");
    SetWindowState (FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(256+CONTROL_MIN_WIDTH, 256);  
    SetTargetFPS(30);

    bool exitWindow = false;

    while (!exitWindow && !WindowShouldClose()) {
        imageDimension = std::min(GetScreenWidth()-CONTROL_MIN_WIDTH, GetScreenHeight());
        controlPanelWidth = GetScreenWidth()-imageDimension;
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawRectangle (0,0,imageDimension,imageDimension, RED);
        DrawText("TODO", 190, 200, 20, LIGHTGRAY);
        exitWindow = GuiButton((Rectangle){(float)imageDimension, 0, (float)controlPanelWidth, 30}, "kill me");
        EndDrawing();
    }

    CloseWindow();
    return 0;
}