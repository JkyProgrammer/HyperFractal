#include <stdint.h>
#include <mutex>

#ifndef IMAGE_H
#define IMAGE_H
class image {
private:
    int width;
    int height;
    uint16_t * rgb_image;
    uint8_t * completed;
    int c_ind;
    std::mutex mut;
public:
    void set (int, int, uint16_t);
    image (int, int);
    int get_uncompleted ();
    bool is_done ();
    int get_ind ();
    void write (std::string path);
    ~image ();
};
#endif