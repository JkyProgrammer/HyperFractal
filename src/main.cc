#include <iostream>
#include <vector>
#include <thread>
#include "fractal.h"
#include "equationparser.h"
#include "image.h"

class hfractal_main {
    int resolution;
    double offset_x;
    double offset_y;
    double zoom;
    string *eq;
    int worker_threads;
    image img = image(0,0);
    equation *main_equation;
    std::vector<std::thread*> thread_pool;

    int fail () {
        cout << "Provide all the correct arguments please:" << endl;
        cout << "int resolution, double offset_x, double offset_y, double zoom, string equation, int worker_threads" << endl;
        cout << "Ensure equation has no spaces" << endl;  
        return 1;
    }

    void thread_main (int i) {
        int y = 0;
        int x = 0;
        

        // TODO: Main execution
    }
public:
    // Arguments: int resolution, double offset_x, double offset_y, double zoom, string equation, int worker_threads
    int main (int argc, char *argv[]) {
        if (argc != 7) {
            cout << argc << endl;
            return fail ();
        }

        resolution = stoi (argv[1]);
        offset_x = stod (argv[2]);
        offset_y = stod (argv[3]);
        zoom = stod (argv[4]);
        eq = new string (argv[5]);
        worker_threads = stoi (argv[6]);
        img = image (resolution, resolution);

        cout << "Parsing equation: \"" << *eq + "\"" << endl;
        main_equation = extract_equation (*eq);

        for (int i = 0; i < worker_threads; i++) {
            std::thread t(&hfractal_main::thread_main, this, i);
            thread_pool.push_back (&t);
        }

        for (auto th : thread_pool) th->join();
        img.write("out.png");
        return 0;
    }

    hfractal_main () {}
};

int main (int argc, char *argv[]) {
    hfractal_main hm;
    hm.main (argc, argv);
}