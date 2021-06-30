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
#define BUTTON_NUM_VERTICAL 4
#define BUTTON_NUM_TOTAL 6
#define CONTROL_MIN_WIDTH 250
#define CONTROL_MIN_HEIGHT BUTTON_HEIGHT*BUTTON_NUM_VERTICAL

Image convert (hfractal_main* hm) { // TODO: Custom mapping between colour vectors
    Color *pixels = (Color *)malloc((hm->resolution)*(hm->resolution)*sizeof(Color));
    for (int x = 0; x < hm->resolution; x++) {
        for (int y = 0; y < hm->resolution; y++) {
            int v = hm->img->get(x,y);
            float f = (float)v/(float)UINT16_MAX;
            pixels[(y*hm->resolution)+x] = (v == hm->eval_limit) ? BLACK : ColorFromHSV(logf(v), 0.5, 0.75);
            if (hm->img->completed[(y*hm->resolution)+x] != 2) pixels[(y*hm->resolution)+x].a = 0;
        }
    }
    Image img = {
        .data = pixels,
        .width = hm->resolution,
        .height = hm->resolution,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    return img;
}

// TODO: Mini-console
// TODO: Implement existing buttons
// TODO: Equation input
// TODO: Add Movement controlls
// TODO: Add equation presets
// TODO: Add numerical zoom/offset inputs
// TODO: Click navigation

int gui_main () {
    hfractal_main* hm = new hfractal_main();
    hfractal_main* lowres_hm = new hfractal_main();

    int imageDimension = WINDOW_INIT_HEIGHT;
    int controlPanelWidth = WINDOW_INIT_WIDTH - imageDimension;

    SetTraceLogLevel (LOG_WARNING | LOG_ERROR);
    InitWindow(WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT, "HyperFractal Mathematical Visualiser");
    SetWindowState (FLAG_WINDOW_RESIZABLE);
    int minHeight = std::max (256, CONTROL_MIN_HEIGHT);
    SetWindowMinSize(minHeight+CONTROL_MIN_WIDTH, minHeight);  
    SetTargetFPS(30);

    hm->resolution = imageDimension;
    hm->eq = string("(z^2)+c");
    hm->eval_limit = 400;
    hm->worker_threads = 1;
    hm->zoom = 1.0;

    lowres_hm->resolution = 64;
    lowres_hm->eq = string("(z^2)+c");
    lowres_hm->eval_limit = 100;
    lowres_hm->worker_threads = 2;
    lowres_hm->zoom = 1.0;
    

    bool buttonStates[BUTTON_NUM_TOTAL] = {false};
    Image bufferImage = {};
    Texture2D tex = {};

    bool imageNeedsUpdate = true;

    bool isRendering = false;
    bool isOutdatedRender = false;
    lowres_hm->generateImage(true);

    while (!WindowShouldClose()) {
        if (buttonStates[0] && !isRendering && !imageNeedsUpdate) {
            std::cout << "Rerendering..." << std::endl;
            isRendering = true;
            lowres_hm->generateImage(true);
            isOutdatedRender = true;
            hm->generateImage(false);
            imageNeedsUpdate = true;
            sleepcp (1);
        }
        if (buttonStates[1]) {
            std::cout << "Zoom in." << std::endl;
            isOutdatedRender = true;
            imageNeedsUpdate = true;
            lowres_hm->zoom *= 2;
            hm->zoom *= 2;
            lowres_hm->generateImage (true);
            sleepcp (1);
        }
        if (buttonStates[2]) std::cout << "Zoom out." << std::endl;
        if (buttonStates[3]) std::cout << "Save Image." << std::endl;
        if (buttonStates[4]) std::cout << "Save Render State." << std::endl;
        if (buttonStates[5]) std::cout << "Load Render State." << std::endl;
        if (imageNeedsUpdate) {
            if (hm->img->is_done() && !isOutdatedRender) {
                UnloadImage (bufferImage);
                UnloadTexture (tex);
                bufferImage = convert (hm); isRendering = false;
                tex = LoadTextureFromImage(bufferImage);
            } else if (lowres_hm->img->is_done()) {
                UnloadImage (bufferImage);
                UnloadTexture (tex);
                bufferImage = convert (lowres_hm);
                ImageResize (&bufferImage, hm->resolution, hm->resolution);
                tex = LoadTextureFromImage(bufferImage);
            }
            imageNeedsUpdate = false;
        }
        if (isRendering) {
            if (hm->img->is_done()) {
                isRendering = false;
                imageNeedsUpdate = true;
                isOutdatedRender = false;
            } else {
                Image tmp = convert (hm);
                ImageDraw (&bufferImage, tmp, (Rectangle){0,0,(float)hm->resolution,(float)hm->resolution}, (Rectangle){0,0,(float)hm->resolution,(float)hm->resolution}, WHITE);
                UnloadImage (tmp);
                UnloadTexture (tex);
                tex = LoadTextureFromImage(bufferImage);
            }
        }

        // TODO: Custom equation input
        // TODO: Help/instructions

        imageDimension = std::min(GetScreenWidth()-CONTROL_MIN_WIDTH, GetScreenHeight());
        controlPanelWidth = GetScreenWidth()-imageDimension;
        if (!isRendering) {hm->resolution = imageDimension;}
        BeginDrawing();
        ClearBackground(RAYWHITE);        
        Vector2 v {0,0};
        DrawTextureEx (tex, v, 0, (float)imageDimension/(float)tex.height, WHITE);
        buttonStates[0] = GuiButton((Rectangle){(float)imageDimension, 0, (float)controlPanelWidth, BUTTON_HEIGHT}, "Rerender Image");
        buttonStates[1] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Zoom In");
        buttonStates[2] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/2, BUTTON_HEIGHT, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Zoom Out");
        buttonStates[3] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT*2, (float)controlPanelWidth, BUTTON_HEIGHT}, "Save Image");
        buttonStates[4] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT*3, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Save Render State");
        buttonStates[5] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/2, BUTTON_HEIGHT*3, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Load Render State");
        EndDrawing();
    }

    UnloadImage (bufferImage);
    UnloadTexture (tex);
    CloseWindow();
    return 0;
}