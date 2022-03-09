#include "gui.hh"
#include "guimain.hh"

#include <math.h>
#include <algorithm>
#include <thread>

#include "utils.hh"
#include "database.hh"

using namespace std;

/**
 * @brief Automatically configure the styling for the GUI.
 * Uses a stylesheet provided by raysan5, creator of the graphics library used in the project, raylib
 * 
 */
void HFractalGui::configureStyling() {
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
    // Iterate over string and extract styling properties
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
    // Tell raylib we've finished updating the styling
    GuiUpdateStyleComplete();
}

/**
 * @brief Configure the GUI itself and all class properties
 * 
 */
void HFractalGui::configureGUI() {
    // Basic class initialisation
    dialog_text = "";
    console_text = "Ready.";
    save_name_buffer = "Untitled";
    for (int i = 0; i < BUTTON_NUM_TOTAL; i++) button_states[i] = false;
    buffer_image = {};
    buffer_texture = {};
    is_rendering = false;
    is_outdated_render = true;
    render_percentage = 0;
    showing_coordinates = false;
    modal_view_state = MVS_NORMAL;
    selected_palette = CP_RAINBOW;
    database_load_dialog_scroll = 0;
    textbox_focus = TEXT_FOCUS_STATE::TFS_NONE;

    // Fetch configuration
    unsigned int thread_count = std::thread::hardware_concurrency ();
    long double start_zoom = 1;
    long double start_x_offset = 0;
    long double start_y_offset = 0;
    image_dimension = WINDOW_INIT_HEIGHT;
    control_panel_width = WINDOW_INIT_WIDTH - WINDOW_INIT_HEIGHT;
    equation_buffer = equationPreset (EQ_MANDELBROT, false);

    // GUI configuration
    SetTraceLogLevel (LOG_WARNING | LOG_ERROR | LOG_DEBUG);
    InitWindow(WINDOW_INIT_WIDTH, WINDOW_INIT_HEIGHT, "HyperFractal Mathematical Visualiser");
    SetWindowState (FLAG_WINDOW_RESIZABLE);
    SetExitKey(-1);
    int min_height = std::max (256, CONTROL_MIN_HEIGHT);
    SetWindowMinSize(min_height+CONTROL_MIN_WIDTH, min_height);  
    SetTargetFPS(24);
    configureStyling();

    // Initialise rendering environment
    lowres_hm = new HFractalMain();
    hm = new HFractalMain();

    // Configure full resolution renderer
    hm->setResolution (image_dimension);
    hm->setEquation (equation_buffer);
    hm->setEvalLimit (200);
    hm->setWorkerThreads (thread_count);
    hm->setZoom (start_zoom);
    hm->setOffsetX (start_x_offset);
    hm->setOffsetY (start_y_offset);

    // Configure preivew renderer
    lowres_hm->setResolution (128);
    lowres_hm->setEquation (equation_buffer);
    lowres_hm->setEvalLimit (200);
    lowres_hm->setWorkerThreads (thread_count/2);
    lowres_hm->setZoom (start_zoom);
    lowres_hm->setOffsetX (start_x_offset);
    lowres_hm->setOffsetY (start_y_offset);
}

/**
 * @brief Called when a rendering parameter has been modified.
 * Causes the rendered image to become 'out of date' and updates the preview render
 * 
 */
void HFractalGui::parametersWereModified() {
    is_outdated_render = true;
    console_text = "Outdated render!";
    updatePreviewRender();
}

/**
 * @brief Generate and show an updated image from the preview renderer
 * 
 * @return True if the update was successful, false if the equation was invalid
 */
bool HFractalGui::updatePreviewRender() {
    // Check if the equation is valid
    if (!lowres_hm->isValidEquation()) return false;
    lowres_hm->generateImage(true); // If it is, run a render
    reloadImageFrom(lowres_hm); // And load it
    return true;
}

/**
 * @brief Trigger a full resolution render to start
 * 
 * @return True if successful, false if the equation was invalid
 */
bool HFractalGui::startFullRender() {
    if (!hm->isValidEquation()) { // Check if this is a valid equation
        console_text = "Invalid equation!";
        return false;
    }
    // If it is, start the render
    is_rendering = true;
    console_text = "Rendering...";
    hm->generateImage(false);
    is_outdated_render = true;
    render_percentage = 0;
    return true;
}

/**
 * @brief Check the status of the full resolution render, and produce an up-to-date display image showing the render progress.
 * Also updates the completion percentage
 * 
 * @return True if the image has finished rendering, false otherwise
 */
bool HFractalGui::updateFullRender() {
    if (!is_rendering) return true;
    // Update completion percentage
    render_percentage = round(hm->getImageCompletionPercentage());
    if (hm->getIsRendering()) { // If still rendering, update the rendered image and overlay it onto the preview
        is_outdated_render = true;
        Image overlay = getImage(hm);
        ImageDraw(&buffer_image, overlay, (Rectangle){0,0,(float)hm->getResolution(),(float)hm->getResolution()}, (Rectangle){0,0,(float)hm->getResolution(),(float)hm->getResolution()}, WHITE);
        UnloadImage(overlay);
        tryUnloadTexture();
        buffer_texture = LoadTextureFromImage(buffer_image);
        return false;
    } else { // Otherwise, set states to indicate completion and update the image
        is_outdated_render = false;
        is_rendering = false;
        reloadImageFrom (hm);
        console_text = "Rendering done.";
        return true;
    }
}

