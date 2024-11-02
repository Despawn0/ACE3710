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
static List* getVars(char* expr, int exprLen) {
    List* outputList = newList();
    for (int i = 0; i < exprLen; i++) {
        if (expr[i] == '\0' || expr[i] == '\n' || expr[i] == ';') {break;}
        if ((expr[i] >= 'a' && expr[i] <= 'z') || (expr[i] >= 'A' && expr[i] <= 'Z') || expr[i] == '_' || expr[i] == '@') {
            char* outputPos;
            char* varName = getVarName(expr + i, exprLen - i, &outputPos);
            appendList(outputList, &varName, sizeof(char*));
            int oldi = i;
            i = (outputPos - expr);
        }
    }
    return outputList;
}

/*
evaluates a variable assignment

toEvaluate: name to resolve
toEvaluateLut: LUT to get other vars to evaluate
parse: output parse information

returns: if an error occured
*/
static char evaluateVar(List* errorList, char* toEvaluate, StringTable toEvaluateLut, StringTable varDefs, StringTable macroDefs, List* visited) {
    int evalLength = strlen(toEvaluate) + 1;

    // nothing needed if already evaluated
    if (readStringTable(varDefs, toEvaluate, evalLength) != NULL) {
        return 0;
    }

    // don't evaluate a bad var
    if (readStringTable(toEvaluateLut, toEvaluate, evalLength) == NULL) {
        return 0;
    }

    // get information on the variable
    VarEvalData evalData = *(VarEvalData*)readStringTable(toEvaluateLut, toEvaluate, evalLength);

    // check for circular dependency
    for (Node* nodei = visited->head; nodei != NULL; nodei = nodei->next) {
        if (!strcmp(*(char**)(nodei->dataptr), toEvaluate)) {
            // get the needed chain length
            int len;
            for (Node* nodej = nodei; nodej != NULL; nodej = nodej->next) {
                len += 4 + strlen(*(char**)(nodej->dataptr));
            }
            
            // get the dependency loop
            char nameStr[len + 1];
            sprintf(nameStr, "%s", *(char**)(nodei->dataptr));
            for (Node* nodej = nodei->next; nodej != NULL; nodej = nodej->next) {
                sprintf(nameStr, "%s <- %s", nameStr, *(char**)(nodej->dataptr));
            }

            // push the error
            char* errorStr = (char*)malloc((35 + evalLength) * sizeof(char));
            sprintf(errorStr, "Circular dependency: %s <- %s", nameStr, toEvaluate);
            ErrorData error = {errorStr, evalData.line, 0, strlen(toEvaluate) + 1, evalData.handle};
            appendList(errorList, &error, sizeof(ErrorData));
            return 1;
        }
    }

    // evaluate the dependencies
    appendList(visited, &toEvaluate, sizeof(char*));
    char retVal = 0;
    for (Node* node = evalData.dependencies->head; node != NULL; node = node->next) {
        char* curDep = *(char**)(node->dataptr);
        if (readStringTable(macroDefs, curDep, strlen(curDep) + 1) == NULL) {
            retVal |= evaluateVar(errorList, *(char**)(node->dataptr), toEvaluateLut, varDefs, macroDefs, visited);
        }
    }
    removeListElement(visited, -1);
    // evaluate the current node
    if (!retVal) {
        // update definitions
        List* updatedDefs = newList();
        for (Node* node = evalData.dependencies->head; node != NULL; node = node->next) {
            char* name = *(char**)(node->dataptr);
            uint16_t* val = (uint16_t*)readStringTable(macroDefs, name, strlen(name) + 1);
            if (val != NULL) {
                DefData defData = {name, *val, 0, 1};
                appendList(updatedDefs, &defData, sizeof(DefData));
                removeStringTableValue(macroDefs, name, strlen(name) + 1);
            }
        }
        for (Node* node = evalData.defines->head; node != NULL; node = node->next) {
            DefData* val = (DefData*)(node->dataptr);
            DefData newData = *val;
            uint16_t* oldVal = (uint16_t*)readStringTable(macroDefs, val->name, strlen(val->name) + 1);
            if (oldVal != NULL) {newData.val = *oldVal;}
            else {newData.hasValue = 0;}
            appendList(updatedDefs, &newData, sizeof(DefData));
            setStringTableValue(macroDefs, val->name, strlen(val->name) + 1, &(val->val), 2);
        }
        deleteList(evalData.defines);

        // handle the evaluation
        ExprErrorShort eval = evalShortExpr(evalData.expr, evalData.exprLen, varDefs, macroDefs);

        // undo def updates
        for (Node* node = updatedDefs->head; node != NULL; node = node->next) {
            DefData* val = (DefData*)(node->dataptr);
            if (val->hasValue) {
                setStringTableValue(macroDefs, val->name, strlen(val->name) + 1, &(val->val), 2);
            } else {
                removeStringTableValue(macroDefs, val->name, strlen(val->name));
            }
            if (val->canFree) {free(val->name);}
        }
        deleteList(updatedDefs);

        // handle errors
        if (eval.errorMessage != NULL) {
            ErrorData error = {eval.errorMessage, evalData.line, eval.errorPos + evalData.col, eval.errorLen, evalData.handle};
            appendList(errorList, &error, sizeof(ErrorData));
            return 1;
        }
        setStringTableValue(varDefs, toEvaluate, evalLength, &(eval.val), 2);

        // remove from the unevaluated set
        for (Node* node = evalData.dependencies->head; node != NULL; node = node->next) {
            free(*(char**)(node->dataptr));
        }
        deleteList(evalData.dependencies);
        free(evalData.expr);
        removeStringTableValue(toEvaluateLut, toEvaluate, evalLength);
    }

    return retVal;
}

