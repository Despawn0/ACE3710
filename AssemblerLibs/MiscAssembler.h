/*
misc. assembler structs and functions

Written by Adam Billings
*/

#ifndef AssemblerStructures_h
#define AssemblerStructures_h

#include <stdio.h>
#include <ctype.h>
#include "DataStructures/List.h"

// package of file read information
typedef struct FileHandle {
    FILE* fptr;
    char* name;
    long length;
    char isBin;
} FileHandle;

// package of error information
typedef struct ErrorData {
    char* errorMsg;
    int line;
    int col;
    int len;
    FileHandle* handle;
} ErrorData;

/*
gets the directory a file is stored in from the canonical path

path: canonical file path
returns: directory
*/
char* getDir(char* path);

/*
prints an error message

errorData: error to print
*/
char printError(ErrorData errorData);

/*
determines if a character can be in a valid name

c: character to evaluate

returns: if the character can be in a valid name
*/
char isValidNameChar(char c);

/*
extracts the name of a variable from the start of a line

line: line to extract from
lineLength: maximum length of the line

returns: variable name, NULL if no valid name
*/
char* getVarName(char* line, int lineLength, char** outputPos);

/*
counts the number of whitespace characters at the start of the line

line: line to evaluate
lineLength: length of the line

returns: number of whitespace characters
*/
unsigned int countWhitespaceChars(char* line, unsigned int lineLength);

/*
determines if a file has valid line sizes (prevent buffer overflow)

handle: handle to the file
errorList: list to write errors to

returns: if an error occured
*/
char validateFile(FileHandle* handle, List* errorList);

/*
determines if the line ending is valid

line: line to evaluate
lineLenght: maximum length of line ending

returns: if the line ending is valid for a macro
*/
char isValidLineEnding(char* line, unsigned int lineLength);

/*
extracts all values in an argument list

line: line to parse
length: length of the line

returns: list of arguments
*/
List* extractArgs(char* line, unsigned int length);

#endif