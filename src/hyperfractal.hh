#ifndef HYPERFRACTAL_H
#define HYPERFRACTAL_H
// When defined, progress updates will be written to terminal.
#define TERMINAL_UPDATES

#include <string>
#include <thread>
#include <vector>
#include <map>
#include "image.hh"
#include "fractal.hh"
#include "utils.hh"
#include "equationparser.hh"

// Class defining a fractal rendering environment, fully encapsulated
class HFractalMain {
private:
    int resolution; // Horizontal and vertical dimension of the desired image
    long double offset_x; // Horizontal offset in the complex plane
    long double offset_y; // Vertical offset in the complex plane
    long double zoom; // Scaling value for the image (i.e. zooming in)

    std::string eq; // String equation being used
    HFractalEquation *main_equation; // Actual pointer to the equation manager class being used for computation

    int worker_threads; // Number of worker threads to be used for computation
    int eval_limit; // Evaluation limit for the rendering environment

    HFractalImage *img = new HFractalImage(0,0); // Pointer to the image class containing data for the rendered image

    std::vector<std::thread*> thread_pool; // Thread pool containing currently active threads
    std::map<std::thread::id, bool> thread_completion; // Map of which threads have finished computing pixels
    bool is_rendering = false; // Marks whether there is currently a render ongoing (locking resources to prevent concurrent modification e.g. changing resolution mid-render)

    void threadMain (); // Method called on each thread when it starts, contains the worker/rendering code

public:
    int generateImage (bool); // Perform the render, and optionally block the current thread until it is done

    HFractalMain (); // Base initialiser

    int getResolution () { return resolution; } // Inline methods to get/set the resolution
    void setResolution (int resolution_) { if (!getIsRendering()) resolution = resolution_; }

    long double getOffsetX () { return offset_x; } // Inline methods to get/set the x offset
    void setOffsetX (long double offset_x_) { if (!getIsRendering()) offset_x = offset_x_; }

    long double getOffsetY () { return offset_y; } // Inline methods to get/set the y offset
    void setOffsetY (long double offset_y_) { if (!getIsRendering()) offset_y = offset_y_; }

    long double getZoom () { return zoom; } // Inline methods to get/set the zoom
    void setZoom (long double zoom_) { if (!getIsRendering()) zoom = zoom_; }

    std::string getEquation () { return eq; } // Inline methods to get/set the equation
    void setEquation (std::string eq_) { if (!getIsRendering()) { eq = eq_; main_equation = HFractalEquationParser::extract_equation (eq); } }

    int getWorkerThreads () { return worker_threads; } // Inline methods to get/set the number of worker threads
    void setWorkerThreads (int wt_) { if (!getIsRendering()) worker_threads = wt_; }

    int getEvalLimit () { return eval_limit; } // Inline methods to get/set the evaluation limit
    void setEvalLimit (int el_) { if (!getIsRendering()) eval_limit = el_; }

    bool isValidEquation () { return main_equation != NULL; } // Check if the equation the user entered was parsed correctly last time it was set

    bool getIsRendering() { return is_rendering; } // Get if there is currently a render happening in this environment

    uint32_t* getRGBAImage (int); // Return a pointer to a 32 bit RGBA formatted image, produced using a particular colour scheme preset, from the generated image

    float getImageCompletionPercentage (); // Get the current percentage of pixels that have been actually computed

    bool autoWriteImage (IMAGE_TYPE); // Automatically write out the render to desktop using a particular image type
};
#endif