#include <stdint.h>
#include <mutex>

#ifndef IMAGE_H
#define IMAGE_H
class HFractalImage {
private:
    int width;
    int height;
    uint16_t * rgb_image;
    int c_ind = 0;
    std::mutex mut;
public:
    HFractalImage (int, int);
    ~HFractalImage ();
    uint8_t * completed;
    void set (int, int, uint16_t);
    uint16_t get (int, int);
    int getUncompleted ();
    bool isDone ();
    int getInd ();
    bool writePGM (std::string path);
};

#include <iostream>
inline std::ostream & operator<<(std::ostream & Str, HFractalImage & v) { 
  std::cout << "Image :" << &v << std::endl;
  std::cout << v.getInd() << std::endl;
  std::cout << v.isDone() << std::endl;
  return Str;
}
#endif