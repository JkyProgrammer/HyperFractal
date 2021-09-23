#include <complex>
#include <chrono>

#ifndef FRACTAL_H
#define FRACTAL_H

using namespace std;

class equation;

bool is_infinity (complex<long double> comp);

struct timing_data {
    chrono::microseconds d_compute;
    chrono::microseconds d_isinf;
    chrono::microseconds d_math;
    chrono::microseconds d_evaluate;
    chrono::microseconds d_get;
    chrono::microseconds d_set;
};

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
    int evaluate (complex<long double> c, int limit, timing_data *d_time);

    equation (value aVal, value bVal, int op);
    equation ();
};

std::ostream & operator<<(std::ostream & Str, equation const & v);

#endif