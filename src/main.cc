#include <iostream>
#include "hyperfractal.h"
#include "gui.h"

int main (int argc, char *argv[]) {
    hfractal_main hm;
    if (argc == 8) {
        hm.resolution = stoi (argv[1]);
        hm.offset_x = stod (argv[2]);
        hm.offset_y = stod (argv[3]);
        hm.zoom = stod (argv[4]);
        hm.eq = new string (argv[5]);
        hm.worker_threads = stoi (argv[6]);
        hm.eval_limit = stoi (argv[7]);
        hm.img = new image (hm.resolution, hm.resolution);
        hm.generateImage();
        return hm.write ("out.pgm");
    } else if (argc != 1) {
        std::cout << "Provide all the correct arguments please:" << std::endl;
        std::cout << "int resolution, double offset_x, double offset_y, double zoom, string equation, int worker_threads, int eval_limit" << std::endl;
        return 1;
    } else {
        return gui_main (&hm);
    }
}