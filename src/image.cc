#include "image.hh"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ostream>

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
    for (int i = 0; i < width*height; i++) { data_image[i] = 0xffff; completed[i] = 0; }
}

/**
 * @brief Write the contents of the image buffer out to a PGM file (a minimal image format using grayscale linear colour space)
 * 
 * @param path Path to the output file
 * @return True for success, false for failure
 */
bool HFractalImage::writePGM (std::string path) {
    if (!isDone()) return false;
    FILE *img_file;
    img_file = fopen(path.c_str(),"wb");

    fprintf(img_file,"P5\n");
    fprintf(img_file,"%d %d\n",width,height);
    fprintf(img_file,"511\n");

    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            uint16_t p = data_image[(y*width)+x];
            fputc (p & 0xff00, img_file);
            fputc (p & 0x00ff, img_file);
        }
    }

    fclose(img_file);
    return true;
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
        uint8_t looped = (uint8_t)(value % 256);
        col |= looped << (8*1); // B
        col |= looped << (8*2); // G
        col |= looped << (8*3); // R
    } else if (colour_preset == 1) {
        uint8_t looped = 255-(uint8_t)(value % 256);
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
    if (c_ind >= height*width) return -1;
    mut.lock();
    int i = -1;
    if (c_ind < height*width) {
        i = c_ind;
        c_ind++;
    }
    mut.unlock();
    return i;
}

/**
 * @brief Check every pixel to see if the image is fully computed. Use with caution, especially with large images
 * 
 * @return True if the image is complete, false otherwise
 */
bool HFractalImage::isDone () {
    if (width == 0 || height == 0) return false;
    for (int i = 0; i < height*width; i++) {
        if (completed[i] != 2) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Get the current completion index of the image
 * 
 * @return The current completion index
 */
int HFractalImage::getInd () {return c_ind;}