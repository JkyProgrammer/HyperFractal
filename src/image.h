#include <stdint.h>
#include <mutex>

#ifndef IMAGE_H
#define IMAGE_H
class image {
private:
    int width;
    int height;
    uint16_t * rgb_image;
    int c_ind = 0;
    std::mutex mut;
public:
    image (int, int);
    ~image ();
    uint8_t * completed;
    void set (int, int, uint16_t);
    uint16_t get (int, int);
    int get_uncompleted ();
    bool is_done ();
    int get_ind ();
    void write (std::string path);
};

#include <iostream>
inline std::ostream & operator<<(std::ostream & Str, image & v) { 
  std::cout << "Image :" << &v << std::endl;
  std::cout << v.get_ind() << std::endl;
  std::cout << v.is_done() << std::endl;
  return Str;
}
#endif