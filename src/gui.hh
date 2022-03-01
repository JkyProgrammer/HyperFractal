#ifndef GUI_H
#define GUI_H

#include "hyperfractal.hh"
#include <map>

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
#define BUTTON_NUM_TOTAL 18         // Total number of buttons in the interface
#define CONTROL_MIN_WIDTH 400       // Minimum width of the control panel
#define CONTROL_MIN_HEIGHT BUTTON_HEIGHT*ELEMENT_NUM_VERTICAL
#define DIALOG_TEXT_SIZE 25
#define HELP_TEXT_SIZE 15

enum BUTTON_ID {
    BUTTON_ID_RENDER = 0,
    BUTTON_ID_ZOOM_IN,
    BUTTON_ID_ZOOM_OUT,
    BUTTON_ID_SAVE_IMAGE,
    BUTTON_ID_SAVE_RSTATE,
    BUTTON_ID_LOAD_RSTATE,
    BUTTON_ID_UP,
    BUTTON_ID_LEFT,
    BUTTON_ID_RIGHT,
    BUTTON_ID_DOWN,
    BUTTON_ID_EQ_PRESETS,
    BUTTON_ID_ZOOM_RESET,
    BUTTON_ID_TOGGLE_COORDS,
    BUTTON_ID_EVAL_LIM_LESS,
    BUTTON_ID_EVAL_LIM_MORE,
    BUTTON_ID_HELP,
    BUTTON_ID_TEXT_DIALOG_CLOSE,
    BUTTON_ID_EQ_INPUTBOX
};

enum MODAL_VIEW_STATE {
    MVS_NORMAL,
    MVS_TEXT_DIALOG,
    MVS_DATABASE_DIALOG,
    MVS_EQUATION_PRESET_SELECTOR,
    MVS_COLOUR_PRESET_SELECTOR
};

class HFractalGui {
private:
    std::map<KeyboardKey, BUTTON_ID> key_map = {
        {KEY_ENTER, BUTTON_ID::BUTTON_ID_RENDER},
        {KEY_EQUAL, BUTTON_ID::BUTTON_ID_ZOOM_IN},
        {KEY_MINUS, BUTTON_ID::BUTTON_ID_ZOOM_OUT},
        {KEY_UP, BUTTON_ID::BUTTON_ID_UP},
        {KEY_DOWN, BUTTON_ID::BUTTON_ID_DOWN},
        {KEY_LEFT, BUTTON_ID::BUTTON_ID_LEFT},
        {KEY_RIGHT, BUTTON_ID::BUTTON_ID_RIGHT},
        {KEY_LEFT_BRACKET, BUTTON_ID::BUTTON_ID_EVAL_LIM_LESS},
        {KEY_RIGHT_BRACKET, BUTTON_ID::BUTTON_ID_EVAL_LIM_MORE}
    };

    HFractalMain* lowres_hm;
    HFractalMain* hm;

    std::string dialog_text;
    std::string console_text;
    std::string equation_buffer;
    bool button_states[BUTTON_NUM_TOTAL];
    Image buffer_image;
    Texture2D buffer_texture;
    bool is_rendering;
    bool is_outdated_render;
    bool is_textbox_focussed;
    int render_percentage;
    bool showing_coordinates;
    MODAL_VIEW_STATE modal_view_state;
    int image_dimension;
    int control_panel_width;
    int selected_palette;

    void configureStyling();
    void configureGUI();

    void parametersWereModified();
    bool updatePreviewRender();
    bool startFullRender();
    bool updateFullRender();

    void checkWindowResize();

    bool handleClickNavigation();
    bool handleButtonPresses();
    bool handleKeyPresses();
    void drawInterface();

    Image getImage(HFractalMain*);
    void escapeEquationPresetDialog(int);
    void enterEquationPresetDialog();
    void launchTextDialog(std::string);
    void closeTextDialog();

    void zoomIn();
    void zoomOut();
    void resetZoom();
    void saveImage();

    void showSaveStateDialog();
    void showLoadStateDialog();

    void moveUp();
    void moveLeft();
    void moveRight();
    void moveDown();

    void toggleCoords();

    void evalLimitLess();
    void evalLimitMore();

    void showHelp();
    void clearButtonStates();

    void tryUnloadImage();
    void tryUnloadTexture();
public:
    int guiMain();

    HFractalGui();
};
#endif