#include <stdint.h>

class image {
private:
    int width;
    int height;
    uint8_t * rgb_image;
public:
    void set (int, int, uint8_t, uint8_t, uint8_t);
    image (int, int);
    void write (const char* path);
    ~image ();
};