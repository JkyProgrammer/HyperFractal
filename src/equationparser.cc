#include "equationparser.hh"
#include <vector>
#include <iostream>

using namespace std;

// TODO: Modulus

void coutToken (IntermediateToken t) {
    switch (t.type) {
    case INT_NUMBER:
        cout << t.num_val << endl;
        break;
    case INT_LETTER:
        cout << t.let_val << endl;
        break;
    case INT_OPERATION:
        cout << t.op_val << endl;
        break;
    case INT_BRACKET:
        cout << "(" << endl;
        for (IntermediateToken t2 : t.bracket_val) {
            coutToken (t2);
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
string epClean (string s) {
    string ret_val = "";
    for (char c : s) if (c != ' ') ret_val += c;
    return ret_val;
}

/**
 * @brief Check that the input string is valid for the HFractalEquation parser to analyse. Checks for the following and returns an integer accordingly:
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
EP_CHECK_STATUS epCheck (string s) {
    int bracket_depth = 0;
    char c_last = '\0';
    int index = 0;
    for (char c : s) {
        switch (c) {
        case '(':
            bracket_depth++;
            if (c_last == '.') return FPOINT_ERROR;
            break;
        case ')':
            bracket_depth--;
            if (c_last == '(') return BRACKET_ERROR;
            if (c_last == '.') return FPOINT_ERROR;
            break;
        case 'z':
        case 'c':
        case 'a':
        case 'b':
        case 'x':
        case 'y':
        case 'i':
            if (c_last == '.') return FPOINT_ERROR;
            break;
        case '*':
        case '/':
        case '+':
        case '^':
            if (c_last == '*' || c_last == '/' || c_last == '-' || c_last == '+' || c_last == '^') return OPERATION_ERROR;
            if (c_last == '.') return FPOINT_ERROR;
            if (index == 0 || index == s.length()-1) return OPERATION_ERROR;
            break;
        case '-':
            if (c_last == '-') return OPERATION_ERROR;
            if (c_last == '.') return FPOINT_ERROR;
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
            if (c_last == 'z' || c_last == 'c' || c_last == 'i' || c_last == 'a' || c_last == 'b' || c_last == 'x' || c_last == 'y') return IMULT_ERROR;
            break;
        case '.':
            if (!(c_last == '0' || c_last == '1' || c_last == '2' || c_last == '3' || c_last == '4' || c_last == '5' || c_last == '6' || c_last == '7' || c_last == '8' || c_last == '9')) return FPOINT_ERROR;
            break;
        default:
            return UNSUPCHAR_ERROR;
            break;
        }
        
        c_last = c;
        index++;
        if (bracket_depth < 0) return BRACKET_ERROR;
    }

    if (bracket_depth != 0) return BRACKET_ERROR;
    return SUCCESS;
}

/**
 * @brief Break string into tokens for processing. Assumes epCheck has been called on `s` previously and that this has returned 0
 * 
 * @param s Input string
 * @return std::vector of tokens
 */
vector<IntermediateToken> epTokenise (string s) {
    vector<IntermediateToken> token_vec;
    string current_token = "";
    int current_token_type = -1;
    bool is_last_run = false;
    bool is_singular_token = false; // Informs the program whether the token is a single-char token

    for (int i = 0; i < s.length(); i++) {
        char current_char = s[i];
        int char_token_type = -1;
        
        // Decide the type of the current character
        switch (current_char) {
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
            char_token_type = 0;
            break;
        case 'z':
            char_token_type = 1;
            break;
        case 'c':
            char_token_type = 2;
            break;
        case 'a':
            char_token_type = 6;
            break;
        case 'b':
            char_token_type = 7;
            break;
        case 'x':
            char_token_type = 8;
            break;
        case 'y':
            char_token_type = 9;
            break;
        case 'i':
            char_token_type = 3;
            break;
        case '*':
        case '/':
        case '-':
        case '+':
        case '^':
            char_token_type = 4;
            break;
        case '(':
            char_token_type = 5;
            break;
        default:
            break;
        }

        // Save the current token the token vector
        if (char_token_type != current_token_type || is_last_run || is_singular_token) {
            is_singular_token = false;
            if (current_token.length () > 0) {
                IntermediateToken token;
                switch (current_token_type) {
                case 0:
                    token.type = INT_NUMBER;
                    token.num_val = stod (current_token);
                    break;
                case 1:
                case 2:
                case 3:
                case 6:
                case 7:
                case 8:
                case 9:
                    token.type = INT_LETTER;
                    token.let_val = current_token[0];
                    break;
                case 4:
                    token.type = INT_OPERATION;
                    token.op_val = current_token[0];
                    break;
                case 5:
                    token.type = INT_BRACKET;
                    token.bracket_val = epTokenise (current_token);
                default:
                    break;
                }
                token_vec.push_back (token);
            }
            current_token = "";
            current_token_type = char_token_type;

            if (is_last_run) break;
        }

        // Jump automatically to the end of the brackets, recursively processing their contents
        if (char_token_type == 5) {
            int bracket_depth = 1;
            int end = -1;
            for (int j = i+1; j < s.length(); j++) {
                if (s[j] == '(') bracket_depth++;
                if (s[j] == ')') bracket_depth--;
                if (bracket_depth == 0) { end = j; break; }
            }

            current_token = s.substr (i+1, end-(i+1));
            i = end;
        } else { // Otherwise append the current character to the current token
            current_token += s[i];
        }

        // Mark a, b, c, x, y, z, and i as singular
        if ((char_token_type >= 1 && char_token_type <= 3) || (char_token_type >= 6 && char_token_type <= 9)) {
            is_singular_token = true;
        }

        // If we've reached the end of the string, jump back and mark it as a last pass in order to save the current token
        if (i == s.length()-1) {
            is_last_run = true;
            i--;
        }
    }

    // Check through and repair any `-...` expressions to be `0-...`
    for (int i = 0; i < token_vec.size(); i++) {
        if (token_vec[i].type == INT_OPERATION && token_vec[i].op_val == '-') {
            if (i == 0 || (i > 0 && token_vec[i-1].type == INT_OPERATION)) {
                IntermediateToken bracket;
                bracket.type = INT_BRACKET;
                int bracket_length = 1;
                bracket.bracket_val.push_back ({
                    .type = INT_NUMBER,
                    .num_val = 0
                });
                bracket.bracket_val.push_back ({
                    .type = INT_OPERATION,
                    .op_val = '-'
                });
                while (token_vec[i+bracket_length].type != INT_OPERATION) {
                    bracket.bracket_val.push_back (token_vec[i+bracket_length]);
                    bracket_length++;
                }

                for (int tmp = 0; tmp < bracket_length; tmp++) token_vec.erase (next(token_vec.begin(), i));
                token_vec.insert (next(token_vec.begin(), i), bracket);
                i -= bracket_length-1;
            }
        }
    }

    return token_vec;
}

/**
 * @brief Search for and replace implicit multiplication (adjacent non-operation tokens such as '5z' or '(...)(...)') with explicit multiplication
 * 
 * @param token_vec Token vector to fix
 * @return Token vector with no implicit multiplication
 */
vector<IntermediateToken> epFixImplicitMul (vector<IntermediateToken> token_vec) {
    vector<IntermediateToken> result = token_vec;
    for (int i = 0; i < result.size()-1; i++) {
        IntermediateToken t1 = result[i];
        IntermediateToken t2 = result[i+1];

        // Fix implicit multiplication within brackets
        if (t1.type == INT_BRACKET) {
            result[i].bracket_val = epFixImplicitMul (t1.bracket_val);
            t1 = result[i];
        }

        // Detect two adjacent tokens, where neither is an operation
        if (!(t1.type == INT_OPERATION || t2.type == INT_OPERATION)) {
            result.erase (next(result.begin(), i));
            result.erase (next(result.begin(), i));
            IntermediateToken explicit_mul;
            explicit_mul.type = INT_BRACKET;
            explicit_mul.bracket_val.push_back (t1);
            explicit_mul.bracket_val.push_back ({
                .type = INT_OPERATION,
                .op_val = '*'
            });
            explicit_mul.bracket_val.push_back (t2);

            result.insert (next(result.begin(), i), explicit_mul);
            i--;
        }
    }

    IntermediateToken last = result[result.size()-1];
    if (last.type == INT_BRACKET) {
        last.bracket_val = epFixImplicitMul (last.bracket_val);
        result[result.size()-1] = last;
    }
    
    return result;
}

/**
 * @brief Ensure BIDMAS (Brackets Indices Division Multiplication Addition Subtraction) order mathematical evaluation by search-and-replacing each with brackets
 * 
 * @param token_vec Token vector to simplify
 * @return Token vector which requires only sequential evaluation
 */
vector<IntermediateToken> epSimplifyBidmas (vector<IntermediateToken> token_vec, bool first_half) {
    vector<IntermediateToken> result = token_vec;
    // Recurse down brackets
    for (int i = 0; i < result.size(); i++) {
        if (result[i].type == INT_BRACKET) {
            result[i].bracket_val = epSimplifyBidmas (result[i].bracket_val, first_half);
        }
    }

    // Order of operations:
    //           IDMAS
    string ops = "^/*+-";

    // First half allows the parser to make indices and division explicit, then process implicit multiplication, and then to process other operations
    // This allows us to maintain BIDMAS even with explicit multiplication (e.g. `5z^2` should be `5*(z^2)` and not `(5*z)^2`)
    if (first_half) {
        ops = "^/";
    } else {
        ops = "*+-";
    }

    if (result.size() < 5) return result;

    // Search and replace each sequentially
    for (char c : ops) {
        for (int t_ind = 0; t_ind < result.size()-2; t_ind++) {
            if (t_ind >= result.size()-2) {
                break;
            }
            if (result[t_ind+1].type == INT_OPERATION && result[t_ind+1].op_val == c) {
                IntermediateToken bracket;
                bracket.type = INT_BRACKET;
                bracket.bracket_val.push_back (result[t_ind]);
                bracket.bracket_val.push_back (result[t_ind+1]);
                bracket.bracket_val.push_back (result[t_ind+2]);

                for (int tmp = 0; tmp < 3; tmp++) result.erase (next(result.begin(), t_ind));
                result.insert (next(result.begin(), t_ind), bracket);
                t_ind -= 2;
            }
        }
    }

    return result;
}

/**
 * @brief Convert intermediate token vector into usable Reverse Polish Notation token queue
 * Ensure that everything else is done before calling this function:
 *  epClean
 *  epTokenise
 *  epSimplifyBidmas first_half=true
 *  epFixImplicitMul
 *  epSimplifyBidmas first_half=false
 * These functions ensure the token vector is ready to be linearly parsed into Reverse Polish
 * 
 * @param intermediate Token vector to convert
 * @return Vector of proper tokens, ready to use in the expression evaluator
 */
vector<token> epReversePolishConvert (vector<IntermediateToken> intermediate) {
    vector<token> output;
    
    IntermediateToken operation = {.op_val = '\0'};

    for (int index = 0; index < intermediate.size(); index++) {
        IntermediateToken current_intermediate_token = intermediate[index];
        if (current_intermediate_token.type == INT_OPERATION) {
            operation.op_val = current_intermediate_token.op_val;
        } else {
            // Append to token(s) rp notation
            if (current_intermediate_token.type == INT_BRACKET) {
                vector<token> inner_result = epReversePolishConvert (current_intermediate_token.bracket_val);
                output.insert (output.end(), inner_result.begin(), inner_result.end());
            } else {
                output.push_back ({
                    .type = (TOKEN_TYPE)current_intermediate_token.type,
                    .num_val = current_intermediate_token.type == INT_NUMBER ? current_intermediate_token.num_val : 0,
                    .other_val = current_intermediate_token.type == INT_LETTER ? current_intermediate_token.let_val : '\0'
                });
            }

            // If relevant, append operation to rp notation
            if (operation.op_val != '\0') {
                output.push_back ({
                    .type = OPERATION,
                    .other_val = operation.op_val
                });
                operation.op_val = '\0';
            }
        }
    }

    return output;
}

/**
 * @brief Convert a string mathematical expression into an HFractalEquation class instance using Reverse Polish Notation
 * 
 * @param sequ String containing a mathematical expression to parse
 * @return Pointer to an HFractalEquation instance representing the input string
 */
HFractalEquation* extract_equation (string sequ) {
    if (sequ.length() < 1) return NULL;
    string cleaned = epClean (sequ);
    if (epCheck (cleaned) != SUCCESS) return NULL;

    vector<IntermediateToken> expression = epTokenise (cleaned);

    expression = epSimplifyBidmas (expression, true);
    expression = epFixImplicitMul (expression);
    expression = epSimplifyBidmas (expression, false);

    vector<token> reverse_polish_expression = epReversePolishConvert (expression);

    return new HFractalEquation (reverse_polish_expression);
}