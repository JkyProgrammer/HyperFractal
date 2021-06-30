#include "image.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ostream>

void image::set(int x, int y, uint16_t p) {
    mut.lock();
    if (x >= width || x < 0) { mut.unlock(); return; }
    if (y >= height || y < 0) { mut.unlock(); return; }
    int offset = ((y*width)+x);
    rgb_image[offset] = p;
    completed[offset] = 2;
    c_ind++;
    mut.unlock();
}

image::image(int w, int h) {
    width = w;
    height = h;
    rgb_image = new uint16_t[width*height];
    completed = new uint8_t[width*height];
    c_ind = 0;
}

void image::write (std::string path) {
    FILE *imgFile;
    imgFile = fopen(path.c_str(),"wb");

    fprintf(imgFile,"P5\n");
    fprintf(imgFile,"%d %d\n",width,height);
    fprintf(imgFile,"255\n");

    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            uint16_t p = rgb_image[(y*width)+x];
            //fprintf(imgFile, (char*)&x);
            //std::cout << p << std::endl;
            //std::cout << (p & 0b11111111) << std::endl;
            //std::cout << (p >> 8) << std::endl << std::endl;
            fputc (p & 0b11111111, imgFile);
            //fputc (p >> 8, imgFile);
            //fputwc (x, imgFile);
        }
    }

    fclose(imgFile);
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