/*
evaluates all local variables between global vars

handle: handle to the file to read; no lines over 255 chars
errorList: list of errors
handleList: list of open handles
segments: segments defined in the configuration
instructionSize: size of the instructions in "words"

returns: parsed global vars
*/
StringTable readGlobalVars(FileHandle* handle, List* errorList, List* handleList, List* segments, StringTable macroDefs, int instructionSize) {
    // setup
    unsigned int lineCount = 0;
    char line[256];
    StringTable varDefs = newStringTable();
    StringTable defines = newStringTable();
    StringTable toEvaluateLut = newStringTable();
    List* toEvaluate = newList();
    SegmentDef* activeSegment = NULL;
    fpos_t filePos;
    Stack* includeStack = newStack();
    Stack* ifStack = newStack();
    Stack* segStack = newStack();
    Stack* macroStack = newStack();

    // add registers
    const uint16_t vals_[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    setStringTableValue(varDefs, "r0", 3, vals_ + 0, 2);
    setStringTableValue(varDefs, "r1", 3, vals_ + 1, 2);
    setStringTableValue(varDefs, "r2", 3, vals_ + 2, 2);
    setStringTableValue(varDefs, "r3", 3, vals_ + 3, 2);
    setStringTableValue(varDefs, "r4", 3, vals_ + 4, 2);
    setStringTableValue(varDefs, "r5", 3, vals_ + 5, 2);
    setStringTableValue(varDefs, "r6", 3, vals_ + 6, 2);
    setStringTableValue(varDefs, "r7", 3, vals_ + 7, 2);
    setStringTableValue(varDefs, "r8", 3, vals_ + 8, 2);
    setStringTableValue(varDefs, "r9", 3, vals_ + 9, 2);
    setStringTableValue(varDefs, "r10", 4, vals_ + 10, 2);
    setStringTableValue(varDefs, "r11", 4, vals_ + 11, 2);
    setStringTableValue(varDefs, "r12", 4, vals_ + 12, 2);
    setStringTableValue(varDefs, "r13", 4, vals_ + 13, 2);
    setStringTableValue(varDefs, "r14", 4, vals_ + 14, 2);
    setStringTableValue(varDefs, "r15", 4, vals_ + 15, 2);
    setStringTableValue(varDefs, "ra", 3, vals_ + 14, 2);
    setStringTableValue(varDefs, "sp", 3, vals_ + 15, 2);

    while (1) {
        // handle new line eof
        fgetpos(handle->fptr, &filePos);
        if (filePos == handle->length) {fgets(line, 256, handle->fptr);}

        // handle end of return
        if (feof(handle->fptr) && includeStack->size > 0) {
            line[0] = '\0';
            handle = includeReturn(includeStack, &lineCount);
            lineCount++;
        }

        // kill on eof
        if (feof(handle->fptr)) {break;}

        fgets(line, 256, handle->fptr);

        // handle non-var lines
        if (!isValidNameChar(line[0]) || (line[0] >= '0' && line[0] <= '9')) {
            // error on the start of the line
            if (line[0] == '.') {
                    handle = executeType2Macro(handle, errorList, handleList, line, strlen(line), &lineCount, 0, includeStack, ifStack, segStack, macroStack, defines, &activeSegment, segments, macroDefs, instructionSize);
                    if (handle == NULL) {break;}
            } else if (!isWhitespace(line[0]) && line[0] != '\0' && line[0] != '@' && line[0] != ';') {
                // get the length of the error
                int i;
                for (i = 0; i < 256; i++) {
                    if (isWhitespace(line[i]) || line[i] == '\0' || line[i] == ';') {break;}
                }
                line[i] = '\0';

                // append the error
                char* errorStr = (char*)malloc((36 + i) * sizeof(char));
                sprintf(errorStr, "Invalid constant or lable name \"%s\"", line);
                ErrorData error = {errorStr, lineCount, 0, i, handle};
                appendList(errorList, &error, sizeof(ErrorData));
            } else if (isWhitespace(line[0])) {
                unsigned int i = countWhitespaceChars(line, strlen(line));
                if (line[i] == '.') {
                    handle = executeType2Macro(handle, errorList, handleList, line + i, strlen(line + i), &lineCount, i, includeStack, ifStack, segStack, macroStack, defines, &activeSegment, segments, macroDefs, instructionSize);
                    if (handle == NULL) {break;}
                } else if (!isValidLineEnding(line, strlen(line))) {
                    // check for macro
                    char* afterMacro = line + i;
                    char* macroName = extractVar(&afterMacro, strlen(line + i));
                    if (readStringTable(macroDefs, macroName, strlen(macroName) + 1) != NULL) {

                        // transfer position to the macro
                        IncludeReturnData retData = {handle, 0, lineCount, errorList->size};
                        fgetpos(handle->fptr, &(retData.filePosition));
                        pushStack(macroStack, &retData, sizeof(IncludeReturnData));
                        MacroDefData macroData = *(MacroDefData*)readStringTable(macroDefs, macroName, strlen(macroName) + 1);
                        handle = macroData.handle;
                        lineCount = macroData.line + 1;
                        fsetpos(handle->fptr, &(macroData.start));
                        continue;
                    }
                    // handle as instruction
                    if (activeSegment == NULL) {
                        char* errorStr = (char*)malloc(18 * sizeof(char));
                        sprintf(errorStr, "No active segment");
                        ErrorData errorData = {errorStr, lineCount, i, strlen(macroName), handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                        lineCount++;
                        continue;
                    }
                    activeSegment->writeAddr += instructionSize;
                    if (activeSegment->writeAddr > activeSegment->size) {
                        char* errorStr = (char*)malloc((25 + strlen(activeSegment->name)) * sizeof(char));
                        sprintf(errorStr, "Segment %s size exceeded", activeSegment->name);
                        ErrorData errorData = {errorStr, lineCount, 0, 1, handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                        break;
                    }
                }
            }

            lineCount++;
            continue;
        }

        // read the var name
        char* endOfVar;
        char* name = getVarName(line, 256, &endOfVar);
        int nameLength = (endOfVar - line);

        // no definitions in a macro
        if (macroStack->size > 0) {
            // prevent a stack trace
            for (Node* node = macroStack->head; node != NULL; node = node->next) {
                IncludeReturnData* retData = (IncludeReturnData*)(node->dataptr);
                (retData->errorCount)++;
            }

            // error
            char* errorStr = (char*)malloc(54 * sizeof(char));
            sprintf(errorStr, "Cannot declare global variables in a macro definition");
            ErrorData errorData = {errorStr, lineCount, 0, strlen(name), handle};
            appendList(errorList, &errorData, sizeof(ErrorData));
            free(name);
            lineCount++;
            continue;
        }

        // prevent repeat definitions
        if (readStringTable(varDefs, name, nameLength + 1) != NULL || readStringTable(toEvaluateLut, name, nameLength + 1) != NULL) {
            // append repeat error
            line[nameLength] = '\0';
            char* errorStr = (char*)malloc((45 + nameLength) * sizeof(char));
            sprintf(errorStr, "Repeat definition for constant or lable \"%s\"", line);
            ErrorData error = {errorStr, lineCount, 0, nameLength, handle};
            appendList(errorList, &error, sizeof(ErrorData));
            lineCount++;
            free(name);
            continue;
        }

        // handle label
        if (endOfVar[0] == ':') {
            if (activeSegment == NULL) {
                char* errorStr = (char*)malloc(18 * sizeof(char));
                sprintf(errorStr, "No active segment");
                ErrorData errorData = {errorStr, lineCount, (endOfVar - line), 1, handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                lineCount++;
                free(name);
                continue;
            }
            uint16_t writeVal = activeSegment->writeAddr + activeSegment->startAddr;
            setStringTableValue(varDefs, name, nameLength + 1, &writeVal, 2);
            lineCount++;
            free(name);
            continue;
        }

        // drop space
        for (int i = nameLength; i < 256; i++) {
            if (!isWhitespace(line[i])) {
                endOfVar = line + i;
                break;
            }
        }

        // error if no assignment
        if (endOfVar[0] != '=') {
            char* errorStr = (char*)malloc(20 * sizeof(char));
            sprintf(errorStr, "Expected assignment");
            ErrorData error = {errorStr, lineCount, (endOfVar - line), nameLength, handle};
            appendList(errorList, &error, sizeof(ErrorData));
            lineCount++;
            free(name);
            continue;
        }

        // add assignment to the evaluation
        appendList(toEvaluate, &name, sizeof(char*));
        List* dependencies = getVars(endOfVar, 256 - (endOfVar - line));
        List* defs = newList();
        for (Node* node = dependencies->head; node != NULL; node = node->next) {
            char* name = *(char**)(node->dataptr);
            uint16_t* value = (uint16_t*)readStringTable(defines, name, strlen(name) + 1);
            if (value != NULL) {
                // append the value to the list
                DefData defData = {name, *value, 1, 1};
                appendList(defs, &defData, sizeof(DefData));

                // remove dependency
                if (node->prev) {node->prev->next = node->next;}
                else {dependencies->head = NULL;}
                if (node->next) {node->next->prev = node->prev;}
                else {dependencies->tail = NULL;}
                dependencies->size -= 1;
                Node* prevNode = node;
                node = node->next;
                free(prevNode->dataptr);
                free(prevNode);
                if (node == NULL) {break;}
            }
        }
        int i;
        for (i = 0; i < (256 - (endOfVar - line)); i++) {
            if (endOfVar[i] == '\n' || endOfVar[i] == '\0' || endOfVar[i] == ';') {
                break;
            }
        }
        char* expr = (char*)memcpy(malloc(strlen(endOfVar + 1) + 1), endOfVar + 1, strlen(endOfVar + 1) + 1);
        VarEvalData evalData = {dependencies, defs, expr, i - 1, lineCount, (endOfVar - line) + 1, handle};
        setStringTableValue(toEvaluateLut, name, nameLength + 1, &evalData, sizeof(VarEvalData));

        lineCount++;
    }

    // evaluate the vars 
    List* visited = newList();
    for (Node* nodei = toEvaluate->head; nodei != NULL; nodei = nodei->next) {
        char* name = *(char**)(nodei->dataptr);
        if (evaluateVar(errorList, name, toEvaluateLut, varDefs, defines, newList())) {
            // clear the table
            for (Node* nodej = toEvaluate->head; nodej != NULL; nodej = nodej->next) {
                char* nameToClear = *(char**)(nodei->dataptr);
                VarEvalData* lutValue = (VarEvalData*)readStringTable(toEvaluateLut ,nameToClear, strlen(nameToClear) + 1);
                if (lutValue != NULL) {
                    for (Node* nodek = lutValue->dependencies->head; nodek != NULL; nodek = nodek -> next) {
                        free(*(char**)(nodek->dataptr));
                    }
                    deleteList(lutValue->dependencies);
                    free(lutValue->expr);
                    removeStringTableValue(toEvaluateLut, nameToClear, strlen(nameToClear) + 1);
                }
            }
            break;
        }
    }

    // clear the evaluation list
    for (Node* node = toEvaluate->head; node != NULL; node = node->next) {
        free(*(char**)(node->dataptr));
    }

    // cleanup
    deleteList(toEvaluate);
    deleteList(visited);
    deleteStack(ifStack);
    deleteStack(segStack);
    deleteStack(includeStack);
    deleteStack(macroStack);
    deleteStringTable(defines);
    deleteStringTable(toEvaluateLut);// known to be empty
    return varDefs;
}

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
void readLocalVars(FileHandle* handle, List* errorList, List* handleList, List* segments, StringTable macroDefs, StringTable varDefs, StringTable defines, int instructionSize, List* localVars, SegmentDef* activeSeg, unsigned int lineCount, Stack* includeStack_, Stack* ifStack_, Stack* segStack_, Stack* macroStack_) {
    // setup
    char line[256];
    StringTable toEvaluateLut = newStringTable();
    List* toEvaluate = newList();
    List* defUpdates = newList();
    SegmentDef* activeSegment = activeSeg;
    fpos_t filePos;
    Stack* includeStack = newStack();
    Stack* ifStack = newStack();
    Stack* segStack = newStack();
    Stack* macroStack = newStack();
    FileHandle* retHandle = handle;
    fpos_t retPos;
    fgetpos(handle->fptr, &retPos);

    // copy the stacks
    for (Node* node = includeStack_->head; node != NULL; node = node->next) {
        appendList(includeStack, node->dataptr, sizeof(IncludeReturnData));
    }
    for (Node* node = ifStack_->head; node != NULL; node = node->next) {
        appendList(ifStack, node->dataptr, sizeof(PosData));
    }
    for (Node* node = segStack_->head; node != NULL; node = node->next) {
        appendList(segStack, node->dataptr, sizeof(SegmentDef));
    }
    for (Node* node = macroStack_->head; node != NULL; node = node->next) {
        appendList(macroStack, node->dataptr, sizeof(IncludeReturnData));
    }

    // clear local vars
    for (Node* node = localVars->head; node != NULL; node = node->next) {
        char* name = *(char**)(node->dataptr);
        removeStringTableValue(varDefs, name, strlen(name) + 1);
        free(name);
    }
    while(localVars->size > 0) {
        removeListElement(localVars, 0);
    }

    // get the starting macro stack size
    int startStackSize = macroStack->size;

    // read the current block
    while (macroStack->size >= startStackSize) {
        // handle new line eof
        fgetpos(handle->fptr, &filePos);
        if (filePos == handle->length) {fgets(line, 256, handle->fptr);}

        // handle end of return
        if (feof(handle->fptr) && includeStack->size > 0) {
            line[0] = '\0';
            handle = includeReturn(includeStack, &lineCount);
            lineCount++;
        }

        // kill on eof
        if (feof(handle->fptr)) {break;}

        fgets(line, 256, handle->fptr);

        // handle non-var lines
        if (line[0] != '@') {
            // error on the start of the line
            if (line[0] == '.') {
                char* afterName;
                char* macroName = extractMacro(line, strlen(line), &afterName);
                if (!strcmp(macroName, ".define") || !strcmp(macroName, ".redef")) {
                    char* afterVar;
                    unsigned int j = countWhitespaceChars(afterName, 249);
                    char* varName = getVarName(afterName + j, 249 - j, &afterVar);
                    uint16_t assignValue = 0;

                    // log a change
                    char hasMatch = 0;
                    for (Node* node = defUpdates->head; node != NULL; node = node->next)
                    {
                        DefData* oldData = (DefData*)(node->dataptr);
                        if (!strcmp(oldData->name, varName)) {
                            hasMatch = 1;
                            break;
                        }
                    }
                    if (!hasMatch) {
                        uint16_t* val = (uint16_t*)readStringTable(defines, varName, strlen(varName) + 1);
                        DefData defData = {varName, 0, 1, 1};
                        if (val != NULL) {defData.val = *val;}
                        else {defData.hasValue = 0;}
                            appendList(defUpdates, &defData, sizeof(DefData));
                    }

                    // parse expression
                    if (!isValidLineEnding(afterVar, 256 - (afterVar - line))) {
                        ExprErrorShort exprOut = evalShortExpr(afterVar, strlen(afterVar), defines, defines);
                        assignValue = exprOut.val;
                    }
                    setStringTableValue(defines, varName, strlen(varName) + 1, &assignValue, 2);
                } else if (!strcmp(macroName, ".undef")) {
                    char* afterVar;
                    unsigned int j = countWhitespaceChars(afterName, 249);
                    char* varName = getVarName(afterName + j, 249 - j, &afterVar);

                    // log a change
                    char hasMatch = 0;
                    for (Node* node = defUpdates->head; node != NULL; node = node->next) {
                        DefData* oldData = (DefData*)(node->dataptr);
                        if (!strcmp(oldData->name, varName)) {
                            hasMatch = 1;
                            break;
                        }
                    }
                    if (!hasMatch) {
                        uint16_t* val = (uint16_t*)readStringTable(defines, varName, strlen(varName) + 1);
                        DefData defData = {varName, 0, 1, 1};
                        if (val != NULL) {defData.val = *val;}
                        else {defData.hasValue = 0;}
                        appendList(defUpdates, &defData, sizeof(DefData));
                    }

                    removeStringTableValue(defines, varName, strlen(varName) + 1);
                } else if (macroStack->size > 0 || strcmp(macroName, ".endmacro")) {
                    handle = executeType2Macro(handle, errorList, handleList, line, strlen(line), &lineCount, 0, includeStack, ifStack, segStack, macroStack, defines, &activeSegment, segments, macroDefs, instructionSize);
                    if (handle == NULL) {free(macroName); break;}
                } else {
                    free(macroName);
                    break;
                }
                free(macroName);
            } else if (!isWhitespace(line[0]) && line[0] != '\0' && line[0] != ';') {
                // break on global var
                break;
            } else if (isWhitespace(line[0])) {
                unsigned int i = countWhitespaceChars(line, strlen(line));
                if (line[i] == '.') {
                    char* afterName;
                    char* macroName = extractMacro(line + i, strlen(line + i), &afterName);
                    if (!strcmp(macroName, ".define") || !strcmp(macroName, ".redef")) {
                        char* afterVar;
                        unsigned int j = countWhitespaceChars(afterName, 249 - i);
                        char* varName = getVarName(afterName + j, 249 - i - j, &afterVar);
                        uint16_t assignValue = 0;

                        // log a change
                        char hasMatch = 0;
                        for (Node* node = defUpdates->head; node != NULL; node = node->next) {
                            DefData* oldData = (DefData*)(node->dataptr);
                            if (!strcmp(oldData->name, varName)) {
                                hasMatch = 1;
                                break;
                            }
                        }
                        if (!hasMatch) {
                            uint16_t* val = (uint16_t*)readStringTable(defines, varName, strlen(varName) + 1);
                            DefData defData = {varName, 0, 1, 1};
                            if (val != NULL) {defData.val = *val;}
                            else {defData.hasValue = 0;}
                            appendList(defUpdates, &defData, sizeof(DefData));
                        }

                        // parse expression
                        if (!isValidLineEnding(afterVar, 256 - (afterVar - line))) {
                            ExprErrorShort exprOut = evalShortExpr(afterVar, strlen(afterVar), defines, defines);
                            assignValue = exprOut.val;
                        }
                        setStringTableValue(defines, varName, strlen(varName) + 1, &assignValue, 2);
                    } else if (!strcmp(macroName, ".undef")) {
                        char* afterVar;
                        unsigned int j = countWhitespaceChars(afterName, 249 - i);
                        char* varName = getVarName(afterName + j, 249 - i - j, &afterVar);

                        // log a change
                        char hasMatch = 0;
                        for (Node* node = defUpdates->head; node != NULL; node = node->next) {
                            DefData* oldData = (DefData*)(node->dataptr);
                            if (!strcmp(oldData->name, varName)) {
                                hasMatch = 1;
                                break;
                            }
                        }
                        if (!hasMatch) {
                            uint16_t* val = (uint16_t*)readStringTable(defines, varName, strlen(varName) + 1);
                            DefData defData = {varName, 0, 1, 1};
                            if (val != NULL) {defData.val = *val;}
                            else {defData.hasValue = 0;}
                            appendList(defUpdates, &defData, sizeof(DefData));
                        }

                        removeStringTableValue(defines, varName, strlen(varName) + 1);
                    } else if (macroStack->size > 0 || strcmp(macroName, ".endmacro")) {
                        handle = executeType2Macro(handle, errorList, handleList, line + i, strlen(line + i), &lineCount, i, includeStack, ifStack, segStack, macroStack, defines, &activeSegment, segments, macroDefs, instructionSize);
                        if (handle == NULL) {free(macroName); break;}
                    } else {
                        free(macroName);
                        break;
                    }
                    free(macroName);
                } else if (!isValidLineEnding(line, strlen(line))) {
                    // check for macro
                    char* afterMacro = line + i;
                    char* macroName = extractVar(&afterMacro, strlen(line + i));
                    if (readStringTable(macroDefs, macroName, strlen(macroName) + 1) != NULL) {
                        // transfer position to the macro
                        IncludeReturnData retData = {handle, 0, lineCount, errorList->size};
                        fgetpos(handle->fptr, &(retData.filePosition));
                        pushStack(macroStack, &retData, sizeof(IncludeReturnData));
                        MacroDefData macroData = *(MacroDefData*)readStringTable(macroDefs, macroName, strlen(macroName) + 1);
                        handle = macroData.handle;
                        lineCount = macroData.line + 1;
                        fsetpos(handle->fptr, &(macroData.start));
                        continue;
                    }
                    
                    // handle as instruction
                    activeSegment->writeAddr += instructionSize;
                    if (activeSegment->writeAddr > activeSegment->size) {
                        char* errorStr = (char*)malloc((25 + strlen(activeSegment->name)) * sizeof(char));
                        sprintf(errorStr, "Segment %s size exceeded", activeSegment->name);
                        ErrorData errorData = {errorStr, lineCount, 0, 1, handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                        break;
                    }
                }
            }

            lineCount++;
            continue;
        }

        // don't bother with definitions inside a macro
        if (macroStack->size > startStackSize) {
            lineCount++;
            continue;
        }

        // read the var name
        char* endOfVar;
        char* name = getVarName(line, 256, &endOfVar);
        int nameLength = (endOfVar - line);

        // append the new local var
        char* namecpy = malloc((nameLength + 1) * sizeof(char));
        memcpy(namecpy, name, (nameLength + 1) * sizeof(char));
        appendList(localVars, &namecpy, sizeof(char*));

        // prevent repeat definitions
        if (readStringTable(varDefs, name, nameLength + 1) != NULL || readStringTable(toEvaluateLut, name, nameLength + 1) != NULL) {
            // append repeat error
            line[nameLength] = '\0';
            char* errorStr = (char*)malloc((45 + nameLength) * sizeof(char));
            sprintf(errorStr, "Repeat definition for constant or lable \"%s\"", line);
            ErrorData error = {errorStr, lineCount, 0, nameLength, handle};
            appendList(errorList, &error, sizeof(ErrorData));
            lineCount++;
            free(name);
            continue;
        }

        // handle label
        if (endOfVar[0] == ':') {
            if (activeSegment == NULL) {
                char* errorStr = (char*)malloc(18 * sizeof(char));
                sprintf(errorStr, "No active segment");
                ErrorData errorData = {errorStr, lineCount, (endOfVar - line), 1, handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                lineCount++;
                free(name);
                continue;
            }
            uint16_t writeVal = activeSegment->writeAddr + activeSegment->startAddr;
            setStringTableValue(varDefs, name, nameLength + 1, &writeVal, 2);
            lineCount++;
            free(name);
            continue;
        }

        // drop space
        for (int i = nameLength; i < 256; i++) {
            if (!isWhitespace(line[i])) {
                endOfVar = line + i;
                break;
            }
        }

        // error if no assignment
        if (endOfVar[0] != '=') {
            char* errorStr = (char*)malloc(20 * sizeof(char));
            sprintf(errorStr, "Expected assignment");
            ErrorData error = {errorStr, lineCount, (endOfVar - line), nameLength, handle};
            appendList(errorList, &error, sizeof(ErrorData));
            lineCount++;
            free(name);
            continue;
        }

        // add assignment to the evaluation
        appendList(toEvaluate, &name, sizeof(char*));
        List* dependencies = getVars(endOfVar, 256 - ((endOfVar - line)));
        List* defs = newList();
        for (Node* node = dependencies->head; node != NULL; node = node->next) {
            char* name = *(char**)(node->dataptr);
            uint16_t* value = (uint16_t*)readStringTable(defines, name, strlen(name) + 1);
            if (value != NULL) {
                // append the value to the list
                DefData defData = {name, *value, 1, 1};
                appendList(defs, &defData, sizeof(DefData));

                // remove dependency
                if (node->prev) {node->prev->next = node->next;}
                else {dependencies->head = NULL;}
                if (node->next) {node->next->prev = node->prev;}
                else {dependencies->tail = NULL;}
                dependencies->size -= 1;
                Node* prevNode = node;
                node = node->next;
                free(prevNode->dataptr);
                free(prevNode);
                if (node == NULL) {break;}
            }
        }
        int i;
        for (i = 0; i < (256 - ((endOfVar - line))); i++) {
            if (endOfVar[i] == '\n' || endOfVar[i] == '\0' || endOfVar[i] == ';') {
                break;
            }
        }
        char* expr = (char*)memcpy(malloc(strlen(endOfVar + 1) + 1), endOfVar + 1, strlen(endOfVar + 1) + 1);
        VarEvalData evalData = {dependencies, defs, expr, i - 1, lineCount, (endOfVar - line) + 1, handle};
        setStringTableValue(toEvaluateLut, name, nameLength + 1, &evalData, sizeof(VarEvalData));

        lineCount++;
    }

    // evaluate the vars 
    List* visited = newList();
    for (Node* nodei = toEvaluate->head; nodei != NULL; nodei = nodei->next) {
        char* name = *(char**)(nodei->dataptr);
        if (evaluateVar(errorList, name, toEvaluateLut, varDefs, defines, newList())) {
            // clear the table
            for (Node* nodej = toEvaluate->head; nodej != NULL; nodej = nodej->next) {
                char* nameToClear = *(char**)(nodei->dataptr);
                VarEvalData* lutValue = (VarEvalData*)readStringTable(toEvaluateLut ,nameToClear, strlen(nameToClear) + 1);
                if (lutValue != NULL) {
                    for (Node* nodek = lutValue->dependencies->head; nodek != NULL; nodek = nodek -> next) {
                        free(*(char**)(nodek->dataptr));
                    }
                    deleteList(lutValue->dependencies);
                    free(lutValue->expr);
                    removeStringTableValue(toEvaluateLut, nameToClear, strlen(nameToClear) + 1);
                }
            }
            break;
        }
    }

    // clear the evaluation list
    for (Node* node = toEvaluate->head; node != NULL; node = node->next) {
        free(*(char**)(node->dataptr));
    }

    // reset reading
    fsetpos(retHandle->fptr, &retPos);

    // undo defines
    for (Node* node = defUpdates->head; node != NULL; node = node->next) {
        DefData* defData = (DefData*)(node->dataptr);
        if (defData->hasValue) {
            setStringTableValue(defines, defData->name, strlen(defData->name) + 1, &(defData->val), 2);
        } else {
            removeStringTableValue(defines, defData->name, strlen(defData->name) + 1);
        }
        free(defData->name);
    }

    // cleanup
    deleteList(defUpdates);
    deleteList(toEvaluate);
    deleteList(visited);
    deleteStack(ifStack);
    deleteStack(segStack);
    deleteStack(includeStack);
    deleteStack(macroStack);
    deleteStringTable(toEvaluateLut);// known to be empty
}

#endif