#include <complex>

#ifndef FRACTAL_H
#define FRACTAL_H

using namespace std;

class equation;

bool is_infinity (complex<double> comp);

class value {
public:
    complex<double> cVal;
    equation *eVal;
    bool lVal; // true - z, false - c
    int type; // 0 - Constant, 1 - letter substitution, 2 - sub-equation

    value (complex<double> c);

    value (double c);

    value (bool l);

    value (equation *e);
    
    value ();
};

class equation {
public:
    value a;
    value b;
    int operation;

    complex<double> compute (complex<double> z, complex<double> c);
    int evaluate (complex<double> c, int limit, float threshold=1.0);

    equation (value aVal, value bVal, int op);
    equation ();
};

std::ostream & operator<<(std::ostream & Str, equation const & v);

#endif