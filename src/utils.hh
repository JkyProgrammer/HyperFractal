#ifndef UTILS_H
#define UTILS_H

#define NUM_EQUATION_PRESETS 7
#define NUM_COLOUR_PRESETS 5

#include <string>
#include "image.hh"

using namespace std;

string textWrap (string, int);

enum EQ_PRESETS {
    EQ_MANDELBROT = 0, // "(z^2)+c"
    EQ_JULIA_1, // "(z^2)+(0.285+0.01i)"
    EQ_JULIA_2, // "(z^2)+(-0.70176-0.3842i)"
    EQ_RECIPROCAL, // "1/((z^2)+c)"
    EQ_ZPOWER, // "(z^z)+(c-0.5)"
    EQ_BARS, // "z^(c^2)"
    EQ_BURNINGSHIP_MODIFIED // "((x^2)^0.5-((y^2)^0.5)i)^2+c"
};

enum CP_PRESETS {
    CP_VAPORWAVE = 0,
    CP_YELLOWGREEN,
    CP_RAINBOW,
    CP_GREYSCALE_BRIGHT,
    CP_GREYSCALE_DARK
};

enum IMAGE_TYPE {
    PGM
};

void crossPlatformDelay (int);

string getDesktopPath ();

string equationPreset (EQ_PRESETS, bool);
string colourPalettePreset (CP_PRESETS);

#endif // !UTILS_H