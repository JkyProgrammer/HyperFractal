#ifndef HYPERFRACTAL_H
#define HYPERFRACTAL_H
// When defined, progress updates will be written to terminal.
//#define TERMINAL_UPDATES

#include <string>
#include <thread>
#include <vector>
#include "image.hh"
#include "fractal.hh"
#include "utils.hh"
#include "equationparser.hh"

class HFractalMain {
private:
    int resolution;
    long double offset_x;
    long double offset_y;
    long double zoom;
    std::string eq;
    int worker_threads;
    int eval_limit;
    HFractalImage *img = new HFractalImage(0,0);
    HFractalEquation *main_equation;
    std::vector<std::thread*> thread_pool;
    bool is_rendering = false;


    void threadMain ();

public:
    // Arguments: int resolution, long double offset_x, long double offset_y, long double zoom, string equation, int worker_threads, int eval_limit
    int generateImage (bool);

    HFractalMain ();

    int getResolution () { return resolution; }
    void setResolution (int resolution_) { if (!getIsRendering()) resolution = resolution_; }

    long double getOffsetX () { return offset_x; }
    void setOffsetX (long double offset_x_) { if (!getIsRendering()) offset_x = offset_x_; }

    long double getOffsetY () { return offset_y; }
    void setOffsetY (long double offset_y_) { if (!getIsRendering()) offset_y = offset_y_; }

    long double getZoom () { return zoom; }
    void setZoom (long double zoom_) { if (!getIsRendering()) zoom = zoom_; }

    std::string getEquation () { return eq; }
    void setEquation (std::string eq_) { if (!getIsRendering()) { eq = eq_; main_equation = HFractalEquationParser::extract_equation (eq); } }

    int getWorkerThreads () { return worker_threads; }
    void setWorkerThreads (int wt_) { if (!getIsRendering()) worker_threads = wt_; }

    int getEvalLimit () { return eval_limit; }
    void setEvalLimit (int el_) { if (!getIsRendering()) eval_limit = el_; }

    bool isValidEquation () { return main_equation != NULL; }

    bool getIsRendering() { return /*getImageCompletionPercentage() < 100*/ is_rendering; }

    uint32_t* getRawImage (int);

    float getImageCompletionPercentage ();

    bool autoWriteImage (IMAGE_TYPE);
};
#endif