#include "gui.hh"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <thread>
#include "utils.hh"

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
#define ELEMENT_NUM_VERTICAL 14     // Number of vertical elements
#define BUTTON_NUM_TOTAL 16         // Total number of buttons in the interface
#define CONTROL_MIN_WIDTH 400       // Minimum width of the control panel
#define CONTROL_MIN_HEIGHT BUTTON_HEIGHT*ELEMENT_NUM_VERTICAL
#define DIALOG_TEXT_SIZE 25
#define HELP_TEXT_SIZE 15

using namespace std;

/**
 * @brief Convert the result of a render (B-W map) into a coloured HFractalImage
 * 
 * @param hm Hyperfractal instance from which to extract the HFractalImage
 * @return Image 
 */
Image convert (HFractalMain* hm) {
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
// DONE: Buttons stop responding sometimes - was a multithreading issue on the HFractalImage

// TODO: Add comments to all the code
// TODO: Rewrite pixel distribution
// TODO: Remove all debugging related stuff
// TODO: Class-ify gui
// TODO: Custom mapping between colour vectors, move into HFractalImage classs
// TODO: Add a 'jump to' dialog
// TODO: Database system
// TODO: PNG writing

// CONGRATS! We're running a 12.21s benchmark vs 37.00s from the Java version
// Ah, unfortunately thats now a 20s benchmark using the more precise infinity comparison
// Using optimisations and with hard-coded presets, we achieve a THREE SECOND BENCHMARK

void configureGuiStyle () {
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
            GuiSetStyle (stoi(stylePointControl), stoi(stylePointProperty), stoll(stylePointValue, nullptr, 16));
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

int guiMain () {
    // Initialise the renderers
    HFractalMain* lowres_hm = new HFractalMain();
    HFractalMain* hm = new HFractalMain();

    // Get config data
    const auto thread_count = std::thread::hardware_concurrency ();
    int image_dimension = WINDOW_INIT_HEIGHT;
    int control_panel_width = WINDOW_INIT_WIDTH - image_dimension;

    // GUI configuration
    SetTraceLogLevel (LOG_WARNING | LOG_ERROR | LOG_DEBUG);
    InitWindow(WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT, "HyperFractal Mathematical Visualiser");
    SetWindowState (FLAG_WINDOW_RESIZABLE);
    int min_height = std::max (256, CONTROL_MIN_HEIGHT);
    SetWindowMinSize(min_height+CONTROL_MIN_WIDTH, min_height);  
    SetTargetFPS(30);
    configureGuiStyle ();

    long double start_zoom = 1.0; //1.477892e+03;
    long double start_x_offset = 0.0; //-1.4112756161337429028227244409698926119745010510087013244628906250000000;
    long double start_y_offset = 0.0;

    string dialog_text = "";
    string equation_default = equationPreset (EQ_MANDELBROT, false);

    // Configure full resolution renderer
    hm->resolution = image_dimension;
    hm->eq = equation_default;
    hm->eval_limit = 200;
    hm->worker_threads = thread_count;
    hm->zoom = start_zoom;
    hm->offset_x = start_x_offset;
    hm->offset_y = start_y_offset;

    // Configure preivew renderer
    lowres_hm->resolution = 64;
    lowres_hm->eq = equation_default;
    lowres_hm->eval_limit = 200;
    lowres_hm->worker_threads = thread_count/2;
    lowres_hm->zoom = start_zoom;
    lowres_hm->offset_x = start_x_offset;
    lowres_hm->offset_y = start_y_offset;
    
    // Declare states and variables for carrying data between mainloop passes
    bool button_states[BUTTON_NUM_TOTAL] = {false}; // The click states of all the buttons
    Image buffer_image = {}; // Image buffer for the fractal HFractalImage
    Texture2D buffer_texture = {}; // Texture buffer for the fractal HFractalImage
    bool image_needs_update = true;
    bool is_rendering = false; // Indicates that we're currently rendering the full resolution HFractalImage (meaning the hm configuration is locked)
    bool is_outdated_render = true; // Indicates that the full-res render is out of date (and thus is not showing)
    bool equation_preset_dialog = false; // Is the HFractalEquation preset dialog enabled
    float preset_dialog_x = 0.0;
    float preset_dialog_y = 0.0;
    string console_text = "Ready."; // Text shown on the console text
    int percent = 0; // Render completion percentage
    bool show_coords = true; // Indicates whether the coordinates of the mouse cursor in math space are shown
    bool needs_to_update_resolution = false;
    int modal_view_state = 0;    // Indicates what modal state the interface is in:
                                /*
                                 * 0 - Interface is in normal mode
                                 * 1 - Interface is in text dialog mode
                                 * 2 - Interface is in render state database mode
                                 * 3 - Interface is in HFractalEquation prest dialog mode
                                 */

    // Generate the initial preview render
    lowres_hm->generateImage(true);
    string equation_tmp = equation_default;
    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            image_dimension = std::min(GetScreenWidth()-CONTROL_MIN_WIDTH, GetScreenHeight());
            control_panel_width = GetScreenWidth()-image_dimension;
            if (!is_rendering) { hm->resolution = image_dimension;}
            else needs_to_update_resolution = true;
        }
        
        if (modal_view_state == 0 || modal_view_state == 3) {
            // Detect clicking to recenter
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !is_rendering) {
                Vector2 mpos = GetMousePosition();
                if (mpos.x <= image_dimension && mpos.y <= image_dimension) {
                    long double change_in_x = (long double)((mpos.x/(image_dimension/2))-1)/hm->zoom;
                    long double change_in_y = (long double)((mpos.y/(image_dimension/2))-1)/hm->zoom;
                    lowres_hm->offset_x += change_in_x;
                    lowres_hm->offset_y -= change_in_y;
                    hm->offset_x += change_in_x;
                    hm->offset_y -= change_in_y;
                    is_outdated_render = true;
                    console_text = "Outdated!";
                    image_needs_update = true;
                    lowres_hm->generateImage (true);
                    crossPlatformDelay (1);
                }
            }
        } else {
            GuiLock();
        }

        // Render button pressed
        if ((button_states[0] || IsKeyPressed(KEY_ENTER)) && !is_rendering && !image_needs_update) {
            std::cout << "Rerendering..." << std::endl;
            is_rendering = true;
            lowres_hm->generateImage(true);
            if (lowres_hm->main_equation != NULL) {
                is_outdated_render = true;
                console_text = "Rendering...";
                hm->generateImage(false);
                image_needs_update = true;
                crossPlatformDelay (1);
            } else {
                console_text = "Invalid HFractalEquation!";
                is_rendering = false;
            }
        }

        // Zoom in button pressed
        if (button_states[1] && !is_rendering) {
            if (lowres_hm->zoom <= SCALE_DEPTH_LIMIT) {
                is_outdated_render = true;
                console_text = "Outdated!";
                image_needs_update = true;
                lowres_hm->zoom *= SCALE_STEP_FACTOR;
                hm->zoom *= SCALE_STEP_FACTOR;
                lowres_hm->generateImage (true);
            } else dialog_text = "Zoom precision limit reached";
            crossPlatformDelay (1);
        }
        
        // Zoom out button pressed
        if (button_states[2] && !is_rendering) {
            is_outdated_render = true;
            console_text = "Outdated!";
            image_needs_update = true;
            lowres_hm->zoom /= SCALE_STEP_FACTOR;
            hm->zoom /= SCALE_STEP_FACTOR;
            lowres_hm->generateImage (true);
            crossPlatformDelay (1);
        }

        // Reset zoom button pressed
        if (button_states[11] && !is_rendering) {
            is_outdated_render = true;
            console_text = "Outdated!";
            image_needs_update = true;
            lowres_hm->zoom = 1.0;
            hm->zoom = 1.0;
            lowres_hm->generateImage (true);
            crossPlatformDelay (1);
        }

        // Save Image button pressed
        if (button_states[3] && !is_rendering && modal_view_state == 0) {
            bool result = false;
            if (is_outdated_render) {
                result = autoWriteImage (lowres_hm->img, imageType::PGM);
            } else {
                result = autoWriteImage (hm->img, imageType::PGM);
            }
            if (result) {
                console_text = "Image saved successfully.";
            } else {
                console_text = "Image saving failed.";
            }
        }

        if (button_states[4] && !is_rendering) std::cout << "Save Render State." << std::endl;
        if (button_states[5] && !is_rendering) std::cout << "Load Render State." << std::endl;
        
        // Movement buttons pressed
        if ((button_states[6] || IsKeyDown(KEY_UP)) && !is_rendering) {
            hm->offset_y += MOVE_STEP_FACTOR/hm->zoom;
            lowres_hm->offset_y += MOVE_STEP_FACTOR/hm->zoom;
            is_outdated_render = true;
            console_text = "Outdated!";
            image_needs_update = true;
            lowres_hm->generateImage (true);
            crossPlatformDelay (1);
        }
        if ((button_states[7] || IsKeyDown(KEY_LEFT)) && !is_rendering) {
            hm->offset_x -= MOVE_STEP_FACTOR/hm->zoom;
            lowres_hm->offset_x -= MOVE_STEP_FACTOR/hm->zoom;
            is_outdated_render = true;
            console_text = "Outdated!";
            image_needs_update = true;
            lowres_hm->generateImage (true);
            crossPlatformDelay (1);
        }
        if ((button_states[8] || IsKeyDown(KEY_RIGHT)) && !is_rendering) {
            hm->offset_x += MOVE_STEP_FACTOR/hm->zoom;
            lowres_hm->offset_x += MOVE_STEP_FACTOR/hm->zoom;
            is_outdated_render = true;
            console_text = "Outdated!";
            image_needs_update = true;
            lowres_hm->generateImage (true);
            crossPlatformDelay (1);
        }
        if ((button_states[9] || IsKeyDown(KEY_DOWN)) && !is_rendering) {
            hm->offset_y -= MOVE_STEP_FACTOR/hm->zoom;
            lowres_hm->offset_y -= MOVE_STEP_FACTOR/hm->zoom;
            is_outdated_render = true;
            console_text = "Outdated!";
            image_needs_update = true;
            lowres_hm->generateImage (true);
            crossPlatformDelay (1);
        }
        
        // Toggle show coordinates button pressed
        if (button_states[12]) {
            show_coords = !show_coords;
        }

        // Decrease eval limit button pressed
        if ((button_states[13] || IsKeyDown((int)'[')) && !is_rendering && !equation_preset_dialog) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT)) {
                hm->eval_limit -= 10;
                lowres_hm->eval_limit -= 10;
            } else {
                hm->eval_limit--;
                lowres_hm->eval_limit--;
            }
            is_outdated_render = true;
            console_text = "Outdated!";
            image_needs_update = true;
            lowres_hm->generateImage (true);
            crossPlatformDelay (1);
        }

        // Increase eval limit button pressed
        if ((button_states[14] || IsKeyDown((int)']')) && !is_rendering && !equation_preset_dialog) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT)) {
                hm->eval_limit += 10;
                lowres_hm->eval_limit += 10;
            } else {
                hm->eval_limit++;
                lowres_hm->eval_limit++;
            }
            is_outdated_render = true;
            console_text = "Outdated!";
            image_needs_update = true;
            lowres_hm->generateImage (true);
            crossPlatformDelay (1);
        }

        // Help button pressed
        if (button_states[15]) {
            system ("open https://github.com/JkyProgrammer/HyperFractal/blob/main/README.md");
        }
        
        BeginDrawing();
        Color bg_col = GetColor (GuiGetStyle(00, BACKGROUND_COLOR));
        ClearBackground(bg_col);

        // Image updating code
        if (image_needs_update) {
            if (hm->img->isDone() && !is_outdated_render) {
                UnloadImage (buffer_image);
                UnloadTexture (buffer_texture);
                buffer_image = convert (hm);
                is_rendering = false;
                console_text = "Rendering done.";
                buffer_texture = LoadTextureFromImage(buffer_image);
                if (needs_to_update_resolution) { hm->resolution = image_dimension; cout << "set finally!" << endl; }
            } else if (lowres_hm->img->isDone()) {
                UnloadImage (buffer_image);
                UnloadTexture (buffer_texture);
                buffer_image = convert (lowres_hm);
                ImageResize (&buffer_image, hm->resolution, hm->resolution);
                buffer_texture = LoadTextureFromImage(buffer_image);
            }
            image_needs_update = false;
        }
        if (is_rendering) {
            console_text = "Rendering: ";
            percent = round(((float)(hm->img->getInd())/(float)(hm->resolution*hm->resolution))*100);
            console_text += std::to_string(percent);
            console_text += "%";
            if (hm->img->isDone()) {
                is_rendering = false;
                image_needs_update = true;
                is_outdated_render = false;
            } else {
                Image tmp = convert (hm); // Convert the current render result into an HFractalImage
                // Write that over the HFractalImage buffer
                ImageDraw (&buffer_image, tmp, (Rectangle){0,0,(float)hm->resolution,(float)hm->resolution}, (Rectangle){0,0,(float)hm->resolution,(float)hm->resolution}, WHITE);
                UnloadImage (tmp); // Unload the old HFractalImage
                UnloadTexture (buffer_texture); // Unload the old texture
                buffer_texture = LoadTextureFromImage(buffer_image); // Reinitialise the texture from the HFractalImage
            }
        } 

        // Draw the rendered HFractalImage      
        Vector2 v {0,0};
        DrawTextureEx (buffer_texture, v, 0, (float)image_dimension/(float)buffer_texture.height, WHITE);
        // Draw Console
        int button_offset = 0;
        GuiTextBox((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, (char*)console_text.c_str(), 1, false);
        // Draw "Render Image" button
        button_offset++;
        button_states[0] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, "Render Image");
        // Draw render progress bar
        button_offset++;
        GuiProgressBar ((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, "", "", percent, 0, 100);
        // Draw zoom buttons
        button_offset++;
        button_states[1] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/3, BUTTON_HEIGHT}, "Zoom In");
        button_states[11] = GuiButton((Rectangle){(float)image_dimension+(float)control_panel_width/3, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/3, BUTTON_HEIGHT}, "Reset Zoom");
        button_states[2] = GuiButton((Rectangle){(float)image_dimension+(float)control_panel_width/(3.0f/2.0f), BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/3, BUTTON_HEIGHT}, "Zoom Out");
        // Draw save HFractalImage button
        button_offset++;
        button_states[3] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, "Save Image");
        // Draw render state load/save buttons
        button_offset++;
        button_states[4] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/2, BUTTON_HEIGHT}, "Save Render State");
        button_states[5] = GuiButton((Rectangle){(float)image_dimension+(float)control_panel_width/2, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/2, BUTTON_HEIGHT}, "Load Render State");
        // Draw movement navigation buttons
        button_offset++;
        button_states[6] = GuiButton((Rectangle){(float)image_dimension+((float)control_panel_width-40)/2, BUTTON_HEIGHT*(float)button_offset, (float)40, BUTTON_HEIGHT}, "up");
        button_offset++;
        button_states[7] = GuiButton((Rectangle){(float)image_dimension+(((float)control_panel_width)/2)-40, BUTTON_HEIGHT*(float)button_offset, (float)40, BUTTON_HEIGHT}, "left");
        button_states[8] = GuiButton((Rectangle){(float)image_dimension+(((float)control_panel_width)/2), BUTTON_HEIGHT*(float)button_offset, (float)40, BUTTON_HEIGHT}, "right");
        button_offset++;
        button_states[9] = GuiButton((Rectangle){(float)image_dimension+((float)control_panel_width-40)/2, BUTTON_HEIGHT*(float)button_offset, (float)40, BUTTON_HEIGHT}, "down");
        button_offset++;
        button_states[10] = GuiButton((Rectangle){(float)image_dimension+(float)control_panel_width/2, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/2, BUTTON_HEIGHT}, "Equation Presets");
        if (button_states[10] && !is_rendering) {
            equation_preset_dialog = true;
            modal_view_state = 3;
            preset_dialog_x = (float)image_dimension+(float)control_panel_width/2;
            preset_dialog_y = BUTTON_HEIGHT*(float)(button_offset+1);
        }

        // Custom HFractalEquation input box
        bool res = GuiTextBox ((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/2, BUTTON_HEIGHT}, equation_tmp.data(), 1, false);
        int key = GetCharPressed();
        if ((((int)'a' <= key && key <= (int)'c') || ((int)'x' <= key && key <= (int)'z') || key == 122 || (key >= 48 && key <= 57) || key == 94 || (key >= 40 && key <= 43) || key == 45 || key == 46 || key == 47 || key == 'i') && !is_rendering) {
            equation_tmp += (char)key;
            hm->eq = equation_tmp;
            lowres_hm->eq = equation_tmp;
            if (lowres_hm->generateImage (true)) console_text = "Invalid HFractalEquation input";
            else {
                is_outdated_render = true;
                console_text = "Outdated!";
                image_needs_update = true;
            }
        } else if (GetKeyPressed () == KEY_BACKSPACE && !is_rendering && equation_tmp.length() > 0) {
            equation_tmp.pop_back();
            hm->eq = equation_tmp;
            lowres_hm->eq = equation_tmp;
            if (lowres_hm->generateImage (true)) console_text = "Invalid HFractalEquation input";
            else {
                is_outdated_render = true;
                console_text = "Outdated!";
                image_needs_update = true;
            }
        }
        button_offset++;

        // Coordinate toggle button
        string coord_button_text = "Hide coordinates";
        if (!show_coords) coord_button_text = "Show coordinates";
        button_states[12] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, coord_button_text.c_str());
        button_offset++;

        button_states[13] = GuiButton((Rectangle){(float)image_dimension+(float)control_panel_width/2, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/4, BUTTON_HEIGHT}, "<");
        button_states[14] = GuiButton((Rectangle){(float)image_dimension+((float)control_panel_width/4)*3, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/4, BUTTON_HEIGHT}, ">");
        char evalLimString[16];
        sprintf (evalLimString, "%d (%d)", hm->eval_limit, lowres_hm->eval_limit);
        GuiTextBox ((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/2, BUTTON_HEIGHT}, evalLimString, 1, false);
        button_offset++;
        
        button_states[15] = GuiButton((Rectangle){(float)image_dimension, (float)GetScreenHeight()-(2*BUTTON_HEIGHT), (float)control_panel_width, BUTTON_HEIGHT*2}, "Help & Instructions");

        // Clear all button states to prevent clicking 'through' HFractalEquation preset buttons
        if (modal_view_state == 3) {
            for (int i = 0; i < BUTTON_NUM_TOTAL; i++) button_states[i] = false;
        }

        // Draw the HFractalEquation preset dialog
        if (equation_preset_dialog && modal_view_state == 3) {
            for (int e = 1; e <= NUM_EQUATION_PRESETS; e++) {
                if (
                    GuiButton((Rectangle){preset_dialog_x, preset_dialog_y+(BUTTON_HEIGHT*(e-1)), (float)control_panel_width/2, BUTTON_HEIGHT}, equationPreset(e, true).c_str())
                && !is_rendering
                ) {
                    equation_preset_dialog = false;
                    modal_view_state = 0;
                    equation_tmp = equationPreset (e, false);
                    hm->eq = equation_tmp;
                    lowres_hm->eq = equation_tmp;
                    if (lowres_hm->generateImage (true)) console_text = "Invalid HFractalEquation input";
                    else {
                        is_outdated_render = true;
                        console_text = "Outdated!";
                        image_needs_update = true;
                    }
                }
            }
            if (GetMouseX() < preset_dialog_x || GetMouseX() > preset_dialog_x + (float)control_panel_width/2 ||  GetMouseY() < preset_dialog_y - BUTTON_HEIGHT || GetMouseY() > preset_dialog_y + (BUTTON_HEIGHT*NUM_EQUATION_PRESETS)) {
                equation_preset_dialog = false;
                modal_view_state = 0;
            }
        }

        // Drawing the info dialog
        if (dialog_text != "") {
            float box_width = (2.0/3.0)*GetScreenWidth();
            DrawRectangle (0, 0, GetScreenWidth(), GetScreenHeight(), (Color){200, 200, 200, 128});

            // bool isHelp = dialog_text == "_HELP";
            // if (isHelp) {
            //     dialog_text = "";
            //     dialog_text += HELP_PARA1;
            //     dialog_text += "\n\n";
            //     dialog_text += HELP_PARA2;
            //     dialog_text += "\n\n";
            //     dialog_text += HELP_PARA3;

            //     float charSize = MeasureText ("A", HELP_TEXT_SIZE);
            //     dialog_text = textWrap (dialog_text, box_width/charSize);
            //     
            //     Rectangle text_rec = (Rectangle){
            //         ((float)GetScreenWidth()-box_width-10)/2, 
            //         ((float)(GetScreenHeight()/2)-10)/2, 
            //         box_width+10, 
            //         (float)(GetScreenHeight()/2)+10
            //     };
            //     DrawRectangleRec (text_rec, RED);
            //     GuiDrawText (dialog_text.c_str(), text_rec, GUI_TEXT_ALIGN_LEFT, BLACK);
            //     dialog_text = "_HELP";
            //} else {
                Rectangle text_rec = (Rectangle){
                    ((float)GetScreenWidth()-box_width-10)/2, 
                    ((float)GetScreenHeight()-DIALOG_TEXT_SIZE-10)/2, 
                    box_width+10, 
                    DIALOG_TEXT_SIZE
                };
                GuiDrawText (dialog_text.c_str(), text_rec, GUI_TEXT_ALIGN_CENTER, BLACK);
            //}

            GuiUnlock();
            bool close = GuiButton((Rectangle){(float)(GetScreenWidth()-box_width-10)/2, (float)(GetScreenHeight()*(3.0/4.0)+10), (float)(box_width+10), (float)(DIALOG_TEXT_SIZE+10)}, "OK");
            if (close) dialog_text = "";
        }

        // Draw coordinates text next to cursor
        if (dialog_text == "" && show_coords && GetMouseX() <= image_dimension && GetMouseY() <= image_dimension) {
            float left = GetMouseX()+15;
            float top = GetMouseY()+15;
            Color col {250, 250, 250, 200};
            
            long double location_x = hm->offset_x + ((long double)(((long double)GetMouseX()/(image_dimension/2))-1))/hm->zoom;
            long double location_y = hm->offset_y - ((long double)(((long double)GetMouseY()/(image_dimension/2))-1))/hm->zoom;
            char t[142];
            sprintf (t, "%.10Lf\n%.10Lf", location_x, location_y);
            DrawRectangle (left, top, 115, 40, col);
            DrawText (t, left+5, top, 15, BLACK);
        }

        EndDrawing();
        GuiUnlock();
    }

    UnloadImage (buffer_image);
    UnloadTexture (buffer_texture);
    CloseWindow();
    return 0;
}