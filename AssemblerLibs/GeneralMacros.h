/*
general-purpose code used for macros in all sweeps

Written by Adam Billings
*/

#ifndef GeneralMacros_h
#define GeneralMacros_h

#include <string.h>
#include <stdlib.h>
#include "MiscAssembler.h"
#include "ExpressionEvaluation.h"
#include "DataStructures/StringTable.h"
#include "DataStructures/Stack.h"

// information needed to find a macro
typedef struct MacroDefData {
    FileHandle* handle;
    long start;
    long end;
    unsigned int line;
    unsigned int lines;
    List* vars;
} MacroDefData;

// information needed to track if macros
typedef struct PosData {
    unsigned int line;
    unsigned int col;
} PosData;

// information needed to track include nesting
typedef struct IncludeReturnData {
    FileHandle* returnFile;
    long filePosition;
    unsigned int returnLine;
    unsigned int errorCount;
} IncludeReturnData;

/*
determines if a character can be in a valid name

c: character to evaluate

returns: if the character can be in a valid name
*/
char isValidMacroChar(char c);

/*
extracts a macro name from a line

line: line to read
lineLength: maximum length of the line
outputPos: output position for after the macro name

returns: macro name or NULL
*/
char* extractMacro(char* line, int lineLength, char** outputPos);

/*
extracts the arguments for a .macro definition

line: line to read
lineLength: maximum length of the line
afterArgs: output of the rest of the string

returns: list of macro arguments
*/
List* extractMacroArgs(char* line, int lineLength, char** afterArgs);

/*
reads a string from the line

line: line to read from, expects string at the start
lineLength: length of the line
afterString: output written to for the next char
outLen: length output

returns: extracted string
*/
char* readString(char* line, unsigned int lineLength, char** afterString, unsigned int* outLen);

/*
attempts to get the handle to an open file

handleList: list to pull from
name: name of the file
isBin: if the file handle is binary

returns: handle to open file or NULL
*/
FileHandle* getHandle(List* handleList, char* name, char isBin);

/*
reads to the end of an if statement

handle: handle to the file
errorList: list of errors
ifData: location data for the if statement
allowElse: if .else and .elseif macros are allowed

returns: end line to the if statement
*/
char* skipIf(FileHandle* handle, List* errorList, PosData* ifData, char allowElse);

/*
returns to the preveous file upon the end of .include

includeStack: stack of include returns
lineptr: pointer to line to update

returns: pointer to handle to update
*/
FileHandle* includeReturn(Stack* includeStack, unsigned int* lineptr);

/*
determines if an "if" condition was met

handle: file handle
errorList: list of errors
name: macro to use
expr: expression to use
exprLen: length of the expression
line: current line being parsed
col: current column being parsed
defines: current defined vars

returns: if the condition is true
*/
char macroIf(FileHandle* handle, List* errorList, char* name, char* expr, unsigned int exprLen, int line, int col, StringTable defines);

/*
counts the args in a macro

line: line to evaluate
len: length of the line
*/
unsigned int countArgs(char* line, unsigned int len);

/*
determines if a macro name is valid

name: macro name

returns: if the name is valid
*/
char isValidMacroName(char* name);

#endif