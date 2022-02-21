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
    is_rendering = false; // TODO: Cleaner
}

int HFractalMain::generateImage (bool wait=true) {
    if (getIsRendering()) { std::cout << "already rendering!"; return 2; }
    is_rendering = true;
    std::setprecision (100);
    std::cout << "Rendering with parameters: " << std::endl;
    std::cout << "Resolution=" << resolution << std::endl;
    std::cout << "EvaluationLimit=" << eval_limit << std::endl;
    std::cout << "Threads=" << worker_threads << std::endl;
    std::cout << "Zoom="; printf ("%Le", zoom); std::cout << std::endl;
    std::cout << "OffsetX="; printf ("%.70Lf", offset_x); std::cout << std::endl;
    std::cout << "OffsetY="; printf ("%.70Lf", offset_y); std::cout << std::endl;

    if (main_equation == NULL) { std::cout << "Stopping!" << std::endl; return 1; }

    // Detect if the HFractalEquation matches the blueprint of a preset
    int preset = -1;
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
        is_rendering = false;
    }
    std::cout << "Rendering done." << std::endl;
    return 0;
}

HFractalMain::HFractalMain () {
    resolution = 1;
    offset_x = 0;
    offset_y = 0;
    zoom = 1;
    img = NULL;
}

uint32_t* HFractalMain::getRawImage (int colour_preset) {
    if (img == NULL) {
        return (uint32_t *)malloc(0);
    }

    int size = resolution;
    int limit = eval_limit;
    
    // Construct a pixel buffer with RGBA channels
    uint32_t *pixels = (uint32_t *)malloc(size*size*sizeof(uint32_t));
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            int v = img->get(x,y);
            pixels[(y*size)+x] = (v == limit) ? HFractalImage::BLACK : HFractalImage::colourFromValue(v, colour_preset);
            
            // If the pixel has not been computed, make it transparent
            if (img->completed[(y*size)+x] != 2) pixels[(y*size)+x] = 0;
        }
    }

    return pixels;
}

float HFractalMain::getImageCompletionPercentage () {
    if (img == NULL) return 100;
    return ((float)(img->getInd())/(float)(resolution*resolution))*100;
}

bool HFractalMain::autoWriteImage (IMAGE_TYPE type) {
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
        return img->writePGM (image_path);
    default:
        return false;
    }
}
