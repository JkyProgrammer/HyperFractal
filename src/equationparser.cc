#include "equationparser.h"
#include <vector>
#include <iostream>

int count_occurrences (string s, char c) {
    int o = 0;
    for (char ch : s) if (ch == c) o++;
    return o;
}

string lower (string s) {
    string r = "";
    for (char c : s) {
        if ('A' <= c && c <= 'Z') r.push_back (c-'A'+'a');
        else r.push_back (c);
    }
    return r;
}

vector<string> components (string seq) {
    vector<string> r;
    string current = "";
    int encased = 0;
    for (char ch : seq) {
        if (encased == 0 && (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^')) {
            r.push_back(current);
            current = "";
            current.push_back (ch);
            r.push_back (current);
            current = "";
        } else if (ch != ' ') current.push_back (ch);
        if (ch == '(') {encased++;} // if (encased == 1) {r.push_back(current); current = "";}}
        if (ch == ')') {encased--;} // if (encased == 0) {r.push_back(current); current = "";}}
    }
    r.push_back(current);
    return r;
}

equation* extract_equation (string sequ) {
    string seq = lower(sequ);
    // while (seq[0] == '(' && seq[seq.length()-1] == ')') {
    //     seq = seq.substr (1, seq.length()-2);
    // }
    //TODO: Fix edge case where (z^2) errors (prune uncessarry brackets)
    //TODO: Fix case of something+(0) which gives all black screen
    int brackets = count_occurrences (seq, '(');
    if (brackets != count_occurrences (seq, ')')) return NULL; /*throw std::runtime_error("enter a valid equation: bracket mismatch");*/
    equation *e = new equation();
    vector<string> compos = components(seq);
    
    if (compos.size() != 3) return NULL; /*throw std::runtime_error("enter a valid equation: malformed expression:" + seq);*/
    switch (compos[1][0]) {
    case '+':
        e->operation = 0;
        break;
    case '-':
        e->operation = 1;
        break;
    case '*':
        e->operation = 2;
        break;
    case '/':
        e->operation = 3;
        break;
    case '^':
        e->operation = 4;
        break;
    default:
        return NULL; /*throw std::runtime_error("enter a valid equation: invalid operation");*/
        break;
    }

    if (compos[0].length() == 0 || compos[2].length() == 0) return NULL;
    // Process first component:
    if (compos[0].find('(') != -1)
        e->a = value (extract_equation(compos[0].substr(1,compos[0].length()-2)));
    else if (compos[0] == "z") e->a = value (true);
    else if (compos[0] == "c") e->a = value (false);
    else if (compos[0].find('i') != -1) e->a = value (complex<long double>(0,stod(compos[0].substr(0,compos[0].length()-1))));
    else e->a = value ((long double)stod(compos[0]));

    // Process second component:
    if (compos[2].find('(') != -1)
        e->b = value (extract_equation(compos[2].substr(1,compos[2].length()-2)));
    else if (compos[2] == "z") e->b = value (true);
    else if (compos[2] == "c") e->b = value (false);
    else if (compos[2].find('i') != -1) e->b = value (complex<long double>(0,stod(compos[2].substr(0,compos[2].length()-1))));
    else e->b = value ((long double)stod(compos[2]));
    return e;
}