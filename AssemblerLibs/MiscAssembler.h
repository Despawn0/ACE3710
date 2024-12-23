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
    fpos_t length;
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
prints an error message

errorData: error to print
*/
void printError(ErrorData errorData) {
    // calculate the formatting to the error lime
    char errorLine[errorData.col + errorData.len + 1];
    for (int i = 0; i < errorData.col; i++) {errorLine[i] = ' ';}
    for (int j = errorData.col; j < errorData.col + errorData.len; j++) {errorLine[j] = '^';}
    errorLine[errorData.col + errorData.len] = '\0';

    // make digit padding
    char digitCounter[15];
    sprintf(digitCounter, "%d", errorData.line + 1);
    for (int i = 0; i < 15; i++) {
        if (digitCounter[i] == '\0') {break;}
        digitCounter[i] = ' ';
    }

    // get the line
    fpos_t restorePoint;
    char lineBuffer[257];
    fgetpos(errorData.handle->fptr, &restorePoint);
    rewind(errorData.handle->fptr);
    for (int i = 0; i < errorData.line; i++) {
        fgets(lineBuffer, 257, errorData.handle->fptr);
        while (strlen(lineBuffer) > 255) {fgets(lineBuffer, 257, errorData.handle->fptr);}
    }
    fgets(lineBuffer, 256, errorData.handle->fptr);
    fsetpos(errorData.handle->fptr, &restorePoint);

    // delete trailing newline
    int lineSize = strlen(lineBuffer);
    if (lineBuffer[lineSize - 1] == '\n') {
        lineBuffer[lineSize - 1] = '\0';
    }

    // replace '\t' with ' '
    for (int i = 0; i < lineSize; i++) {
        if (lineBuffer[i] == '\t') {lineBuffer[i] = ' ';}
    }

    // print the message
    printf("\e[1m\e[31mERROR:\e[0;1m %s\e[0m\n", errorData.handle->name);
    printf("  %d |\t%s\n", errorData.line + 1, lineBuffer);
    printf("  %s |\t\e[31m%s\e[0m\n", digitCounter, errorLine);
    printf("  %s |\t\e[31m%s\e[0m\n\n", digitCounter, errorData.errorMsg);
}

/*
determines if a character can be in a valid name

c: character to evaluate

returns: if the character can be in a valid name
*/
char isValidNameChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

/*
determines if a character is whitespace

c: charater to evaluate

returns: if the character is whitespace
*/
char isWhitespace(char c) {
    return isspace(c);
}

/*
extracts the name of a variable from the start of a line

line: line to extract from
lineLength: maximum length of the line

returns: variable name, NULL if no valid name
*/
char* getVarName(char* line, int lineLength, char** outputPos) {
    for (int i = 0; i < lineLength; i++) {
        if (i == 0 && line[i] == '@') {continue;}
        if (i == 0 && ((line[i] >= '0' && line[i] <= '9') || !isValidNameChar(line[i]))) {return NULL;}
        if (!isValidNameChar(line[i])) {
            char* retVal = (char*)malloc((i + 1) * sizeof(char));
            memcpy(retVal, line, i * sizeof(char));
            retVal[i] = '\0';
            *outputPos = line + i;
            return retVal;
        }
    }
    return NULL;
}

/*
counts the number of whitespace characters at the start of the line

line: line to evaluate
lineLength: length of the line

returns: number of whitespace characters
*/
unsigned int countWhitespaceChars(char* line, unsigned int lineLength) {
    for (int i = 0; i < lineLength; i++) {
        if (!isWhitespace(line[i])) {return i;}
    }
    return lineLength;
}

/*
determines if a file has valid line sizes (prevent buffer overflow)

handle: handle to the file
errorList: list to write errors to

returns: if an error occured
*/
char validateFile(FileHandle* handle, List* errorList) {
    // setup
    char hasError = 0;
    char buffer[257];
    unsigned int lineCounter = 0;

    // start from the file start
    fpos_t posPreserve;
    fgetpos(handle->fptr, &posPreserve);
    rewind(handle->fptr);
    
    // read all lines
    while (!feof(handle->fptr)) {
        fgets(buffer, 257, handle->fptr);
        int len = strlen(buffer);
        if (len > 255) {
            hasError = 1;
            char* errorStr = (char*)malloc(34 * sizeof(char));
            sprintf(errorStr, "Line size cannot exceed 255 chars");
            ErrorData errorData = {errorStr, lineCounter, 0, 1, handle};
            appendList(errorList, &errorData, sizeof(errorData));
        }
        while (!feof(handle->fptr) && strlen(buffer) > 255) {
            fgets(buffer, 257, handle->fptr);
        }
        lineCounter++;
    }

    // cleanup and return
    fsetpos(handle->fptr, &posPreserve);
    return hasError;
}

/*
determines if the line ending is valid

line: line to evaluate
lineLenght: maximum length of line ending

returns: if the line ending is valid for a macro
*/
char isValidLineEnding(char* line, unsigned int lineLength) {
    int endCharPos = countWhitespaceChars(line, lineLength);
    char endChar = line[endCharPos];
    return endChar == '\0' || endChar == '\n' || endChar == ';';
}

/*
extracts all values in an argument list

line: line to parse
length: length of the line

returns: list of arguments
*/
List* extractArgs(char* line, unsigned int length) {
    List* outputList = newList();
    int j = 0;
    for (int i = 0; i < length; i++) {
        char c = line[i];
        if (c == ',') {
            int s = i - j + 1;
            char* arg = malloc(s * sizeof(char));
            memcpy(arg, line + j, s - 1);
            arg[s - 1] = '\0';
            j = i + 1;
            appendList(outputList, &arg, sizeof(char*));
        }
        if (c == ';' || c == '\0' || c == '\n') {break;}
    }
    int s = length - j + 1;
    char* arg = malloc(s * sizeof(char));
    memcpy(arg, line + j, s - 1);
    arg[s - 1] = '\0';
    appendList(outputList, &arg, sizeof(char*));
    return outputList;
}

#endif