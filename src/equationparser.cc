#include "equationparser.h"
#include <vector>
#include <iostream>

using namespace std;

// TODO: Modulus

enum intermediate_token_type {
    INT_NUMBER,
    INT_LETTER,
    INT_OPERATION,
    INT_BRACKET
};

struct intermediate_token {
    intermediate_token_type type;
    double numVal;
    char letVal;
    char opVal;
    vector<intermediate_token> bracketval;
};

void cout_token (intermediate_token t) {
    switch (t.type) {
    case INT_NUMBER:
        cout << t.numVal << endl;
        break;
    case INT_LETTER:
        cout << t.letVal << endl;
        break;
    case INT_OPERATION:
        cout << t.opVal << endl;
        break;
    case INT_BRACKET:
        cout << "(" << endl;
        for (intermediate_token t2 : t.bracketval) {
            cout_token (t2);
        }
        cout << ")" << endl;
        break;
    default:
        break;
    }
}

/**
 * @brief Clean whitespace out of the input string
 * 
 * @param s Input string
 * @return Cleaned string
 */
string ep_clean (string s) {
    string retVal = "";
    for (char c : s) if (c != ' ') retVal += c;
    return retVal;
}

enum ep_check_status {
    SUCCESS,
    BRACKET_ERROR,
    OPERATION_ERROR,
    IMULT_ERROR,
    FPOINT_ERROR,
    UNSUPCHAR_ERROR
};

/**
 * @brief Check that the input string is valid for the equation parser to analyse. Checks for the following and returns an integer accordingly:
 * 
 * 0 - No error found
 * 1 - Bracket error: '()', '(' not equal number to ')', ')...('
 * 2 - Operation error: '**', '*-' or any other repetition of an operation
 * 3 - Implicit multiplication error: 'z2' rather than correct synax '2z'
 * 4 - Floating point error: '.46', '34.'
 * 5 - Unsupported character error: '$', 'd', or any other character not accounted for
 * 
 * @param s Input string
 * @return Either the reference of the first error detected or zero if no error is found
 */
ep_check_status ep_check (string s) {
    int bracketDepth = 0;
    char cLast = '\0';
    int index = 0;
    for (char c : s) {
        switch (c) {
        case '(':
            bracketDepth++;
            if (cLast == '.') return FPOINT_ERROR;
            break;
        case ')':
            bracketDepth--;
            if (cLast == '(') return BRACKET_ERROR;
            if (cLast == '.') return FPOINT_ERROR;
            break;
        case 'z':
        case 'c':
        case 'a':
        case 'b':
        case 'x':
        case 'y':
        case 'i':
            if (cLast == '.') return FPOINT_ERROR;
            break;
        case '*':
        case '/':
        case '+':
        case '^':
            if (cLast == '*' || cLast == '/' || cLast == '-' || cLast == '+' || cLast == '^') return OPERATION_ERROR;
            if (cLast == '.') return FPOINT_ERROR;
            if (index == 0 || index == s.length()-1) return OPERATION_ERROR;
            break;
        case '-':
            if (cLast == '-') return OPERATION_ERROR;
            if (cLast == '.') return FPOINT_ERROR;
            if (index == s.length()-1) return OPERATION_ERROR;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if (cLast == 'z' || cLast == 'c' || cLast == 'i' || cLast == 'a' || cLast == 'b' || cLast == 'x' || cLast == 'y') return IMULT_ERROR;
            break;
        case '.':
            if (!(cLast == '0' || cLast == '1' || cLast == '2' || cLast == '3' || cLast == '4' || cLast == '5' || cLast == '6' || cLast == '7' || cLast == '8' || cLast == '9')) return FPOINT_ERROR;
            break;
        default:
            return UNSUPCHAR_ERROR;
            break;
        }
        
        cLast = c;
        index++;
        if (bracketDepth < 0) return BRACKET_ERROR;
    }

    if (bracketDepth != 0) return BRACKET_ERROR;
    return SUCCESS;
}

/**
 * @brief Break string into tokens for processing. Assumes ep_check has been called on `s` previously and that this has returned 0
 * 
 * @param s Input string
 * @return std::vector of tokens
 */
