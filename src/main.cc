#include <iostream>
#include "hyperfractal.h"
#include "gui.h"

int main (int argc, char *argv[]) {
    //extract_equation ("5z + 2c/5"); // 5z*2c5/*+
    //exit(0);
    //extract_equation ("5z + 16ci - 3(2z+(c^2z))"); //5z*16c*i*+3 2z*c2z*^+*-
    //extract_equation ("z/5 + (c^z)*(z^3)");
    //exit (0);

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