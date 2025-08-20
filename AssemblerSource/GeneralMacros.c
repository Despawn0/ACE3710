/*
general-purpose code used for macros in all sweeps

Written by Adam Billings
*/

#include <string.h>
#include <stdlib.h>
#include "MiscAssembler.h"
#include "ExpressionEvaluation.h"
#include "DataStructures/StringTable.h"
#include "DataStructures/Stack.h"
#include "GeneralMacros.h"

/*
determines if a character can be in a valid name

c: character to evaluate

returns: if the character can be in a valid name
*/
char isValidMacroChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/*
extracts a macro name from a line

line: line to read
lineLength: maximum length of the line
outputPos: output position for after the macro name

returns: macro name or NULL
*/
char* extractMacro(char* line, int lineLength, char** outputPos) {
    if (lineLength <= 0 || line[0] != '.') {return NULL;}
    int i;
    for (i = 1; i < lineLength; i++) {
        if (isspace(line[i]) || line[i] == ';' || line[i] == '\0') {break;}
    }
    *outputPos = line + i;
    char* output = (char*)memcpy(malloc((i + 1) * sizeof(char)), line, (i + 1) * sizeof(char));
    output[i] = '\0';
    return output;
}

/*
extracts the arguments for a .macro definition

line: line to read
lineLength: maximum length of the line
afterArgs: output of the rest of the string

returns: list of macro arguments
*/
List* extractMacroArgs(char* line, int lineLength, char** afterArgs) {
    List* outputList = newList();
    char noComma = 0;
    int i;

    for (i= 0; i < lineLength; i++) {
        // go to the start of a name
        for (; i < lineLength; i++) {
            if (!isspace(line[i])) {break;}
        }

        // handle all space
        if (i == lineLength) {break;}

        // handle no more args
        if (line[i] == '\n' || line[i] == ';' || line[i] == '\0') {*afterArgs = line + i; return outputList;}

        // handle start char
        if (!(isValidNameChar(line[i]) && !(line[i] >= '0' && line[i] <= '9')) || noComma) {
            for (Node* node = outputList->head; node != NULL; node = node->next) {
                free(*(char**)(node->dataptr));
            }
            deleteList(outputList);
            return NULL;
        }

        // read in a argument name
        int nameStart = i++;
        for (; i < lineLength; i++) {
            if (!isValidNameChar(line[i])) {break;}
        }
        unsigned int nameLen = (i - nameStart) + 1;
        char* argName = (char*)memcpy(malloc(nameLen * sizeof(char)), line + nameStart, nameLen * sizeof(char));
        argName[nameLen - 1] = '\0';
        appendList(outputList, &argName, sizeof(char*)); 

        // drop trailing spaces
        for (; i < lineLength; i++) {
            if (!isspace(line[i])) {break;}
        }

        // handle all space
        if (i == lineLength) {break;}

        // drop comma
        if (line[i] != ',') {noComma = 1; i--;}
    }
    *afterArgs = line + i;
    return outputList;
}

/*
reads a string from the line

line: line to read from, expects string at the start
lineLength: length of the line
afterString: output written to for the next char
outLen: length output

returns: extracted string
*/
char* readString(char* line, unsigned int lineLength, char** afterString, unsigned int* outLen) {
    // ensure the line starts with a string
    if (line[0] != '\"') {return NULL;}

    // read the string
    char readLine[lineLength];
    char isEscape = 0;
    int i;
    int j = 0;
    for (i = 1; i < lineLength; i++) {
        if (line[i] == '\n' || line[i] == '\0') {*afterString = line + i; return NULL;}
        if (!isEscape && line[i] == '\"') {break;}
        if (!isEscape && line[i] == '\\') {isEscape = 1; continue;}
        else if (isEscape && line[i] == '0') {readLine[j] = '\0';}
        else if (isEscape && line[i] == 't') {readLine[j] = '\t';}
        else if (isEscape && line[i] == 'n') {readLine[j] = '\n';}
        else if (isEscape && line[i] == 'e') {readLine[j] = '\e';}
        else if (isEscape && line[i] == '\\') {readLine[j] = '\\';}
        else if (isEscape && line[i] == '\"') {readLine[j] = '\"';}
        else if (isEscape && line[i] == '\'') {readLine[j] = '\'';}
        else if (isEscape) {*afterString = line + i; return NULL;}
        else {readLine[j] = line[i];}
        isEscape = 0;
        j++;
    }
    if (i == lineLength) {*afterString = NULL; return NULL;}
    readLine[j] = '\0';

    // allocate the output
    char* newLine = (char*)malloc((j + 1) * sizeof(char));
    memcpy(newLine, readLine, (j + 1) * sizeof(char));
    *afterString = line + i + 1;
    *outLen = j + 1;
    return newLine;
}

