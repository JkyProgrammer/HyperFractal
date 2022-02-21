#include "hyperfractal.hh"
#include "utils.hh"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono;

void HFractalMain::threadMain () {
    long double p = 2/(zoom*resolution);
    long double q = (1/zoom)-offset_x;
    long double r = (1/zoom)+offset_y;

    int next = img->getUncompleted();
    while (next != -1) {
        int x = next%resolution;
        int y = next/resolution;
        long double a = (p*x) - q;
        long double b = r - (p*y);
        complex<long double> c = complex<long double> (a,b);
        int res = (main_equation->evaluate (c, eval_limit));
        img->set (x, y, res);
        next = img->getUncompleted();
    }
}

int HFractalMain::generateImage (bool wait=true) {
    std::setprecision (100);
    std::cout << "Rendering with parameters: " << std::endl;
    std::cout << "Resolution=" << resolution << std::endl;
    std::cout << "EvaluationLimit=" << eval_limit << std::endl;
    std::cout << "Threads=" << worker_threads << std::endl;
    std::cout << "Zoom="; printf ("%Le", zoom); std::cout << std::endl;
    std::cout << "OffsetX="; printf ("%.70Lf", offset_x); std::cout << std::endl;
    std::cout << "OffsetY="; printf ("%.70Lf", offset_y); std::cout << std::endl;
    std::cout << "Parsing HFractalEquation: \"" << eq + "\"" << std::endl;
    main_equation = HFractalEquationParser::extract_equation (eq);

    if (main_equation == NULL) { std::cout << "Stopping!" << std::endl; return 1; }

    // Detect if the HFractalEquation matches the blueprint of a preset
    preset = -1;
    for (int i = 1; i <= NUM_EQUATION_PRESETS; i++) {
        if (eq == equationPreset (i, false)) {
            preset = i;
            break;
        }
    }
    main_equation->setPreset (preset);
    

    //if (main_equation != NULL) std::cout << *main_equation << std::endl;
    if (img != NULL) img->~HFractalImage();
    img = new HFractalImage (resolution, resolution);

    auto t_a = high_resolution_clock::now();
    thread_pool.clear();
    for (int i = 0; i < worker_threads; i++) {
        std::thread *t = new std::thread(&HFractalMain::threadMain, this);
        thread_pool.push_back (t);
    }

    if (wait) {
        while (true) {
            if (img->isDone()) break;
            #ifdef TERMINAL_UPDATES
            float percent = ((float)(img->getInd())/(float)(resolution*resolution))*100;
            std::cout << "\r";
            std::cout << "Working: ";
            for (int k = 2; k <= 100; k+=2) { if (k <= percent) std::cout << "█"; else std::cout << "_"; }
            std::cout << " | ";
            std::cout << round(percent) << "%";
            #endif
            crossPlatformDelay (2);
        }
        for (auto th : thread_pool) th->join();

    }
    std::cout << "Rendering done." << std::endl;
    return 0;
}

bool HFractalMain::write (string path) {
    std::cout << "Writing... ";
    img->writePGM(path);
    std::cout << "Done." << endl;
    return 0;
}
