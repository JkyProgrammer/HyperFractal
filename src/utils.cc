#include "utils.hh"
#include <chrono>
#include <ctime>
#include <vector>

#ifdef _WIN32
//#include "dll.h"
#include <windows.h>
#include <shlobj.h>
#else
    #include <unistd.h>
#endif

/**
 * @brief 
 * 
 * @param i 
 * @param t 
 * @return string 
 */
string equationPreset (EQ_PRESETS p, bool t) {
    switch (p) {
    case EQ_MANDELBROT:
        return t ? "Mandelbrot" : "(z^2)+c";
    case EQ_JULIA_1:
        return t ? "Juila 1" : "(z^2)+(0.285+0.01i)";
    case EQ_JULIA_2:
        return t ? "Julia 2" : "(z^2)+(-0.70176-0.3842i)";
    case EQ_RECIPROCAL:
        return t ? "Reciprocal" : "1/((z^2)+c)";
    case EQ_ZPOWER:
        return t ? "Z Power" : "(z^z)+(c-0.5)";
    case EQ_BARS:
        return t ? "Bars" : "z^(c^2)";
    case EQ_BURNINGSHIP_MODIFIED:
        return t ? "Burning Ship Modified" : "((x^2)^0.5-((y^2)^0.5)i)^2+c";
    default:
        return "NONE";
    }
    return "";
}

string colourPalettePreset (CP_PRESETS p) {
    switch (p) {
    case CP_VAPORWAVE:
        return "Vaporwave";
    case CP_YELLOWGREEN:
        return "Yellow-Green";
    case CP_RAINBOW:
        return "Rainbow";
    case CP_GREYSCALE_BRIGHT:
        return "Greyscale Bright";
    case CP_GREYSCALE_DARK:
        return "Greyscale Dark";
    default:
        return "NONE";
    }
    return "";
}

string textWrap (string s, int lineLength) {
    string output = "";
    int lineOffset = 0;
    for (char c : s) {
        if (c == '\n') lineOffset = -1;
        if (lineOffset == lineLength-1) { if (c != ' ') output += "-"; output += "\n"; lineOffset = 0; }
        if (!(lineOffset == 0 && c == ' ')) {
            output += c;
            lineOffset++;
        }
    }
    return output;
}

using namespace std;
using namespace chrono;

string getDesktopPath () {
    #ifdef _WIN32
    static char path[MAX_PATH+1];
    if (SHGetSpecialFolderPathA(HWND_DESKTOP, path, CSIDL_DESKTOP, FALSE))
        return string(path) + string("\\");
    else
        return "";
    #else
    return string(getenv ("HOME")) + string("/Desktop/");
    #endif
}

void crossPlatformDelay(int milliseconds) {
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif
}