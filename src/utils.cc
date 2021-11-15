#include "utils.h"

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