/**
 * @brief Reload the image and texture used for drawing to the screen from the specified render environment
 * 
 * @param h Rendering environment to grab the image from
 */
void HFractalGui::reloadImageFrom(HFractalMain* h) {
    tryUnloadImage(); // Unload image and texture
    tryUnloadTexture();
    buffer_image = getImage(h);
    if (buffer_image.height != image_dimension) // Resize it to fill the frame
        ImageResize(&buffer_image, image_dimension, image_dimension);
    buffer_texture = LoadTextureFromImage(buffer_image);
}

/**
 * @brief Check if the window has been resized, and handle it if so
 * 
 */
void HFractalGui::checkWindowResize() {
    if (is_rendering) { // If we're mid-render, snap back to previous window dimensions
        SetWindowSize(image_dimension+control_panel_width, image_dimension);
        return;
    }
    if (IsWindowResized()) { // Update render resolution and image dimension based on new size
        image_dimension = std::min(GetScreenWidth()-CONTROL_MIN_WIDTH, GetScreenHeight());
        control_panel_width = GetScreenWidth()-image_dimension;
        hm->setResolution(image_dimension);
        parametersWereModified(); // Notify that parameters have changed
    }
}

/**
 * @brief Draw the entire interface
 * 
 */
void HFractalGui::drawInterface() {
    BeginDrawing(); // Tell raylib we're about to start drawing

    // Clear the background
    Color bg_col = GetColor (GuiGetStyle(00, BACKGROUND_COLOR));
    ClearBackground(bg_col);

    // Draw the rendered HFractalImage      
    Vector2 v {0,0};
    DrawTextureEx (buffer_texture, v, 0, (float)image_dimension/(float)buffer_texture.height, WHITE);
    // Draw Console
    int button_offset = 0;
    GuiTextBox((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, (char*)console_text.c_str(), 1, false);
    // Draw "Render Image" button
    button_offset++;
    button_states[BUTTON_ID::BUTTON_ID_RENDER] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, "Render Image") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    // Draw render progress bar
    button_offset++;
    GuiProgressBar ((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, "", "", render_percentage, 0, 100);
    // Draw zoom buttons
    button_offset++;
    button_states[BUTTON_ID::BUTTON_ID_ZOOM_IN] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/3, BUTTON_HEIGHT}, "Zoom In") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    button_states[BUTTON_ID::BUTTON_ID_ZOOM_RESET] = GuiButton((Rectangle){(float)image_dimension+(float)control_panel_width/3, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/3, BUTTON_HEIGHT}, "Reset Zoom") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    button_states[BUTTON_ID::BUTTON_ID_ZOOM_OUT] = GuiButton((Rectangle){(float)image_dimension+(float)control_panel_width/(3.0f/2.0f), BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/3, BUTTON_HEIGHT}, "Zoom Out") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    // Draw save image button
    button_offset++;
    button_states[BUTTON_ID::BUTTON_ID_SAVE_IMAGE] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, "Save Image") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    // Draw render state load/save buttons
    button_offset++;
    button_states[BUTTON_ID::BUTTON_ID_SAVE_RSTATE] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/2, BUTTON_HEIGHT}, "Save Render State") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    button_states[BUTTON_ID::BUTTON_ID_LOAD_RSTATE] = GuiButton((Rectangle){(float)image_dimension+(float)control_panel_width/2, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/2, BUTTON_HEIGHT}, "Load Render State") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    // Draw movement navigation buttons
    button_offset++;
    button_states[BUTTON_ID::BUTTON_ID_UP] = GuiButton((Rectangle){(float)image_dimension+((float)control_panel_width-40)/2, BUTTON_HEIGHT*(float)button_offset, (float)40, BUTTON_HEIGHT}, "up") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    button_offset++;
    button_states[BUTTON_ID::BUTTON_ID_LEFT] = GuiButton((Rectangle){(float)image_dimension+(((float)control_panel_width)/2)-40, BUTTON_HEIGHT*(float)button_offset, (float)40, BUTTON_HEIGHT}, "left") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    button_states[BUTTON_ID::BUTTON_ID_RIGHT] = GuiButton((Rectangle){(float)image_dimension+(((float)control_panel_width)/2), BUTTON_HEIGHT*(float)button_offset, (float)40, BUTTON_HEIGHT}, "right") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    button_offset++;
    button_states[BUTTON_ID::BUTTON_ID_DOWN] = GuiButton((Rectangle){(float)image_dimension+((float)control_panel_width-40)/2, BUTTON_HEIGHT*(float)button_offset, (float)40, BUTTON_HEIGHT}, "down") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    button_offset++;
    button_states[BUTTON_ID::BUTTON_ID_EQ_PRESETS] = GuiButton((Rectangle){(float)image_dimension+(float)control_panel_width/2, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/2, BUTTON_HEIGHT}, "Equation Presets") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    
    // Draw equation input box
    button_states[BUTTON_ID::BUTTON_ID_EQ_INPUTBOX] = GuiTextBox ((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/2, BUTTON_HEIGHT}, equation_buffer.data(), 1, false) && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    button_offset++;

    // Coordinate toggle button
    string coord_button_text = "Hide coordinates";
    if (!showing_coordinates) coord_button_text = "Show coordinates";
    button_states[BUTTON_ID::BUTTON_ID_TOGGLE_COORDS] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, coord_button_text.c_str()) && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    button_offset++;

    // Eval limit controls
    button_states[BUTTON_ID::BUTTON_ID_EVAL_LIM_LESS] = GuiButton((Rectangle){(float)image_dimension+(float)control_panel_width/2, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/4, BUTTON_HEIGHT}, "<") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    button_states[BUTTON_ID::BUTTON_ID_EVAL_LIM_MORE] = GuiButton((Rectangle){(float)image_dimension+((float)control_panel_width/4)*3, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/4, BUTTON_HEIGHT}, ">") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);
    char evalLimString[16];
    sprintf (evalLimString, "%d (%d)", hm->getEvalLimit(), lowres_hm->getEvalLimit());
    GuiTextBox ((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width/2, BUTTON_HEIGHT}, evalLimString, 1, false);
    button_offset++;

    // Colour palette preset selector button
    button_states[BUTTON_ID::BUTTON_ID_CP_PRESETS] = GuiButton((Rectangle){(float)image_dimension, BUTTON_HEIGHT*(float)button_offset, (float)control_panel_width, BUTTON_HEIGHT}, "Colour Palettes") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);

    // Draw help button
    button_states[BUTTON_ID::BUTTON_ID_HELP] = GuiButton((Rectangle){(float)image_dimension, (float)GetScreenHeight()-(2*BUTTON_HEIGHT), (float)control_panel_width, BUTTON_HEIGHT*2}, "Help & Instructions") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);

    // Draw the equation preset dialog
    if (modal_view_state == MODAL_VIEW_STATE::MVS_EQUATION_PRESET_SELECTOR) {
        float preset_dialog_x = (float)image_dimension+(float)control_panel_width/2;
        float preset_dialog_y = BUTTON_HEIGHT*10.0f;
        for (int e = 0; e < NUM_EQUATION_PRESETS; e++) {
            // Draw a button for each option
            if (
                GuiButton((Rectangle){preset_dialog_x, preset_dialog_y+(BUTTON_HEIGHT*e), (float)control_panel_width/2, BUTTON_HEIGHT}, equationPreset((EQ_PRESETS)e, true).c_str())
            && !is_rendering
            ) {
                escapeEquationPresetDialog(e);
            }
        }
        if (GetMouseX() < preset_dialog_x || GetMouseX() > preset_dialog_x + (float)control_panel_width/2 ||  GetMouseY() < preset_dialog_y - BUTTON_HEIGHT || GetMouseY() > preset_dialog_y + (BUTTON_HEIGHT*NUM_EQUATION_PRESETS)) {
            escapeEquationPresetDialog(-1);
        }
    }

    // Draw the colour palette preset dialog
    if (modal_view_state == MODAL_VIEW_STATE::MVS_COLOUR_PRESET_SELECTOR) {
        float preset_dialog_x = (float)image_dimension;
        float preset_dialog_y = BUTTON_HEIGHT*13.0f;
        for (int c = 0; c < NUM_COLOUR_PRESETS; c++) {
            // Draw a button for each option
            if (
                GuiButton((Rectangle){preset_dialog_x, preset_dialog_y+(BUTTON_HEIGHT*c), (float)control_panel_width, BUTTON_HEIGHT}, colourPalettePreset((CP_PRESETS)c).c_str())
            && !is_rendering
            ) {
                escapeColourPalettePresetDialog(c);
            }
        }
        if (GetMouseX() < preset_dialog_x || GetMouseX() > preset_dialog_x + (float)control_panel_width ||  GetMouseY() < preset_dialog_y - BUTTON_HEIGHT || GetMouseY() > preset_dialog_y + (BUTTON_HEIGHT*NUM_COLOUR_PRESETS)) {
            escapeColourPalettePresetDialog(-1);
        }
    }

    // Draw the info dialog
    if (modal_view_state == MODAL_VIEW_STATE::MVS_TEXT_DIALOG) {
        float box_width = (2.0/3.0)*GetScreenWidth();
        DrawRectangle (0, 0, GetScreenWidth(), GetScreenHeight(), (Color){200, 200, 200, 128});
        Rectangle text_rec = (Rectangle){
            ((float)GetScreenWidth()-box_width-10)/2, 
            ((float)GetScreenHeight()-DIALOG_TEXT_SIZE-10)/2, 
            box_width+10, 
            DIALOG_TEXT_SIZE
        };
        GuiDrawText (dialog_text.c_str(), text_rec, GUI_TEXT_ALIGN_CENTER, BLACK);
        button_states[BUTTON_ID::BUTTON_ID_TEXT_DIALOG_CLOSE] = GuiButton((Rectangle){(float)(GetScreenWidth()-box_width-10)/2, (float)(GetScreenHeight()*(3.0/4.0)+10), (float)(box_width+10), (float)(DIALOG_TEXT_SIZE+10)}, "OK");
    }

    // Draw database dialog
    if (modal_view_state == MODAL_VIEW_STATE::MVS_DATABASE_SAVE_DIALOG || modal_view_state == MODAL_VIEW_STATE::MVS_DATABASE_LOAD_DIALOG) {
        DrawRectangle (0, 0, GetScreenWidth(), GetScreenHeight(), (Color){200, 200, 200, 128});
        float box_width = (2.0/3.0)*GetScreenWidth();
        button_states[BUTTON_ID::BUTTON_ID_DATABASE_CANCEL] = GuiButton(
            (Rectangle){
                (float)(GetScreenWidth()-box_width-10)/2, 
                (float)(GetScreenHeight()*(4.0/5.0)+10), 
                (float)((box_width+10)/2.0), 
                (float)(DIALOG_TEXT_SIZE+10)
                }, 
        "Cancel");
        
        // Branch depending on whether the saving dialog or the loading dialog is open
        if (modal_view_state == MODAL_VIEW_STATE::MVS_DATABASE_SAVE_DIALOG) {
            button_states[BUTTON_ID::BUTTON_ID_SAVE] = GuiButton(
                (Rectangle){
                    (float)(GetScreenWidth()-10)/2, 
                    (float)(GetScreenHeight()*(4.0/5.0)+10), 
                    (float)((box_width+10)/2.0), 
                    (float)(DIALOG_TEXT_SIZE+10)
                    }, 
            "Save");

            button_states[BUTTON_ID::BUTTON_ID_SAVE_NAME_INPUTBOX] = GuiTextBox (
                (Rectangle){
                    (float)((GetScreenWidth()-box_width)/2.0), 
                    (float)(GetScreenHeight()*(1.0/5.0)), 
                    (float)(box_width), 
                    (float)(BUTTON_HEIGHT*2)
                    }, 
            save_name_buffer.data(), 2, false);

        } else if (modal_view_state == MODAL_VIEW_STATE::MVS_DATABASE_LOAD_DIALOG) {
            button_states[BUTTON_ID::BUTTON_ID_LOAD] = GuiButton(
                (Rectangle){
                    (float)(GetScreenWidth()-10)/2, 
                    (float)(GetScreenHeight()*(4.0/5.0)+10), 
                    (float)((box_width+10)/2.0), 
                    (float)(DIALOG_TEXT_SIZE+10)
                    }, 
            "Load (overwrites current config)");

            button_states[BUTTON_ID::BUTTON_ID_SCROLL_UP] = GuiButton(
                (Rectangle){
                    (float)(GetScreenWidth()/2), 
                    (float)(GetScreenHeight()*(1.0/5.0))+9*BUTTON_HEIGHT, 
                    (float)120, 
                    (float)(BUTTON_HEIGHT)
                    }, 
            "Scroll up");

            button_states[BUTTON_ID::BUTTON_ID_SCROLL_DOWN] = GuiButton(
                (Rectangle){
                    (float)(GetScreenWidth()/2), 
                    (float)(GetScreenHeight()*(1.0/5.0))+10*BUTTON_HEIGHT, 
                    (float)120, 
                    (float)(BUTTON_HEIGHT)
                    }, 
            "Scroll down");

            auto descriptions = database.getConfigDescriptions();
            int row_offset = 0;

            for (auto item : descriptions) {
                int draw_row = row_offset-database_load_dialog_scroll;
                if (draw_row >= 0 && draw_row <= 8) {
                    if (
                        GuiButton(
                    (Rectangle){
                        (float)(GetScreenWidth()-box_width)/2, 
                        (float)(GetScreenHeight()*(1.0/5.0) + BUTTON_HEIGHT*draw_row), 
                        (float)((box_width)-120),
                        (float)(BUTTON_HEIGHT)
                        }, 
                    (((item.first == selected_profile_id) ? "(x)" : "( )") + item.second).c_str())
                    ) {
                        selected_profile_id = item.first;
                    }
                    if (
                        GuiButton(
                    (Rectangle){
                        (float)((GetScreenWidth()+box_width)/2)-120, 
                        (float)(GetScreenHeight()*(1.0/5.0) + BUTTON_HEIGHT*draw_row), 
                        (float)(120),
                        (float)(BUTTON_HEIGHT)
                        }, 
                    "Delete? (!)")
                    ) {
                        database.removeConfig(item.first);
                        database.commit();
                    }
                }
                row_offset++;
            }
        }
    }

    // Draw coordinates text next to cursor
    if (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL && showing_coordinates && GetMouseX() <= image_dimension && GetMouseY() <= image_dimension) {
        float left = GetMouseX()+15;
        float top = GetMouseY()+15;
        Color col {250, 250, 250, 200};
        
        long double location_x = hm->getOffsetX() + ((long double)(((long double)GetMouseX()/(image_dimension/2))-1))/hm->getZoom();
        long double location_y = hm->getOffsetY() - ((long double)(((long double)GetMouseY()/(image_dimension/2))-1))/hm->getZoom();
        char t[142];
        sprintf (t, "%.10Lf\n%.10Lf", location_x, location_y);
        DrawRectangle (left, top, 115, 40, col);
        DrawText (t, left+5, top, 15, BLACK);
    }
    
    EndDrawing(); // Tell raylib we're done drawing
}