vector<intermediate_token> ep_tokenise (string s) {
    vector<intermediate_token> tokenVec;
    string currentToken = "";
    int currentTokenType = -1;
    bool isLastRun = false;

    for (int i = 0; i < s.length(); i++) {
        char currentChar = s[i];
        int charTokenType = -1;
        
        // Decide the type of the current character
        switch (currentChar) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
            charTokenType = 0;
            break;
        case 'z':
            charTokenType = 1;
            break;
        case 'c':
            charTokenType = 2;
            break;
        case 'a':
            charTokenType = 6;
            break;
        case 'b':
            charTokenType = 7;
            break;
        case 'x':
            charTokenType = 8;
            break;
        case 'y':
            charTokenType = 9;
            break;
        case 'i':
            charTokenType = 3;
            break;
        case '*':
        case '/':
        case '-':
        case '+':
        case '^':
            charTokenType = 4;
            break;
        case '(':
            charTokenType = 5;
            break;
        default:
            break;
        }

        // Save the current token the token vector
        if (charTokenType != currentTokenType || isLastRun) {
            if (currentToken.length () > 0) {
                intermediate_token token;
                switch (currentTokenType) {
                case 0:
                    token.type = INT_NUMBER;
                    token.numVal = stod (currentToken);
                    break;
                case 1:
                case 2:
                case 3:
                case 6:
                case 7:
                case 8:
                case 9:
                    token.type = INT_LETTER;
                    token.letVal = currentToken[0];
                    break;
                case 4:
                    token.type = INT_OPERATION;
                    token.opVal = currentToken[0];
                    break;
                case 5:
                    token.type = INT_BRACKET;
                    token.bracketval = ep_tokenise (currentToken);
                default:
                    break;
                }
                tokenVec.push_back (token);
            }
            currentToken = "";
            currentTokenType = charTokenType;

            if (isLastRun) break;
        }

        // Jump automatically to the end of the brackets, recursively processing their contents
        if (charTokenType == 5) {
            int bracketDepth = 1;
            int end = -1;
            for (int j = i+1; j < s.length(); j++) {
                if (s[j] == '(') bracketDepth++;
                if (s[j] == ')') bracketDepth--;
                if (bracketDepth == 0) { end = j; break; }
            }

            currentToken = s.substr (i+1, end-(i+1));
            i = end;
        } else { // Otherwise append the current character to the current token
            currentToken += s[i];
        }

        // If we've reached the end of the string, jump back and mark it as a last pass in order to save the current token
        if (i == s.length()-1) {
            isLastRun = true;
            i--;
        }
    }

    // Check through and repair any `-...` expressions to be `0-...`
    for (int i = 0; i < tokenVec.size(); i++) {
        if (tokenVec[i].type == INT_OPERATION && tokenVec[i].opVal == '-') {
            if (i == 0 || (i > 0 && tokenVec[i-1].type == INT_OPERATION)) {
                intermediate_token bracket;
                bracket.type = INT_BRACKET;
                int bracketLength = 1;
                bracket.bracketval.push_back ({
                    .type = INT_NUMBER,
                    .numVal = 0
                });
                bracket.bracketval.push_back ({
                    .type = INT_OPERATION,
                    .opVal = '-'
                });
                while (tokenVec[i+bracketLength].type != INT_OPERATION) {
                    bracket.bracketval.push_back (tokenVec[i+bracketLength]);
                    bracketLength++;
                }

                for (int tmp = 0; tmp < bracketLength; tmp++) tokenVec.erase (next(tokenVec.begin(), i));
                tokenVec.insert (next(tokenVec.begin(), i), bracket);
                i -= bracketLength-1;
            }
        }
    }

    return tokenVec;
}

/**
 * @brief Search for and replace implicit multiplication (adjacent non-operation tokens such as '5z' or '(...)(...)') with explicit multiplication
 * 
 * @param tokenVec Token vector to fix
 * @return Token vector with no implicit multiplication
 */
vector<intermediate_token> ep_fixImplicitMul (vector<intermediate_token> tokenVec) {
    vector<intermediate_token> result = tokenVec;
    for (int i = 0; i < result.size()-1; i++) {
        intermediate_token t1 = result[i];
        intermediate_token t2 = result[i+1];

        // Fix explicit multiplication within brackets
        if (t1.type == INT_BRACKET) {
            result[i].bracketval = ep_fixImplicitMul (t1.bracketval);
            t1 = result[i];
        }

        // Detect two adjacent tokens, where neither is an operation
        if (!(t1.type == INT_OPERATION || t2.type == INT_OPERATION)) {
            result.erase (next(result.begin(), i));
            result.erase (next(result.begin(), i));

            intermediate_token explicitMul;
            explicitMul.type = INT_BRACKET;
            explicitMul.bracketval.push_back (t1);
            explicitMul.bracketval.push_back ({
                .type = INT_OPERATION,
                .opVal = '*'
            });
            explicitMul.bracketval.push_back (t2);

            result.insert (next(result.begin(), i), explicitMul);
            i--;
        }
    }

    intermediate_token last = result[result.size()-1];
    if (last.type == INT_BRACKET) {
        last.bracketval = ep_fixImplicitMul (last.bracketval);
        result[result.size()-1] = last;
    }
    
    return result;
}

