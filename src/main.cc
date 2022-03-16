// src/main.cc

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
        // If we have the required arguments, run a console-only render
        HFractalMain hm;
        int argument_error = 0;
        try {
            hm.setResolution (stoi (argv[1]));
            if (hm.getResolution() <= 0) throw runtime_error("Specified resolution too low.");
            argument_error++;
            hm.setOffsetX (stod (argv[2]));
            argument_error++;
            hm.setOffsetY (stod (argv[3]));
            argument_error++;
            hm.setZoom (stod (argv[4]));
            argument_error++;
            hm.setEquation (string (argv[5]));
            if (!hm.isValidEquation()) throw runtime_error("Specified equation is invalid.");
            argument_error++;
            hm.setWorkerThreads (stoi (argv[6]));
            if (hm.getWorkerThreads() <= 0) throw runtime_error("Must use at least one worker thread.");
            argument_error++;
            hm.setEvalLimit (stoi (argv[7]));
            if (hm.getEvalLimit() <= 0) throw runtime_error("Must use at least one evaluation iteration.");
            argument_error++;
            hm.generateImage(true);
            return hm.autoWriteImage (IMAGE_TYPE::PGM);
        } catch (runtime_error e) {
            cout << "Parameter error on argument number " << argument_error << ":" << endl;
            cout << "  " << e.what() << endl;
            return 1;
        }
    } else if (argc != 1) {
        // If we have only some arguments, show the user what arguments they need to provide
        cout << "Provide all the correct arguments please:" << endl;
        cout << "int resolution, long double offset_x, long double offset_y, long double zoom, string HFractalEquation, int worker_threads, int eval_limit" << endl;
        return 1;
    } else {
        // Otherwise, start the GUI
        return guiMain(argv[0]);
    }
}

// TODO: Rewrite pixel distribution
// TODO: PNG writing
