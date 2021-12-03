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
string equationPreset (int i, bool t) {
    switch (i) {
    case 1:
        return t ? "Mandelbrot" : "(z^2)+c";
    case 2:
        return t ? "Juila 1" : "(z^2)+(0.285+0.01i)";
    case 3:
        return t ? "Julia 2" : "(z^2)+(-0.70176-0.3842i)";
    case 4:
        return t ? "Reciprocal" : "1/((z^2)+c)";
    case 5:
        return t ? "Z Power" : "(z^z)+(c-0.5)";
    case 6:
        return t ? "Bars" : "z^(c^2)";
    case 7:
        return t ? "Burning Ship Modified" : "((x^2)^0.5-((y^2)^0.5)i)^2+c";
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


bool autoWriteImage (HFractalImage* im, imageType type) {
    string image_name = "Fractal render from ";

    // Get current system time
    auto time = system_clock::to_time_t (system_clock::now());
    string c_time = string (ctime (&time));

    // Separate ctime result into components
    vector<string> time_components;
    string current_component = "";
    for (char c : c_time) {
        if (c == ' ') {
            time_components.push_back (current_component);
            current_component = "";
        } else if (c != '\n') current_component.push_back (c != ':' ? c : '.');
    }
    time_components.push_back (current_component);

    // Get milliseconds, not part of ctime
    system_clock::duration dur = system_clock::now().time_since_epoch();
    seconds s = duration_cast<seconds> (dur);
    dur -= s;
    milliseconds ms = duration_cast<milliseconds> (dur);

    // Components are in the form: dayofweek month day hour:minute:second year
    image_name += time_components[2] + " ";
    image_name += time_components[1] + " ";
    image_name += time_components[4] + " ";
    image_name += "at ";
    image_name += time_components[3];
    image_name += ".";
    image_name += to_string(ms.count());

    cout << image_name << endl;

    string image_path = "";

    image_path += getDesktopPath();
    image_path += image_name;


    switch (type) {
    case PGM:
        image_path += ".pgm";
        return im->writePGM (image_path);
    default:
        return false;
    }
    
}

void crossPlatformDelay(int milliseconds) {
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif
}