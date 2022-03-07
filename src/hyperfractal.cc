#include "hyperfractal.hh"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

#include "utils.hh"

using namespace std;
using namespace std::chrono;

/**
 * @brief Main function called when each worker thread starts. Contains code to actually fetch and render pixels
 * 
 */
void HFractalMain::threadMain () {
    // Pre-compute constants to increase performance
    long double p = 2/(zoom*resolution);
    long double q = (1/zoom)-offset_x;
    long double r = (1/zoom)+offset_y;

    // Get the next unrendered pixel
    int next = img->getUncompleted();
    while (next != -1) {
        // Find the x and y coordinates based on the pixel index
        int x = next%resolution;
        int y = next/resolution;
        // Apply the mathematical transformation of offsets and zoom to find a and b, which form a coordinate pair representing this pixel in the complex plane
        long double a = (p*x) - q;
        long double b = r - (p*y);
        // Construct the initial coordinate value, and perform the evaluation on the main equation
        complex<long double> c = complex<long double> (a,b);
        int res = (main_equation->evaluate (c, eval_limit));
        // Set the result back into the image class, and get the next available unrendered pixel
        img->set (x, y, res);
        next = img->getUncompleted();
    }
    
    // When there appear to be no more pixels to compute, mark this thread as completed
    thread_completion[std::this_thread::get_id()] = true;
    
    // Check to see if any other threads are still rendering, if not then set the flag to mark the environment as no longer rendering
    bool is_incomplete = false;
    for (auto p : thread_completion) is_incomplete |= !p.second;
    if (!is_incomplete) is_rendering = false;
}

/**
 * @brief Generate a fractal image based on all the environment parameters
 * 
 * @param wait Whether to wait and block the current thread until the image has been fully computed, useful if you want to avoid concurrency somewhere else (functionality hiding)
 * @return Integer representing status code, 0 for success, else for failure
 */
int HFractalMain::generateImage (bool wait=true) {
    if (getIsRendering()) { std::cout << "Aborting!" << std::endl; return 2; } // Prevent overlapping renders from starting
    // Output a summary of the rendering parameters
    std::setprecision (100);
    std::cout << "Rendering with parameters: " << std::endl;
    std::cout << "Resolution=" << resolution << std::endl;
    std::cout << "EvaluationLimit=" << eval_limit << std::endl;
    std::cout << "Threads=" << worker_threads << std::endl;
    std::cout << "Zoom="; printf ("%Le", zoom); std::cout << std::endl;
    std::cout << "OffsetX="; printf ("%.70Lf", offset_x); std::cout << std::endl;
    std::cout << "OffsetY="; printf ("%.70Lf", offset_y); std::cout << std::endl;

    // Abort rendering if the equation is invalid
    if (!isValidEquation()) { std::cout << "Aborting!" << std::endl; return 1; }
    
    // Mark the environment as now rendering, locking resources/parameters
    is_rendering = true;

    // Clear and reinitialise the image class with the requested resolution
    if (img != NULL) img->~HFractalImage();
    img = new HFractalImage (resolution, resolution);

    // Clear the thread pool, and populate it with fresh worker threads
    thread_pool.clear();
    thread_completion.clear();
    for (int i = 0; i < worker_threads; i++) {
        std::thread *t = new std::thread(&HFractalMain::threadMain, this);
        thread_completion[t->get_id()] = false;
        thread_pool.push_back(t);
    }

    // Optionally, wait for the render to complete before returning
    if (wait) {
        while (true) {
            // If enabled at compile time, show a progress bar in the terminal 
            #ifdef TERMINAL_UPDATES
            float percent = getImageCompletionPercentage();
            std::cout << "\r";
            std::cout << "Working: ";
            for (int k = 2; k <= 100; k+=2) { if (k <= percent) std::cout << "█"; else std::cout << "_"; }
            std::cout << " | ";
            std::cout << round(percent) << "%";
            #endif
            // Break out when the image has been fully completed (all pixels computed)
            if (img->isDone()) break;
            crossPlatformDelay (10);
        }
        // Wait for all the threads to join, then finish up
        for (auto th : thread_pool) th->join();
        is_rendering = false;
    }
    std::cout << std::endl << "Rendering done." << std::endl;
    return 0;
}

/**
 * @brief Construct a new rendering environment, with blank parameters
 * 
 */
HFractalMain::HFractalMain () {
    resolution = 1;
    offset_x = 0;
    offset_y = 0;
    zoom = 1;
    img = NULL;
}

/**
 * @brief Convert the raw data stored in the image class into a coloured RGBA 32 bit image using a particular colour scheme preset
 * 
 * @param colour_preset The colour scheme to use
 * @return uint32_t* Pointer to the image stored in memory as an array of 4-byte chunks
 */
uint32_t* HFractalMain::getRGBAImage (int colour_preset) {
    // Return a blank result if the image is uninitialised, other conditions should ensure this never occurs
    if (img == NULL) {
        return (uint32_t *)malloc(0);
    }

    // Copy parameters to local
    int size = resolution;
    int limit = eval_limit;
    
    // Construct a pixel buffer with RGBA channels
    uint32_t *pixels = (uint32_t *)malloc(size*size*sizeof(uint32_t));
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            int v = img->get(x,y);
            pixels[(y*size)+x] = (v == limit) ? 0x000000ff : HFractalImage::colourFromValue(v, colour_preset);
            
            // If the pixel has not been computed, make it transparent
            if (img->completed[(y*size)+x] != 2) pixels[(y*size)+x] = 0;
        }
    }

    // Return the pointer to the pixel buffer
    return pixels;
}

/**
 * @brief Get the percentage of pixels in the image which have been computed
 * 
 * @return Unrounded percentage
 */
float HFractalMain::getImageCompletionPercentage () {
    if (img == NULL) return 100;
    return ((float)(img->getInd())/(float)(resolution*resolution))*100;
}

/**
 * @brief Automatically write an image to a generated file address.
 * File path will be dynamically constructed so that the file name is unique, timestamped, and placed on the user's desktop
 * 
 * @param type Image format to write image out to
 * @return True for success, false for failure 
 */
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

    // Call into the image's writer to write out data
    switch (type) {
    case PGM:
        image_path += ".pgm";
        return img->writePGM (image_path);
    default:
        return false;
    }
}
