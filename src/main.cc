#include <iostream>

#include "hyperfractal.hh"
#include "guimain.hh"
#include "utils.hh"

using namespace std;
/**
 * Naming Convention:
 * Classes & Structs - CapitalisedCamelCase
 * Variables - snake_case
 * Functions - uncapitalisedCamelCase
 * Constants - SCREAMING_SNAKE_CASE
 * 
 **/

int main (int argc, char *argv[]) {
    if (argc == 8) {
        HFractalMain hm;
        hm.setResolution (stoi (argv[1]));
        hm.setOffsetX (stod (argv[2]));
        hm.setOffsetY (stod (argv[3]));
        hm.setZoom (stod (argv[4]));
        hm.setEquation (string (argv[5]));
        hm.setWorkerThreads (stoi (argv[6]));
        hm.setEvalLimit (stoi (argv[7]));
        hm.generateImage(true);
        return hm.autoWriteImage (IMAGE_TYPE::PGM);
    } else if (argc != 1) {
        cout << "Provide all the correct arguments please:" << endl;
        cout << "int resolution, long double offset_x, long double offset_y, long double zoom, string HFractalEquation, int worker_threads, int eval_limit" << endl;
        return 1;
    } else {
        return guiMain();
    }
}

// TODO: Add comments to all the code
// TODO: Rewrite pixel distribution
// TODO: Remove all debugging related stuff
// TODO: Add a 'jump to' dialog
// TODO: PNG writing