/*
Basic expression evaluation from string
note: the number of cases warrents the switch statements, even if there may be up to 126 jump table entries

note: operator precedence from https://cc65.github.io/doc/ca65.html

Written by Adam Billings
*/

#ifndef ExpressionEvaluation_h
#define ExpressionEvaluation_h

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "DataStructures/Stack.h"
#include "DataStructures/StringTable.h"

// used to determine the type of parse
enum ParseType {
    op, dec, hex, bin, chr, var
};

// return type for list with error encoding
typedef struct ExprErrorShort {
    uint16_t val;
    int errorPos;
    int errorLen;
    char* errorMessage;
} ExprErrorShort;

/*
outputs the precedence of the operator

opchar: operator to examine

returns: operator precedence, -1 if invalid
*/
static char getPrec(char opChar) {
    switch(opChar) {
        case '(':
        case ')':
            return 10;
        case (char)('+' + 0x80):  // unary
        case (char)('-' + 0x80):  // unary
        case '~':
        case (char)('<' + 0x80):  // unary
        case (char)('>' + 0x80):  // unary
        case '!':
            return 1;
        case '*':
        case '/':
        case (char)('/' + 0x80):  // "//"
        case '&':
        case '^':
        case (char)('<' + 0x87):  // "<<"
        case (char)('>' + 0x87):  // ">>"
            return 2;
        case '+':
        case '-':
        case '|':
            return 3;
        case '=':                 // "=="
        case (char)('=' + 0x80):  // "!="
        case '<':
        case '>':
        case (char)('<' + 0x8f):  // "<="
        case (char)('>' + 0x8f):  // ">="
            return 4;
        case (char)('&' + 0x80):  // "&&"
        case (char)('^' + 0x80):  // "^^"
            return 5;
        case (char)('|' + 0x80):  // "||"
            return 6;
        case '?':
        case ':':
            return 7;
        default:
            return -1;
    }
}

/*
applies a unary operator to the top value of a stack

opChar: operator character
valStack: value stack

returns: if an error occurs
*/
static char applyUnaryOp(char opChar, Stack* valStack) {
    // catch errors
    if (valStack->size == 0) {return 1;}

    // apply the operator
    int16_t* valueP = (int16_t*)peekStack(valStack);
    int16_t value = *valueP;
    switch (opChar) {
        case (char)('+' + 0x80):
            break;
        case (char)('-' + 0x80):
            value = -value;
            break;
        case '~':
            value = ~value;
            break;
        case '!':
            value = !value;
            break;
        case (char)('<' + 0x80):
            value = value & 0x00ff;
            break;
        case (char)('>' + 0x80):
            value = (value >> 8) & 0x00ff;
            break;
    }
    *valueP = value;
    return 0;
}

/*
applies a binary operation char

opChar: operation symbol
valStack: value stack

returns: if an error occured
*/
static char applyBinaryOp(char opChar, Stack* valStack, Stack* condStack) {
    // catch errors
    if (valStack->size < 2) {return 1;}

    // apply the operation
    uint16_t* rightP = (uint16_t*)popStack(valStack);
    uint16_t* leftP = (uint16_t*)peekStack(valStack);
    uint16_t right = *rightP;
    uint16_t left = *leftP;
    free(rightP);
    switch (opChar) {
        case '*':
            left *= right;
            break;
        case '/':
            if (right == 0) {return 1;}
            left /= right;
            break;
        case (char)('/' + 0x80):  // "//"
            if (right == 0) {return 1;}
            left %= right;
            break;
        case '&':
            left &= right;
            break;
        case '^':
            left ^= right;
            break;
        case (char)('<' + 0x87):  // "<<"
            left <<= right;
            break;
        case (char)('>' + 0x87):  // ">>"
            left >>= right;
            break;
        case '+':
            left += right;
            break;
        case '-':
            left -= right;
            break;
        case '|':
            left |= right;
            break;
        case '=':                 // "=="
            left = left == right;
            break;
        case (char)('=' + 0x80):  // "!="
            left = left != right;
            break;
        case '<':
            left = left < right;
            break;
        case '>':
            left = left > right;
            break;
        case (char)('<' + 0x8f):  // "<="
            left = left <= right;
            break;
        case (char)('>' + 0x8f):  // ">="
            left = left >= right;
            break;
        case (char)('&' + 0x80):  // "&&"
            left = left && right;
        case (char)('^' + 0x80):  // "^^"
            left = (left || right) && !(left && right);
            break;
        case (char)('|' + 0x80):  // "||"
            left = left || right;
            break;
        case ':':
            if (condStack->size == 0) {return 1;}
            uint16_t* condP = (uint16_t*)popStack(condStack);
            left = (*condP) ? left : right;
            free(condP);
            break;
        default:
            return 1;
    }
    *(uint16_t*)peekStack(valStack) = left;
    return 0;
}

