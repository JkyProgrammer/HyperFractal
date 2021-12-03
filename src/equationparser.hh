#include "fractal.hh"
#include <string>
#include <vector>

#ifndef EQUATIONPARSER_H
#define EQUATIONPARSER_H

enum INTERMEDIATE_TOKEN_TYPE {
    INT_NUMBER,
    INT_LETTER,
    INT_OPERATION,
    INT_BRACKET
};

struct IntermediateToken {
    INTERMEDIATE_TOKEN_TYPE type;
    double num_val;
    char let_val;
    char op_val;
    vector<IntermediateToken> bracket_val;
};

enum EP_CHECK_STATUS {
    SUCCESS,
    BRACKET_ERROR,
    OPERATION_ERROR,
    IMULT_ERROR,
    FPOINT_ERROR,
    UNSUPCHAR_ERROR
};

void coutToken (IntermediateToken);

std::string epClean (std::string);

EP_CHECK_STATUS epCheck (std::string);

std::vector<IntermediateToken> epTokenise (std::string);

std::vector<IntermediateToken> epFixImplicitMul (std::vector<IntermediateToken>);

std::vector<IntermediateToken> epSimplifyBidmas (std::vector<IntermediateToken>, bool);

std::vector<token> epReversePolishConvert (std::vector<IntermediateToken>);

HFractalEquation* extract_equation (std::string);



#endif