#include "gui.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <thread>

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "../lib/raygui.h"
#include "../lib/ricons.h"

#define SCALE_STEP_FACTOR 1.5       // Factor by which scaling changes
#define SCALE_DEPTH_LIMIT 1.0e15    // Limit to prevent user from going too deep due to limited precision
#define MOVE_STEP_FACTOR 0.1        // Factor by which position changes
#define WINDOW_INIT_WIDTH 900       // Initial window - width
#define WINDOW_INIT_HEIGHT 550      //                - height
#define BUTTON_HEIGHT 30            // Height of a single button in the interface
#define ELEMENT_NUM_VERTICAL 12     // Number of vertical elements
#define BUTTON_NUM_TOTAL 15         // Total number of buttons in the interface
#define CONTROL_MIN_WIDTH 400       // Minimum width of the control panel
#define CONTROL_MIN_HEIGHT BUTTON_HEIGHT*ELEMENT_NUM_VERTICAL

using namespace std;

/**
 * @brief Convert the result of a render (B-W map) into a coloured image
 * 
 * @param hm Hyperfractal instance from which to extract the image
 * @return Image 
 */
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

// DONE: Crash when fullscreening
// DONE: Buttons stop responding sometimes - was a multithreading issue on the image

// TODO: Add comments to all the code (CURRENT)
// TODO: Implement existing buttons
// TODO: Add numerical zoom/offset inputs (TF)
// TODO: Better equation input?
// TODO: Help/instructions

// TODO: Remove all debugging related stuff

// CONGRATS! We're running a 12.21s benchmark vs 37.00s from the Java version

#define EQ_MANDELBROT 1 // "(z^2)+c"
#define EQ_JULIA_1 2 // "(z^2)+(0.285+0.01i)"
#define EQ_JULIA_2 3 // "(z^2)+(-0.70176-0.3842i)"
#define EQ_RECIPROCAL 4 // "1/((z^2)+c)"
#define EQ_ZPOWER 5 // "(z^z)+(c-0.5)"
#define EQ_BARS 6 // "z^(c^2)"

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

/**
 * @brief 
 * 
 * @param i 
 * @param t 
 * @return string 
 */
string equationPreset (int i, bool t) {
    switch (i) {
    case 1:
        return t ? "Mandelbrot" : "(z^2)+c";
    case 2:
        return t ? "Juila 1" : "(z^2)+(0.285+0.01i)";
    case 3:
        return t ? "Julia 2" : "(z^2)+(-0.70176-0.3842i)";
    case 4:
        return t ? "Reciprocal" : "1/((z^2)+c)";
    case 5:
        return t ? "Z Power" : "(z^z)+(c-0.5)";
    case 6:
        return t ? "Bars" : "z^(c^2)";
    }
    return "";
}

