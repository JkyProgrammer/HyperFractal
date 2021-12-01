#ifndef UTILS_H
#define UTILS_H

#define NUM_EQUATION_PRESETS 7

#include <string>
#include "image.hh"

using namespace std;
string equationPreset (int, bool);
string textWrap (string, int);
enum EQ_PRESETS {
    EQ_MANDELBROT = 1, // "(z^2)+c"
    EQ_JULIA_1 = 2, // "(z^2)+(0.285+0.01i)"
    EQ_JULIA_2 = 3, // "(z^2)+(-0.70176-0.3842i)"
    EQ_RECIPROCAL = 4, // "1/((z^2)+c)"
    EQ_ZPOWER = 5, // "(z^z)+(c-0.5)"
    EQ_BARS = 6, // "z^(c^2)"
    EQ_BURNINGSHIP_MODIFIED = 7 // "((x^2)^0.5-((y^2)^0.5)i)^2+c"
};

enum imageType {
    PGM
};


bool autoWriteImage (image* im, imageType type);
#endif // !UTILS_H