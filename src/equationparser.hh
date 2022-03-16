// src/equationparser.hh

#ifndef EQUATIONPARSER_H
#define EQUATIONPARSER_H

#include <string>
#include <vector>

#include "fractal.hh"

// Enum describing the token type for the intermediate parser
enum INTERMEDIATE_TOKEN_TYPE {
    INT_NUMBER,
    INT_LETTER,
    INT_OPERATION,
    INT_BRACKET
};

// Struct describing the token for the intermediate parser
struct IntermediateToken {
    INTERMEDIATE_TOKEN_TYPE type;
    double num_val;
    char let_val;
    char op_val;
    std::vector<IntermediateToken> bracket_val;
};

// Enum describing the error types from the equation processor checking function
enum EP_CHECK_STATUS {
    SUCCESS,
    BRACKET_ERROR,
    OPERATION_ERROR,
    IMULT_ERROR,
    FPOINT_ERROR,
    UNSUPCHAR_ERROR
};

// Class containing static methods used to parse a string into a postfix token vector
class HFractalEquationParser {
private:
    static std::string epClean (std::string); // Preprocess the string to remove whitespace
    static EP_CHECK_STATUS epCheck (std::string); // Check for formatting errors in the equation (such as mismatched brackets)
    static std::vector<IntermediateToken> epTokenise (std::string); // Split the string into intermediate tokens
    static std::vector<IntermediateToken> epFixImplicitMul (std::vector<IntermediateToken>); // Remove implicit multiplication
    static std::vector<IntermediateToken> epSimplifyBidmas (std::vector<IntermediateToken>, bool); // Convert BIDMAS rules into explicit writing
    static std::vector<Token> epReversePolishConvert (std::vector<IntermediateToken>); // Convert intermediate tokens into a final output postfix notation

public:
    static HFractalEquation* extractEquation (std::string); // Extract an equation containing postfix tokens from a string input
};


#endif