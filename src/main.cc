#include <iostream>
#include <chrono>

/**
 * Naming Convention:
 * Classes & Structs - CapitalisedCamelCase
 * Variables - snake_case
 * Functions - uncapitalisedCamelCase
 * Constants - SCREAMING_SNAKE_CASE
 * 
 **/



#include "hyperfractal.hh"
#include "database.hh"

#define minclude
#include "gui.hh"

int main (int argc, char *argv[]) {
    if (argc == 8) {
        HFractalMain hm;
        hm.setResolution (std::stoi (argv[1]));
        hm.setOffsetX (std::stod (argv[2]));
        hm.setOffsetY (std::stod (argv[3]));
        hm.setZoom (std::stod (argv[4]));
        hm.setEquation (std::string (argv[5]));
        hm.setWorkerThreads (std::stoi (argv[6]));
        hm.setEvalLimit (std::stoi (argv[7]));
        hm.generateImage(true);
        return hm.autoWriteImage (IMAGE_TYPE::PGM);
    } else if (argc != 1) {
        std::cout << "Provide all the correct arguments please:" << std::endl;
        std::cout << "int resolution, long double offset_x, long double offset_y, long double zoom, string HFractalEquation, int worker_threads, int eval_limit" << std::endl;
        return 1;
    } else {
        HFractalGui gui = HFractalGui ();
        int res = gui.guiMain();
        return res;
    }
}