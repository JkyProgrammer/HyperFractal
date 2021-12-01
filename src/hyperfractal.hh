#ifndef HYPERFRACTAL_H
#define HYPERFRACTAL_H
//#define TERMINAL_UPDATES

#include <string>
#include <thread>
#include <vector>
#include "image.hh"
#include "fractal.hh"
#include "equationparser.hh"

void sleepcp(int);

class hfractal_main {
public:
    timing_data *d_all;
    int resolution = 1;
    long double offset_x = 0;
    long double offset_y = 0;
    long double zoom = 1;
    string eq;
    int worker_threads;
    int eval_limit;
    int preset = -1;
    image *img = new image(0,0);
    equation *main_equation;
    std::vector<std::thread*> thread_pool;

    void thread_main ();
    // Arguments: int resolution, long double offset_x, long double offset_y, long double zoom, string equation, int worker_threads, int eval_limit
    int generateImage (bool);

    bool write (string);

    hfractal_main () {}
};
#endif