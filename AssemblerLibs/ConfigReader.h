/*
code to read a basic config file

Written by Adam Billings
*/

#ifndef ConfigReader_h
#define ConfigReader_h

#include <stdint.h>
#include "DataStructures/StringTable.h"
#include "ExpressionEvaluation.h"
#include "MiscAssembler.h"

// memory access type
typedef enum AccessType {
    rw, ro, bss
} AccessType;

// segment information
typedef struct SegmentDef {
    char* name;
    uint16_t startAddr;
    uint16_t size;
    uint16_t align;
    AccessType accessType;
    char fill;
    uint16_t writeAddr;
    uint8_t* outputArr;
} SegmentDef;

// information on what has been parsed for a segment
typedef struct SegmentParseData {
    char readSize;
    char readStart;
    char readType;
    char readLoad;
    char readFill;
    char readAlign;
} SegmentParseData;

// sample uninitialized segment
const SegmentDef NEW_SEGMENT = {NULL, 0, 0, 1, bss, 0, 0, NULL};

/*
determines if the line ending is valid

line: line to inspect
length: line length
isLineEnding: if the ending is a line ending

returns: if the line ending is valid
*/
char isValidConfigEnding(char *line, unsigned int length, char isLineEnding) {
    for (int i = 0; i < length; i++) {
        if (line[i] == '\0' || line[i] == '\n' || line[i] == '#' || (!isLineEnding && (line[i] == ',' || line[i] == ';'))) {return 1;}
        else if (!(isWhitespace(line[i]))) {return 0;}
    }
    return 1;
}