/**
 * @brief Close the equation preset dialog, and switch to a given preset
 * 
 * @param e The equation preset to switch to, or -1 if the dialog was cancelled
 */
void HFractalGui::escapeEquationPresetDialog(int e) {
    modal_view_state = MODAL_VIEW_STATE::MVS_NORMAL; // Switch back to normal mode
    if (is_rendering) return;
    if (e != -1) { // If an option was selected, make it the current equation and notify that parameters have changed
        equation_buffer = equationPreset ((EQ_PRESETS)e, false);
        hm->setEquation (equation_buffer);
        lowres_hm->setEquation (equation_buffer);
        // Check whether the equation is valid
        if (!hm->isValidEquation()) console_text = "Invalid equation input";
        else {
            parametersWereModified();
        }
    }
}

/**
 * @brief Show the equation preset dialog
 * 
 */
void HFractalGui::enterEquationPresetDialog() {
    if (is_rendering) return;
    // Switch to equation preset selector mode
    modal_view_state = MODAL_VIEW_STATE::MVS_EQUATION_PRESET_SELECTOR;
}

/**
 * @brief Show the colour palette preset dialog
 * 
 */
void HFractalGui::enterColourPalettePresetDialog() {
    if (is_rendering) return;
    // Switch to colour preset selector mode
    modal_view_state = MODAL_VIEW_STATE::MVS_COLOUR_PRESET_SELECTOR;
}

