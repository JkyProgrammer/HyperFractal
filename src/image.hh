#include <stdint.h>
#include <mutex>

#ifndef IMAGE_H
#define IMAGE_H

// Class containing information about an image currently being generated
class HFractalImage {
private:
    int width; // Width of the image
    int height; // Heigh of the image
    uint16_t * data_image; // Computed data values of the image
    int c_ind = 0; // Index of the next pixel to be sent out to a rendering thread
    std::mutex mut; // Mutex object used to lock class resources during multi-threading events

public:
    HFractalImage (int, int); // Constructor, creates a new image buffer of the specified size
    ~HFractalImage (); // Destructor, destroys and deallocates resources used in the current image
    
    void set (int, int, uint16_t); // Set the value of a pixel
    uint16_t get (int, int); // Get the value of a pixel
    uint8_t * completed; // Stores the completion status of each pixel, 0 = not computed, 1 = in progress, 2 = computed
    int getUncompleted (); // Get the index of an uncomputed pixel, to be sent to a rendering thread, and update completion data
    bool isDone (); // Check if the image has been completed or not
    int getInd (); // Get the current completion index
    bool writePGM (std::string); // Write out the contents of the data buffer to a simple image file, PGM format, with the given path

    static uint32_t colourFromValue (uint16_t, int); // Convert a computed value into a 32 bit RGBA colour value, using the specified palette
    static const uint32_t BLACK = 0x000000ff; // Colour constant for black
};

#include <iostream>
inline std::ostream & operator<<(std::ostream & Str, HFractalImage & v) { 
  std::cout << "Image :" << &v << std::endl;
  std::cout << v.getInd() << std::endl;
  std::cout << v.isDone() << std::endl;
  return Str;
}
#endif