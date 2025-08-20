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
returns a variable name in a new string and updates the string pointer to after the variable name
*/
char* extractVar(char** exprptr, int exprLen);

/*
Evaluates the expressed short integer expression

expr: expression string
exprLen: maximum length of the expression string
varTable: LUT to get variable values

returns: ErrorShort of evaluation or error message
*/
ExprErrorShort evalShortExpr(char* expr, int exprLen, StringTable varTable, StringTable macroTable);

#endif