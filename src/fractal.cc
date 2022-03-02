#include "fractal.hh"

#include <stack>
#include <complex>

#include "utils.hh"

using namespace std;

/**
 * @brief Check if a complex number has tended to infinity. Allows methods which use this check to be implementation independent
 * Tending to infinity is typically defined as |z| > 2, which here is expanded to maximise optimsation
 * 
 * @param comp Complex number to check
 * @return True if the number has tended to infinity, False otherwise
 */
bool HFractalEquation::isInfinity (complex<long double> comp) {
    return (comp.real()*comp.real()) + (comp.imag()*comp.imag()) > (long double)4;
}

/**
 * @brief Set the equation preset value
 * 
 * @param i Integer representing the preset ID, linked with EQ_PRESETS, or -1 to disable preset mode in this instance
 */
void HFractalEquation::setPreset (int i) {
    is_preset = (i != -1);
    preset = i;
}

/**
 * @brief Parse the Reverse Polish notation Token vector and evaluate the mathematical expression it represents
 * 
 * @param z Current value of the z variable to feed in
 * @param c Current value of the c variable to feed in
 * @return Complex number with the value of the evaluated equation
 */
complex<long double> HFractalEquation::compute (complex<long double> z, complex<long double> c) {    
    stack<complex<long double>> value_stack;

    for (Token t : reverse_polish_vector) {
        if (t.type == NUMBER) {
            // Push number arguments onto the stack
            value_stack.push (t.num_val);
        } else if (t.type == LETTER) {
            // Select based on letter and swap in the letter's value, before pushing it onto the stack
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
            // Perform an actual computation based on an operation token
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

    // Return the final value
    return value_stack.top();
}

/**
 * @brief Evaluate a complex coordinate (i.e. a pixel) to find the point at which it tends to infinity, by iteratively applying the equation as a mathematical function
 * 
 * @param c Coordinate in the complex plane to initialise with
 * @param limit Limit for the number of iterations to compute before giving up, if the number does not tend to infinity
 * @return Integer representing the number of iterations performed before the number tended to infinity, or the limit if this was reached first
 */
int HFractalEquation::evaluate (complex<long double> c, int limit) {
    complex<long double> last = c;
    if (is_preset && preset == EQ_BURNINGSHIP_MODIFIED) {
        last = complex<long double> (0, 0);
    }

    int depth = 0;
    while (depth < limit) {
        // Switch between custom parsing mode and preset mode for more efficient computing of presets
        if (!is_preset) {
            last = compute (last, c); // Slow custom compute
        } else {
            // Much faster hard coded computation
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
        // Check if the value has tended to infinity, and escape the loop if so
        bool b = isInfinity (last);
        if (b) break;
    }
    return depth;
}

/**
 * @brief Initialise with the token sequence in postfix form which this class should use
 * 
 * @param rp_vec Reverse Polish formatted vector of tokens
 */
HFractalEquation::HFractalEquation (vector<Token> rp_vec) {
    reverse_polish_vector = rp_vec;
}

/**
 * @brief Base initialiser. Should only be used to construct presets, as the equation token vector cannot be assigned after initialisation
 * 
 */
HFractalEquation::HFractalEquation () {}