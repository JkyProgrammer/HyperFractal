#include "hyperfractal.h"
#include "utils.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono;

void hfractal_main::thread_main () {
    long double p = 2/(zoom*resolution);
    long double q = (1/zoom)-offset_x;
    long double r = (1/zoom)+offset_y;

    int next = img->get_uncompleted();
    while (next != -1) {
        //auto t_a = high_resolution_clock::now();
        int x = next%resolution;
        int y = next/resolution;
        long double a = (p*x) - q;
        long double b = r - (p*y);
        //auto t_b = high_resolution_clock::now();
        complex<long double> c = complex<long double> (a,b);
        int res = (main_equation->evaluate (c, eval_limit, d_all));
        //auto t_c = high_resolution_clock::now();
        img->set (x, y, res);
        //auto t_d = high_resolution_clock::now();
        next = img->get_uncompleted();
        //auto t_e = high_resolution_clock::now();
        //d_all->d_math += duration_cast<microseconds> (t_b-t_a);
        //d_all->d_evaluate += duration_cast<microseconds> (t_c-t_b);
        //d_all->d_set += duration_cast<microseconds> (t_d-t_c);
        //d_all->d_get += duration_cast<microseconds> (t_e-t_d);
    }
}

int hfractal_main::generateImage (bool wait=true) {
    std::setprecision (100);
    std::cout << "Rendering with parameters: " << std::endl;
    std::cout << "Resolution=" << resolution << std::endl;
    std::cout << "EvaluationLimit=" << eval_limit << std::endl;
    std::cout << "Threads=" << worker_threads << std::endl;
    std::cout << "Zoom="; printf ("%Le", zoom); std::cout << std::endl;
    std::cout << "OffsetX="; printf ("%.70Lf", offset_x); std::cout << std::endl;
    std::cout << "OffsetY="; printf ("%.70Lf", offset_y); std::cout << std::endl;
    std::cout << "Parsing equation: \"" << eq + "\"" << std::endl;
    main_equation = extract_equation (eq);

    if (main_equation == NULL) { std::cout << "Stopping!" << std::endl; return 1; }

    // Detect if the equation matches the blueprint of a preset
    preset = -1;
    for (int i = 1; i < 7; i++) {
        if (eq == equationPreset (i, false)) {
            preset = i;
            break;
        }
    }
    main_equation->preset = preset;
    if (preset != -1) {
        main_equation->isPreset = true;
    }

    if (main_equation != NULL) std::cout << *main_equation << std::endl;
    if (img != NULL) img->~image();
    img = new image (resolution, resolution);
    d_all = new timing_data {
        microseconds(0),
        microseconds(0),
        microseconds(0),
        microseconds(0),
        microseconds(0),
        microseconds(0)
    };

    auto t_a = high_resolution_clock::now();
    thread_pool.clear();
    for (int i = 0; i < worker_threads; i++) {
        std::thread *t = new std::thread(&hfractal_main::thread_main, this);
        thread_pool.push_back (t);
    }

    if (wait) {
        while (true) {
            if (img->is_done()) break;
            #ifdef TERMINAL_UPDATES
            float percent = ((float)(img->get_ind())/(float)(resolution*resolution))*100;
            std::cout << "\r";
            std::cout << "Working: ";
            for (int k = 2; k <= 100; k+=2) { if (k <= percent) std::cout << "█"; else std::cout << "_"; }
            std::cout << " | ";
            std::cout << round(percent) << "%";
            #endif
            sleepcp (2);
        }
        for (auto th : thread_pool) th->join();

        auto t_b = high_resolution_clock::now();

        cout << "Timing report:" << endl;
        cout << "Total          " << duration_cast<microseconds>(t_b-t_a).count() << endl;
        cout << "Math           " << d_all->d_math.count() << endl;
        cout << "Evaluation     " << d_all->d_evaluate.count() << endl;
        cout << "   Compute     " << d_all->d_compute.count() << endl;
        cout << "   Is Infinity " << d_all->d_isinf.count() << endl;
        cout << "Set Pixel      " << d_all->d_set.count() << endl;
        cout << "Get Pixel      " << d_all->d_get.count() << endl;
        cout << "END" << endl;
    }
    std::cout << "Rendering done." << std::endl;
    return 0;
}

bool hfractal_main::write (string path) {
    std::cout << "Writing... ";
    img->write(path);
    std::cout << "Done." << endl;
    return 0;
}

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

void sleepcp(int milliseconds) {
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif
}