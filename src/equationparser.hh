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

class HFractalEquationParser {
private:
    static void coutToken (IntermediateToken);
    static std::string epClean (std::string);
    static EP_CHECK_STATUS epCheck (std::string);
    static std::vector<IntermediateToken> epTokenise (std::string);
    static std::vector<IntermediateToken> epFixImplicitMul (std::vector<IntermediateToken>);
    static std::vector<IntermediateToken> epSimplifyBidmas (std::vector<IntermediateToken>, bool);
    static std::vector<token> epReversePolishConvert (std::vector<IntermediateToken>);

public:
    static HFractalEquation* extract_equation (std::string);
};


#endif