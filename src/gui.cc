#include "gui.h"
#include <math.h>
#include <algorithm>
#include <iostream>

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "../lib/raygui.h"
#include "../lib/ricons.h"

#define SCALE_STEP_FACTOR 1.5
#define SCALE_DEPTH_LIMIT 1.0e15
#define MOVE_STEP_FACTOR 0.1
#define WINDOW_INIT_WIDTH 800
#define WINDOW_INIT_HEIGHT 450
#define BUTTON_HEIGHT 40
#define ELEMENT_NUM_VERTICAL 10
#define BUTTON_NUM_TOTAL 11
#define CONTROL_MIN_WIDTH 250
#define CONTROL_MIN_HEIGHT BUTTON_HEIGHT*ELEMENT_NUM_VERTICAL

using namespace std;

Image convert (hfractal_main* hm) { // TODO: Custom mapping between colour vectors
    Color *pixels = (Color *)malloc((hm->resolution)*(hm->resolution)*sizeof(Color));
    for (int x = 0; x < hm->resolution; x++) {
        for (int y = 0; y < hm->resolution; y++) {
            int v = hm->img->get(x,y);
            pixels[(y*hm->resolution)+x] = (v == hm->eval_limit) ? BLACK : ColorFromHSV(v * 2, 0.5, 0.75);
            if (hm->img->completed[(y*hm->resolution)+x] != 2) pixels[(y*hm->resolution)+x].a = 0;
        }
    }
    Image img = {
        .data = pixels,
        .width = hm->resolution,
        .height = hm->resolution,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    return img;
}

// TODO: Implement existing buttons
// TODO: Add equation presets (CURRENT)
// TODO: Add numerical zoom/offset inputs (TF)
// TODO: Worker threads & eval limit controls (TF)
// TODO: Benchmark everything

#define EQ_MANDELBROT 1
#define EQ_JULIA_1 2
#define EQ_JULIA_2 3
#define EQ_RECIPROCAL 4
#define EQ_POLY 5
#define EQ_ZPOWER 6

void ConfigureGuiStyle () {
    // This function implements the 'cyber' interface style provided by raygui's documentation.
    const char* stylesheet = R"(p 00 00 0x2f7486ff    DEFAULT_BORDER_COLOR_NORMAL
p 00 01 0x024658ff    DEFAULT_BASE_COLOR_NORMAL
p 00 02 0x51bfd3ff    DEFAULT_TEXT_COLOR_NORMAL
p 00 03 0x82cde0ff    DEFAULT_BORDER_COLOR_FOCUSED
p 00 04 0x3299b4ff    DEFAULT_BASE_COLOR_FOCUSED
p 00 05 0xb6e1eaff    DEFAULT_TEXT_COLOR_FOCUSED
p 00 06 0xeb7630ff    DEFAULT_BORDER_COLOR_PRESSED
p 00 07 0xffbc51ff    DEFAULT_BASE_COLOR_PRESSED
p 00 08 0xd86f36ff    DEFAULT_TEXT_COLOR_PRESSED
p 00 09 0x134b5aff    DEFAULT_BORDER_COLOR_DISABLED
p 00 10 0x02313dff    DEFAULT_BASE_COLOR_DISABLED
p 00 11 0x17505fff    DEFAULT_TEXT_COLOR_DISABLED
p 00 16 0x00000012    DEFAULT_TEXT_SIZE
p 00 17 0x00000001    DEFAULT_TEXT_SPACING
p 00 18 0x81c0d0ff    DEFAULT_LINE_COLOR
p 00 19 0x00222bff    DEFAULT_BACKGROUND_COLOR)";
    int offset = 0;
    int stylePointIndex = 0;
    string stylePointControl = "";
    string stylePointProperty = "";
    string stylePointValue = "";
    while (offset <= strlen(stylesheet)) {
        if (stylesheet[offset] == ' ') {
            stylePointIndex++;
        } else if (stylesheet[offset] == '\n' || stylesheet[offset] == '\0') {
            GuiSetStyle (stoi(stylePointControl), stoi(stylePointProperty), stol(stylePointValue, nullptr, 16));
            stylePointControl = "";
            stylePointProperty = "";
            stylePointValue = "";
            stylePointIndex = 0;
        } else {
            switch (stylePointIndex) {
            case 1:
                stylePointControl += stylesheet[offset];
                break;
            case 2:
                stylePointProperty += stylesheet[offset];
                break;
            case 3:
                stylePointValue += stylesheet[offset];
                break;
            default:
                break;
            }
        }
        offset++;
    }
    GuiUpdateStyleComplete();
}

