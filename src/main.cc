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
    int block_size;
    string *eq;
    int worker_threads;
    int eval_limit;
    image *img = new image(0,0);
    equation *main_equation;
    std::vector<std::thread*> thread_pool;

    int fail () {
        cout << "Provide all the correct arguments please:" << endl;
        cout << "int resolution, double offset_x, double offset_y, double zoom, string equation, int worker_threads, int eval_limit" << endl;
        cout << "Ensure equation has no spaces" << endl;  
        return 1;
    }

    void thread_main (int i) {
        // Range of execution is between ((resolution*resolution)/worker_threads)*i and ((resolution*resolution)/worker_threads)*(i+1)
        for (int k = block_size*i; k < block_size*(i+1); k++) {
            int x = k%resolution;
            int y = k/resolution;
            complex<double> c = complex<double> ((x-offset_x)/zoom, (y-offset_y)/zoom); // TODO: Factor in zoom and offset
            int res = main_equation->evaluate (c, eval_limit);
            cout << "fuck" << endl;
            img->set (x, y, res, res, res);
            cout << "fuck2" << endl;
        }
    }
public:
    // Arguments: int resolution, double offset_x, double offset_y, double zoom, string equation, int worker_threads, int eval_limit
    int main (int argc, char *argv[]) {
        if (argc != 8) {
            cout << argc << endl;
            return fail ();
        }

        resolution = stoi (argv[1]);
        offset_x = stod (argv[2]);
        offset_y = stod (argv[3]);
        zoom = stod (argv[4]);
        eq = new string (argv[5]);
        worker_threads = stoi (argv[6]);
        eval_limit = stoi (argv[7]);
        img = new image (resolution, resolution);
        block_size = (resolution*resolution)/worker_threads;

        cout << "Parsing equation: \"" << *eq + "\"" << endl;
        main_equation = extract_equation (*eq);

        for (int i = 0; i < worker_threads; i++) {
            std::thread t(&hfractal_main::thread_main, this, i);
            thread_pool.push_back (&t);
        }

        for (auto th : thread_pool) th->join();
        img->write("out.png");
        return 0;
    }

    hfractal_main () {}
};

int main (int argc, char *argv[]) {
    hfractal_main hm;
    hm.main (argc, argv);
}