#include <complex>

#ifndef FRACTAL_H
#define FRACTAL_H

#include <boost/multiprecision/cpp_complex.hpp>

using cpp_complex_512 = boost::multiprecision::cpp_complex<100, boost::multiprecision::backends::digit_base_10, void, std::int32_t, -262142, 262143>;

using namespace std;

class equation;

bool is_infinity (cpp_complex_512 comp);

class value {
public:
    cpp_complex_512 cVal;
    equation *eVal;
    bool lVal; // true - z, false - c
    int type; // 0 - Constant, 1 - letter substitution, 2 - sub-equation

    value (cpp_complex_512 c);

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

    cpp_complex_512 compute (cpp_complex_512 z, cpp_complex_512 c);
    int evaluate (cpp_complex_512 c, int limit, float threshold=1.0);

    equation (value aVal, value bVal, int op);
    equation ();
};

std::ostream & operator<<(std::ostream & Str, equation const & v);

#endif