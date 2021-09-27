#include "fractal.h"
#include <iostream>
#include <chrono>
using namespace std::chrono;

bool is_infinity (complex<long double> comp) {
    return (abs(comp.real()) + abs(comp.imag())) >= 2;
    //return pow(comp.real(),2.0) + pow(comp.imag(),2.0) > (long double)4;
}

value::value (complex<long double> c) {
    cVal = c;
    type = 0;
}

value::value (long double c) {
    cVal = complex<long double> (c, 0);
    type = 0;
}

value::value (bool l) {
    lVal = l;
    type = 1;
}

value::value (equation *e) {
    eVal = e;
    type = 2;
}

value::value () {

}

complex<long double> equation::compute (complex<long double> z, complex<long double> c) {
    complex<long double> v1;
    complex<long double> v2;
    switch (a.type) {
    case 0:
        v1 = a.cVal;
        break;
    case 1:
        v1 = a.lVal ? z : c;
        break;
    case 2:
        v1 = a.eVal->compute(z, c);
        break;
    default:
        return 0;
        break;
    }
    
    switch (b.type) {
    case 0:
        v2 = b.cVal;
        break;
    case 1:
        v2 = b.lVal ? z : c;
        break;
    case 2:
        v2 = b.eVal->compute(z, c);
        break;
    default:
        return 0;
        break;
    }

    complex<long double> rv = 0;
    switch (operation) {
    case 0:
        rv = v1+v2;
        break;
    case 1:
        rv = v1-v2;
        break;
    case 2:
        rv = v1*v2;
        break;
    case 3:
        rv = v1/v2;
        break;
    case 4:
        rv = pow(v1, v2);
        break;
    }
    return rv;
}

using namespace std;
int equation::evaluate (complex<long double> c, int limit, timing_data *d_time) {
    //microseconds d_compute = microseconds(0);
    //microseconds d_isinf = microseconds(0);

    complex<long double> last = c;
    int depth = 0;
    while (depth < limit) {
        //auto t_a = high_resolution_clock::now();
        last = compute (last, c);
        depth++;
        //auto t_b = high_resolution_clock::now();
        bool b = is_infinity (last);
        //auto t_c = high_resolution_clock::now();
        //d_compute += duration_cast<microseconds> (t_b-t_a);
        //d_isinf += duration_cast<microseconds> (t_c-t_b);
        if (b) break;
    }
    //d_time->d_compute += d_compute;
    //d_time->d_isinf += d_isinf;
    return depth;
}

equation::equation (value aVal, value bVal, int op) {
    operation = op;
    a = aVal;
    b = bVal;
}

equation::equation () {}

#define OPS "+-*/^"

std::ostream & operator<<(std::ostream & Str, equation const & v) { 
  if (v.a.type == 0) Str << v.a.cVal;
  if (v.a.type == 1) Str << (v.a.lVal ? "Z" : "C");
  if (v.a.type == 2) Str << "(" << *v.a.eVal << ")";
  Str << " " << OPS[v.operation] << " ";
  if (v.b.type == 0) Str << v.b.cVal;
  if (v.b.type == 1) Str << (v.b.lVal ? "Z" : "C");
  if (v.b.type == 2) Str << "(" << *v.b.eVal << ")";
  return Str;
}