#ifndef HYPERFRACTAL_H
#define HYPERFRACTAL_H
#define TERMINAL_UPDATES

#include <string>
#include <thread>
#include <vector>
#include "image.h"
#include "fractal.h"
#include "equationparser.h"

void sleepcp(int);

class hfractal_main {
public:
    int resolution;
    double offset_x;
    double offset_y;
    double zoom;
    string eq;
    int worker_threads;
    int eval_limit;
    image *img = new image(0,0);
    equation *main_equation;
    std::vector<std::thread*> thread_pool;

    void thread_main ();
    // Arguments: int resolution, double offset_x, double offset_y, double zoom, string equation, int worker_threads, int eval_limit
    int generateImage (bool);

    bool write (string);

    hfractal_main () {}
};
#endif