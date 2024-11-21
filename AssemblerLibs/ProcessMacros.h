/*
information needed to process macros

Written by Adam Billings
*/

#ifndef ProcessMacros_h
#define ProcessMacros_h

#include <stdio.h>
#include "GeneralMacros.h"
#include "ExpressionEvaluation.h"
#include "DataStructures/Stack.h"
#include "DataStructures/StringTable.h"
#include "ConfigReader.h"

/*
process type 1 macro

handle: current file handle
errorList: list of errors
handleList: list of handles
line: current line, expects macro at the start
lineLength: length of the line
lineCount: current line number, updated if macro skips lines
curCol: start index for the error messages
includeStack: return stack for included files
ifStack: scope stack for the if statements
defines: defined constant information
macroDefs: defined macros
isInMacro: if a macro is currently being read
macroData: output to the location of the macro definition, NULL if macro definitions should be skipped
macroDeleteTracker: list of the argument lists for the macros

returns: handle to new "main" file
*/
FileHandle* executeType1Macro(FileHandle* handle, List* errorList, List* handleList, char* line, int lineLength, unsigned int* lineCount, unsigned int curCol, Stack* includeStack, Stack* ifStack, StringTable defines, StringTable macroDefs, char* isInMacro, PosData* macroData, List* macroDeleteTracker) {
    // track macro endings
    static char* curMacro = NULL;

    // get the macro to execute
    char* afterName;
    char* macroName = extractMacro(line, lineLength, &afterName);
    unsigned int updatedLength = 256 - (afterName - line);

    // handle the case of definitions being in a macro
    if (*isInMacro && (!strcmp(macroName, ".define") || !strcmp(macroName, ".redef") || !strcmp(macroName, ".undef") || !strcmp(macroName, ".macro"))) {
        char* errorStr = (char*)malloc(52 * sizeof(char));
        sprintf(errorStr, "Cannot update definitions inside a macro definition");
        ErrorData errorData = {errorStr, *lineCount, curCol, 1, handle};
        appendList(errorList, &errorData, sizeof(ErrorData));
        free(macroName);
        return handle;
    }

    // execute any type 1 macros
    if (!strcmp(macroName, ".incbin") || !strcmp(macroName, ".include")) {
        // include mode
        char incMode = !strcmp(macroName, ".incbin");

        // get the name
        char hasNameError = 0;
        char* afterString;
        unsigned int len;
        unsigned int i = countWhitespaceChars(afterName, updatedLength);
        char* fileName = readString(afterName + i, updatedLength - i, &afterString, &len);
        if (fileName == NULL) {
            char* errorStr = (char*)malloc(22 * sizeof(char));
            sprintf(errorStr, "Expected valid string");
            ErrorData errorData = {errorStr, *lineCount, 256 - updatedLength + curCol + i, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        // make sure the rest of the line is clear
        if (!isValidLineEnding(afterString, 256 - (afterString - line))) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, *lineCount, (afterString - line) + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            free(fileName);
            return handle;
        }

        // attempt to get the file handle
        char isValidated = 1;
        FileHandle* newHandle = getHandle(handleList, fileName, incMode);
        if (newHandle == NULL) {
            // open a new file
            isValidated = 0;
            FileHandle openHandle = {fopen(fileName, incMode ? "rb" : "r"), fileName, 0,  incMode};
            if (openHandle.fptr == NULL) {
                char* errorStr = (char*)malloc(18 * sizeof(char) + strlen(fileName));
                sprintf(errorStr, "Could not open %s", fileName);
                ErrorData errorData = {errorStr, *lineCount, 256 - updatedLength + curCol + i, strlen(fileName) + 2, handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                free(macroName);
                free(fileName);
                return handle;
            }
            fseek(openHandle.fptr, 0, SEEK_END);
            fgetpos(openHandle.fptr, &(openHandle.length));
            rewind(openHandle.fptr);
            appendList(handleList, &openHandle, sizeof(FileHandle));
            newHandle = (FileHandle*)indexList(handleList, -1);
        }

        // stop on incbin
        if (incMode) {free(macroName); return handle;}

        // validate the file
        if (!isValidated && validateFile(newHandle, errorList)) {free(macroName); return handle;}

        // push to the include stack
        fpos_t positionPreserve;
        fgetpos(handle->fptr, &positionPreserve);
        IncludeReturnData retData = {handle, positionPreserve, *lineCount, errorList->size};
        pushStack(includeStack, &retData, sizeof(IncludeReturnData));

        // check for circular dependency
        char hasDepError = 0;
        for (Node* node = includeStack->head; node != NULL; node = node->next) {
            if (!strcmp(fileName, ((IncludeReturnData*)(node->dataptr))->returnFile->name)) {hasDepError = 1; break;}
        }
        if (hasDepError) {
            // generate circular dependency report
            unsigned int nameLen = strlen(fileName) + 1;
            for (Node* node = includeStack->head; node != NULL; node = node->next) {
                nameLen += strlen(((IncludeReturnData*)(node->dataptr))->returnFile->name) + 4;
                if (!strcmp(fileName, ((IncludeReturnData*)(node->dataptr))->returnFile->name)) {break;}
            }
            char* depStr = (char*)malloc(nameLen * sizeof(char));
            sprintf(depStr, "%s", fileName);
            for (Node* node = includeStack->head; node != NULL; node = node->next) {
                sprintf(depStr, "%s <- %s", depStr, ((IncludeReturnData*)(node->dataptr))->returnFile->name);
                if (!strcmp(fileName, ((IncludeReturnData*)(node->dataptr))->returnFile->name)) {break;}
            }
            
            // push the error
            char* errorStr = (char*)malloc((29 + strlen(depStr)) * sizeof(char));
            sprintf(errorStr, "Circular file dependency: %s", depStr);
            ErrorData errorData = {errorStr, *lineCount, curCol, (afterString - line), handle};
            appendList(errorList, &errorData, sizeof(ErrorData));

            // pop the return value and return
            free(popStack(includeStack));
            free(macroName);
            return handle;
        }

        // don't include an empty file
        fseek(newHandle->fptr, 0, SEEK_END);
        long endPos = ftell(newHandle->fptr);
        if (endPos == 0) {
            free(popStack(includeStack));
            free(macroName);
            return handle;
        }

        // return the new open file
        rewind(newHandle->fptr);
        *lineCount = -1;
        free(macroName);
        return newHandle;
    } else if (!strcmp(macroName, ".define") || !strcmp(macroName, ".redef")) {
        // get the name
        char* afterVar;
        unsigned int i = countWhitespaceChars(afterName, 249 - curCol);
        char* varName = getVarName(afterName + i, 249 - curCol - i, &afterVar);
        uint16_t assignValue = 0;
        if (varName == NULL) {
            char* errorStr = (char*)malloc(20 * sizeof(char));
            sprintf(errorStr, "Expected identifier");
            ErrorData errorData = {errorStr, *lineCount, 7 + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(varName);
            free(macroName);
            return handle;
        }

        // parse expression
        if (!isValidLineEnding(afterVar, 256 - (afterVar - line))) {
            ExprErrorShort exprOut = evalShortExpr(afterVar, strlen(afterVar), defines, defines);
            if (exprOut.errorMessage != NULL) {
                ErrorData errorData = {exprOut.errorMessage, *lineCount, (afterVar - line) + curCol + exprOut.errorPos, exprOut.errorLen, handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                free(varName);
                free(macroName);
                return handle;
            }
            assignValue = exprOut.val;
        }

        // add the assignment to the table with warnings
        if (!strcmp(macroName, ".define") && readStringTable(defines, varName, strlen(varName) + 1) != NULL) {
            printf("\e[1;33mWARNING:\e[0;1m %s, line %d:\e[0m redefinition of \"%s\" (consider using .redef)\n\n", handle->name, *lineCount, varName);
        } else if (!strcmp(macroName, ".redef") && readStringTable(defines, varName, strlen(varName) + 1) == NULL) {
            char* errorStr = (char*)malloc((20 + strlen(varName)) * sizeof(char));
            sprintf(errorStr, "\"%s\" is not defined", varName);
            ErrorData errorData = {errorStr, *lineCount, 6 + i + curCol, strlen(varName), handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(varName);
            free(macroName);
            return handle;
        }
        setStringTableValue(defines, varName, strlen(varName) + 1, &assignValue, 2);
        free(varName);

        // expression guarantees no garbage
    } else if (!strcmp(macroName, ".undef")) {
        // get the name
        char* afterVar;
        unsigned int i = countWhitespaceChars(afterName, 249 - curCol);
        char* varName = getVarName(afterName + i, 249 - curCol - i, &afterVar);
        if (varName == NULL) {
            char* errorStr = (char*)malloc(20 * sizeof(char));
            sprintf(errorStr, "Expected identifier");
            ErrorData errorData = {errorStr, *lineCount, 7 + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(varName);
            free(macroName);
            return handle;
        }

        // ensure no garbage
        if (!isValidLineEnding(afterVar, 256 - (afterVar - line))) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, *lineCount, (afterVar - line) + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(varName);
            free(macroName);
            return handle;
        }

        // undefine with warning
        if (readStringTable(defines, varName, strlen(varName) + 1) == NULL) {
            printf("\e[1;33mWARNING:\e[0;1m %s, line %d:\e[0m undefining an undefined value \"%s\"\n\n", handle->name, *lineCount, varName);
        } else {removeStringTableValue(defines, varName, strlen(varName) + 1);}
        free(varName);
    } else if (!strcmp(macroName, ".if") || !strcmp(macroName, ".ifdef") || !strcmp(macroName, ".ifndef")) {
        // run all if statements until success or end
        char ifVal = macroIf(handle, errorList, macroName, afterName, updatedLength, *lineCount, curCol + strlen(macroName), defines);
        PosData ifData = {*lineCount, curCol};
        int errorCount = errorList->size;
        while (!ifVal) {
            // run the new line to check
            char* newLine = skipIf(handle, errorList, &ifData, 1);
            if (errorList->size > errorCount) {free(newLine); free(macroName); return handle;}
            *lineCount = ifData.line;
            int i = countWhitespaceChars(newLine, 256);
            macroName = extractMacro(newLine + i, lineLength, &afterName);
            if (!strcmp(macroName, ".endif")) {free(macroName); return handle;}
            updatedLength = 256 - (afterName - line);
            ifVal = macroIf(handle, errorList, macroName, afterName, updatedLength, *lineCount, strlen(macroName), defines);
            free(newLine);
        }

        // push the success to the if stack
        pushStack(ifStack, &ifData, sizeof(PosData));
    } else if (!strcmp(macroName, ".else") || !strcmp(macroName, ".elseif") || !strcmp(macroName, ".elseifdef") || !strcmp(macroName, ".elseifndef") || !strcmp(macroName, ".endif")) {
        // make sure that there is an "if" to pull from
        if (ifStack->size == 0) {
            char* errorStr = (char*)malloc(13 * sizeof(char));
            sprintf(errorStr, "Expected .if");
            ErrorData errorData = {errorStr, *lineCount, curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
        }

        // pop the scope
        PosData* ifData = (PosData*)popStack(ifStack);

        // endif is done
        if (!strcmp(macroName, ".endif")) {free(ifData); free(macroName); return handle;}

        // go to the end
        free(skipIf(handle, errorList, ifData, 0));
        free(ifData);
    } else if (!strcmp(macroName, ".macro")) {
        // get name
        int i = countWhitespaceChars(afterName, 249 - curCol);
        afterName += i;
        char* defName = extractVar(&afterName, 249 - i - curCol);
        int errPos = (afterName - line);
        if (defName == NULL) {
            char* errorStr = (char*)malloc(20 * sizeof(char));
            sprintf(errorStr, "Expected identifier");
            ErrorData errorData = {errorStr, *lineCount, 7 + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        // make sure there is no comma
        i = countWhitespaceChars(afterName, strlen(afterName) + 1);
        if (afterName[i] == ',') {
            char* errorStr = (char*)malloc(20 * sizeof(char));
            sprintf(errorStr, "Expected identifier");
            ErrorData errorData = {errorStr, *lineCount, 7 + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            free(defName);
            return handle;
        }

        // get the args
        List* macroVars = extractMacroArgs(afterName, strlen(afterName) + 1, &afterName);
        if (macroVars == NULL) {
            char* errorStr = (char*)malloc(27 * sizeof(char));
            sprintf(errorStr, "Could not parse parameters");
            ErrorData errorData = {errorStr, *lineCount, errPos + curCol, strlen(afterName), handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            free(defName);
            return handle;
        }

        // no trailing garbage from parameter parse

        // define the macro
        *isInMacro = 1;
        fpos_t pos;
        fgetpos(handle->fptr, &pos);
        MacroDefData defData = {handle, pos, pos, *lineCount, *lineCount, macroVars};
        setStringTableValue(macroDefs, defName, strlen(defName) + 1, &defData, sizeof(MacroDefData));
        appendList(macroDeleteTracker, &(defData.vars), sizeof(List));
        curMacro = defName;
    } else if (!strcmp(macroName, ".endmacro")) {
        // ensure a macro is being ended
        if (!(*isInMacro)) {
            char* errorStr = (char*)malloc(16 * sizeof(char));
            sprintf(errorStr, "Expected .macro");
            ErrorData errorData = {errorStr, *lineCount, curCol, 9, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        // end the macro
        *isInMacro = 0;

        // handle trailing garbage problem
        if (!isValidLineEnding(afterName, 247 - curCol)) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, *lineCount, 9 + curCol, strlen(afterName), handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(curMacro);
        }

        // set the macro ending
        MacroDefData* macroData = (MacroDefData*)readStringTable(macroDefs, curMacro, strlen(curMacro) + 1);
        macroData->lines = *lineCount - macroData->line;
        fgetpos(handle->fptr, &(macroData->end));
        free(curMacro);
    }

    // no change in file
    free(macroName);
    return handle;
}

/*
process type 2 macro

handle: current file handle
errorList: list of errors
handleList: list of handles
line: current line, expects macro at the start
lineLength: length of the line
lineCount: current line number, updated if macro skips lines
curCol: start index for the error messages
includeStack: return stack for included files
ifStack: scope stack for the if statements
segStack: stack to push/pop segments
macroStack: macro callstack
defines: defined constant information
activeSeg: active segment
segments: list of segments
macroDefs: macro definitions
wordSize: size of the word in addresses accessed

returns: handle to new "main" file
*/
FileHandle* executeType2Macro(FileHandle* handle, List* errorList, List* handleList, char* line, int lineLength, unsigned int* lineCount, unsigned int curCol, Stack* includeStack, Stack* ifStack, Stack* segStack, Stack* macroStack, StringTable defines, SegmentDef** activeSeg, List* segments, StringTable macroDefs, int wordSize) {
    // track .byte warning
    static char byteWarningPrinted = 0;

    // get the macro to execute
    char* afterName;
    char* macroName = extractMacro(line, lineLength, &afterName);
    unsigned int updatedLength = 256 - (afterName - line);

    // execute type 2 macros
    if (!strcmp(macroName, ".segment")) {
        // get the name
        char hasNameError = 0;
        char* afterString;
        unsigned int len;
        unsigned int i = countWhitespaceChars(afterName, updatedLength);
        char* segName = readString(afterName + i, updatedLength - i, &afterString, &len);
        if (segName == NULL) {
            char* errorStr = (char*)malloc(22 * sizeof(char));
            sprintf(errorStr, "Expected valid string");
            ErrorData errorData = {errorStr, *lineCount, 256 - updatedLength + curCol + i, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        // change to the appropriate segment
        char foundSeg = 0;
        for (Node* node = segments->head; node != NULL; node = node->next) {
            SegmentDef* segDef = (SegmentDef*)(node->dataptr);
            if (!strcmp(segDef->name, segName)) {
                foundSeg = 1;
                *activeSeg = segDef;
                break;
            }
        }
        if (!foundSeg) {
            char* errorStr = (char*)malloc((20 + strlen(segName)) * sizeof(char));
            sprintf(errorStr, "Invalid segment \"%s\"", segName);
            ErrorData errorData = {errorStr, *lineCount, curCol + i + 8, strlen(segName) + 2, handle};
            appendList(errorList, &errorData, sizeof(errorData));
        }

        // make sure the rest of the line is clear
        if (!isValidLineEnding(afterString, 256 - (afterString - line))) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, *lineCount, (afterString - line) + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
        }
        free(segName);
    } else if (!strcmp(macroName, ".pushseg")) {
        // make sure the rest of the line is clear
        if (!isValidLineEnding(afterName, 256 - (afterName - line))) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, *lineCount, (afterName - line) + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
        }

        // push the segment
        pushStack(segStack, activeSeg, sizeof(SegmentDef*));
    } else if (!strcmp(macroName, ".popseg")) {
        // make sure the rest of the line is clear
        if (!isValidLineEnding(afterName, 256 - (afterName - line))) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, *lineCount, (afterName - line) + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
        }

        // make sure there is a segment on the stack
        if (segStack->size == 0) {
            char* errorStr = (char*)malloc(25 * sizeof(char));
            sprintf(errorStr, "No segments on the stack");
            ErrorData errorData = {errorStr, *lineCount, curCol, 7, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        // push the segment
        SegmentDef** newSeg = (SegmentDef**)popStack(segStack);
        *activeSeg = *newSeg;
        free(newSeg);
    } else if (!strcmp(macroName, ".macro")) {
        // get name
        int i = countWhitespaceChars(afterName, 249 - curCol);
        afterName += i;
        char* defName = extractVar(&afterName, 249 - i - curCol); // known to be valid
        int errPos = (afterName - line);

        // skip the macro code
        MacroDefData* macroData = (MacroDefData*)readStringTable(macroDefs, defName, strlen(defName) + 1);
        *lineCount += macroData->lines;
        fsetpos(handle->fptr, &(macroData->end));
    } else if (!strcmp(macroName, ".endmacro")) {
        // stack is known to not be empty
        IncludeReturnData* retData = (IncludeReturnData*)popStack(macroStack);
        FileHandle* newHandle = retData->returnFile;
        *lineCount = retData->returnLine;
        fsetpos(newHandle->fptr, &(retData->filePosition));
        if (errorList->size > retData->errorCount) {
            char* errorStr = (char*)malloc(39 * sizeof(char));
            sprintf(errorStr, "An error occured inside the macro call");
            ErrorData errorData = {errorStr, *lineCount, 0, 1, newHandle};
            appendList(errorList, &errorData, sizeof(ErrorData));
        }
        free(retData);
        return newHandle;
    } else if (!strcmp(macroName, ".res")) {
        // handle no segment
        if (*activeSeg == NULL) {
            char* errorStr = (char*)malloc(18 * sizeof(char));
            sprintf(errorStr, "No active segment");
            ErrorData errorData = {errorStr, *lineCount, curCol, 7, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        ExprErrorShort exprOut = evalShortExpr(afterName, strlen(afterName), defines, defines);
        if (exprOut.errorMessage != NULL) {
            ErrorData errorData = {exprOut.errorMessage, *lineCount, exprOut.errorPos + (afterName - line) + curCol, exprOut.errorLen, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }
        (*activeSeg)->writeAddr += exprOut.val;
        if ((*activeSeg)->writeAddr > (*activeSeg)->size) {
            char* errorStr = (char*)malloc((25 + strlen((*activeSeg)->name)) * sizeof(char));
            sprintf(errorStr, "Segment %s size exceeded", (*activeSeg)->name);
            ErrorData errorData = {errorStr, *lineCount, curCol, 4, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return NULL;
        }
    } else if (!strcmp(macroName, ".word")) {
        // handle no segment
        if (*activeSeg == NULL) {
            char* errorStr = (char*)malloc(18 * sizeof(char));
            sprintf(errorStr, "No active segment");
            ErrorData errorData = {errorStr, *lineCount, curCol, 7, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        int argCount = countArgs(afterName, strlen(afterName));
        (*activeSeg)->writeAddr += argCount * wordSize;
        if ((*activeSeg)->writeAddr > (*activeSeg)->size) {
            char* errorStr = (char*)malloc((25 + strlen((*activeSeg)->name)) * sizeof(char));
            sprintf(errorStr, "Segment %s size exceeded", (*activeSeg)->name);
            ErrorData errorData = {errorStr, *lineCount, curCol, 5, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return NULL;
        }
    } else if (!strcmp(macroName, ".byte")) {
        // handle no segment
        if (*activeSeg == NULL) {
            char* errorStr = (char*)malloc(18 * sizeof(char));
            sprintf(errorStr, "No active segment");
            ErrorData errorData = {errorStr, *lineCount, curCol, 7, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        if (wordSize == 1 && !byteWarningPrinted) {
            printf("\e[1;33mWARNING:\e[0m Assembling in word mode; .byte data will be padded with an upper byte of 0.\n\n");
            byteWarningPrinted = 1;
        }
        int argCount = countArgs(afterName, strlen(afterName));
        (*activeSeg)->writeAddr += argCount;
        if ((*activeSeg)->writeAddr > (*activeSeg)->size) {
            char* errorStr = (char*)malloc((25 + strlen((*activeSeg)->name)) * sizeof(char));
            sprintf(errorStr, "Segment %s size exceeded", (*activeSeg)->name);
            ErrorData errorData = {errorStr, *lineCount, curCol, 5, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return NULL;
        }
    } else if (!strcmp(macroName, ".align")) {
        // handle no segment
        if (*activeSeg == NULL) {
            char* errorStr = (char*)malloc(18 * sizeof(char));
            sprintf(errorStr, "No active segment");
            ErrorData errorData = {errorStr, *lineCount, curCol, 7, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        ExprErrorShort exprOut = evalShortExpr(afterName, strlen(afterName), defines, defines);
        if (exprOut.errorMessage != NULL) {
            ErrorData errorData = {exprOut.errorMessage, *lineCount, exprOut.errorPos + (afterName - line) + curCol, exprOut.errorLen, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }
        exprOut.val = exprOut.val ? exprOut.val : 1;
        uint16_t ref = (*activeSeg)->writeAddr / (wordSize == 1 ? 2 : 1);
        uint16_t val = exprOut.val - (ref % exprOut.val);
        if (val == exprOut.val) {val = 0;}
        (*activeSeg)->writeAddr += val * (wordSize == 1 ? 2 : 1);
        if ((*activeSeg)->writeAddr > (*activeSeg)->size) {
            char* errorStr = (char*)malloc((25 + strlen((*activeSeg)->name)) * sizeof(char));
            sprintf(errorStr, "Segment %s size exceeded", (*activeSeg)->name);
            ErrorData errorData = {errorStr, *lineCount, curCol, 6, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return NULL;
        }
    } else if (!strcmp(macroName, ".incbin")) {
        // get the file, known to be open and valid
        char hasNameError = 0;
        char* afterString;
        unsigned int len;
        unsigned int i = countWhitespaceChars(afterName, updatedLength);
        char* fileName = readString(afterName + i, updatedLength - i, &afterString, &len);
        FileHandle* incHandle = getHandle(handleList, fileName, 1);

        // handle no segment
        if (*activeSeg == NULL) {
            char* errorStr = (char*)malloc(18 * sizeof(char));
            sprintf(errorStr, "No active segment");
            ErrorData errorData = {errorStr, *lineCount, curCol, 7, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(fileName);
            free(macroName);
            return handle;
        }

        // allocate space for the file
        if (wordSize == 1) {(*activeSeg)->writeAddr += (incHandle->length + 1) / 2;}
        else {(*activeSeg)->writeAddr += incHandle->length;}
        if ((*activeSeg)->writeAddr > (*activeSeg)->size) {
            char* errorStr = (char*)malloc((25 + strlen((*activeSeg)->name)) * sizeof(char));
            sprintf(errorStr, "Segment %s size exceeded", (*activeSeg)->name);
            ErrorData errorData = {errorStr, *lineCount, curCol, 7, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return NULL;
        }
        free(fileName);
    } else if (!strcmp(macroName, ".ascii") || !strcmp(macroName, ".asciiz")) {
        // handle no segment
        if (*activeSeg == NULL) {
            char* errorStr = (char*)malloc(18 * sizeof(char));
            sprintf(errorStr, "No active segment");
            ErrorData errorData = {errorStr, *lineCount, curCol, 7, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        // get the string
        char hasNameError = 0;
        char* afterString;
        unsigned int i = countWhitespaceChars(afterName, updatedLength);
        unsigned int len;
        char* string = readString(afterName + i, updatedLength - i, &afterString, &len);
        if (string == NULL) {
            char* errorStr = (char*)malloc(22 * sizeof(char));
            sprintf(errorStr, "Expected valid string");
            ErrorData errorData = {errorStr, *lineCount, 256 - updatedLength + curCol + i, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        // make sure the rest of the line is clear
        if (!isValidLineEnding(afterString, 256 - (afterString - line))) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, *lineCount, (afterString - line) + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            free(string);
            return handle;
        }

        // write file update
        if (!strcmp(macroName, ".ascii")) {(*activeSeg)->writeAddr += len - 1;}
        else {(*activeSeg)->writeAddr += len;}
        free(string);
        if ((*activeSeg)->writeAddr > (*activeSeg)->size) {
            char* errorStr = (char*)malloc((25 + strlen((*activeSeg)->name)) * sizeof(char));
            sprintf(errorStr, "Segment %s size exceeded", (*activeSeg)->name);
            ErrorData errorData = {errorStr, *lineCount, curCol, 7, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return NULL;
        }
    } else {
        char isInMacro = 0;
        free(macroName);
        return executeType1Macro(handle, errorList, handleList, line, lineLength, lineCount, curCol, includeStack, ifStack, defines, NULL, &isInMacro, NULL, NULL);
    }

    free(macroName);
    return handle;
}

/*
process type 3 macro

handle: current file handle
errorList: list of errors
handleList: list of handles
line: current line, expects macro at the start
lineLength: length of the line
lineCount: current line number, updated if macro skips lines
curCol: start index for the error messages
includeStack: return stack for included files
ifStack: scope stack for the if statements
segStack: stack to push/pop segments
macroStack: macro callstack
defines: defined constant information
activeSeg: active segment
segments: list of segments
macroDefs: macro definitions
wordSize: size of the word in addresses accessed
isLittleEndian: if the code is little endian
macroVars: variables to remove from scope if a macro ends
vars: variable table

returns: handle to new "main" file
*/
FileHandle* executeType3Macro(FileHandle* handle, List* errorList, List* handleList, char* line, int lineLength, unsigned int* lineCount, unsigned int curCol, Stack* includeStack, Stack* ifStack, Stack* segStack, Stack* macroStack, StringTable defines, SegmentDef** activeSeg, List* segments, StringTable macroDefs, int wordSize, char isLittleEndian, List* macroVars, StringTable vars) {
    // get the macro to execute
    char* afterName;
    char* macroName = extractMacro(line, lineLength, &afterName);
    unsigned int updatedLength = 256 - (afterName - line);

    if (!strcmp(macroName, ".res")) {
        // handle ro segment
        if ((*activeSeg)->accessType == ro) {
            char* errorStr = (char*)malloc((36 + strlen((*activeSeg)->name)) * sizeof(char));
            sprintf(errorStr, "Cannot reserve in read-only segment %s", (*activeSeg)->name);
            ErrorData errorData = {errorStr, *lineCount, curCol, 4, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        ExprErrorShort exprOut = evalShortExpr(afterName, strlen(afterName), vars, defines);
        if (exprOut.errorMessage != NULL) {
            ErrorData errorData = {exprOut.errorMessage, *lineCount, exprOut.errorPos + (afterName - line) + curCol, exprOut.errorLen, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        // fill 0s
        if ((*activeSeg)->accessType != bss) {
            for (int i = 0; i < exprOut.val * (wordSize == 1 ? 2 : 1); i++) {
                (*activeSeg)->outputArr[i + (*activeSeg)->writeAddr] = 0;
            }
        }
        (*activeSeg)->writeAddr += exprOut.val * (wordSize == 1 ? 2 : 1);
    } else if (!strcmp(macroName, ".word")) {
        List* args = extractArgs(afterName, strlen(afterName));
        char hasError = 0;
        for (Node* node = args->head; node != NULL; node = node->next){
            char* arg = *(char**)(node->dataptr);
            if (!hasError) {
                ExprErrorShort exprOut = evalShortExpr(arg, strlen(arg), vars, defines);
                if (exprOut.errorMessage != NULL) {
                    char* errorStr = (char*)malloc((30 + strlen(exprOut.errorMessage)) * sizeof(char));
                    sprintf(errorStr, "Could not parse arguments: %s", exprOut.errorMessage);
                    free(exprOut.errorMessage);
                    ErrorData errorData = {errorStr, *lineCount, curCol + 5, strlen(line) - 5, handle};
                    appendList(errorList, &errorData, sizeof(ErrorData));
                    hasError = 1;
                }
                if (isLittleEndian) {
                    uint16_t val = exprOut.val;
                    (*activeSeg)->outputArr[(*activeSeg)->writeAddr] = (val & 0x00ff);
                    (*activeSeg)->outputArr[(*activeSeg)->writeAddr + 1] = ((val >> 8) & 0x00ff);
                } else {
                    uint16_t val = exprOut.val;
                    (*activeSeg)->outputArr[(*activeSeg)->writeAddr + 1] = (val & 0x00ff);
                    (*activeSeg)->outputArr[(*activeSeg)->writeAddr] = ((val >> 8) & 0x00ff);
                }
            }
            (*activeSeg)->writeAddr += 2;
            free(arg);
        }
        deleteList(args);
    } else if (!strcmp(macroName, ".byte")) {
        List* args = extractArgs(afterName, strlen(afterName));
        char hasError = 0;
        for (Node* node = args->head; node != NULL; node = node->next){
            char* arg = *(char**)(node->dataptr);
            if (!hasError) {
                ExprErrorShort exprOut = evalShortExpr(arg, strlen(arg), vars, defines);
                if (exprOut.errorMessage != NULL) {
                    char* errorStr = (char*)malloc((30 + strlen(exprOut.errorMessage))* sizeof(char));
                    sprintf(errorStr, "Could not parse arguments: %s", exprOut.errorMessage);
                    free(exprOut.errorMessage);
                    ErrorData errorData = {errorStr, *lineCount, curCol + 5, strlen(line) - 5, handle};
                    appendList(errorList, &errorData, sizeof(ErrorData));
                    hasError = 1;
                }
                if (wordSize == 1) {
                    if (isLittleEndian) {
                        uint16_t val = exprOut.val;
                        (*activeSeg)->outputArr[(*activeSeg)->writeAddr] = (val & 0x00ff);
                        (*activeSeg)->outputArr[(*activeSeg)->writeAddr + 1] = 0;
                    } else {
                        uint16_t val = exprOut.val;
                        (*activeSeg)->outputArr[(*activeSeg)->writeAddr + 1] = (val & 0x00ff);
                        (*activeSeg)->outputArr[(*activeSeg)->writeAddr] = 0;
                    }
                } else {
                    uint16_t val = exprOut.val;
                    (*activeSeg)->outputArr[(*activeSeg)->writeAddr] = (val & 0x00ff);
                }
            }
            (*activeSeg)->writeAddr += wordSize == 1 ? 2 : 1;
            free(arg);
        }
        deleteList(args);
    } else if (!strcmp(macroName, ".align")) {
        ExprErrorShort exprOut = evalShortExpr(afterName, strlen(afterName), vars, defines);
        if (exprOut.errorMessage != NULL) {
            ErrorData errorData = {exprOut.errorMessage, *lineCount, exprOut.errorPos + (afterName - line) + curCol, exprOut.errorLen, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }
        exprOut.val = exprOut.val ? exprOut.val : 1;
        uint16_t ref = (*activeSeg)->writeAddr / (wordSize == 1 ? 2 : 1);
        uint16_t val = exprOut.val - (ref % exprOut.val);
        if (val == exprOut.val) {val = 0;}

        // fill 0s
        if ((*activeSeg)->accessType != bss) {
            for (int i = 0; i < val * (wordSize == 1 ? 2 : 1); i++) {
                (*activeSeg)->outputArr[i + (*activeSeg)->writeAddr] = 0;
            }
        }
        (*activeSeg)->writeAddr += val * (wordSize == 1 ? 2 : 1);
    } else if (!strcmp(macroName, ".incbin")) {
        // get the file, known to be open and valid
        char hasNameError = 0;
        char* afterString;
        unsigned int len;
        unsigned int i = countWhitespaceChars(afterName, updatedLength);
        char* fileName = readString(afterName + i, updatedLength - i, &afterString, &len);
        FileHandle* incHandle = getHandle(handleList, fileName, 1);

        // read the file
        rewind(incHandle->fptr);
        fread((*activeSeg)->outputArr + (*activeSeg)->writeAddr, 1, incHandle->length, incHandle->fptr);
        (*activeSeg)->writeAddr += incHandle->length;
        if (wordSize == 1) {
            if (!isLittleEndian && (incHandle->length % 2) == 1) {
                uint8_t val = (*activeSeg)->outputArr[(*activeSeg)->writeAddr - 1];
                (*activeSeg)->outputArr[(*activeSeg)->writeAddr - 1] = 0;
                (*activeSeg)->outputArr[(*activeSeg)->writeAddr] = val;
            }
            if ((incHandle->length % 2) == 1) {(*activeSeg)->writeAddr += 1;}
        }
        free(fileName);
    } else if (!strcmp(macroName, ".error")) {
        // get the string
        char hasNameError = 0;
        char* afterString;
        unsigned int i = countWhitespaceChars(afterName, updatedLength);
        unsigned int len;
        char* string = readString(afterName + i, updatedLength - i, &afterString, &len);
        if (string == NULL) {
            char* errorStr = (char*)malloc(22 * sizeof(char));
            sprintf(errorStr, "Expected valid string");
            ErrorData errorData = {errorStr, *lineCount, 256 - updatedLength + curCol + i, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        // make sure the rest of the line is clear
        if (!isValidLineEnding(afterString, 256 - (afterString - line))) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, *lineCount, (afterString - line) + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            free(string);
            return handle;
        }

        // append the error
        ErrorData errorData = {string, *lineCount, curCol, 6, handle};
        appendList(errorList, &errorData, sizeof(ErrorData));
    } else if (!strcmp(macroName, ".warning")) {
        // get the string
        char hasNameError = 0;
        char* afterString;
        unsigned int i = countWhitespaceChars(afterName, updatedLength);
        unsigned int len;
        char* string = readString(afterName + i, updatedLength - i, &afterString, &len);
        if (string == NULL) {
            char* errorStr = (char*)malloc(22 * sizeof(char));
            sprintf(errorStr, "Expected valid string");
            ErrorData errorData = {errorStr, *lineCount, 256 - updatedLength + curCol + i, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            return handle;
        }

        // make sure the rest of the line is clear
        if (!isValidLineEnding(afterString, 256 - (afterString - line))) {
            char* errorStr = (char*)malloc(28 * sizeof(char));
            sprintf(errorStr, "Unexpected trailing garbage");
            ErrorData errorData = {errorStr, *lineCount, (afterString - line) + curCol, 1, handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(macroName);
            free(string);
            return handle;
        }

        // append the error
        printf("\e[1;33mWARNING:\e[0m %s\n\n", string);
        free(string);
    } else if (!strcmp(macroName, ".ascii") || !strcmp(macroName, ".asciiz")) {
        // get the string, known to be valid
        char hasNameError = 0;
        char* afterString;
        unsigned int i = countWhitespaceChars(afterName, updatedLength);
        unsigned int len;
        char* string = readString(afterName + i, updatedLength - i, &afterString, &len);

        // get the length
        unsigned int stringSize = strlen(string);
        if (!strcmp(macroName, ".asciiz")) {stringSize++;}

        // write the string
        for (int j = 0; j < stringSize; j++) {
            if (wordSize == 1) {
                (*activeSeg)->outputArr[(*activeSeg)->writeAddr + 2 * j + (isLittleEndian ? 0 : 1)] = string[j];
            } else {
                (*activeSeg)->outputArr[(*activeSeg)->writeAddr + j] = string[j];
            }
        }
        (*activeSeg)->writeAddr += stringSize * (wordSize == 1 ? 2 : 1);
        free(string);
    } else if (!strcmp(macroName, ".endmacro")) {
        // undefine the macro vars
        for (Node* node = macroVars->head; node != NULL; node = node->next) {
            char* varName = *(char**)(node->dataptr);
            removeStringTableValue(vars, varName, strlen(varName) + 1);
        }
        deleteNode(macroVars->head);
        macroVars->head = NULL;
        macroVars->tail = NULL;
        macroVars->size = 0;

        // stack is known to not be empty
        IncludeReturnData* retData = (IncludeReturnData*)popStack(macroStack);
        FileHandle* newHandle = retData->returnFile;
        *lineCount = retData->returnLine;
        fsetpos(newHandle->fptr, &(retData->filePosition));
        if (errorList->size > retData->errorCount) {
            char* errorStr = (char*)malloc(39 * sizeof(char));
            sprintf(errorStr, "An error occured inside the macro call");
            ErrorData errorData = {errorStr, *lineCount, 0, 1, newHandle};
            appendList(errorList, &errorData, sizeof(ErrorData));
        }
        free(retData);
        return newHandle;
    } else if (isValidMacroName(macroName)){
        free(macroName);
        return executeType2Macro(handle, errorList, handleList, line, lineLength, lineCount, curCol, includeStack, ifStack, segStack, macroStack, defines, activeSeg, segments, macroDefs, wordSize);
    } else {
        char* errorStr = malloc((18 + strlen(macroName)) * sizeof(char));
        sprintf(errorStr, "Invalid macro: %s", macroName);
        ErrorData errorData = {errorStr, *lineCount, curCol, strlen(macroName), handle};
        appendList(errorList, &errorData, sizeof(ErrorData));
    }

    free(macroName);
    return handle;
}

#endif