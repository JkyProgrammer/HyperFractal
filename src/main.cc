#include <iostream>
#include <vector>
#include <thread>
#include "fractal.h"
#include "equationparser.h"
#include "image.h"

#define TERMINAL_UPDATES

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

class hfractal_main {
    int resolution;
    double offset_x;
    double offset_y;
    double zoom;
    string *eq;
    int worker_threads;
    int eval_limit;
    image *img = new image(0,0);
    equation *main_equation;
    std::vector<std::thread*> thread_pool;

    int fail () {
        cout << "Provide all the correct arguments please:" << endl;
        cout << "int resolution, double offset_x, double offset_y, double zoom, string equation, int worker_threads, int eval_limit" << endl;
        return 1;
    }

    void thread_main (int i) {
        // Range of execution is between ((resolution*resolution)/worker_threads)*i and ((resolution*resolution)/worker_threads)*(i+1)
        
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
            img->set (x, y, res, res, res);
            next = img->get_uncompleted();
        }
    }
public:
    // Arguments: int resolution, double offset_x, double offset_y, double zoom, string equation, int worker_threads, int eval_limit
    int main (int argc, char *argv[]) {
        if (argc != 8) {
            cout << argc << endl;
            return fail ();
        }

        resolution = stoi (argv[1]);
        offset_x = stod (argv[2]);
        offset_y = stod (argv[3]);
        zoom = stod (argv[4]);
        eq = new string (argv[5]);
        worker_threads = stoi (argv[6]);
        eval_limit = stoi (argv[7]);
        img = new image (resolution, resolution);

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
        std::cout << "Writing... ";
        img->write("out.png");
        std::cout << "Done." << endl;
        return 0;
    }

    hfractal_main () {}
};

int main (int argc, char *argv[]) {
    hfractal_main hm;
    hm.main (argc, argv);
}