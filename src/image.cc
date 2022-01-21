#include "image.hh"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ostream>

void HFractalImage::set(int x, int y, uint16_t p) {
    int offset = ((y*width)+x);
    rgb_image[offset] = p;
    completed[offset] = 2;
    //mut.lock(); // ~NOTE: DISABLED FOR PERFORMANCE~
    c_ind++;
    //mut.unlock();
}

uint16_t HFractalImage::get(int x, int y) {
    //mut.lock();
    //if (x >= width || x < 0) { mut.unlock(); return 0; }
    //if (y >= height || y < 0) { mut.unlock(); return 0; }
    //uint16_t v = rgb_image[(y*width)+x];
    //mut.unlock();
    return rgb_image[(y*width)+x];
}

HFractalImage::HFractalImage(int w, int h) : width(w), height(h), c_ind(0) {
    rgb_image = new uint16_t[width*height];
    completed = new uint8_t[width*height];
    for (int i = 0; i < width*height; i++) completed[i] = 0;
}

bool HFractalImage::writePGM (std::string path) {
    if (!isDone()) return false;
    FILE *img_file;
    img_file = fopen(path.c_str(),"wb");

    fprintf(img_file,"P5\n");
    fprintf(img_file,"%d %d\n",width,height);
    fprintf(img_file,"255\n");

    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            uint16_t p = rgb_image[(y*width)+x];
            fputc (p & 0b11111111, img_file);
        }
    }

    fclose(img_file);
    return true;
}

HFractalImage::~HFractalImage () {
    free (rgb_image);
    free (completed);
}

int HFractalImage::getUncompleted () {
    mut.lock();
    for (int i = c_ind; i < height*width; i++) {
        if (completed[i] == 0) {
            
            completed[i] = 1;
            mut.unlock();
            return i;
        }
    }
    mut.unlock();
    return -1;
}

bool HFractalImage::isDone () {
    if (width == 0 || height == 0) return false;
    for (int i = 0; i < height*width; i++) {
        if (completed[i] != 2) return false;
    }
    return true;
}

int HFractalImage::getInd () {return c_ind;}