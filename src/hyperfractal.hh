#ifndef HYPERFRACTAL_H
#define HYPERFRACTAL_H
// When defined, progress updates will be written to terminal.
//#define TERMINAL_UPDATES

#include <string>
#include <thread>
#include <vector>
#include "image.hh"
#include "fractal.hh"
#include "equationparser.hh"

class HFractalMain {
public:
    int resolution = 1;
    long double offset_x = 0;
    long double offset_y = 0;
    long double zoom = 1;
    std::string eq;
    int worker_threads;
    int eval_limit;
    int preset = -1;
    HFractalImage *img = new HFractalImage(0,0);
    HFractalEquation *main_equation;
    std::vector<std::thread*> thread_pool;

    void threadMain ();

    // Arguments: int resolution, long double offset_x, long double offset_y, long double zoom, string equation, int worker_threads, int eval_limit
    int generateImage (bool);

    bool write (std::string);

    HFractalMain () {}
};
#endif