/*
Applies the operation on top of the opStack to the value(s) on tope of the valStack

opStack: operator stack
valStack: values stack

returns: if an error occured
*/
static char applyOp(Stack* opStack, Stack* valStack, Stack* condStack) {
    // handle nop case
    if (opStack->size == 0) {return 0;}

    // apply operators
    char* opCharP = (char*)(popStack(opStack));
    char opChar = *opCharP;
    free(opCharP);
    if (opChar == (char)('+' + 0x80) || opChar == (char)('-' + 0x80) || opChar == '~' || opChar == (char)('<' + 0x80) || opChar == (char)('>' + 0x80) || opChar == '!') {
        return applyUnaryOp(opChar, valStack);
    } else if (opChar == '?') {
        // error check
        if (valStack->size == 0) {return 1;}

        // no need to evaluate, will always be evaluated
        uint16_t* condVal = (uint16_t*)popStack(valStack);
        pushStack(condStack, condVal, 2);
        free(condVal);
        return 0;
    }

    return applyBinaryOp(opChar, valStack, condStack);
}

/*
parses an integer with the given radix, then adds it to a value stack

expr: expression to parse
index: index to parse from
radix: base to parse in
valStack: stack to push to

returns: new expression index
*/
static int parseUShort(char* expr, int index, int radix, Stack* valStack) {
    char* uShortParseEnd;
    uint16_t uShortParse = (uint16_t)strtol(expr + index, &uShortParseEnd, radix);
    pushStack(valStack, &uShortParse, 2);
    return (uShortParseEnd - expr);
}

/*
checks if a character can be in a variable name

c: character to check

returns: if the character can be in a variable name
*/
static char charIsName(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')  || c == '_' || c == '@');
}

/*
returns a variable name in a new string and updates the string pointer to after the variable name
*/
static char* extractVar(char** exprptr, int exprLen) {
    for (int i = 0; i < exprLen; i++) {
        if (!charIsName((*exprptr)[i])) {
            // create the output string
            char* outputStr = (char*)malloc((i + 1) * sizeof(char));
            memcpy(outputStr, *exprptr, i * sizeof(char));
            outputStr[i] = '\0';

            // update the pointer
            *exprptr += i;

            // output
            return outputStr;
        }
    }
    // create the output string
    char* outputStr = (char*)malloc((exprLen + 1) * sizeof(char));
    memcpy(outputStr, *exprptr, exprLen * sizeof(char));
    outputStr[exprLen] = '\0';

    // update the pointer
    *exprptr += exprLen;

    // output
    return outputStr;
}

