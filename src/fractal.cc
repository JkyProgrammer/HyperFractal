#include "fractal.h"
#include "utils.h"
#include <iostream>
#include <chrono>
#include <stack>
using namespace std::chrono;
using namespace std;

bool is_infinity (complex<long double> comp) {
    //return (abs(comp.real()) + abs(comp.imag())) >= 2;
    return (comp.real()*comp.real()) + (comp.imag()*comp.imag()) > (long double)4;
}

complex<long double> equation::compute (complex<long double> z, complex<long double> c) {    
    stack<complex<long double>> valueStack;

    for (token t : reversePolishVector) {
        if (t.type == NUMBER) {
            valueStack.push (t.numVal);
        } else if (t.type == LETTER) {
            switch (t.otherVal) {
            case 'z':
                valueStack.push (z);
                break;
            case 'c':
                valueStack.push (c);
                break;
            case 'a':
                valueStack.push (c.real());
                break;
            case 'b':
                valueStack.push (c.imag());
                break;
            case 'x':
                valueStack.push (z.real());
                break;
            case 'y':
                valueStack.push (z.imag());
                break;
            case 'i':
                valueStack.push (complex<long double> (0,1));
                break;
            default:
                break;
            }
        } else if (t.type == OPERATION) {
            complex<long double> v2 = valueStack.top(); valueStack.pop();
            complex<long double> v1 = valueStack.top(); valueStack.pop();
            switch (t.otherVal) {
            case '^':
                valueStack.push (pow (v1, v2));
                break;
            case '/':
                valueStack.push (v1/v2);
                break;
            case '*':
                valueStack.push (v1*v2);
                break;
            case '+':
                valueStack.push (v1+v2);
                break;
            case '-':
                valueStack.push (v1-v2);
                break;
            default:
                break;
            }
        }
    }

    return valueStack.top();
}

int equation::evaluate (complex<long double> c, int limit, timing_data *d_time) {
    //microseconds d_compute = microseconds(0);
    //microseconds d_isinf = microseconds(0);
    complex<long double> last = c;
    if (isPreset && preset == EQ_BURNINGSHIP_MODIFIED) {
        last = complex<long double> (0, 0);
    }

    int depth = 0;
    while (depth < limit) {
        //auto t_a = high_resolution_clock::now();
        // Switch between custom parsing mode and preset mode for more efficient computing of presets
        if (!isPreset) {
            last = compute (last, c);
        } else {
            switch (preset) {
            case EQ_MANDELBROT:
                last = (last*last)+c;
                break;
            case EQ_JULIA_1:
                last = (last*last)+complex<long double>(0.285, 0.01);
                break;
            case EQ_JULIA_2:
                last = (last*last)-complex<long double>(0.70176, 0.3842);
                break;
            case EQ_RECIPROCAL:
                last = complex<long double>(1,0)/((last*last)+c);
                break;
            case EQ_ZPOWER:
                last = pow(last,last)+c-complex<long double>(0.5, 0);
                break;
            case EQ_BARS:
                last = pow(last, c*c);
                break;
            case EQ_BURNINGSHIP_MODIFIED:
                last = pow ((complex<long double>(abs(last.real()),0) - complex<long double>(0, abs(last.imag()))),2)+c;
                break;
            default:
                break;
            }
        }
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

equation::equation (vector<token> rpVec) {
    reversePolishVector = rpVec;
}

equation::equation () {}

#define OPS "+-*/^"

std::ostream & operator<<(std::ostream & Str, equation const & v) { 
    for (token t : v.reversePolishVector) {
        switch (t.type) {
        case LETTER:
        case OPERATION:
            Str << t.otherVal;
            break;
        case NUMBER:
            Str << t.numVal;
            break;
        default:
            break;
        }
        //Str << endl;
    }
    return Str;
}