int gui_main () {
    // Initialise the renderers
    hfractal_main* lowres_hm = new hfractal_main();
    hfractal_main* hm = new hfractal_main();

    const auto thread_count = std::thread::hardware_concurrency ();
    int imageDimension = WINDOW_INIT_HEIGHT;
    int controlPanelWidth = WINDOW_INIT_WIDTH - imageDimension;

    SetTraceLogLevel (LOG_WARNING | LOG_ERROR | LOG_DEBUG);
    InitWindow(WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT, "HyperFractal Mathematical Visualiser");
    SetWindowState (FLAG_WINDOW_RESIZABLE);
    int minHeight = std::max (256, CONTROL_MIN_HEIGHT);
    SetWindowMinSize(minHeight+CONTROL_MIN_WIDTH, minHeight);  
    SetTargetFPS(30);
    ConfigureGuiStyle ();

    long double start_zoom = 1.0; //1.477892e+03;
    long double start_x_offset = 0.0; //-1.4112756161337429028227244409698926119745010510087013244628906250000000;
    long double start_y_offset = 0.0;

    string dialogText = "";
    string equationDefault = equationPreset (EQ_MANDELBROT, false);

    // Configure full resolution renderer
    hm->resolution = imageDimension;
    hm->eq = equationDefault;
    hm->eval_limit = 400;
    hm->worker_threads = thread_count;
    hm->zoom = start_zoom;
    hm->offset_x = start_x_offset;
    hm->offset_y = start_y_offset;

    // Configure preivew renderer
    lowres_hm->resolution = 64;
    lowres_hm->eq = equationDefault;
    lowres_hm->eval_limit = 300;
    lowres_hm->worker_threads = thread_count/2;
    lowres_hm->zoom = start_zoom;
    lowres_hm->offset_x = start_x_offset;
    lowres_hm->offset_y = start_y_offset;
    
    // Declare states and variables for carrying data between mainloop passes
    bool buttonStates[BUTTON_NUM_TOTAL] = {false}; // The click states of all the buttons
    Image bufferImage = {}; // Image buffer for the fractal image
    Texture2D tex = {}; // Texture buffer for the fractal image
    bool imageNeedsUpdate = true;
    bool isRendering = false; // Indicates that we're currently rendering the full resolution image (meaning the hm configuration is locked)
    bool isOutdatedRender = false; // Indicates that the full-res render is out of date (and thus is not showing)
    bool equationPresetDialog = false; // Is the equation preset dialog enabled
    float presetDialogX = 0.0;
    float presetDialogY = 0.0;
    string consoleText = "Ready."; // Text shown on the console text
    int percent = 0; // Render completion percentage
    bool showCoords = true; // Indicates whether the coordinates of the mouse cursor in math space are shown
    bool needsToUpdateResolution = false;

    // Generate the initial preview render
    lowres_hm->generateImage(true);
    string equationTmp = equationDefault;
    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            imageDimension = std::min(GetScreenWidth()-CONTROL_MIN_WIDTH, GetScreenHeight());
            controlPanelWidth = GetScreenWidth()-imageDimension;
            if (!isRendering) { hm->resolution = imageDimension; cout << "set!" << endl; }
            else needsToUpdateResolution = true;
        }
        
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

        // Render button pressed
        if ((buttonStates[0] || IsKeyPressed(KEY_ENTER)) && !isRendering && !imageNeedsUpdate) {
            std::cout << "Rerendering..." << std::endl;
            isRendering = true;
            lowres_hm->generateImage(true);
            if (lowres_hm->main_equation != NULL) {
                isOutdatedRender = true;
                consoleText = "Rendering...";
                hm->generateImage(false);
                imageNeedsUpdate = true;
                sleepcp (1);
            } else {
                consoleText = "Invalid equation!";
                isRendering = false;
            }
        }

        // Zoom in button pressed
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
        
        // Zoom out button pressed
        if (buttonStates[2] && !isRendering) {
            isOutdatedRender = true;
            consoleText = "Outdated!";
            imageNeedsUpdate = true;
            lowres_hm->zoom /= SCALE_STEP_FACTOR;
            hm->zoom /= SCALE_STEP_FACTOR;
            lowres_hm->generateImage (true);
            sleepcp (1);
        }

        // Reset zoom button pressed
        if (buttonStates[11] && !isRendering) {
            isOutdatedRender = true;
            consoleText = "Outdated!";
            imageNeedsUpdate = true;
            lowres_hm->zoom = 1.0;
            hm->zoom = 1.0;
            lowres_hm->generateImage (true);
            sleepcp (1);
        }

        if (buttonStates[3] && !isRendering) std::cout << "Save Image." << std::endl;
        if (buttonStates[4] && !isRendering) std::cout << "Save Render State." << std::endl;
        if (buttonStates[5] && !isRendering) std::cout << "Load Render State." << std::endl;
        
        // Movement buttons pressed
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
        
        // Toggle show coordinates button pressed
        if (buttonStates[12]) {
            showCoords = !showCoords;
        }

        // Decrease eval limit button pressed
        if ((buttonStates[13] || IsKeyDown((int)'[')) && !isRendering && !equationPresetDialog) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT)) {
                hm->eval_limit -= 10;
                lowres_hm->eval_limit -= 10;
            } else {
                hm->eval_limit--;
                lowres_hm->eval_limit--;
            }
            isOutdatedRender = true;
            consoleText = "Outdated!";
            imageNeedsUpdate = true;
            lowres_hm->generateImage (true);
            sleepcp (1);
        }

        // Increase eval limit button pressed
        if ((buttonStates[14] || IsKeyDown((int)']')) && !isRendering && !equationPresetDialog) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT)) {
                hm->eval_limit += 10;
                lowres_hm->eval_limit += 10;
            } else {
                hm->eval_limit++;
                lowres_hm->eval_limit++;
            }
            isOutdatedRender = true;
            consoleText = "Outdated!";
            imageNeedsUpdate = true;
            lowres_hm->generateImage (true);
            sleepcp (1);
        }
        
        BeginDrawing();
        Color bgcol = GetColor (GuiGetStyle(00, BACKGROUND_COLOR));
        ClearBackground(bgcol);

        // Image updating code  
        if (imageNeedsUpdate) {
            if (hm->img->is_done() && !isOutdatedRender) {
                UnloadImage (bufferImage);
                UnloadTexture (tex);
                bufferImage = convert (hm);
                isRendering = false;
                consoleText = "Rendering done.";
                tex = LoadTextureFromImage(bufferImage);
                if (needsToUpdateResolution) { hm->resolution = imageDimension; cout << "set finally!" << endl; }
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
                Image tmp = convert (hm); // Convert the current render result into an image
                // Write that over the image buffer
                ImageDraw (&bufferImage, tmp, (Rectangle){0,0,(float)hm->resolution,(float)hm->resolution}, (Rectangle){0,0,(float)hm->resolution,(float)hm->resolution}, WHITE);
                UnloadImage (tmp); // Unload the old image
                UnloadTexture (tex); // Unload the old texture
                tex = LoadTextureFromImage(bufferImage); // Reinitialise the texture from the image
            }
        }
        

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
        buttonStates[1] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/3, BUTTON_HEIGHT}, "Zoom In");
        buttonStates[11] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/3, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/3, BUTTON_HEIGHT}, "Reset Zoom");
        buttonStates[2] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/(3.0f/2.0f), BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/3, BUTTON_HEIGHT}, "Zoom Out");
        // Draw save image button
        buttonOffset++;
        buttonStates[3] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth, BUTTON_HEIGHT}, "Save Image");
        // Draw render state load/save buttons
        buttonOffset++;
        buttonStates[4] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Save Render State");
        buttonStates[5] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/2, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Load Render State");
        // Draw movement navigation buttons
        buttonOffset++;
        buttonStates[6] = GuiButton((Rectangle){(float)imageDimension+((float)controlPanelWidth-40)/2, BUTTON_HEIGHT*(float)buttonOffset, (float)40, BUTTON_HEIGHT}, "up");
        buttonOffset++;
        buttonStates[7] = GuiButton((Rectangle){(float)imageDimension+(((float)controlPanelWidth)/2)-40, BUTTON_HEIGHT*(float)buttonOffset, (float)40, BUTTON_HEIGHT}, "left");
        buttonStates[8] = GuiButton((Rectangle){(float)imageDimension+(((float)controlPanelWidth)/2), BUTTON_HEIGHT*(float)buttonOffset, (float)40, BUTTON_HEIGHT}, "right");
        buttonOffset++;
        buttonStates[9] = GuiButton((Rectangle){(float)imageDimension+((float)controlPanelWidth-40)/2, BUTTON_HEIGHT*(float)buttonOffset, (float)40, BUTTON_HEIGHT}, "down");
        buttonOffset++;
        buttonStates[10] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/2, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/2, BUTTON_HEIGHT}, "Equation Presets");
        if (buttonStates[10] && !isRendering) {
            equationPresetDialog = true;
            presetDialogX = (float)imageDimension+(float)controlPanelWidth/2;
            presetDialogY = BUTTON_HEIGHT*(float)(buttonOffset+1);
        }

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
        } else if (GetKeyPressed () == KEY_BACKSPACE && !isRendering && equationTmp.length() > 0) {
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
        buttonOffset++;

        // Coordinate toggle button
        string b12Text = "Hide coordinates";
        if (!showCoords) b12Text = "Show coordinates";
        buttonStates[12] = GuiButton((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth, BUTTON_HEIGHT}, b12Text.c_str());
        buttonOffset++;

        buttonStates[13] = GuiButton((Rectangle){(float)imageDimension+(float)controlPanelWidth/2, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/4, BUTTON_HEIGHT}, "<");
        buttonStates[14] = GuiButton((Rectangle){(float)imageDimension+((float)controlPanelWidth/4)*3, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/4, BUTTON_HEIGHT}, ">");
        char evalLimString[16];
        sprintf (evalLimString, "%d (%d)", hm->eval_limit, lowres_hm->eval_limit);
        GuiTextBox ((Rectangle){(float)imageDimension, BUTTON_HEIGHT*(float)buttonOffset, (float)controlPanelWidth/2, BUTTON_HEIGHT}, evalLimString, 1, false);
        buttonOffset++;
        

        // Draw the equation preset dialog
        if (equationPresetDialog) {
            for (int e = 1; e < 7; e++) {
                if (
                    GuiButton((Rectangle){presetDialogX, presetDialogY+(BUTTON_HEIGHT*(e-1)), (float)controlPanelWidth/2, BUTTON_HEIGHT}, equationPreset(e, true).c_str())
                && !isRendering
                ) {
                    equationPresetDialog = false;
                    equationTmp = equationPreset (e, false);
                    hm->eq = equationTmp;
                    lowres_hm->eq = equationTmp;
                    if (lowres_hm->generateImage (true)) consoleText = "Invalid equation input";
                    else {
                        isOutdatedRender = true;
                        consoleText = "Outdated!";
                        imageNeedsUpdate = true;
                    }
                }
            }
            if (GetMouseX() < presetDialogX || GetMouseX() > presetDialogX + (float)controlPanelWidth/2 ||  GetMouseY() < presetDialogY - BUTTON_HEIGHT || GetMouseY() > presetDialogY + (BUTTON_HEIGHT*6)) {
                equationPresetDialog = false;
            }
        }

        

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

        // Draw coordinates text next to cursor
        if (dialogText == "" && showCoords && GetMouseX() <= imageDimension && GetMouseY() <= imageDimension) {
            float left = GetMouseX()+15;
            float top = GetMouseY()+15;
            Color col {250, 250, 250, 200};
            Font oldFont = GuiGetFont();
            
            long double locX = hm->offset_x + ((long double)(((long double)GetMouseX()/(imageDimension/2))-1))/hm->zoom;
            long double locY = hm->offset_y - ((long double)(((long double)GetMouseY()/(imageDimension/2))-1))/hm->zoom;
            char t[142];
            sprintf (t, "%.10Lf\n%.10Lf", locX, locY);
            DrawRectangle (left, top, 115, 40, col);
            DrawText (t, left+5, top, 15, BLACK);
        }

        EndDrawing();
        GuiUnlock();
    }

    UnloadImage (bufferImage);
    UnloadTexture (tex);
    CloseWindow();
    return 0;
}