#include <complex>

#ifndef FRACTAL_H
#define FRACTAL_H

using namespace std;

class equation;

bool is_infinity (complex<long double> comp);

class value {
public:
    complex<long double> cVal;
    equation *eVal;
    bool lVal; // true - z, false - c
    int type; // 0 - Constant, 1 - letter substitution, 2 - sub-equation

    value (complex<long double> c);

    value (long double c);

    value (bool l);

    value (equation *e);
    
    value ();
};

class equation {
public:
    value a;
    value b;
    int operation;

    complex<long double> compute (complex<long double> z, complex<long double> c);
    int evaluate (complex<long double> c, int limit);

    equation (value aVal, value bVal, int op);
    equation ();
};

std::ostream & operator<<(std::ostream & Str, equation const & v);

#endif