/*
Evaluates the expressed short integer expression

expr: expression string
exprLen: maximum length of the expression string
varTable: LUT to get variable values

returns: ErrorShort of evaluation or error message
*/
ExprErrorShort evalShortExpr(char* expr, int exprLen, StringTable varTable, StringTable macroTable) {
    // global variables shared by loops
    int i = 0;
    char lastIsOp = 1;
    Stack* valStack = newStack();
    Stack* opStack = newStack();
    Stack* condStack = newStack();

    // pick apart the string
    while (expr[i] != '\0' && expr[i] != ';' && expr[i] != '\n' && expr[i] != ',' && i < exprLen) {
        // determine the operation mode
        enum ParseType parseType = op;
        if (isWhitespace(expr[i])) {
            i++;
            continue;
        } else if (expr[i] >= '0' && expr[i] <= '9') {parseType = dec;}
        else if (expr[i] == '$') {
            parseType = hex;
            i++;
        } else if (expr[i] == '%') {
            parseType = bin;
            i++;
        } else if (expr[i] == '\'') {
            parseType = chr;
            i++;
        } else if (charIsName(expr[i])) {parseType = var;}
        else if ((expr[i] & 0x80) || expr[i] < 0 || getPrec(expr[i]) < 0) {
            // return an error
            char* errorStr = (char*)malloc(23 * sizeof(char));
            sprintf(errorStr, "Unexpected symbol \'%c\'", expr[i]);
            deleteStack(condStack);
            deleteStack(valStack);
            deleteStack(opStack);
            return (ExprErrorShort){0, i, 1, errorStr};
        }
        
        // prevent "1 1 +" style notation
        if ((parseType == dec || parseType == hex || parseType == bin) && !lastIsOp) {
            char* errorStr = (char*)malloc(18 * sizeof(char));
            sprintf(errorStr, "Expected operator");
            return (ExprErrorShort){0, i, 1, errorStr};
        }

        // run the parse for the operation
        char* lenTracker;
        char* varName;
        int charsExtracted;
        uint16_t* varptr;
        char opParse;
        int opPrec;
        char chrParseLen = 1;
        char chrParseError = 0;
        uint16_t chrParse;
        switch (parseType) {
            case dec:
                i = parseUShort(expr, i, 10, valStack);
                lastIsOp = 0;
                break;
            case hex:
                i = parseUShort(expr, i, 16, valStack);
                lastIsOp = 0;
                break;
            case bin:
                i = parseUShort(expr, i, 2, valStack);
                lastIsOp = 0;
                break;
            case chr:
                if (expr[i] == '\\') {
                    chrParseLen = 2;
                    switch (expr[i + 1]) {
                        case '0':
                            chrParse = '\0';
                            break;
                        case 't':
                            chrParse = '\t';
                            break;
                        case 'n':
                            chrParse = '\n';
                            break;
                        case 'e':
                            chrParse = '\e';
                            break;
                        case '\\':
                            chrParse = '\\';
                            break;
                        case '\"':
                            chrParse = '\"';
                            break;
                        case '\'':
                            chrParse = '\'';
                            break;
                        default:
                            chrParseError = 1;
                            break;
                    }
                } else {chrParse = expr[i];}
                if (expr[i + chrParseLen] != '\'') {chrParseError = 1;}
                if (chrParseError) {
                    deleteStack(condStack);
                    deleteStack(opStack);
                    deleteStack(valStack);
                    char* errorStr = (char*)malloc(26 * sizeof(char));
                    sprintf(errorStr, "Could not parse character");
                    return (ExprErrorShort){0, i - 1, chrParseLen + 1, errorStr};
                }
                pushStack(valStack, &chrParse, 2);
                i += chrParseLen + 1;
                lastIsOp = 0;
                break;
            case var:
                lenTracker = expr + i;
                varName = extractVar(&lenTracker, exprLen - i);
                charsExtracted = (lenTracker - (expr + i));
                varptr = (uint16_t*)readStringTable(macroTable, varName, charsExtracted + 1);
                if (varptr == NULL) {varptr = (uint16_t*)readStringTable(varTable, varName, charsExtracted + 1);}
                if (varptr == NULL) {
                    deleteStack(condStack);
                    deleteStack(opStack);
                    deleteStack(valStack);
                    char* errorStr = (char*)malloc((25 + charsExtracted) * sizeof(char));
                    sprintf(errorStr, "Uninitialized value \"%s\"", varName);
                    return (ExprErrorShort){0, i, charsExtracted, errorStr};
                }
                pushStack(valStack, varptr, 2);
                i += charsExtracted;
                lastIsOp = 0;
                break;
            case op:
                // get the operation
                opParse = expr[i];

                // apply parenteses
                if (opParse == ')') {
                    while (opStack->size > 0 && *(char*)peekStack(opStack) != '(') {
                        applyOp(opStack, valStack, condStack);
                    }
                    if (opStack->size == 0) {
                        deleteStack(condStack);
                        deleteStack(opStack);
                        deleteStack(valStack);
                        char* errorStr = (char*)malloc(27 * sizeof(char));
                        sprintf(errorStr, "Could not parse expression");
                        return (ExprErrorShort){0, 0, exprLen, errorStr};
                    }
                    free(popStack(opStack));
                    i++;
                    break;
                } else if (opParse == '(') {
                    pushStack(opStack, &opParse, sizeof(char));
                    i++;
                    break;
                }

                // determine the exact operation
                if (lastIsOp && (opParse == '+' || opParse == '-' || opParse == '<' || opParse == '>')) {
                    opParse |= 0x80;
                } else if (i < exprLen - 1) {
                    switch (opParse) {
                        case '/':
                            if (expr[i + 1] == '/') {
                                i++;
                                opParse += 0x80;
                            }
                            break;
                        case '>':
                            if (expr[i + 1] == '>') {
                                i++;
                                opParse += 0x87;
                            } else if (expr[i + 1] == '=') {
                                i++;
                                opParse += 0x8f;
                            }
                            break;
                        case '<':
                            if (expr[i + 1] == '<') {
                                i++;
                                opParse += 0x87;
                            } else if (expr[i + 1] == '=') {
                                i++;
                                opParse += 0x8f;
                            }
                        case '!':
                            if (expr[i + 1] == '=') {
                                i++;
                                opParse = (char)('=' + 0x80);
                            }
                            break;
                        case '=':
                            if (expr[i + 1] != '=') {
                                deleteStack(condStack);
                                deleteStack(opStack);
                                deleteStack(valStack);
                                char* errorStr = (char*)malloc(21 * sizeof(char));
                                sprintf(errorStr, "Invalid operator '='");
                                return (ExprErrorShort){0, i, 1, errorStr};
                            }
                            i++;
                            break;
                        case '&':
                            if (expr[i + 1] == '&') {
                                i++;
                                opParse += 0x80;
                            }
                            break;
                        case '|':
                            if (expr[i + 1] == '|') {
                                i++;
                                opParse += 0x80;
                            }
                            break;
                        case '^':
                            if (expr[i + 1] == '^') {
                                i++;
                                opParse += 0x80;
                            }
                        default:
                            break;
                    }
                } else if (opParse == '=') {
                    deleteStack(condStack);
                    deleteStack(opStack);
                    deleteStack(valStack);
                    char* errorStr = (char*)malloc(21 * sizeof(char));
                    sprintf(errorStr, "Invalid operator '='");
                    return (ExprErrorShort){0, i, 1, errorStr};
                }

                // apply higher prececence operations
                opPrec = getPrec(opParse);
                while  (opStack->size > 0 && getPrec(*(char*)peekStack(opStack)) <= opPrec) {
                    if (applyOp(opStack, valStack, condStack)) {
                        deleteStack(condStack);
                        deleteStack(opStack);
                        deleteStack(valStack);
                        char* errorStr = (char*)malloc(27 * sizeof(char));
                        sprintf(errorStr, "Could not parse expression");
                        return (ExprErrorShort){0, 0, exprLen, errorStr};
                    }
                }

                // push to the stack
                pushStack(opStack, &opParse, sizeof(char));
                i++;
                lastIsOp = 1;

                // force evaluate ?
                if (opParse == '?') {
                    if (applyOp(opStack, valStack, condStack)) {
                        deleteStack(condStack);
                        deleteStack(opStack);
                        deleteStack(valStack);
                        char* errorStr = (char*)malloc(27 * sizeof(char));
                        sprintf(errorStr, "Could not parse expression");
                        return (ExprErrorShort){0, 0, exprLen, errorStr};
                    }
                }
                break;
        }
    }

    // more precise error
    if (valStack->size == 0) {
        deleteStack(condStack);
        deleteStack(valStack);
        deleteStack(opStack);
        char* errorStr = (char*)malloc(15 * sizeof(char));
        sprintf(errorStr, "Expected value");
        return (ExprErrorShort){0, 0, exprLen, errorStr};
    }

    // apply remaining operators
    while(opStack->size > 0) {
        int i = applyOp(opStack, valStack, condStack);
        if (i) {
            deleteStack(condStack);
            deleteStack(valStack);
            deleteStack(opStack);
            char* errorStr = (char*)malloc(27 * sizeof(char));
            sprintf(errorStr, "Could not parse expression");
            return (ExprErrorShort){0, 0, exprLen, errorStr};
        }
    }

    // output result
    uint16_t outputVal = *(uint16_t*)peekStack(valStack);
    deleteStack(condStack);
    deleteStack(valStack);
    deleteStack(opStack);
    return (ExprErrorShort){outputVal, 0, 0, NULL};
}

#endif