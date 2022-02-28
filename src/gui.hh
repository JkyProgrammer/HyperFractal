#ifndef GUI_H
#define GUI_H

#include "hyperfractal.hh"

int guiMain();

class HFractalGui {
private:
    HFractalMain* lowres_hm;
    HFractalMain* hm;

    std::string dialog_text;
    std::string console_text;
    bool button_states[BUTTON_NUM_TOTAL];
    Image buffer_image;
    Texture2D buffer_texture;
    bool image_needs_update;
    bool is_rendering;
    bool is_outdated_render;
    int render_percentage;
    bool showing_coordinates;
    int modal_view_state;

    void configureStyling ();
    void configureGUI ();

public:
    int guiMain ();
// TODO
};
#endif