string equationPreset () {
    // TODO:
}

int gui_main () {
    hfractal_main* lowres_hm = new hfractal_main();
    hfractal_main* hm = new hfractal_main();

    int imageDimension = WINDOW_INIT_HEIGHT;
    int controlPanelWidth = WINDOW_INIT_WIDTH - imageDimension;

    SetTraceLogLevel (LOG_WARNING | LOG_ERROR | LOG_DEBUG);
    InitWindow(WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT, "HyperFractal Mathematical Visualiser");
    SetWindowState (FLAG_WINDOW_RESIZABLE);
    int minHeight = std::max (256, CONTROL_MIN_HEIGHT);
    SetWindowMinSize(minHeight+CONTROL_MIN_WIDTH, minHeight);  
    SetTargetFPS(30);
    ConfigureGuiStyle ();


    long double start_zoom = /*1.0;*/ 1.477892e+03;
    long double start_x_offset = /*0.0;*/-1.4112756161337429028227244409698926119745010510087013244628906250000000;
    long double start_y_offset = 0.0;

    string dialogText = "";
    int equationPreset = 0;

    // Configure full resolution renderer
    hm->resolution = imageDimension;
    hm->eq = string("(z^2)+c");
    hm->eval_limit = 800;
    hm->worker_threads = 12;
    hm->zoom = start_zoom;
    hm->offset_x = start_x_offset;
    hm->offset_y = start_y_offset;

    // Configure preivew renderer
    lowres_hm->resolution = 64;
    lowres_hm->eq = string("(z^2)+c");
    lowres_hm->eval_limit = 100;
    lowres_hm->worker_threads = 1;
    lowres_hm->zoom = start_zoom;
    lowres_hm->offset_x = start_x_offset;
    lowres_hm->offset_y = start_y_offset;
    
    // Declare states and variables for carrying data between mainloop passes
    bool buttonStates[BUTTON_NUM_TOTAL] = {false}; // The click states of all the buttons
    Image bufferImage = {}; // Image buffer for the fractal image
    Texture2D tex = {}; // Texture buffer for the fractal image
    bool imageNeedsUpdate = true;
    bool isRendering = false; // Indicates that ...
    bool isOutdatedRender = false; // Indicates that the full-res render is out of date (and thus is not showing)
    string consoleText = "Ready.";
    int percent = 0;

    // Generate the initial preview render
    lowres_hm->generateImage(true);
    string equationTmp = "(z^2)+c";
    while (!WindowShouldClose()) {
        if (dialogText == "") {
        // Detect clicking to recenter
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isRendering) {
            Vector2 mpos = GetMousePosition();
            if (mpos.x <= imageDimension && mpos.y <= imageDimension) {
                long double changeInX = (long double)((mpos.x/(imageDimension/2))-1)/hm->zoom;
                long double changeInY = (long double)((mpos.y/(imageDimension/2))-1)/hm->zoom;
                lowres_hm->offset_x += changeInX;
                lowres_hm->offset_y -= changeInY;
                hm->offset_x += changeInX;
                hm->offset_y -= changeInY;
                isOutdatedRender = true;
                consoleText = "Outdated!";
                imageNeedsUpdate = true;
                lowres_hm->generateImage (true);
                sleepcp (1);
            }
        }
        } else {
            GuiLock();
        }

        // Respond to button presses
        if (buttonStates[0] && !isRendering && !imageNeedsUpdate) {
            std::cout << "Rerendering..." << std::endl;
            isRendering = true;
            lowres_hm->generateImage(true);
            isOutdatedRender = true;
            consoleText = "Rendering...";
            hm->generateImage(false);
            imageNeedsUpdate = true;
            sleepcp (1);
        }
        if (buttonStates[1] && !isRendering) {
            if (lowres_hm->zoom <= SCALE_DEPTH_LIMIT) {
                isOutdatedRender = true;
                consoleText = "Outdated!";
                imageNeedsUpdate = true;
                lowres_hm->zoom *= SCALE_STEP_FACTOR;
                hm->zoom *= SCALE_STEP_FACTOR;
                lowres_hm->generateImage (true);
            } else dialogText = "Zoom precision limit reached";
            sleepcp (1);
        }
        if (buttonStates[2] && !isRendering) {
            isOutdatedRender = true;
            consoleText = "Outdated!";
            imageNeedsUpdate = true;
            lowres_hm->zoom /= SCALE_STEP_FACTOR;
            hm->zoom /= SCALE_STEP_FACTOR;
            lowres_hm->generateImage (true);
            sleepcp (1);
        }
        if (buttonStates[3]) std::cout << "Save Image." << std::endl;
        if (buttonStates[4]) std::cout << "Save Render State." << std::endl;
        if (buttonStates[5]) std::cout << "Load Render State." << std::endl;
        if ((buttonStates[6] || IsKeyDown(KEY_UP)) && !isRendering) {
            hm->offset_y += MOVE_STEP_FACTOR/hm->zoom;
            lowres_hm->offset_y += MOVE_STEP_FACTOR/hm->zoom;
            isOutdatedRender = true;
            consoleText = "Outdated!";
            imageNeedsUpdate = true;
            lowres_hm->generateImage (true);
            sleepcp (1);
        }
        if ((buttonStates[7] || IsKeyDown(KEY_LEFT)) && !isRendering) {
            hm->offset_x -= MOVE_STEP_FACTOR/hm->zoom;
            lowres_hm->offset_x -= MOVE_STEP_FACTOR/hm->zoom;
            isOutdatedRender = true;
            consoleText = "Outdated!";
            imageNeedsUpdate = true;
            lowres_hm->generateImage (true);
            sleepcp (1);
        }
        if ((buttonStates[8] || IsKeyDown(KEY_RIGHT)) && !isRendering) {
            hm->offset_x += MOVE_STEP_FACTOR/hm->zoom;
            lowres_hm->offset_x += MOVE_STEP_FACTOR/hm->zoom;
            isOutdatedRender = true;
            consoleText = "Outdated!";
            imageNeedsUpdate = true;
            lowres_hm->generateImage (true);
            sleepcp (1);
        }
        if ((buttonStates[9] || IsKeyDown(KEY_DOWN)) && !isRendering) {
            hm->offset_y -= MOVE_STEP_FACTOR/hm->zoom;
            lowres_hm->offset_y -= MOVE_STEP_FACTOR/hm->zoom;
            isOutdatedRender = true;
            consoleText = "Outdated!";
            imageNeedsUpdate = true;
            lowres_hm->generateImage (true);
            sleepcp (1);
        }
        
        // Image updating code  
        if (imageNeedsUpdate) {
            if (hm->img->is_done() && !isOutdatedRender) {
                UnloadImage (bufferImage);
                UnloadTexture (tex);
                bufferImage = convert (hm);
                isRendering = false;
                consoleText = "Rendering done.";
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
            consoleText = "Rendering: ";
            percent = round(((float)(hm->img->get_ind())/(float)(hm->resolution*hm->resolution))*100);
            consoleText += std::to_string(percent);
            consoleText += "%";
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

        // TODO: Help/instructions

        imageDimension = std::min(GetScreenWidth()-CONTROL_MIN_WIDTH, GetScreenHeight());
        controlPanelWidth = GetScreenWidth()-imageDimension;
        if (!isRendering) {hm->resolution = imageDimension;}
        BeginDrawing();
        Color bgcol = GetColor (GuiGetStyle(00, BACKGROUND_COLOR));
        ClearBackground (RED);
        ClearBackground(bgcol);
        

        // Draw the rendered image      
        Vector2 v {0,0};
        DrawTextureEx (tex, v, 0, (float)imageDimension/(float)tex.height, WHITE);
        // Draw Console
        int buttonOffset = 0;
        GuiTextBox((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth, BUTTON_HEIGHT}, (char*)consoleText.c_str(), 1, false);
        // Draw "Render Image" button
        buttonOffset++;
        buttonStates[0] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth, BUTTON_HEIGHT}, "Render Image");
        // Draw render progress bar
        buttonOffset++;
        GuiProgressBar ((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth, BUTTON_HEIGHT}, "", "", percent, 0, 100);
        // Draw zoom buttons
        buttonOffset++;
        buttonStates[1] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Zoom In");
        buttonStates[2] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/2, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Zoom Out");
        // Draw save image button
        buttonOffset++;
        buttonStates[3] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth, BUTTON_HEIGHT}, "Save Image");
        // Draw render state load/save buttons
        buttonOffset++;
        buttonStates[4] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Save Render State");
        buttonStates[5] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/2, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Load Render State");
        // Draw movement navigation buttons
        buttonOffset++;
        buttonStates[6] = GuiButton((Rectangle){(float)imageDimension+((float)controlPanelWidth-BUTTON_HEIGHT)/2, BUTTON_HEIGHT*(float)buttonOffset, (float)BUTTON_HEIGHT, BUTTON_HEIGHT}, "up");
        buttonOffset++;
        buttonStates[7] = GuiButton((Rectangle){(float)imageDimension+(((float)controlPanelWidth)/2)-BUTTON_HEIGHT, BUTTON_HEIGHT*(float)buttonOffset, (float)BUTTON_HEIGHT, BUTTON_HEIGHT}, "left");
        buttonStates[8] = GuiButton((Rectangle){(float)imageDimension+(((float)controlPanelWidth)/2), BUTTON_HEIGHT*(float)buttonOffset, (float)BUTTON_HEIGHT, BUTTON_HEIGHT}, "right");
        buttonOffset++;
        buttonStates[9] = GuiButton((Rectangle){(float)imageDimension+((float)controlPanelWidth-BUTTON_HEIGHT)/2, BUTTON_HEIGHT*(float)buttonOffset, (float)BUTTON_HEIGHT, BUTTON_HEIGHT}, "down");
        buttonOffset++;
        
        // Custom equation input box
        bool res = GuiTextBox ((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/2, BUTTON_HEIGHT}, equationTmp.data(), 1, false);
        int key = GetCharPressed();
        if ((key == 122 || key == 99 || (key >= 48 && key <= 57) || key == 94 || (key >= 40 && key <= 43) || key == 45 || key == 46 || key == 47 || key == 'i') && !isRendering) {
            equationTmp += (char)key;
            std::cout << equationTmp << std::endl;
            hm->eq = equationTmp;
            lowres_hm->eq = equationTmp;
            if (lowres_hm->generateImage (true)) consoleText = "Invalid equation input";
            else {
                isOutdatedRender = true;
                consoleText = "Outdated!";
                imageNeedsUpdate = true;
            }
        } else if (GetKeyPressed () == KEY_BACKSPACE && !isRendering) {
            equationTmp.pop_back();
            std::cout << equationTmp << std::endl;
            hm->eq = equationTmp;
            lowres_hm->eq = equationTmp;
            if (lowres_hm->generateImage (true)) consoleText = "Invalid equation input";
            else {
                isOutdatedRender = true;
                consoleText = "Outdated!";
                imageNeedsUpdate = true;
            }
        }
        // Equation preset loading
        buttonStates[10] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/2, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Equation Presets");
        

        // Drawing the info dialog
        if (dialogText != "") {
            int textwidth = MeasureText(dialogText.c_str(), GetFontDefault().baseSize);
            DrawRectangle (0, 0, GetScreenWidth(), GetScreenHeight(), (Color){200, 200, 200, 128});
            DrawRectangle ((GetScreenWidth()-textwidth-10)/2, (GetScreenHeight()-GetFontDefault().baseSize-10)/2, textwidth+10, GetFontDefault().baseSize+10, WHITE);
            DrawText (dialogText.c_str(), (GetScreenWidth()-textwidth)/2, (GetScreenHeight()-GetFontDefault().baseSize)/2, GetFontDefault().baseSize, BLACK);
            GuiUnlock();
            bool close = GuiButton((Rectangle){(float)(GetScreenWidth()-textwidth-10)/2, (float)((GetScreenHeight()-GetFontDefault().baseSize-10)/2)+30, (float)(textwidth+10), (float)(GetFontDefault().baseSize+10)}, "OK");
            if (close) dialogText = "";
        }

        EndDrawing();
        GuiUnlock();
    }

    UnloadImage (bufferImage);
    UnloadTexture (tex);
    CloseWindow();
    return 0;
}