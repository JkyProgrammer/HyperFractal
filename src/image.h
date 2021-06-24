#include <stdint.h>
#include <mutex>

class image {
private:
    int width;
    int height;
    uint8_t * rgb_image;
    uint8_t * completed;
    int c_ind;
    std::mutex mut;
public:
    void set (int, int, uint8_t, uint8_t, uint8_t);
    image (int, int);
    int get_uncompleted ();
    bool is_done ();
    int get_ind ();
    void write (const char* path);
    ~image ();
};