/**
 * @brief Close the colour palette preset dialog and switch to a given palette
 * 
 * @param c Palette to switch to, or -1 if the dialog was cancelled
 */
void HFractalGui::escapeColourPalettePresetDialog(int c) {
    modal_view_state = MODAL_VIEW_STATE::MVS_NORMAL; // Return to normal GUI mode
    if (is_rendering) return;
    if (c != -1) {
        // If an option was selected, reload the image with the selected palette (no rerender necessarry)
        selected_palette = (CP_PRESETS)c;
        if (is_outdated_render) {
            reloadImageFrom(lowres_hm);
        } else {
            reloadImageFrom(hm);
        }
    }
}

/**
 * @brief Get an image handleable by raylib from a given rendering environment
 * 
 * @param h Rendering environment to extract from
 * @return A raylib-style image for drawing into the GUI
 */
Image HFractalGui::getImage(HFractalMain* h) {
    // Fetch a 32 bit RGBA image in the selected colour palette
    int size = h->getResolution();
    uint32_t *data = h->getRGBAImage(selected_palette);
    Color *pixels = (Color *)malloc (size*size*sizeof(Color));
    // Convert the image data to a format raylib will accept
    for (int i = 0; i < size*size; i++) pixels[i] = GetColor(data[i]);
    // Construct a raylib image from the data
    Image img = {
        .data = pixels,
        .width = size,
        .height = size,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    return img;
}

/**
 * @brief Handle when the user clicks on the image.
 * Automatically centres the area they clicked and notifies the GUI that rendering parameters have been modified, triggering a preview update
 * 
 * @return True if a click was handled, otherwise false
 */
bool HFractalGui::handleClickNavigation() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !is_rendering) {
        Vector2 mpos = GetMousePosition();
        // Check if the mouse click was inside the image
        if (mpos.x <= image_dimension && mpos.y <= image_dimension) {
            long double change_in_x = (long double)((mpos.x / (image_dimension / 2)) - 1) / hm->getZoom();
            long double change_in_y = (long double)((mpos.y / (image_dimension / 2)) - 1) / hm->getZoom();
            long double new_offset_x = hm->getOffsetX() + change_in_x;
            long double new_offset_y = hm->getOffsetY() - change_in_y;
            // Update parameters and notify of the modification
            lowres_hm->setOffsetX(new_offset_x);
            lowres_hm->setOffsetY(new_offset_y);
            hm->setOffsetX(new_offset_x);
            hm->setOffsetY(new_offset_y);
            parametersWereModified();
            return true;
        }
    }
    return false;
}

