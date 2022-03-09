// src/fractal.hh

#ifndef FRACTAL_H
#define FRACTAL_H

#include <complex>
#include <vector>

// Enum describing the token type
enum TOKEN_TYPE {
    NUMBER,
    LETTER,
    OPERATION
};

// Struct describing the token
struct Token {
    TOKEN_TYPE type;
    double num_val;
    char other_val;
};

// Class holding the equation and providing functions to evaluate it
class HFractalEquation {
private:
    static bool isInfinity (std::complex<long double> comp); // Check if a complex number has exceeded the 'infinity' threshold
    std::vector<Token> reverse_polish_vector; // Sequence of equation tokens in postfix form

    bool is_preset = false; // Records whether this equation is using an equation preset
    int preset = -1; // Records the equation preset being used, if none, set to -1

public:
    void setPreset (int); // Set this equation to be a preset, identified numerically

    std::complex<long double> compute (std::complex<long double>, std::complex<long double>); // Perform a single calculation using the equation and the specified z and c values
    int evaluate (std::complex<long double>, int); // Perform the fractal calculation 

    HFractalEquation (std::vector<Token>); // Initialise with a sequence of equation tokens
    HFractalEquation (); // Base initialiser
};

#endif