#include <string>

#ifndef DATABASE_H
#define DATABASE_H

struct HFractalConfigProfile {
    long profile_id;

    long double x_offset;
    long double y_offset;
    long double zoom;
    int iterations;
    std::string equation;
    std::string name;
    std::string preview_filename;
    long user_id;
};

struct HFractalUserProfile {
    long user_id;

    std::string user_name;
}

class HFractalDatabase {
public:
     
}

#endif