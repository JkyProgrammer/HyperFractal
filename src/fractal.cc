#include "fractal.hh"
#include "utils.hh"
#include <iostream>
#include <chrono>
#include <stack>
using namespace std::chrono;
using namespace std;

bool HFractalEquation::isInfinity (complex<long double> comp) {
    return (comp.real()*comp.real()) + (comp.imag()*comp.imag()) > (long double)4;
}

void HFractalEquation::setPreset (int i) {
    is_preset = (i != -1);
    preset = i;
}

complex<long double> HFractalEquation::compute (complex<long double> z, complex<long double> c) {    
    stack<complex<long double>> value_stack;

    for (token t : reverse_polish_vector) {
        if (t.type == NUMBER) {
            value_stack.push (t.num_val);
        } else if (t.type == LETTER) {
            switch (t.other_val) {
            case 'z':
                value_stack.push (z);
                break;
            case 'c':
                value_stack.push (c);
                break;
            case 'a':
                value_stack.push (c.real());
                break;
            case 'b':
                value_stack.push (c.imag());
                break;
            case 'x':
                value_stack.push (z.real());
                break;
            case 'y':
                value_stack.push (z.imag());
                break;
            case 'i':
                value_stack.push (complex<long double> (0,1));
                break;
            default:
                break;
            }
        } else if (t.type == OPERATION) {
            complex<long double> v2 = value_stack.top(); value_stack.pop();
            complex<long double> v1 = value_stack.top(); value_stack.pop();
            switch (t.other_val) {
            case '^':
                value_stack.push (pow (v1, v2));
                break;
            case '/':
                value_stack.push (v1/v2);
                break;
            case '*':
                value_stack.push (v1*v2);
                break;
            case '+':
                value_stack.push (v1+v2);
                break;
            case '-':
                value_stack.push (v1-v2);
                break;
            default:
                break;
            }
        }
    }

    return value_stack.top();
}

int HFractalEquation::evaluate (complex<long double> c, int limit) {
    complex<long double> last = c;
    if (is_preset && preset == EQ_BURNINGSHIP_MODIFIED) {
        last = complex<long double> (0, 0);
    }

    int depth = 0;
    while (depth < limit) {
        // Switch between custom parsing mode and preset mode for more efficient computing of presets
        if (!is_preset) {
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
        bool b = isInfinity (last);
        if (b) break;
    }
    return depth;
}

HFractalEquation::HFractalEquation (vector<token> rp_vec) {
    reverse_polish_vector = rp_vec;
}

HFractalEquation::HFractalEquation () {}

// std::ostream & operator<<(std::ostream & Str, HFractalEquation const & v) { 
//     for (token t : v.reverse_polish_vector) {
//         switch (t.type) {
//         case LETTER:
//         case OPERATION:
//             Str << t.other_val;
//             break;
//         case NUMBER:
//             Str << t.num_val;
//             break;
//         default:
//             break;
//         }
//         //Str << endl;
//     }
//     return Str;
// }