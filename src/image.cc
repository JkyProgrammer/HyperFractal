#include "image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void image::set(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x >= width || x < 0) return;
    if (y >= height || y < 0) return;
    int offset = ((y*width)+x)*3;
    rgb_image[offset] = r;
    rgb_image[offset+1] = g;
    rgb_image[offset+2] = b;
}

image::image(int, int) {
    rgb_image = new uint8_t[width*height*3];
}

void image::write (const char* path) {
    stbi_write_png(path, width, height, 3, rgb_image, width*3);
}

image::~image () {
    free (rgb_image);
}