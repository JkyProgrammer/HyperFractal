#include "gui.h"
#include <math.h>
#include <algorithm>
#include <iostream>

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "../lib/raygui.h"

#define WINDOW_INIT_WIDTH 800
#define WINDOW_INIT_HEIGHT 450
#define BUTTON_HEIGHT 40
#define BUTTON_NUM_VERTICAL 2
#define BUTTON_NUM_TOTAL 3
#define CONTROL_MIN_WIDTH 200
#define CONTROL_MIN_HEIGHT BUTTON_HEIGHT*BUTTON_NUM_VERTICAL

Image* convert (hfractal_main*);

int gui_main (hfractal_main* hm) {
    int imageDimension = WINDOW_INIT_HEIGHT;
    int controlPanelWidth = WINDOW_INIT_WIDTH - imageDimension;

    InitWindow(WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT, "HyperFractal Mathematical Visualiser");
    SetWindowState (FLAG_WINDOW_RESIZABLE);
    int minHeight = std::max (256, CONTROL_MIN_HEIGHT);
    SetWindowMinSize(minHeight+CONTROL_MIN_WIDTH, minHeight);  
    SetTargetFPS(30);

    bool buttonStates[BUTTON_NUM_TOTAL] = {false};

    while (!WindowShouldClose()) {
        if (buttonStates[0]) std::cout << "Rerendering..." << std::endl;
        if (buttonStates[1]) std::cout << "Zoom in." << std::endl;
        if (buttonStates[2]) std::cout << "Zoom out." << std::endl;

        imageDimension = std::min(GetScreenWidth()-CONTROL_MIN_WIDTH, GetScreenHeight());
        controlPanelWidth = GetScreenWidth()-imageDimension;
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawRectangle (0,0,imageDimension,imageDimension, RED);
        DrawText("TODO", 190, 200, 20, LIGHTGRAY);
        buttonStates[0] = GuiButton((Rectangle){(float)imageDimension, 0, (float)controlPanelWidth, BUTTON_HEIGHT}, "Rerender Image");
        buttonStates[1] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Zoom In");
        buttonStates[2] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/2, BUTTON_HEIGHT, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Zoom Out");
        EndDrawing();
    }

    CloseWindow();
    return 0;
}