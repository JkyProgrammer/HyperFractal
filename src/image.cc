#include "image.h"
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void image::set(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    mut.lock();
    if (x >= width || x < 0) { mut.unlock(); return; }
    if (y >= height || y < 0) { mut.unlock(); return; }
    int offset = ((y*width)+x)*3;
    rgb_image[offset] = r;
    rgb_image[offset+1] = g;
    rgb_image[offset+2] = b;
    completed[offset/3] = 2;
    c_ind++;
    mut.unlock();
}

image::image(int w, int h) {
    width = w;
    height = h;
    rgb_image = new uint8_t[width*height*3];
    completed = new uint8_t[width*height];
    c_ind = 0;
}

void image::write (const char* path) {
    stbi_write_png(path, width, height, 3, rgb_image, width*3);
}

image::~image () {
    free (rgb_image);
    free (completed);
}

int image::get_uncompleted () {
    mut.lock();
    for (int i = c_ind; i < height*width; i++) if (completed[i] == 0) { completed[i] = 1; mut.unlock(); return i; }
    mut.unlock();
    return -1;
}

bool image::is_done () {
    for (int i = 0; i < height*width; i++) {
        if (completed[i] != 2) return false;
    }
    return true;
}

int image::get_ind () {return c_ind;}