/*
read one attribute for a segment definition

file: source file
errorList: list of errors
line: line to read from
curLine: current line count
curCol: start column
segment: segment to update
segmentData: parse data for the segment
nullST: null string table (avoid malloc)

returns: number of chars read
*/
unsigned int readMemoryAttribute(FileHandle* handle, List* errorList, char* line, int curLine, int curCol, SegmentDef* segment, SegmentParseData* segmentData, StringTable nullST) {
    // read the attribute name
    unsigned int i = countWhitespaceChars(line, strlen(line));
    unsigned int j;
    for (j = i; j < strlen(line); j++) {
        if (!((line[j] >= 'a' && line[j] <= 'z') || line[j] >= 'A' && line[j] <= 'Z')) {break;}
    }
    if (j == i) {
        char* errorStr = (char*)malloc(26 * sizeof(char));
        sprintf(errorStr, "Expected memory attribute");
        ErrorData errorData = {errorStr, curLine, curCol, 1, handle};
        appendList(errorList, &errorData, sizeof(ErrorData));
        return 0;
    }
    char attrName[j - i + 1];
    memcpy(attrName, line + i, j - i);
    attrName[j - i] = '\0';
    
    // read '='
    int k = countWhitespaceChars(line + j, strlen(line + j)) + j;
    if (line[k] != '=') {
        char* errorStr = (char*)malloc(11 * sizeof(char));
        sprintf(errorStr, "Expected =");
        ErrorData errorData = {errorStr, curLine, curCol + j, 1, handle};
        appendList(errorList, &errorData, sizeof(ErrorData));
        return 0;
    }

    // extract expression
    int l;
    for (l = k + 1; l < strlen(line); l++) {
        if (line[l] == ',' || line[l] == ';' || line[l] == '\n') {break;}
    }
    char expr[l - k];
    memcpy(expr, line + k + 1, l - k - 1);
    expr[l - k - 1] = '\0';
    
    // parse the expression
    uint16_t val;
    if (strcmp(attrName, "type")) {
        ExprErrorShort exprOut = evalShortExpr(expr, l - k, nullST, nullST);
        if (exprOut.errorMessage != NULL) {
            ErrorData errorData = {exprOut.errorMessage, curLine, curCol + k + 1 + exprOut.errorPos, exprOut.errorLen, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            return 0;
        }
        val = exprOut.val;
    }

    // set the attribute
    char hasRepeat = 0;
    if (!strcmp(attrName, "start")) {
        if (segmentData->readStart) {hasRepeat = 1;}
        segmentData->readStart = 1;
        segment->startAddr = val;
    } else if (!strcmp(attrName, "size")) {
        if (segmentData->readSize) {hasRepeat = 1;}
        segmentData->readSize = 1;
        segment->size = val;
    } else if (!strcmp(attrName, "type")) {
        if (segmentData->readType) {hasRepeat = 1;}
        segmentData->readType = 1;

        // get the type
        unsigned int m = countWhitespaceChars(expr, strlen(expr));
        unsigned int n;
        for (n = m; n < strlen(expr); n++) {
            if (!((expr[n] >= 'a' && expr[n] <= 'z') || (expr[n] >= 'A' && expr[n] <= 'Z'))) {break;}
        }
        char type[n - m + 1];
        memcpy(type, expr + m, n - m);
        type[n - m] = '\0';

        // set the type
        if (!strcmp(type, "ro")) {segment->accessType = ro;}
        else if (!strcmp(type, "rw")) {segment->accessType = rw;}
        else if (!strcmp(type, "bss")) {segment->accessType = bss;}
        else {
            char* errorStr = (char*)malloc((29 + strlen(type) * sizeof(char)));
            sprintf(errorStr, "Unrecognized access type: %s", type);
            ErrorData errorData = {errorStr, curLine, curCol + k + m + 1, n - m, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            return 0;
        }

        // check for trailing garbage
        if(!isValidConfigEnding(line + k + n + 1, strlen(line + k + n + 1), 0)) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, curLine, curCol + k + n + 1, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            return 0;
        }

    } else {
        char* errorStr = (char*)malloc((27 + strlen(attrName)) * sizeof(char));
        sprintf(errorStr, "Unrecognized attribute: %s", attrName);
        ErrorData errorData = {errorStr, curLine, curCol + i, strlen(attrName), handle};
        appendList(errorList, &errorData, sizeof(errorData));
        return 0;
    }
    if (hasRepeat) {
        char* errorStr = (char*)malloc((27 + strlen(attrName)) * sizeof(char));
        sprintf(errorStr, "Redeclaration of value: %s", attrName);
        ErrorData errorData = {errorStr, curLine, curCol + i, strlen(attrName), handle};
        appendList(errorList, &errorData, sizeof(ErrorData));
        return 0;
    }

    return l;
}

/*
read memory information

handle: file handle
errorList: list of errors

returns: memory information as segments
*/
List* readMemory(FileHandle* handle, List* errorList, unsigned int* curLine) {
    List* parses = newList();
    long curPos;
    StringTable nullST = newStringTable();
    char isInScope = 0;
    char hasReadHeader = 0;
    char line[256];
    while (!feof(handle->fptr)) {
        // read a line
        if (fgets(line, 256, handle->fptr)) {return NULL;}
        
        if (!hasReadHeader) {
            if (!isValidConfigEnding(line, strlen(line), 1)) {
                // read MEMORY
                int i = countWhitespaceChars(line, strlen(line));
                int j;
                for (j = i; j < strlen(line); j++) {
                    if (!((line[j] >= 'a' && line[j] <= 'z') || (line[j] >= 'A' && line[j] <= 'Z'))) {break;}
                }
                char header[j - i + 1];
                memcpy(header, line + i, j - i);
                header[j - i] = '\0';
                if (strcmp(header, "MEMORY")) {
                    char* errorStr = (char*)malloc(23 * sizeof(char));
                    sprintf(errorStr, "Expected MEMORY header");
                    ErrorData errorData = {errorStr, *curLine, 0, 1, handle};
                    appendList(errorList, &errorData, sizeof(ErrorData));
                    return parses;
                }
                hasReadHeader = 1;

                // check for {
                i = j + countWhitespaceChars(line + j, strlen(line + j));
                if (line[i] == '{') {
                    isInScope = 1;
                    j = i + 1;
                }

                // trailing garbage check
                if (!isValidConfigEnding(line + j, strlen(line + j), 1)) {
                    char* errorStr = (char*)malloc(28 * sizeof(char));
                    sprintf(errorStr, "Unexpected trailing garbage");
                    ErrorData errorData = {errorStr, *curLine, j, 1, handle};
                    appendList(errorList, &errorData, sizeof(ErrorData));
                }
            }
        }
        else if (!isInScope) {
            // check for '{'
            if (!isValidConfigEnding(line, strlen(line), 1)) {
                // check for {
                int i = countWhitespaceChars(line, strlen(line));
                if (line[i] == '{') {isInScope = 1; i++;}

                // trailing garbage check
                if (!isValidConfigEnding(line + i, strlen(line + i), 1)) {
                    char* errorStr = (char*)malloc(28 * sizeof(char));
                    sprintf(errorStr, "Unexpected trailing garbage");
                    ErrorData errorData = {errorStr, *curLine, i, 1, handle};
                    appendList(errorList, &errorData, sizeof(ErrorData));
                } 
            }
        } else if (!isValidConfigEnding(line, strlen(line), 1)) {
            // check for }
            int wsc = countWhitespaceChars(line, strlen(line));
            if (line[wsc] == '}') {
                if (!isValidConfigEnding(line + wsc + 1, strlen(line + wsc + 1), 1)) {
                    char* errorStr = (char*)malloc(28 * sizeof(char));
                    sprintf(errorStr, "Unexpected trailing garbage");
                    ErrorData errorData = {errorStr, *curLine, wsc, 1, handle};
                    appendList(errorList, &errorData, sizeof(ErrorData));
                }
                return parses;
            }

            // setup
            SegmentParseData segData = {0, 0, 0, 0, 0, 0};
            SegmentDef segDef = NEW_SEGMENT;

            // read the memory name
            char* nameptr = line + wsc;
            char* memoryName = extractVar(&nameptr, strlen(nameptr));
            if (memoryName == NULL) {
                char* errorStr = (char*)malloc(20 * sizeof(char));
                sprintf(errorStr, "Expected identifier");
                ErrorData errorData = {errorStr, *curLine, 0, 1, handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                (*curLine)++;
                continue;
            }
            wsc = countWhitespaceChars(nameptr, strlen(nameptr));
            if (nameptr[wsc] != ':') {
                char* errorStr = (char*)malloc(11 * sizeof(char));
                sprintf(errorStr, "Expected :");
                ErrorData errorData = {errorStr, *curLine, (nameptr - line), 1, handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                free(memoryName);
                (*curLine)++;
                continue;
            }
            segDef.name = memoryName;

            // check for repeats
            SegmentDef* eqName = NULL;
            for (Node* node = parses->head; node != NULL; node = node->next) {
                if (!strcmp(((SegmentDef*)(node->dataptr))->name, segDef.name)) {
                    eqName = (SegmentDef*)(node->dataptr);
                }
            }
            if (eqName != NULL) {
                char* errorStr = (char*)malloc((24 + strlen(segDef.name)));
                sprintf(errorStr, "Repeat definition of %s", segDef.name);
                ErrorData errorData = {errorStr, *curLine, (nameptr - line - strlen(segDef.name)), strlen(segDef.name), handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                free(memoryName);
                (*curLine)++;
                continue;
            }

            // read the memory attributes
            int i = (nameptr - line) + wsc + 1;
            while (i < 256) {
                int j = readMemoryAttribute(handle, errorList, line + i, *curLine, i, &segDef, &segData, nullST);
                if (j == 0) {break;}
                i += j;
                if (line[i] == ',') {i++; continue;}
                if (line[i] == ';') {
                    // garbage error 
                    if (!isValidConfigEnding(line + i + 1, strlen(line + i + 1), 1)) {
                        char* errorStr = (char*)malloc(28 * sizeof(char));
                        sprintf(errorStr, "Unexpected trailing garbage");
                        ErrorData errorData = {errorStr, *curLine, i, 1, handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                    }

                    // missing data errors
                    if (!segData.readStart) {
                        char* errorStr = (char*)malloc(26 * sizeof(char));
                        sprintf(errorStr, "Expected start definition");
                        ErrorData errorData = {errorStr, *curLine, i, 1, handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                    }
                    if (!segData.readSize) {
                        char* errorStr = (char*)malloc(25 * sizeof(char));
                        sprintf(errorStr, "Expected size definition");
                        ErrorData errorData = {errorStr, *curLine, i, 1, handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                    }
                    if (!segData.readType) {
                        char* errorStr = (char*)malloc(25 * sizeof(char));
                        sprintf(errorStr, "Expected type definition");
                        ErrorData errorData = {errorStr, *curLine, i, 1, handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                    }
                    break;
                }
            }
            
            // save the memory segment
            appendList(parses, &segDef, sizeof(SegmentDef));
        }

        // handle "troll" line
        curPos = ftell(handle->fptr);
        if (!feof(handle->fptr) && curPos == handle->length) {
            if (fgets(line, 256, handle->fptr)) {return NULL;}
        }

        (*curLine)++;
    }
    
    // did not read the memory block
    char* errorStr = (char*)malloc(28 * sizeof(char));
    sprintf(errorStr, "Could not read MEMORY block");
    ErrorData errorData = {errorStr, *curLine, 0, 1, handle};
    appendList(errorList, &errorData, sizeof(ErrorData));
    return parses;
}

/*
read one attribute for a segment definition

file: source file
errorList: list of errors
line: line to read from
curLine: current line count
curCol: start column
segment: segment to update
segmentData: parse data for the segment
nullST: null string table (avoid malloc)

returns: number of chars read
*/
unsigned int readSegmentAttribute(FileHandle* handle, List* errorList, char* line, int curLine, int curCol, SegmentDef* segment, SegmentParseData* segmentData, List* memoryData, StringTable nullST) {
    // read the attribute name
    unsigned int i = countWhitespaceChars(line, strlen(line));
    unsigned int j;
    for (j = i; j < strlen(line); j++) {
        if (!((line[j] >= 'a' && line[j] <= 'z') || line[j] >= 'A' && line[j] <= 'Z')) {break;}
    }
    if (j == i) {
        char* errorStr = (char*)malloc(26 * sizeof(char));
        sprintf(errorStr, "Expected memory attribute");
        ErrorData errorData = {errorStr, curLine, curCol, 1, handle};
        appendList(errorList, &errorData, sizeof(ErrorData));
        return 0;
    }
    char attrName[j - i + 1];
    memcpy(attrName, line + i, j - i);
    attrName[j - i] = '\0';
    
    // read '='
    int k = countWhitespaceChars(line + j, strlen(line + j)) + j;
    if (line[k] != '=') {
        char* errorStr = (char*)malloc(11 * sizeof(char));
        sprintf(errorStr, "Expected =");
        ErrorData errorData = {errorStr, curLine, curCol + j, 1, handle};
        appendList(errorList, &errorData, sizeof(ErrorData));
        return 0;
    }

    // extract expression
    int l;
    for (l = k + 1; l < strlen(line); l++) {
        if (line[l] == ',' || line[l] == ';' || line[l] == '\n') {break;}
    }
    char expr[l - k];
    memcpy(expr, line + k + 1, l - k - 1);
    expr[l - k - 1] = '\0';
    
    // parse the expression
    uint16_t val;
    if (!strcmp(attrName, "align")) {
        ExprErrorShort exprOut = evalShortExpr(expr, l - k, nullST, nullST);
        if (exprOut.errorMessage != NULL) {
            ErrorData errorData = {exprOut.errorMessage, curLine, curCol + k + 1 + exprOut.errorPos, exprOut.errorLen, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            return 0;
        }
        val = exprOut.val ? exprOut.val : 1;
    }

    // set the attribute
    char hasRepeat = 0;
    if (!strcmp(attrName, "load")) {
        if (segmentData->readLoad) {hasRepeat = 1;}
        segmentData->readLoad = 1;

        // get the type
        unsigned int m = countWhitespaceChars(expr, strlen(expr));
        unsigned int n;
        for (n = m; n < strlen(expr); n++) {
            if (isWhitespace(expr[n])) {break;}
        }
        char loc[n - m + 1];
        memcpy(loc, expr + m, n - m);
        loc[n - m] = '\0';

        // set the type
        SegmentDef* memptr = NULL;
        for (Node* node = memoryData->head; node != NULL; node = node->next) {
            SegmentDef* curData = (SegmentDef*)(node->dataptr);
            if (!strcmp(curData->name, loc)) {
                memptr = curData;
                break;
            }
        }
        if (memptr == NULL) {
            char* errorStr = (char*)malloc((33 + strlen(loc)) * sizeof(char));
            sprintf(errorStr, "Unrecognized memory location: %s", loc);
            ErrorData errorData = {errorStr, curLine, curCol + k + m + 1, n - m, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            return 0;
        }
        segment->startAddr = memptr->startAddr;
        segment->size = memptr->size;
        segment->accessType = memptr->accessType;

        // check for trailing garbage
        if(!isValidConfigEnding(line + k + n + 1, strlen(line + k + n + 1), 0)) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, curLine, curCol + k + n + 1, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            return 0;
        }

    } else if (!strcmp(attrName, "fill")) {
        if (segmentData->readFill) {hasRepeat = 1;}
        segmentData->readFill = 1;

        // get the type
        unsigned int m = countWhitespaceChars(expr, strlen(expr));
        unsigned int n;
        for (n = m; n < strlen(expr); n++) {
            if (!((expr[n] >= 'a' && expr[n] <= 'z') || (expr[n] >= 'A' && expr[n] <= 'Z'))) {break;}
        }
        char type[n - m + 1];
        memcpy(type, expr + m, n - m);
        type[n - m] = '\0';

        // set the type
        if (!strcmp(type, "yes")) {segment->fill = 1;}
        else if (!strcmp(type, "no")) {segment->fill = 0;}
        else {
            char* errorStr = (char*)malloc((29 + strlen(type)) * sizeof(char));
            sprintf(errorStr, "Unrecognized fill option: %s", type);
            ErrorData errorData = {errorStr, curLine, curCol + k + m + 1, n - m, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            return 0;
        }

        // check for trailing garbage
        if(!isValidConfigEnding(line + k + n + 1, strlen(line + k + n + 1), 0)) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, curLine, curCol + k + n + 1, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            return 0;
        }

    } else if (!strcmp(attrName, "align")) {
        if (segmentData->readAlign) {hasRepeat = 1;}
        segmentData->readAlign = 1;
        segment->align = val;
    } else {
        char* errorStr = (char*)malloc((27 + strlen(attrName)) * sizeof(char));
        sprintf(errorStr, "Unrecognized attribute: %s", attrName);
        ErrorData errorData = {errorStr, curLine, curCol + i, strlen(attrName), handle};
        appendList(errorList, &errorData, sizeof(errorData));
        return 0;
    }
    if (hasRepeat) {
        char* errorStr = (char*)malloc((27 + strlen(attrName)) * sizeof(char));
        sprintf(errorStr, "Redeclaration of value: %s", attrName);
        ErrorData errorData = {errorStr, curLine, curCol + i, strlen(attrName), handle};
        appendList(errorList, &errorData, sizeof(ErrorData));
        return 0;
    }

    return l;
}

/*
read segment information

handle: file handle
errorList: list of errors

returns: memory information as segments
*/
List* readSegment(FileHandle* handle, List* errorList, List* memoryList, unsigned int* curLine) {
    List* parses = newList();
    long curPos;
    StringTable nullST = newStringTable();
    char isInScope = 0;
    char hasReadHeader = 0;
    char line[256];
    while (!feof(handle->fptr)) {
        // read a line
        if (fgets(line, 256, handle->fptr)) {return NULL;}

        if (!hasReadHeader) {
            if (!isValidConfigEnding(line, strlen(line), 1)) {
                // read MEMORY
                int i = countWhitespaceChars(line, strlen(line));
                int j;
                for (j = i; j < strlen(line); j++) {
                    if (!((line[j] >= 'a' && line[j] <= 'z') || (line[j] >= 'A' && line[j] <= 'Z'))) {break;}
                }
                char header[j - i + 1];
                memcpy(header, line + i, j - i);
                header[j - i] = '\0';
                if (strcmp(header, "SEGMENTS")) {
                    char* errorStr = (char*)malloc(25 * sizeof(char));
                    sprintf(errorStr, "Expected SEGMENTS header");
                    ErrorData errorData = {errorStr, *curLine, 0, 1, handle};
                    appendList(errorList, &errorData, sizeof(ErrorData));
                    return parses;
                }
                hasReadHeader = 1;

                // check for {
                i = j + countWhitespaceChars(line + j, strlen(line + j));
                if (line[i] == '{') {
                    isInScope = 1;
                    j = i + 1;
                }

                // trailing garbage check
                if (!isValidConfigEnding(line + j, strlen(line + j), 1)) {
                    char* errorStr = (char*)malloc(28 * sizeof(char));
                    sprintf(errorStr, "Unexpected trailing garbage");
                    ErrorData errorData = {errorStr, *curLine, j, 1, handle};
                    appendList(errorList, &errorData, sizeof(ErrorData));
                }
            }
        } else if (!isInScope) {
            // check for '{'
            if (!isValidConfigEnding(line, strlen(line), 1)) {
                // check for {
                int i = countWhitespaceChars(line, strlen(line));
                if (line[i] == '{') {isInScope = 1; i++;}

                // trailing garbage check
                if (!isValidConfigEnding(line + i, strlen(line + i), 1)) {
                    char* errorStr = (char*)malloc(28 * sizeof(char));
                    sprintf(errorStr, "Unexpected trailing garbage");
                    ErrorData errorData = {errorStr, *curLine, i, 1, handle};
                    appendList(errorList, &errorData, sizeof(ErrorData));
                } 
            }
        } else if (!isValidConfigEnding(line, strlen(line), 1)) {
            // check for }
            int wsc = countWhitespaceChars(line, strlen(line));
            if (line[wsc] == '}') {
                if (!isValidConfigEnding(line + wsc + 1, strlen(line + wsc + 1), 1)) {
                    char* errorStr = (char*)malloc(28 * sizeof(char));
                    sprintf(errorStr, "Unexpected trailing garbage");
                    ErrorData errorData = {errorStr, *curLine, wsc, 1, handle};
                    appendList(errorList, &errorData, sizeof(ErrorData));
                }
                return parses;
            }

            // setup
            SegmentParseData segData = {0, 0, 0, 0, 0, 0};
            SegmentDef segDef = NEW_SEGMENT;

            // read the memory name
            char* nameptr = line + wsc;
            char* segmentName = extractVar(&nameptr, strlen(nameptr));
            if (segmentName == NULL) {
                char* errorStr = (char*)malloc(20 * sizeof(char));
                sprintf(errorStr, "Expected identifier");
                ErrorData errorData = {errorStr, *curLine, 0, 1, handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                (*curLine)++;
                continue;
            }
            wsc = countWhitespaceChars(nameptr, strlen(nameptr));
            if (nameptr[wsc] != ':') {
                char* errorStr = (char*)malloc(11 * sizeof(char));
                sprintf(errorStr, "Expected :");
                ErrorData errorData = {errorStr, *curLine, (nameptr - line), 1, handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                free(segmentName);
                (*curLine)++;
                continue;
            }
            segDef.name = segmentName;

            // check for repeats
            SegmentDef* eqName = NULL;
            for (Node* node = parses->head; node != NULL; node = node->next) {
                if (!strcmp(((SegmentDef*)(node->dataptr))->name, segDef.name)) {
                    eqName = (SegmentDef*)(node->dataptr);
                }
            }
            if (eqName != NULL) {
                char* errorStr = (char*)malloc((24 + strlen(segDef.name)));
                sprintf(errorStr, "Repeat definition of %s", segDef.name);
                ErrorData errorData = {errorStr, *curLine, (nameptr - line - strlen(segDef.name)), strlen(segDef.name), handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                free(segmentName);
                (*curLine)++;
                continue;
            }

            // read the memory attributes
            int i = (nameptr - line) + wsc + 1;
            while (i < 256) {
                int j = readSegmentAttribute(handle, errorList, line + i, *curLine, i, &segDef, &segData, memoryList, nullST);
                if (j == 0) {break;}
                i += j;
                if (line[i] == ',') {i++; continue;}
                if (line[i] == ';') {
                    // garbage error 
                    if (!isValidConfigEnding(line + i + 1, strlen(line + i + 1), 1)) {
                        char* errorStr = (char*)malloc(28 * sizeof(char));
                        sprintf(errorStr, "Unexpected trailing garbage");
                        ErrorData errorData = {errorStr, *curLine, i, 1, handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                    }

                    // missing data errors
                    if (!segData.readLoad) {
                        char* errorStr = (char*)malloc(25 * sizeof(char));
                        sprintf(errorStr, "Expected load definition");
                        ErrorData errorData = {errorStr, *curLine, i, 1, handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                    }
                    break;
                }
            }
            
            // save the memory segment
            appendList(parses, &segDef, sizeof(SegmentDef));
        }

        // handle "troll" line
        curPos = ftell(handle->fptr);
        if (curPos == handle->length) {
            if (fgets(line, 256, handle->fptr)) {return NULL;}
        }

        (*curLine)++;
    }

    // did not read the segments block
    char* errorStr = (char*)malloc(30 * sizeof(char));
    sprintf(errorStr, "Could not read SEGMENTS block");
    ErrorData errorData = {errorStr, *curLine, 0, 1, handle};
    appendList(errorList, &errorData, sizeof(ErrorData));
    return parses;
}

/*
reads a configuration file

handle: file handle
errorList: list of errors

returns: configuration segment information
*/
List* readConfigFile(FileHandle* handle, List* errorList) {
    // read the memory attributes
    unsigned int curLine = 0;
    List* memData = readMemory(handle, errorList, &curLine);

    // read the segment information
    if (errorList->size > 0) {return NULL;}
    curLine++;
    List* segData = readSegment(handle, errorList, memData, &curLine);

    // make sure the rest of the file is clear
    curLine++;
    while (!feof(handle->fptr)) {
        char buffer[256];

        if (fgets(buffer, 256, handle->fptr)) {return NULL;}
        if (!isValidConfigEnding(buffer, strlen(buffer), 1)) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, curLine, 0, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
        }

        curLine++;
    }

    // cleanup
    for (Node* node = memData->head; node != NULL; node = node->next) {
        free(((SegmentDef*)(node->dataptr))->name);
    }
    deleteList(memData);
    return segData;
}

#endif