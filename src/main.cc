#include <iostream>
#include "hyperfractal.h"
#include "gui.h"

// int main () {
//     string s = "z+c";
//     int charIndex = 0;
//     int sLen = s.length();
//     cout << sLen << endl;
//     while (charIndex < sLen) {
//         cout << "STATUS" << endl;
//         cout << (charIndex < sLen) << endl;
//         cout << "charIndex: " << charIndex << endl;
//         cout << sLen << endl;

//         charIndex++;

//         if (charIndex == sLen) {
//             cout << "hello" << endl;
//             break; // Should not be necessarry
//         }
//     }
// }


int main (int argc, char *argv[]) {
    //extract_equation ("z+2");
    //cout << endl;
    //extract_equation ("(z+2)");
    //cout << endl;
    //extract_equation ("(z+(i-1)-3) + c");
    //cout << endl;
    //extract_equation ("z^2 + c");
    //cout << endl;
    //extract_equation ("z + (2 * c)");
    //cout << endl;
    //extract_equation ("20z");
    cout << endl;
    extract_equation ("z * c + 2");

    exit (0);

    if (argc == 8) {
        hfractal_main hm;
        hm.resolution = stoi (argv[1]);
        hm.offset_x = stod (argv[2]);
        hm.offset_y = stod (argv[3]);
        hm.zoom = stod (argv[4]);
        hm.eq = string (argv[5]);
        hm.worker_threads = stoi (argv[6]);
        hm.eval_limit = stoi (argv[7]);
        hm.generateImage(true);
        return hm.write ("out.pgm");
    } else if (argc != 1) {
        std::cout << "Provide all the correct arguments please:" << std::endl;
        std::cout << "int resolution, long double offset_x, long double offset_y, long double zoom, string equation, int worker_threads, int eval_limit" << std::endl;
        return 1;
    } else {
        return gui_main ();
    }
}