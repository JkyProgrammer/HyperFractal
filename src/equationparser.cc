#include "equationparser.h"
#include <vector>
#include <iostream>
// TODO: Rewrite using a postfix system
using namespace std;

int count_occurrences (string s, char c) {
    int o = 0;
    for (char ch : s) if (ch == c) o++;
    return o;
}

int expectedElementType (char c) {
    if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '.') {
        return 2;
    } else if (c == 'i') return 3;
    else if (c == 'z') return 0;
    else if (c == 'c') return 1;
    else if (c == '(' || c == ')') return 4;
    else if (c == '*' || c == '/' || c == '+' || c == '-' || c == '^') return 5;
    else throw runtime_error ("you put a dud character in: " + string{c});
    return -1;
}

string fixExplicitMultiplication (string s) {
    string innerCorrected = s;

    // First fix all inner brackets
    int start = 0;
    start = s.find ('(', start);
    while (start != string::npos) {
        int end = start;
        int bDepth = 1;
        while (bDepth > 0 && end < innerCorrected.length()) {
            end++;
            if (innerCorrected[end] == '(') bDepth++;
            if (innerCorrected[end] == ')') bDepth--;
        }
        string inner = innerCorrected.substr (start+1, end-(start+1));
        string corrected = fixExplicitMultiplication (inner);
        innerCorrected.replace (start+1, end-(start+1), corrected);
        start = end + (corrected.length()-inner.length());
        start = innerCorrected.find ('(', start+1);
    }

    innerCorrected = fixDuplicateBrackets (innerCorrected);
    // TODO: Remove uncessesarry stacked (nested) brackets

    // Convert implicit multiplication to explicit
    string r = "";
    for (int charIndex = 0; charIndex < innerCorrected.length()-1; charIndex++) {
        
        // Match the currently examined char-pair with an implicit multiplcation pattern
        
        // TODO: Find the start of an element
        // TODO: Find the end
        // TODO: Find the end of the next element
        // TODO: If there's implicit multiplication between them, bracket them and add *
        // TODO: Otherwise, skip to the end of the element
    }
    return r;
}

string prepare (string s) {
    // Convert the input to lowercase and remove spaces
    string lowercased = "";
    for (char c : s) {
        if ('A' <= c && c <= 'Z') lowercased.push_back (c-'A'+'a');
        else if (c != ' ') lowercased.push_back (c);
    }

    string r = fixExplicitMultiplication (lowercased);

    return r;
}

vector<string> components (string seq) {
    vector<string> r;
    string current = "";
    int encased = 0;
    int i = 0;
    //cout << "getting components from " << seq << endl;
    //cout << "here1" << endl;
    for (char ch : seq) {
        if (encased == 0 && (ch == '+' || (ch == '-' && i != 0)  || ch == '*' || ch == '/' || ch == '^')) {
            r.push_back(current);
            current = "";
            current.push_back (ch);
            r.push_back (current);
            current = "";
        } else if (ch != ' ') current.push_back (ch);
        if (ch == '(') {encased++;} // if (encased == 1) {r.push_back(current); current = "";}}
        if (ch == ')') {encased--;} // if (encased == 0) {r.push_back(current); current = "";}}
        i++;
    }
    //cout << "here2" << endl;
    r.push_back(current);
    //for (string s : r) cout << s << endl;
    return r;
}



int findEndBracket (string s, int startOffset) {
    int bracketDepth = 1;
    for (int i = startOffset+1; i < s.length(); i++) {
        if (s[i] == '(') bracketDepth++;
        if (s[i] == ')') bracketDepth--;
        if (bracketDepth == 0) return i;
    }
    return -1;
}

vector<string> getPostfixFrom (string s) {
    cout << "grabbing postfix from: \'" << s << "\'" << endl;
    vector<string> postfix;
    int charIndex = 0;
    bool isWaitingForExpression = false;
    string currentElement = "";
    string operand = "";
    int currentElementType = -1;
    int sLen = s.length();

    while (charIndex < sLen) {
        
        cout << "STATUS" << endl;
        cout << "charIndex: " << charIndex << endl;
        cout << "isWaitingForExpression: " << isWaitingForExpression << endl;
        cout << "currentElement: " << currentElement << endl;
        
        int nextElType = expectedElementType (s[charIndex]);
        if (nextElType == 4) {
            int bracketEndIndex = findEndBracket (s, charIndex);
            string sub = s.substr(charIndex+1, bracketEndIndex-charIndex-1);
            vector<string> inner = getPostfixFrom (sub);
            postfix.insert (postfix.end(), inner.begin(), inner.end());
            if (isWaitingForExpression) {
                currentElement = "";
                currentElementType = -1;
                postfix.push_back (operand);
                isWaitingForExpression = false;
            }
            charIndex = bracketEndIndex;
            currentElement = "";
            currentElementType = -1;
        } else if (nextElType == 5) {
            if (currentElement != "") postfix.push_back (currentElement);
            currentElement = "";
            currentElementType = -1;
            if (isWaitingForExpression) {
                postfix.push_back (operand);
            }
            isWaitingForExpression = true;
            operand = "";
            operand += s[charIndex];
        } else {
            if (currentElementType == -1) currentElementType = nextElType;
            if (nextElType == 3) {
                if (currentElementType == 2 || currentElementType == 3 || currentElementType == -1) currentElementType = 3;
                else throw runtime_error ("you can't put an i there!");
            }
            currentElement += s[charIndex];
        }

        charIndex++;

        if (charIndex == sLen && currentElement != "") {
            postfix.push_back (currentElement);
            currentElement = "";
            currentElementType = -1;
            postfix.push_back (operand);
        }
    }
    
    return postfix;
}

equation* extract_equation (string sequ) {
    string seq = prepare(sequ);
    int brackets = count_occurrences (seq, '(');
    if (brackets != count_occurrences (seq, ')')) return NULL; /*throw std::runtime_error("enter a valid equation: bracket mismatch");*/
    equation *e = new equation ();
    vector<string> postfix = getPostfixFrom (seq);
    for (string s : postfix) cout << s << endl;
    return e;
}