/**
 * @brief Show a text dialog with a given string as text
 * 
 * @param text Text to display
 */
void HFractalGui::launchTextDialog(std::string text) {
    modal_view_state = MODAL_VIEW_STATE::MVS_TEXT_DIALOG;
    dialog_text = text;
}

/**
 * @brief Close the currently open text dialog and go back to normal GUI mode
 * 
 */
void HFractalGui::closeTextDialog() {
    modal_view_state = MODAL_VIEW_STATE::MVS_NORMAL;
    dialog_text = "";
}

/**
 * @brief Handler for Zoom In button
 * 
 */
void HFractalGui::zoomIn() {
    if (hm->getZoom() <= SCALE_DEPTH_LIMIT) { // Check the zoom has not exceeded the depth limit 
        long double new_zoom = hm->getZoom() * SCALE_STEP_FACTOR;
        lowres_hm->setZoom (new_zoom);
        hm->setZoom (new_zoom);
        parametersWereModified();
    } else launchTextDialog ("Zoom precision limit reached"); // Present a text dialog to report the issue to the user
}

/**
 * @brief Handler for Zoom Out button
 * 
 */
void HFractalGui::zoomOut() {
    long double new_zoom = hm->getZoom() / SCALE_STEP_FACTOR;
    lowres_hm->setZoom (new_zoom);
    hm->setZoom (new_zoom);
    parametersWereModified();
}

