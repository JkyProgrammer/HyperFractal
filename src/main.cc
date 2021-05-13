#include <iostream>
#include "fractal.cc"

using namespace std;

int main () {
    equation e = equation (value(true), value (false), 0); // TODO: FIX
    cout << e.compute(complex<double>(5), complex<double>(25)) << endl;
    cout << "Hello, World" << endl;
    return 0;
}