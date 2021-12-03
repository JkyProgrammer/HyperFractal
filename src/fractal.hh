#include <complex>
#include <chrono>
#include <vector>

#ifndef FRACTAL_H
#define FRACTAL_H

using namespace std;

enum TOKEN_TYPE {
    NUMBER,
    LETTER,
    OPERATION
};

struct token {
    TOKEN_TYPE type;
    double num_val;
    char other_val;
};

class HFractalEquation;

bool isInfinity (complex<long double> comp);

struct TimingData {
    chrono::microseconds d_compute;
    chrono::microseconds d_isinf;
    chrono::microseconds d_math;
    chrono::microseconds d_evaluate;
    chrono::microseconds d_get;
    chrono::microseconds d_set;
};

class HFractalEquation {
public:
    vector<token> reverse_polish_vector;

    bool is_preset = false;
    int preset = -1;

    complex<long double> compute (complex<long double>, complex<long double>);
    int evaluate (complex<long double>, int, TimingData*);

    HFractalEquation (vector<token>);
    HFractalEquation ();
};

std::ostream & operator<<(std::ostream & Str, HFractalEquation const & v);

#endif