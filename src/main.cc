#include <iostream>
#include "hyperfractal.h"
#include "gui.h"

int main (int argc, char *argv[]) {
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