#include "utils.hh"
#include <chrono>

using namespace std;

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
    #include <unistd.h>
#endif

/**
 * @brief Get the details of an equation preset
 * 
 * @param p The preset to return
 * @param t True - return the name of the preset, False - return the string equation of it instead
 * @return Either the name or the equation string of the equation preset 
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

/**
 * @brief Return the name of a colour palette preset
 * 
 * @param p Preset to return
 * @return The string name of the colour palette
 */
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

/**
 * @brief Wrap text given a certain line length
 * 
 * @param s String to wrap
 * @param line_length Numbere of characters which limit the length of the line
 * @return string 
 */
string textWrap (string s, int line_length) {
    string output = "";
    int line_offset = 0;
    for (char c : s) {
        if (c == '\n') line_offset = -1;
        if (line_offset == line_length-1) { if (c != ' ') output += "-"; output += "\n"; line_offset = 0; }
        if (!(line_offset == 0 && c == ' ')) {
            output += c;
            line_offset++;
        }
    }
    return output;
}

/**
 * @brief Get the desktop path of the user
 * 
 * @return The user's desktop path
 */
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

/**
 * @brief Delay for a number of millisecods, across any platform
 * 
 * @param milliseconds Time to delay
 */
void crossPlatformDelay(int milliseconds) {
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif
}