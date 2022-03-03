#ifndef GUI_H
#define GUI_H

#include <map>

#define RAYGUI_IMPLEMENTATION
#define RAYGUI_SUPPORT_ICONS
#include "../lib/raygui.h"
#include "../lib/ricons.h"

#include "hyperfractal.hh"
#include "utils.hh"
#include "database.hh"

#define SCALE_STEP_FACTOR 1.5       // Factor by which scaling changes
#define SCALE_DEPTH_LIMIT 1.0e15    // Limit to prevent user from going too deep due to limited precision
#define MOVE_STEP_FACTOR 0.1        // Factor by which position changes
#define WINDOW_INIT_WIDTH 900       // Initial window - width
#define WINDOW_INIT_HEIGHT 550      //                - height
#define BUTTON_HEIGHT 30            // Height of a single button in the interface
#define ELEMENT_NUM_VERTICAL 15     // Number of vertical elements
#define BUTTON_NUM_TOTAL 25         // Total number of buttons in the interface
#define CONTROL_MIN_WIDTH 400       // Minimum width of the control panel
#define CONTROL_MIN_HEIGHT BUTTON_HEIGHT*ELEMENT_NUM_VERTICAL // Minimum height of the panel
#define DIALOG_TEXT_SIZE 25         // Size of text in dialog windows

// Enum listing button IDs to abstract and make code clearer
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
    BUTTON_ID_EQ_INPUTBOX,
    BUTTON_ID_CP_PRESETS,
    BUTTON_ID_SAVE_NAME_INPUTBOX,
    BUTTON_ID_SAVE,
    BUTTON_ID_LOAD,
    BUTTON_ID_SCROLL_DOWN,
    BUTTON_ID_SCROLL_UP,
    BUTTON_ID_DATABASE_CANCEL
};

// Enum listing GUI states for cases when a dialog or modal is open (i.e. to disable certain interface elements)
enum MODAL_VIEW_STATE {
    MVS_NORMAL,
    MVS_TEXT_DIALOG,
    MVS_DATABASE_SAVE_DIALOG,
    MVS_DATABASE_LOAD_DIALOG,
    MVS_EQUATION_PRESET_SELECTOR,
    MVS_COLOUR_PRESET_SELECTOR
};

// Enum listing text focus states to enable/disable input to specific fields
enum TEXT_FOCUS_STATE {
    TFS_NONE,
    TFS_EQUATION,
    TFS_SAVE_NAME
};

// Class managing the GUI environment
class HFractalGui {
private:
    // Lists which keys on the keyboard map to which interface buttons
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

    HFractalMain* hm; // Pointer to main rendering environment
    HFractalMain* lowres_hm; // Pointer to an identical rendering environment, but with a lower resolution for preview renders

    std::string dialog_text; // Text to show in the text dialog widget
    std::string console_text; // Text to show in the application console
    std::string equation_buffer; // Contains the equation being used by both renderer classes
    std::string save_name_buffer; // Contains the text shown/edited in the name field in the save render state dialog
    bool button_states[BUTTON_NUM_TOTAL]; // Contains the current states of every button in the GUI (true for pressed, false for not pressed)
    Image buffer_image; // Image being used by raygui for displaying the render result
    Texture2D buffer_texture; // Texture being used by raygui for displaying the render result
    bool is_rendering; // Stores whether the GUI is currently waiting on a full-resolution render (and thus should freeze controls)
    bool is_outdated_render; // Stores whether the GUI is showing a preview render (i.e. needs a full-resolution render to be run by the user)
    TEXT_FOCUS_STATE textbox_focus; // Stores the currently focussed text box
    int render_percentage; // Stores the percentage completion of the current render
    bool showing_coordinates; // Stores whether coordinates are currently being shown on the mouse cursor
    MODAL_VIEW_STATE modal_view_state; // Stores the current modal state of the GUI, allowing certain controls to be enabled and disabled in different modes
    int image_dimension; // Stores the size of the image, used for sizing the window, scaling and rendering images, and positioning elements
    int control_panel_width; // Stores the width of the control panel
    CP_PRESETS selected_palette; // Determines the colour palette in which the GUI is currently displaying the rendered image
    HFractalDatabase database = HFractalDatabase ("FractalSavedStates.csv"); // Database which manages saved profile states
    long selected_profile_id; // Records the ID of the profile currently selected in the load render state dialog
    int database_load_dialog_scroll; // Records the current amount of scroll in the load render state dialog

    void configureStyling(); // Configures the GUI styling from a stylesheet provided by raylib's creator as part of the library
    void configureGUI(); // Configures the GUI and initialises all class variables ready for the first GUI mainloop update

    void parametersWereModified(); // Marks the GUI as using an outdated render and triggers a preview render update
    bool updatePreviewRender(); // Rerenders the preview image
    bool startFullRender(); // Triggers a full resolution render
    bool updateFullRender(); // Updates the image and texture buffers from the partially-finished rendering environment image, and finalises if the render has completed
    void reloadImageFrom(HFractalMain*); // Automatically fetch and reload the image and texture buffers from a given rendering environment

    void checkWindowResize(); // Check to see if the window has been resized, and handle it

    bool handleClickNavigation(); // Check to see if the user has clicked somewhere on the image, and jump to focus that location if so
    bool handleButtonPresses(); // Handle any interface button presses the user has made since the last update
    bool handleKeyPresses(); // Handle any keyboard key presses the user has made since the last update
    void drawInterface(); // Draw the entire interface, called each update

    Image getImage(HFractalMain*); // Extract image data from a rendering environment

    void enterEquationPresetDialog(); // Show the equation preset selector and disable other GUI controls
    void escapeEquationPresetDialog(int); // Close the equation preset selector and return to normal GUI mode
    void enterColourPalettePresetDialog(); // Show the colour palette preset selector and disable other GUI controls
    void escapeColourPalettePresetDialog(int); // Close the colour palette preset selector and return to normal GUI mode
    void launchTextDialog(std::string); // Show a text dialog over the window with a given string as text
    void closeTextDialog(); // Close the text dialog currently being shown

    void zoomIn(); // Handler for Zoom In button
    void zoomOut(); // Handler for Zoom Out button
    void resetZoom(); // Handler for Reset Zoom button
    void saveImage(); // Handler for Save Image button

    void moveUp(); // Handler for Move Up button
    void moveLeft(); // Handler for Move Left button
    void moveRight(); // Handler for Move Right button
    void moveDown(); // Handler for Move Down button

    void toggleCoords(); // Handler for Show/Hide Coordinates button

    void evalLimitLess(); // Handler for '<' button
    void evalLimitMore(); // Handler for '>' button

    void showHelp(); // Handler for Help & Instructions button
    void clearButtonStates(); // Clears current button states to ignore unhandled button presses

    void tryUnloadImage(); // Unload the image buffer, prevents memory leaks
    void tryUnloadTexture(); // Unload the texture buffer, prevents memory leaks

    void showSaveStateDialog(); // Make the save render state dialog visible
    void showLoadStateDialog(); // Make the load render state dialog visible
    void saveStateToDatabase(); // Save the current render state to the database
    void loadStateFromDatabase(); // Load the selected config profile from the database to be the current render state
    void closeDatabaseDialog(); // Hide the save/load render state dialog
    void databaseLoadScrollDown(); // Scroll down in the load render state dialog
    void databaseLoadScrollUp(); // Scroll up in the load render state dialog
public:
    int guiMain(); // Start and run the entire GUI. Blocks on current thread

    HFractalGui(); // Basic constructor
};

#endif