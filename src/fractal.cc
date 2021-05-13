#include <complex>

using namespace std;

class equation;

class value {
public:
    complex<double> cVal;
    equation *eVal;
    bool lVal; // true - z, false - c
    int type; // 0 - Constant, 1 - letter substitution, 2 - sub-equation

    value (complex<double> c) {
        cVal = c;
        type = 0;
    }

    value (bool l) {
        lVal = l;
        type = 1;
    }

    value (equation *e) {
        eVal = e;
        type = 2;
    }
    
    value () {

    }
};

class equation {
public:
    value a;
    value b;
    int operation;

    complex<double> compute (complex<double> z, complex<double> c) {
        complex<double> v1;
        complex<double> v2;
        if (a.type == 0) v1 = a.cVal;
        else if (a.type == 1) v1 = a.lVal ? z : c;
        else v1 = a.eVal->compute(z, c);

        if (b.type == 0) v2 = b.cVal;
        else if (b.type == 1) v1 = b.lVal ? z : c;
        else v2 = b.eVal->compute(z, c);

        complex<double> rv = 0;
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

    equation (value aVal, value bVal, int op) {
        operation = op;
        a = aVal;
        b = bVal;
    }
};