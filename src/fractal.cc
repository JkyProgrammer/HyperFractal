#include "fractal.h"
#include <iostream>

bool is_infinity (cpp_complex_512 comp) {
    return pow(comp.real(),2.0) + pow(comp.imag(),2.0) > (long double)2;
}

value::value (cpp_complex_512 c) {
    cVal = c;
    type = 0;
}

value::value (long double c) {
    cVal = cpp_complex_512 {c, 0};
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

cpp_complex_512 equation::compute (cpp_complex_512 z, cpp_complex_512 c) {
    cpp_complex_512 v1;
    cpp_complex_512 v2;
    if (a.type == 0) v1 = a.cVal;
    else if (a.type == 1) v1 = a.lVal ? z : c;
    else v1 = a.eVal->compute(z, c);

    if (b.type == 0) v2 = b.cVal;
    else if (b.type == 1) v2 = b.lVal ? z : c;
    else v2 = b.eVal->compute(z, c);
    cpp_complex_512 rv {0,0};
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
    }
    return rv;
}
using namespace std;
int equation::evaluate (cpp_complex_512 c, int limit, float threshold) {
    std::cout << c << std::endl;
    cpp_complex_512 last = compute (c, c);
    int depth = 0;
    while (abs(last) < 2 && (depth < limit)) {
        depth++;
        last = compute (last, c);
    }
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