/**
 * @brief Handler for Reset Zoom button
 * 
 */
void HFractalGui::resetZoom() {
    lowres_hm->setZoom(1);
    hm->setZoom(1);
    parametersWereModified();
}

/**
 * @brief Handler for Save Image button
 * 
 */
void HFractalGui::saveImage() {
    bool result = false;
    // Switch depending on whether there is a full render available to save
    if (is_outdated_render) {
        result = lowres_hm->autoWriteImage(IMAGE_TYPE::PGM);
        console_text = "Saved preview render to desktop.";
    } else {
        result = hm->autoWriteImage (IMAGE_TYPE::PGM);
        console_text = "Saved render to desktop.";
    }
    if (!result) {
        console_text = "Image saving failed.";
    }
}

/**
 * @brief Make the save render state dialog visible
 * 
 */
void HFractalGui::showSaveStateDialog() {
    if (is_rendering) return;
    modal_view_state = MODAL_VIEW_STATE::MVS_DATABASE_SAVE_DIALOG;
}

/**
 * @brief Make the load render state dialog visible
 * 
 */
void HFractalGui::showLoadStateDialog() {
    if (is_rendering) return;
    modal_view_state = MODAL_VIEW_STATE::MVS_DATABASE_LOAD_DIALOG;
    database_load_dialog_scroll = 0;
}

/**
 * @brief Save the current render state to the database and close the dialog
 * 
 */
void HFractalGui::saveStateToDatabase() {
    // Create the new config profile and populate its fields
    HFractalConfigProfile *cp = new HFractalConfigProfile();
    cp->equation = hm->getEquation();
    cp->iterations = hm->getEvalLimit();
    cp->name = save_name_buffer;
    cp->palette = selected_palette;
    cp->x_offset = hm->getOffsetX();
    cp->y_offset = hm->getOffsetY();
    cp->zoom = hm->getZoom();
    
    // Fetch or create the default user if necessarry
    HFractalUserProfile *default_user = database.getUser(0);
    if (default_user == NULL) {
        default_user = new HFractalUserProfile();
        default_user->user_name = "default";
        database.insertUser (default_user);
    }

    cp->user_id = default_user->user_id;

    // Insert the new profile into the database
    database.insertConfig(cp);
    database.commit();
    console_text = "Profile saved to database!";
    closeDatabaseDialog(); // Escape from the dialog
}

/**
 * @brief Load the selected render state from the database and close the dialog
 * 
 */
void HFractalGui::loadStateFromDatabase() {
    // Try to fetch the config profile
    HFractalConfigProfile *cp = database.getConfig (selected_profile_id);
    if (cp == NULL) {
        console_text = "No profile selected to load.";
    } else { // On success, load all properties into the rendering environments
        hm->setEquation(cp->equation);
        lowres_hm->setEquation(cp->equation);

        hm->setEvalLimit(cp->iterations);
        lowres_hm->setEvalLimit(cp->iterations);

        hm->setOffsetX(cp->x_offset);
        lowres_hm->setOffsetX(cp->x_offset);

        hm->setOffsetY(cp->y_offset);
        lowres_hm->setOffsetY(cp->y_offset);
        
        hm->setZoom(cp->zoom);
        lowres_hm->setZoom(cp->zoom);

        selected_palette = (CP_PRESETS)cp->palette;
        save_name_buffer = cp->name;

        updatePreviewRender();
        console_text = "Profile '" + save_name_buffer + "' loaded from database.";
    }
    closeDatabaseDialog(); // Close the dialog
}

/**
 * @brief Hide the database dialog and return to normal GUI mode
 * 
 */
void HFractalGui::closeDatabaseDialog() {
    modal_view_state = MODAL_VIEW_STATE::MVS_NORMAL;
}

/**
 * @brief Scroll down inside the load render state dialog
 * 
 */
void HFractalGui::databaseLoadScrollDown() {
    database_load_dialog_scroll++;
}

/**
 * @brief Scroll up inside the load render state dialog
 * 
 */
void HFractalGui::databaseLoadScrollUp() {
    database_load_dialog_scroll--;
    if (database_load_dialog_scroll < 0) database_load_dialog_scroll = 0;
}

