#include "image.hh"
#include "utils.hh"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ostream>
#include <math.h>

/**
 * @brief Set the value of a pixel, and automatically mark it as complete
 * 
 * @param x Horizontal coordinate
 * @param y Vertical coordinate
 * @param p Value of the pixel to assign
 */
void HFractalImage::set(int x, int y, uint16_t p) {
    int offset = ((y*width)+x);
    data_image[offset] = p;
    completed[offset] = 2;
}

/**
 * @brief Get the value of the pixel at the specified coordinates, as measured from top-left
 * 
 * @param x Horizontal coordinate
 * @param y Vertical coordinate
 * @return The value of the pixel at the coordinates
 */
uint16_t HFractalImage::get(int x, int y) {
    return data_image[(y*width)+x];
}

/**
 * @brief Initialise a new image with a specified width and height
 * 
 * @param w Horizontal size
 * @param h Vertical size
 */
HFractalImage::HFractalImage(int w, int h) {
    width = w;
    height = h;
    c_ind = 0; 
    data_image = new uint16_t[width*height];
    completed = new uint8_t[width*height];
    // Clear both buffers
    for (int i = 0; i < width*height; i++) { data_image[i] = 0xffff; completed[i] = 0; }
}

/**
 * @brief Write the contents of the image buffer out to a PGM file (a minimal image format using grayscale linear colour space)
 * 
 * @param path Path to the output file
 * @return True for success, false for failure
 */
bool HFractalImage::writePGM (std::string path) {
    // Abort if the image is incomplete
    if (!isDone()) return false;
    FILE *img_file;
    img_file = fopen(path.c_str(),"wb");

    // Write the header
    fprintf(img_file,"P5\n");
    fprintf(img_file,"%d %d\n",width,height);
    fprintf(img_file,"65535\n");

    // Write each pixel
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            uint16_t p = data_image[(y*width)+x];
           
            fputc (p & 0x00ff, img_file);
            fputc ((p & 0xff00) >> 8, img_file);
        }
    }

    // Close and return success
    fclose(img_file);
    return true;
}

/**
 * @brief Create an RGBA 32 bit colour from hue, saturation, value components
 * 
 * @param h Hue value
 * @param s Saturation value
 * @param v Value (brightness) value
 * @return A 32 bit colour in RGBA form
 */
uint32_t HFractalImage::HSVToRGB (float h, float s, float v) { // TODO: Test this
    float c = v * s;
    float h_ = fmod(h,1)*6;
    float x = c * (1 - fabsf(fmodf(h_, 2)-1));
    float m = v - c;

    float r;
    float g;
    float b;

    if (h_ >= 0 && h_ < 1) {
        r = c; g = x; b = 0;
    } else if (h_ < 2) {
        r = x; g = c; b = 0;
    } else if (h_ < 3) {
        r = 0; g = c; b = x;
    } else if (h_ < 4) {
        r = 0; g = x; b = c;
    } else if (h_ < 5) {
        r = x; g = 0; b = c;
    } else if (h_ < 6) {
        r = c; g = 0; b = x;
    }

    r = r + m;
    g = g + m;
    b = b + m;

    uint32_t final_value = 0x000000ff;
    final_value |= (int)(r*255) << (8*3);
    final_value |= (int)(g*255) << (8*2);
    final_value |= (int)(b*255) << (8*1);

    return final_value;
}

/**
 * @brief Convert a computed value (i.e. from the image_data buffer) into a renderable RGBA 32 bit colour
 * 
 * @param value The value to convert
 * @param colour_preset Colour palette preset to map colour onto
 * @return The converted colour as a 32 bit integer
 */
uint32_t HFractalImage::colourFromValue (uint16_t value, int colour_preset) {
    uint32_t col = 0x000000ff;
    if (colour_preset == 0) {
        col |= 0x3311ff00;
        col |= ((value % 256) << (8*3)) + 0x33000000;
    } else if (colour_preset == 1) {
        uint8_t looped = 255-(uint8_t)(value % 256);
        col |= looped << (8*3);
        col |= (2*looped) << (8*2);
        col |= 0x00007000;
    } else if (colour_preset == 2) {
        float hue = (float)value/(float)0xffff;
        hue = fmod(512*hue, 1);
        col = HFractalImage::HSVToRGB (hue, 0.45, 0.8);
    } else if (colour_preset == 3) {
        uint8_t looped = 255-(uint8_t)(value % 256);
        col |= looped << (8*1); // B
        col |= looped << (8*2); // G
        col |= looped << (8*3); // R
    } else if (colour_preset == 4) {
        uint8_t looped = (uint8_t)(value % 256);
        col |= looped << (8*1); // B
        col |= looped << (8*2); // G
        col |= looped << (8*3); // R
    }
    
    
    return col;
}

/**
 * @brief Destroy the image class, freeing the buffers
 */
HFractalImage::~HFractalImage () {
    free (data_image);
    free (completed);
}

/**
 * @brief Fetch the index (i.e. (y*width)+x) of the next pixel which needs to be computed
 * 
 * @return The index of the next pixel to compute, -1 if there is no available pixel
 */
int HFractalImage::getUncompleted () {
    // Lock resources to prevent collisions
    mut.lock();
    int i = -1;
    // Find the next available pixel index and increment c_ind
    if (c_ind < height*width) {
        i = c_ind;
        c_ind++;
    }
    // Unlock before returning
    mut.unlock();
    return i;
}

/**
 * @brief Check every pixel to see if the image is fully computed. Use with caution, especially with large images
 * 
 * @return True if the image is complete, false otherwise
 */
bool HFractalImage::isDone () {
    // Iterate over every pixel to check its status
    for (int i = 0; i < height*width; i++) {
        if (completed[i] != 2) {
            // Fail if the pixel is not complete
            return false;
        }
    }
    // Succeed if every pixel is fully computed
    return true;
}

/**
 * @brief Get the current completion index of the image
 * 
 * @return The current completion index
 */
int HFractalImage::getInd () { return c_ind; }