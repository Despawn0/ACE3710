/*
All needed code to create "variables"

Written by Adam Billings
*/

#ifndef VarEvaluation_h
#define VarEvaluation_h

#include "ExpressionEvaluation.h"
#include "DataStructures/List.h"
#include "DataStructures/StringTable.h"
#include "MiscAssembler.h"
#include "ConfigReader.h"
#include "ProcessMacros.h"
#include <stdio.h>

// stores data to evaluate a variable
typedef struct VarEvalData {
    List* dependencies;
    List* defines;
    char* expr;
    int exprLen;
    int line;
    int col;
    FileHandle* handle;
} VarEvalData;

// stores define data
typedef struct DefData {
    char* name;
    uint16_t val;
    char canFree;
    char hasValue;
} DefData;

/*
enumerates the variables in an expression

expr: expression to evaluate
exprLen: length of the expression

returns: list of variables in the expression
*/
static List* getVars(char* expr, int exprLen);

/*
evaluates a variable assignment

toEvaluate: name to resolve
toEvaluateLut: LUT to get other vars to evaluate
parse: output parse information

returns: if an error occured
*/
static char evaluateVar(List* errorList, char* toEvaluate, StringTable toEvaluateLut, StringTable varDefs, StringTable macroDefs, List* visited);

/*
evaluates all local variables between global vars

handle: handle to the file to read; no lines over 255 chars
errorList: list of errors
handleList: list of open handles
segments: segments defined in the configuration
instructionSize: size of the instructions in "words"

returns: parsed global vars
*/
StringTable readGlobalVars(FileHandle* handle, List* errorList, List* handleList, List* segments, StringTable macroDefs, int instructionSize);

/*
evaluates all global variables in a file

handle: handle to the file to read; no lines over 255 chars
errorList: list of errors
handleList: list of open handles
segments: segments defined in the configuration
instructionSize: size of the instructions in "words"
localVars: list of defined local vars
lineCount: number of lines read
includeStack_: include stack to copy
ifStack_: if stack to copy
segStack_: segment stack to copy
macroStack_ macroStack to copy

returns: parsed global vars
*/
char readLocalVars(FileHandle* handle, List* errorList, List* handleList, List* segments, StringTable macroDefs, StringTable varDefs, StringTable defines, int instructionSize, List* localVars, SegmentDef* activeSeg, unsigned int lineCount, Stack* includeStack_, Stack* ifStack_, Stack* segStack_, Stack* macroStack_);

#endif