/**
 * @brief Ensure BIDMAS (Brackets Indices Division Multiplication Addition Subtraction) order mathematical evaluation by search-and-replacing each with brackets
 * 
 * @param tokenVec Token vector to simplify
 * @return Token vector which requires only sequential evaluation
 */
vector<intermediate_token> ep_simplifyBidmas (vector<intermediate_token> tokenVec, bool firstHalf) {
    vector<intermediate_token> result = tokenVec;
    // Recurse down brackets
    for (int i = 0; i < result.size(); i++) {
        if (result[i].type == INT_BRACKET) {
            result[i].bracketval = ep_simplifyBidmas (result[i].bracketval, firstHalf);
        }
    }

    // Order of operations:
    //           IDMAS
    string ops = "^/*+-";

    // First half allows the parser to make indices and division explicit, then process implicit multiplication, and then to process other operations
    // This allows us to maintain BIDMAS even with explicit multiplication (e.g. `5z^2` should be `5*(z^2)` and not `(5*z)^2`)
    if (firstHalf) {
        ops = "^/";
    } else {
        ops = "*+-";
    }

    if (result.size() < 5) return result;

    // Search and replace each sequentially
    for (char c : ops) {
        for (int tInd = 0; tInd < result.size()-2; tInd++) {
            if (tInd >= result.size()-2) {
                break;
            }
            if (result[tInd+1].type == INT_OPERATION && result[tInd+1].opVal == c) {
                intermediate_token bracket;
                bracket.type = INT_BRACKET;
                bracket.bracketval.push_back (result[tInd]);
                bracket.bracketval.push_back (result[tInd+1]);
                bracket.bracketval.push_back (result[tInd+2]);

                for (int tmp = 0; tmp < 3; tmp++) result.erase (next(result.begin(), tInd));
                result.insert (next(result.begin(), tInd), bracket);
                tInd -= 2;
            }
        }
    }

    return result;
}

/**
 * @brief Convert intermediate token vector into usable Reverse Polish Notation token queue
 * Ensure that everything else is done before calling this function:
 *  ep_clean
 *  ep_tokenise
 *  ep_simplifyBidmas firstHalf=true
 *  ep_fixImplicitMul
 *  ep_simplifyBidmas firstHalf=false
 * These functions ensure the token vector is ready to be linearly parsed into Reverse Polish
 * 
 * @param intermediate Token vector to convert
 * @return Vector of proper tokens, ready to use in the expression evaluator
 */
vector<token> ep_rpConvert (vector<intermediate_token> intermediate) {
    vector<token> output;
    
    intermediate_token operation = {.opVal = '\0'};

    for (int index = 0; index < intermediate.size(); index++) {
        intermediate_token currentITok = intermediate[index];
        if (currentITok.type == INT_OPERATION) {
            operation.opVal = currentITok.opVal;
        } else {
            // Append to token(s) rp notation
            if (currentITok.type == INT_BRACKET) {
                vector<token> innerResult = ep_rpConvert (currentITok.bracketval);
                output.insert (output.end(), innerResult.begin(), innerResult.end());
            } else {
                output.push_back ({
                    .type = (token_type)currentITok.type,
                    .numVal = currentITok.type == INT_NUMBER ? currentITok.numVal : 0,
                    .otherVal = currentITok.type == INT_LETTER ? currentITok.letVal : '\0'
                });
            }

            // If relevant, append operation to rp notation
            if (operation.opVal != '\0') {
                output.push_back ({
                    .type = OPERATION,
                    .otherVal = operation.opVal
                });
                operation.opVal = '\0';
            }
        }
    }

    return output;
}

/**
 * @brief Convert a string mathematical expression into an equation class instance using Reverse Polish Notation
 * 
 * @param sequ String containing a mathematical expression to parse
 * @return Pointer to an equation instance representing the input string
 */
equation* extract_equation (string sequ) {
    if (sequ.length() < 1) return NULL;
    string cleaned = ep_clean (sequ);
    if (ep_check (cleaned) != SUCCESS) return NULL;

    vector<intermediate_token> expression = ep_tokenise (cleaned);

    expression = ep_simplifyBidmas (expression, true);
    expression = ep_fixImplicitMul (expression);
    expression = ep_simplifyBidmas (expression, false);

    vector<token> reversePolishExpression = ep_rpConvert (expression);

    return new equation (reversePolishExpression);
}