/*
attempts to get the handle to an open file

handleList: list to pull from
name: name of the file
isBin: if the file handle is binary

returns: handle to open file or NULL
*/
FileHandle* getHandle(List* handleList, char* name, char isBin) {
    for (Node* node = handleList->head; node != NULL; node = node->next) {
        FileHandle* handle = (FileHandle*)(node->dataptr);
        if (!strcmp(name, handle->name) && handle->isBin == isBin) {return handle;}
    }
    return NULL;
}

/*
reads to the end of an if statement

handle: handle to the file
errorList: list of errors
ifData: location data for the if statement
allowElse: if .else and .elseif macros are allowed

returns: end line to the if statement
*/
char* skipIf(FileHandle* handle, List* errorList, PosData* ifData, char allowElse) {
    // setup
    char buffer[256];
    int ifCount = 0;
    int startLine = ifData->line;
    int curLine = ifData->line;

    // read to .endif
    while (!feof(handle->fptr)) {
        // get the next line
        if (fgets(buffer, 256, handle->fptr)== NULL && !feof(handle->fptr)) {return NULL;}
        
        // skip spaces
        int i;
        for (i = 0; i < 256; i++) {
            if (!isspace(buffer[i])) {break;}
        }

        // check for macro
        if (buffer[i] == '.') {
            char* nameEnd;
            char* macroName = extractMacro(buffer + i, (256 - i), &nameEnd);
            
            // check for if/endif
            if (!strcmp(macroName, ".if")) {ifCount++;}
            else if (!strcmp(macroName, ".ifdef")) {ifCount++;}
            else if (!strcmp(macroName, ".ifndef")) {ifCount++;}
            else if (!strcmp(macroName, ".endif") && ifCount == 0) {
                (ifData->line)++;
                ifData->col = i;
                char* out = (char*)malloc((1 + strlen(buffer)) * sizeof(char));
                sprintf(out, "%s", buffer);
                return out;
            } else if (!strcmp(macroName, ".endif")) {ifCount--;}
            else if ((!strcmp(macroName, ".else") || !strcmp(macroName, ".elseif") || !strcmp(macroName, ".elseifdef") || !strcmp(macroName, ".elseifndef")) && ifCount == 0 && allowElse) {
                (ifData->line)++;
                ifData->col = i;
                char* out = (char*)malloc((1 + strlen(buffer)) * sizeof(char));
                sprintf(out, "%s", buffer);
                return out;
            } else if (!strcmp(macroName, ".elseif") && ifCount == 0 && allowElse) {
                (ifData->line)++;
                ifData->col = i;
                char* out = (char*)malloc((1 + strlen(buffer)) * sizeof(char));
                sprintf(out, "%s", buffer);
                return out;
            }
        }

        (ifData->line)++;
    }

    // no .endif, push an error
    char* errorStr = (char*)malloc(16  * sizeof(char));
    sprintf(errorStr, "Expected .endif");
    ErrorData errorData = {errorStr, startLine, ifData->col, 1, handle};
    appendList(errorList, &errorData, sizeof(ErrorData));
    return NULL;
}