/**
 * @brief Handler for Move Up button
 * 
 */
void HFractalGui::moveUp() {
    long double new_offset = hm->getOffsetY() + (MOVE_STEP_FACTOR/hm->getZoom());
    hm->setOffsetY (new_offset);
    lowres_hm->setOffsetY (new_offset);
    parametersWereModified();
}

/**
 * @brief Handler for Move Left button
 * 
 */
void HFractalGui::moveLeft() {
    long double new_offset = hm->getOffsetX() - (MOVE_STEP_FACTOR/hm->getZoom());
    hm->setOffsetX (new_offset);
    lowres_hm->setOffsetX (new_offset);
    parametersWereModified();
}

/**
 * @brief Handler for Move Right button
 * 
 */
void HFractalGui::moveRight() {
    long double new_offset = hm->getOffsetX() + (MOVE_STEP_FACTOR/hm->getZoom());
    hm->setOffsetX (new_offset);
    lowres_hm->setOffsetX (new_offset);
    parametersWereModified();
}

/**
 * @brief Handler for Move Down button
 * 
 */
void HFractalGui::moveDown() {
    long double new_offset = hm->getOffsetY() - (MOVE_STEP_FACTOR/hm->getZoom());
    hm->setOffsetY (new_offset);
    lowres_hm->setOffsetY (new_offset);
    parametersWereModified();
}

/**
 * @brief Handler for Show/Hide Coordinates button
 * 
 */
void HFractalGui::toggleCoords() {
    showing_coordinates = !showing_coordinates;
}

/**
 * @brief Handlder for '<' button
 * 
 */
void HFractalGui::evalLimitLess() {
    int new_el = hm->getEvalLimit();
    // Allow faster jumping if shift is held
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT)) {
        new_el -= 10;
    } else {
        new_el--;
    }
    hm->setEvalLimit (new_el);
    lowres_hm->setEvalLimit (new_el);
    parametersWereModified();
}

/**
 * @brief Handler for '>' button
 * 
 */
void HFractalGui::evalLimitMore() {
    int new_el = hm->getEvalLimit();
    // Allow faster jumping if shift is held
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT)) {
        new_el += 10;
    } else {
        new_el++;
    }
    hm->setEvalLimit (new_el);
    lowres_hm->setEvalLimit (new_el);
    parametersWereModified();
}

/**
 * @brief Handler for Help & Instructions button
 * 
 */
void HFractalGui::showHelp() {
    // Open the help page in the repository, cross-platform
    #ifdef _WIN32
        system("explorer https://github.com/JkyProgrammer/HyperFractal/blob/main/README.md#help--instructions");
    #else
        system("open https://github.com/JkyProgrammer/HyperFractal/blob/main/README.md#help--instructions");
    #endif
}

/**
 * @brief Handle the user pressing a GUI button
 * 
 * @return True if a button press was handled, otherwise false
 */
bool HFractalGui::handleButtonPresses() {
    if (is_rendering) return false;
    // Branch to different handling modes depending on the dialog state, allowing certain sets of buttons to be disabled when dialogs are open
    if (modal_view_state == MODAL_VIEW_STATE::MVS_TEXT_DIALOG) {
        if (button_states[BUTTON_ID::BUTTON_ID_TEXT_DIALOG_CLOSE]) { closeTextDialog(); return true; }
    } else if (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL) {
        if (button_states[BUTTON_ID::BUTTON_ID_RENDER]) { startFullRender(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_ZOOM_IN]) { zoomIn(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_ZOOM_OUT]) { zoomOut(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_SAVE_IMAGE]) { saveImage(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_SAVE_RSTATE]) { showSaveStateDialog(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_LOAD_RSTATE]) { showLoadStateDialog(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_UP]) { moveUp(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_LEFT]) { moveLeft(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_RIGHT]) { moveRight(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_DOWN]) { moveDown(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_EQ_PRESETS]) { enterEquationPresetDialog(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_ZOOM_RESET]) { resetZoom(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_TOGGLE_COORDS]) { toggleCoords(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_EVAL_LIM_LESS]) { evalLimitLess(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_EVAL_LIM_MORE]) { evalLimitMore(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_HELP]) { showHelp(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_EQ_INPUTBOX]) { textbox_focus = TEXT_FOCUS_STATE::TFS_EQUATION; return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_CP_PRESETS]) { enterColourPalettePresetDialog(); return true; }
    } else if (modal_view_state == MODAL_VIEW_STATE::MVS_DATABASE_SAVE_DIALOG) {
        if (button_states[BUTTON_ID::BUTTON_ID_SAVE_NAME_INPUTBOX]) { textbox_focus = TEXT_FOCUS_STATE::TFS_SAVE_NAME; return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_SAVE]) { saveStateToDatabase(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_DATABASE_CANCEL]) { closeDatabaseDialog(); return true; }
    } else if (modal_view_state == MODAL_VIEW_STATE::MVS_DATABASE_LOAD_DIALOG) {
        if (button_states[BUTTON_ID::BUTTON_ID_LOAD]) { loadStateFromDatabase(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_SCROLL_DOWN]) { databaseLoadScrollDown(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_SCROLL_UP]) { databaseLoadScrollUp(); return true; }
        if (button_states[BUTTON_ID::BUTTON_ID_DATABASE_CANCEL]) { closeDatabaseDialog(); return true; }
    }

    return false;
}

