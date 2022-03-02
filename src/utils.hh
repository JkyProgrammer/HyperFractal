#ifndef UTILS_H
#define UTILS_H

#include <string>

#define NUM_EQUATION_PRESETS 7
#define NUM_COLOUR_PRESETS 5

// Wrap text along a line length
std::string textWrap (std::string, int);

// Enum describing possible equation presets
enum EQ_PRESETS {
    EQ_MANDELBROT = 0, // "(z^2)+c"
    EQ_JULIA_1, // "(z^2)+(0.285+0.01i)"
    EQ_JULIA_2, // "(z^2)+(-0.70176-0.3842i)"
    EQ_RECIPROCAL, // "1/((z^2)+c)"
    EQ_ZPOWER, // "(z^z)+(c-0.5)"
    EQ_BARS, // "z^(c^2)"
    EQ_BURNINGSHIP_MODIFIED // "((x^2)^0.5-((y^2)^0.5)i)^2+c"
};

// Enum describing possible colour palette presets
enum CP_PRESETS {
    CP_VAPORWAVE = 0,
    CP_YELLOWGREEN,
    CP_RAINBOW,
    CP_GREYSCALE_BRIGHT,
    CP_GREYSCALE_DARK
};

// Enum describing available image types which can be saved to disk
enum IMAGE_TYPE {
    PGM
};

// Delay for a given number of milliseconds
void crossPlatformDelay (int);

// Get the user's desktop path
std::string getDesktopPath ();

// Get information about an equation preset
std::string equationPreset (EQ_PRESETS, bool);
// Get information about a colour palette preset
std::string colourPalettePreset (CP_PRESETS);

#endif // !UTILS_H