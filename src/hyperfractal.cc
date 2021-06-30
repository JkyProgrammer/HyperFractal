#include "hyperfractal.h"
#include <iostream>

void hfractal_main::thread_main (int i) {        
    double p = 2/(zoom*resolution);
    double q = (1-offset_x)/zoom;
    double r = (1+offset_y)/zoom;
    int next = img->get_uncompleted();
    while (next != -1) {
        int x = next%resolution;
        int y = next/resolution;
        double a = (p*x) - q;
        double b = r - (p*y);
        complex<double> c = complex<double> (a,b);
        int res = logf((float)(main_equation->evaluate (c, eval_limit))/(float)eval_limit)*255;
        img->set (x, y, res);
        next = img->get_uncompleted();
    }
}

int hfractal_main::generateImage () {
    cout << "Parsing equation: \"" << *eq + "\"" << endl;
    main_equation = extract_equation (*eq);

    for (int i = 0; i < worker_threads; i++) {
        std::thread *t = new std::thread(&hfractal_main::thread_main, this, i);
        thread_pool.push_back (t);
    }

    while (true) {
        #ifdef TERMINAL_UPDATES
        float percent = ((float)(img->get_ind())/(float)(resolution*resolution))*100;
        std::cout << "\r";
        std::cout << "Working: ";
        for (int k = 2; k <= 100; k+=2) { if (k <= percent) std::cout << "█"; else std::cout << "_"; }
        std::cout << " | ";
        std::cout << round(percent) << "%";
        #endif
        sleepcp (2);
        if (img->is_done()) break;
    }
    std::cout << std::endl;
    for (auto th : thread_pool) th->join();
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