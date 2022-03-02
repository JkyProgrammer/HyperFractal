#include "gui.hh"
#include "guimain.hh"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <thread>
#include "utils.hh"
#include <cassert>

using namespace std;

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

void HFractalGui::configureGUI() {
    // Basic class initialisation
    dialog_text = "";
    console_text = "Ready.";
    for (int i = 0; i < BUTTON_NUM_TOTAL; i++) button_states[i] = false;
    buffer_image = {};
    buffer_texture = {};
    is_rendering = false;
    is_outdated_render = true;
    render_percentage = 0;
    showing_coordinates = false;
    modal_view_state = MVS_NORMAL;
    selected_palette = 0;
    is_textbox_focussed = false;

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
    SetTargetFPS(30);
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

void HFractalGui::parametersWereModified() {
    is_outdated_render = true;
    console_text = "Outdated render!";
    updatePreviewRender();
}

bool HFractalGui::updatePreviewRender() {
    if (!lowres_hm->isValidEquation()) return false;
    lowres_hm->generateImage(true);
    tryUnloadImage();
    tryUnloadTexture();
    buffer_image = getImage(lowres_hm);
    ImageResize(&buffer_image, image_dimension, image_dimension);
    buffer_texture = LoadTextureFromImage(buffer_image);
    return true;
}

bool HFractalGui::startFullRender() {
    if (!hm->isValidEquation()) {
        console_text = "Invalid equation!";
        return false;
    }
    is_rendering = true;
    console_text = "Rendering...";
    hm->generateImage(false);
    is_outdated_render = true;
    render_percentage = 0;
    return true;
}

bool HFractalGui::updateFullRender() {
    if (!is_rendering) return true;
    render_percentage = round(hm->getImageCompletionPercentage());
    if (hm->getIsRendering()) {
        is_outdated_render = true;
        Image overlay = getImage(hm);
        ImageDraw(&buffer_image, overlay, (Rectangle){0,0,(float)hm->getResolution(),(float)hm->getResolution()}, (Rectangle){0,0,(float)hm->getResolution(),(float)hm->getResolution()}, WHITE);
        UnloadImage(overlay);
        tryUnloadTexture();
        buffer_texture = LoadTextureFromImage(buffer_image);
        return false;
    } else {
        is_outdated_render = false;
        is_rendering = false;
        tryUnloadImage();
        tryUnloadTexture();
        buffer_image = getImage(hm);
        buffer_texture = LoadTextureFromImage(buffer_image);
        console_text = "Rendering done.";
        return true;
    }
}

void HFractalGui::checkWindowResize() {
    if (is_rendering) {
        SetWindowSize(image_dimension+control_panel_width, image_dimension);
        return;
    }
    if (IsWindowResized()) {
        image_dimension = std::min(GetScreenWidth()-CONTROL_MIN_WIDTH, GetScreenHeight());
        control_panel_width = GetScreenWidth()-image_dimension;
        hm->setResolution(image_dimension);
        parametersWereModified();
    }
}

void HFractalGui::drawInterface() {
    BeginDrawing();
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
        
    button_states[BUTTON_ID::BUTTON_ID_HELP] = GuiButton((Rectangle){(float)image_dimension, (float)GetScreenHeight()-(2*BUTTON_HEIGHT), (float)control_panel_width, BUTTON_HEIGHT*2}, "Help & Instructions") && (modal_view_state == MODAL_VIEW_STATE::MVS_NORMAL);

    // Draw the equation preset dialog
    if (modal_view_state == MODAL_VIEW_STATE::MVS_EQUATION_PRESET_SELECTOR) {
        float preset_dialog_x = (float)image_dimension+(float)control_panel_width/2;
        float preset_dialog_y = BUTTON_HEIGHT*10.0f;
        for (int e = 1; e <= NUM_EQUATION_PRESETS; e++) {
            if (
                GuiButton((Rectangle){preset_dialog_x, preset_dialog_y+(BUTTON_HEIGHT*(e-1)), (float)control_panel_width/2, BUTTON_HEIGHT}, equationPreset(e, true).c_str())
            && !is_rendering
            ) {
                escapeEquationPresetDialog(e);
            }
        }
        if (GetMouseX() < preset_dialog_x || GetMouseX() > preset_dialog_x + (float)control_panel_width/2 ||  GetMouseY() < preset_dialog_y - BUTTON_HEIGHT || GetMouseY() > preset_dialog_y + (BUTTON_HEIGHT*NUM_EQUATION_PRESETS)) {
            escapeEquationPresetDialog(-1);
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
    
    EndDrawing();
}

void HFractalGui::escapeEquationPresetDialog(int e) {
    modal_view_state = MODAL_VIEW_STATE::MVS_NORMAL;
    if (e != -1) {
        equation_buffer = equationPreset (e, false);
        hm->setEquation (equation_buffer);
        lowres_hm->setEquation (equation_buffer);
        if (!hm->isValidEquation()) console_text = "Invalid equation input";
        else {
            parametersWereModified();
        }
    }
}

void HFractalGui::enterEquationPresetDialog() {
    if (is_rendering) return;
    modal_view_state = MODAL_VIEW_STATE::MVS_EQUATION_PRESET_SELECTOR;
}

Image HFractalGui::getImage(HFractalMain* h) {
    int size = h->getResolution();
    uint32_t *data = h->getRGBAImage(selected_palette);
    Color *pixels = (Color *)malloc (size*size*sizeof(Color));
    for (int i = 0; i < size*size; i++)
        pixels[i] = (Color) {(unsigned char)((data[i] & 0xff000000) >> (8*3)),
                             (unsigned char)((data[i] & 0x00ff0000) >> (8*2)),
                             (unsigned char)((data[i] & 0x0000ff00) >> (8*1)),
                             (unsigned char)(data[i] & 0x000000ff)};
    Image img = {
        .data = pixels,
        .width = size,
        .height = size,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    return img;
}

bool HFractalGui::handleClickNavigation() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !is_rendering) {
        Vector2 mpos = GetMousePosition();
        if (mpos.x <= image_dimension && mpos.y <= image_dimension) {
            long double change_in_x = (long double)((mpos.x / (image_dimension / 2)) - 1) / hm->getZoom();
            long double change_in_y = (long double)((mpos.y / (image_dimension / 2)) - 1) / hm->getZoom();
            long double new_offset_x = hm->getOffsetX() + change_in_x;
            long double new_offset_y = hm->getOffsetY() - change_in_y;
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

void HFractalGui::launchTextDialog(std::string text) {
    modal_view_state = MODAL_VIEW_STATE::MVS_TEXT_DIALOG;
    dialog_text = text;
}

void HFractalGui::closeTextDialog() {
    modal_view_state = MODAL_VIEW_STATE::MVS_NORMAL;
    dialog_text = "";
}

void HFractalGui::zoomIn() {
    if (hm->getZoom() <= SCALE_DEPTH_LIMIT) {
        long double new_zoom = hm->getZoom() * SCALE_STEP_FACTOR;
        lowres_hm->setZoom (new_zoom);
        hm->setZoom (new_zoom);
        parametersWereModified();
    } else launchTextDialog ("Zoom precision limit reached");
}

void HFractalGui::zoomOut() {
    long double new_zoom = hm->getZoom() / SCALE_STEP_FACTOR;
    lowres_hm->setZoom (new_zoom);
    hm->setZoom (new_zoom);
    parametersWereModified();
}

void HFractalGui::resetZoom() {
    lowres_hm->setZoom(1);
    hm->setZoom(1);
    parametersWereModified();
}

void HFractalGui::saveImage() {
    bool result = false;
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

void HFractalGui::showSaveStateDialog() {
    assert (false);
}

void HFractalGui::showLoadStateDialog() {
    assert (false);
}

void HFractalGui::moveUp() {
    long double new_offset = hm->getOffsetY() + (MOVE_STEP_FACTOR/hm->getZoom());
    hm->setOffsetY (new_offset);
    lowres_hm->setOffsetY (new_offset);
    parametersWereModified();
}

void HFractalGui::moveLeft() {
    long double new_offset = hm->getOffsetX() - (MOVE_STEP_FACTOR/hm->getZoom());
    hm->setOffsetX (new_offset);
    lowres_hm->setOffsetX (new_offset);
    parametersWereModified();
}

void HFractalGui::moveRight() {
    long double new_offset = hm->getOffsetX() + (MOVE_STEP_FACTOR/hm->getZoom());
    hm->setOffsetX (new_offset);
    lowres_hm->setOffsetX (new_offset);
    parametersWereModified();
}

void HFractalGui::moveDown() {
    long double new_offset = hm->getOffsetY() - (MOVE_STEP_FACTOR/hm->getZoom());
    hm->setOffsetY (new_offset);
    lowres_hm->setOffsetY (new_offset);
    parametersWereModified();
}

void HFractalGui::toggleCoords() {
    showing_coordinates = !showing_coordinates;
    assert (false);
}

void HFractalGui::evalLimitLess() {
    int new_el = hm->getEvalLimit();
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT)) {
        new_el -= 10;
    } else {
        new_el--;
    }
    hm->setEvalLimit (new_el);
    lowres_hm->setEvalLimit (new_el);
    parametersWereModified();
}

void HFractalGui::evalLimitMore() {
    int new_el = hm->getEvalLimit();
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown (KEY_RIGHT_SHIFT)) {
        new_el += 10;
    } else {
        new_el++;
    }
    hm->setEvalLimit (new_el);
    lowres_hm->setEvalLimit (new_el);
    parametersWereModified();
}

void HFractalGui::showHelp() {
    system ("open https://github.com/JkyProgrammer/HyperFractal/blob/main/README.md");
}

bool HFractalGui::handleButtonPresses() {
    if (is_rendering) return false;
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
        if (button_states[BUTTON_ID::BUTTON_ID_EQ_INPUTBOX]) { is_textbox_focussed = true; return true; }
    }

    return false;
}

void HFractalGui::clearButtonStates() {
    for (int i = 0; i < BUTTON_NUM_TOTAL; i++) {
        button_states[i] = false;
    }
}

void HFractalGui::tryUnloadImage() {
    UnloadImage (buffer_image);
}

void HFractalGui::tryUnloadTexture() {
    UnloadTexture (buffer_texture);
}

bool HFractalGui::handleKeyPresses() {
    if (IsKeyDown(KEY_ESCAPE)) { is_textbox_focussed = false; return true; }
    if (!is_textbox_focussed) {
        for (auto key : key_map) {
            if (IsKeyDown (key.first)) {
                button_states[key.second] = true;
                return true;
            }
        }
    } else {
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
    }
    return false;
}

int HFractalGui::guiMain() {
    configureGUI();
    parametersWereModified();
    while(!WindowShouldClose()) {
        checkWindowResize();
        if (!is_rendering && modal_view_state == MVS_NORMAL) {
            bool click_handled = handleClickNavigation();
            if (click_handled) { is_textbox_focussed = false; }
        }
        handleKeyPresses();
        bool button_pressed = handleButtonPresses();
        if (button_pressed && !button_states[BUTTON_ID::BUTTON_ID_EQ_INPUTBOX]) { is_textbox_focussed = false; }
        clearButtonStates();
        if (is_rendering) updateFullRender();
        drawInterface();
    }
    tryUnloadImage();
    tryUnloadTexture();
    CloseWindow();
    return 0;
}

HFractalGui::HFractalGui() {}

int guiMain () {
    HFractalGui gui = HFractalGui ();
    int res = gui.guiMain();
    return res;
}

