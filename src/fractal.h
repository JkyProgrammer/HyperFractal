#include <complex>
#include <chrono>
#include <vector>

#ifndef FRACTAL_H
#define FRACTAL_H

using namespace std;

enum token_type {
    NUMBER,
    LETTER,
    OPERATION
};

struct token {
    token_type type;
    double numVal;
    char otherVal;
};

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

class equation {
public:
    vector<token> reversePolishVector;

    bool isPreset = false;
    int preset = -1;

    complex<long double> compute (complex<long double> z, complex<long double> c);
    int evaluate (complex<long double> c, int limit, timing_data *d_time);

    equation (vector<token> rpVec);
    equation ();
};

std::ostream & operator<<(std::ostream & Str, equation const & v);

#endif