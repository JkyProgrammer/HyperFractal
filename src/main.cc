#include <iostream>
#include <vector>
#include <thread>
#include "fractal.h"
#include "equationparser.h"
#include "image.h"

using namespace std;

int fail () {
    cout << "Provide all the correct arguments please:" << endl;
    cout << "int resolution, double offset_x, double offset_y, double zoom, string equation, int worker_threads" << endl;
    cout << "Ensure equation has no spaces" << endl;  
    return 1;
}

void thread_main (equation *e/*, int thread_num*/) {
    int y = 0/*thread_num*/;
    int x = 0;
    

    // TODO: Main execution
}

// Arguments: int resolution, double offset_x, double offset_y, double zoom, string equation, int worker_threads
int main (int argc, char *argv[]) {
    if (argc != 7) {
        cout << argc << endl;
        return fail ();
    }

    int resolution = stoi (argv[1]);
    double offset_x = stod (argv[2]);
    double offset_y = stod (argv[3]);
    double zoom = stod (argv[4]);
    string *eq = new string (argv[5]);
    int worker_threads = stoi (argv[6]);
    image img = image (resolution, resolution);

    cout << "Parsing equation: \"" << *eq + "\"" << endl;
    equation *main_equation = extract_equation (*eq);

    vector<thread> thread_pool;

    for (int i = 0; i < worker_threads; i++) {
        std::thread t(thread_main, ref(main_equation));
        thread_pool.push_back (t);
    }

    for (auto& th : thread_pool) th.join();
    img.write("out.png");
    return 0;
}