/**
 * @brief Clear the contents of the button states array to prevent unhandled button presses hanging over to the next update
 * 
 */
void HFractalGui::clearButtonStates() {
    for (int i = 0; i < BUTTON_NUM_TOTAL; i++) {
        button_states[i] = false;
    }
}

/**
 * @brief Unload the image buffer to prevent memory leaks
 * 
 */
void HFractalGui::tryUnloadImage() {
    UnloadImage (buffer_image);
}

/**
 * @brief Unload the texture buffer to prevent memory leaks
 * 
 */
void HFractalGui::tryUnloadTexture() {
    UnloadTexture (buffer_texture);
}

/**
 * @brief Handle when the user presses a key
 * 
 * @return True if a key press was handled, false otherwise 
 */
bool HFractalGui::handleKeyPresses() {
    // Escape currently editing text box when escape is pressed
    if (IsKeyDown(KEY_ESCAPE)) { textbox_focus = TEXT_FOCUS_STATE::TFS_NONE; return true; }

    // Handle keys depending on which text box is focussed (if none, use them for navigation)
    if (textbox_focus == TEXT_FOCUS_STATE::TFS_NONE) {
        for (auto key : key_map) {
            if (IsKeyDown (key.first)) {
                button_states[key.second] = true;
                return true;
            }
        }
    } else if (textbox_focus == TEXT_FOCUS_STATE::TFS_EQUATION) {
        if (IsKeyDown(KEY_ENTER)) { button_states[BUTTON_ID::BUTTON_ID_RENDER] = true; return true; }
        int key = GetCharPressed();
        if ((((int)'a' <= key && key <= (int)'c') || ((int)'x' <= key && key <= (int)'z') || key == 122 || (key >= 48 && key <= 57) || key == 94 || (key >= 40 && key <= 43) || key == 45 || key == 46 || key == 47 || key == 'i') && !is_rendering) {
            equation_buffer += (char)key;
            hm->setEquation (equation_buffer);
            lowres_hm->setEquation (equation_buffer);
            if (!hm->isValidEquation()) console_text = "Invalid equation input";
            else parametersWereModified();
        } else if (GetKeyPressed () == KEY_BACKSPACE && !is_rendering && equation_buffer.length() > 0) {
            equation_buffer.pop_back();
            hm->setEquation(equation_buffer);
            lowres_hm->setEquation(equation_buffer);
            if (!hm->isValidEquation()) console_text = "Invalid equation input";
            else parametersWereModified();
        }
    } else if (textbox_focus == TEXT_FOCUS_STATE::TFS_SAVE_NAME) {
        int key = GetCharPressed();
        if (((int)'a' <= key && key <= (int)'z') || ((int)'A' <= key && key <= (int)'Z')) {
            save_name_buffer += (char)key;
        } else if (GetKeyPressed() == KEY_BACKSPACE) {
            if (save_name_buffer.length() > 0) save_name_buffer.pop_back();
        }
    }
    return false;
}

/**
 * @brief Start the GUI and run the mainloop.
 * Blocks on current thread
 * 
 * @return Integer showing exit status 
 */
int HFractalGui::guiMain() {
    // Run the setup code
    configureGUI();
    parametersWereModified();
    while(!WindowShouldClose()) { // Loop until the application closes
        checkWindowResize();
        if (!is_rendering && modal_view_state == MVS_NORMAL) {
            bool click_handled = handleClickNavigation();
            // Defocus the textbox if a click is handled somewhere
            if (click_handled) { textbox_focus = TEXT_FOCUS_STATE::TFS_NONE; }
        }
        handleKeyPresses();
        bool button_pressed = handleButtonPresses();
        // Defocus the textbox if a button press is handled
        if (button_pressed && !button_states[BUTTON_ID::BUTTON_ID_EQ_INPUTBOX] && !button_states[BUTTON_ID::BUTTON_ID_SAVE_NAME_INPUTBOX]) { textbox_focus = TEXT_FOCUS_STATE::TFS_NONE; }
        clearButtonStates();
        // If a render is in progress, update the status of it
        if (is_rendering) updateFullRender();
        // Finally, draw everything
        drawInterface();
    }
    
    // Release resources and close
    tryUnloadImage();
    tryUnloadTexture();
    CloseWindow();
    return 0;
}

/**
 * @brief Construct a new GUI object
 * 
 */
HFractalGui::HFractalGui() {}

/**
 * @brief Method to start the GUI, isolates the GUI module from the main module to prevent linker conflicts with raylib
 * 
 * @return Integer showing exit status 
 */
int guiMain () {
    HFractalGui gui = HFractalGui ();
    int res = gui.guiMain();
    return res;
}