/*
returns to the preveous file upon the end of .include

includeStack: stack of include returns
lineptr: pointer to line to update

returns: pointer to handle to update
*/
FileHandle* includeReturn(Stack* includeStack, unsigned int* lineptr) {
    // pop from the stack
    IncludeReturnData* retData = (IncludeReturnData*)popStack(includeStack);

    // set the new line number
    *lineptr = retData->returnLine;

    // restore the handle
    FileHandle* handle = retData->returnFile;
    fseek(handle->fptr, retData->filePosition, SEEK_SET);
    return handle;
}

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
char macroIf(FileHandle* handle, List* errorList, char* name, char* expr, unsigned int exprLen, int line, int col, StringTable defines) {
    // handle the different cases
    if (!strcmp(name, ".if") || !strcmp(name, ".elseif")) {
        // evaluate the expression
        ExprErrorShort evalOut = evalShortExpr(expr, strlen(expr), defines, defines);
        if (evalOut.errorMessage != NULL) {
            ErrorData errorData = {evalOut.errorMessage, line, col + evalOut.errorPos, evalOut.errorLen, handle};
            appendList(errorList, &errorData, sizeof(errorData));
            return 1;
        }
        return evalOut.val != 0;
    } else if (!strcmp(name, ".ifdef") || !strcmp(name, ".elseifdef") || !strcmp(name, ".ifndef") || !strcmp(name, ".elseifndef")) {
        // get the name
        char* afterVar;
        unsigned int i = countWhitespaceChars(expr, exprLen);
        char* varName = getVarName(expr + i, exprLen - i, &afterVar);
        if (varName == NULL) {
            char* errorStr = (char*)malloc(20 * sizeof(char));
            sprintf(errorStr, "Expected identifier");
            ErrorData errorData = {errorStr, line, strlen(name) + 1, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            return 1;
        }

        // ensure no garbage
        if (!isValidLineEnding(afterVar, exprLen - (afterVar - expr))) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, line, (afterVar - expr) + col, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            return 1;
        }

        // return value
        void* isDefined = readStringTable(defines, varName, strlen(varName) + 1);
        if (!strcmp(name, ".ifdef") || !strcmp(name, ".elseifdef")) {return isDefined != NULL;}
        else {return isDefined == NULL;}
    } else {return 1;}
}

/*
counts the args in a macro

line: line to evaluate
len: length of the line
*/
unsigned int countArgs(char* line, unsigned int len) {
    int count = 1; // assume 1 arg
    for (int i = 0; i < len; i++) {
        if (line[i] == '\n' || line[i] == ';' || line[i] == '\0') {return count;}
        if (line[i] == ',') {count++;}
    }
    return count;
}

/*
determines if a macro name is valid

name: macro name

returns: if the name is valid
*/
char isValidMacroName(char* name) {
    char type1 = !strcmp(name, ".define") || !strcmp(name, ".macro") || !strcmp(name, ".endmacro") || !strcmp(name, ".undef") || !strcmp(name, ".if") || !strcmp(name, ".else") || !strcmp(name, ".elseif") || !strcmp(name, ".endif") || !strcmp(name, ".ifndef") || !strcmp(name, ".elseifndef") || !strcmp(name, ".ifdef") || !strcmp(name, ".elseifdef") || !strcmp(name, ".redef") || !strcmp(name, ".include") || !strcmp(name, ".incbin");
    char type2 = !strcmp(name, ".segment") || !strcmp(name, ".pushseg") || !strcmp(name, ".popseg") || !strcmp(name, ".res") || !strcmp(name, ".word") || !strcmp(name, ".byte") || !strcmp(name, ".align") || !strcmp(name, ".ascii") || !strcmp(name, ".asciiz");
    char type3 = !strcmp(name, ".error") || !strcmp(name, ".warning");
    return type1 || type2 || type3;
}