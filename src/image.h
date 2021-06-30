#include <stdint.h>
#include <mutex>

#ifndef IMAGE_H
#define IMAGE_H
class image {
private:
    int width;
    int height;
    uint16_t * rgb_image;
    int c_ind;
    std::mutex mut;
public:
    uint8_t * completed;
    void set (int, int, uint16_t);
    uint16_t get (int, int);
    image (int, int);
    int get_uncompleted ();
    bool is_done ();
    int get_ind ();
    void write (std::string path);
    ~image ();
};
#endif