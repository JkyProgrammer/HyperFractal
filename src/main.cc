#include <iostream>
#include "fractal.h"
#include "equationparser.h"

using namespace std;

int fail () {
    cout << "Provide all the correct arguments please:" << endl;
    cout << "int resolution, double offset_x, double offset_y, double zoom, string equation" << endl;
    cout << "Ensure equation has no spaces" << endl;  
    return 1;
}

// Arguments: int resolution, double offset_x, double offset_y, double zoom, string equation
int main (int argc, char *argv[]) {
    if (argc != 6) {
        cout << argc << endl;
        return fail ();
    }

    int resolution = stoi (argv[1]);
    double offset_x = stod (argv[2]);
    double offset_y = stod (argv[3]);
    double zoom = stod (argv[4]);
    string *eq = new string (argv[5]);

    cout << *eq << endl;

    equation *e1 = new equation (value(true), value (2.0), 4);
    equation *e = new equation (value(e1), value(false), 0);
    cout << e->evaluate(complex<double>(0), 100) << endl;
    cout << "Hello, World" << endl;
    return 0;
}