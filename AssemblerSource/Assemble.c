/*
code to assemble

Written by Adam Billings
*/

#include "GeneralMacros.h"
#include "VarEvaluation.h"
#include "ExpressionEvaluation.h"
#include "CodeGeneration.h"
#include "Assemble.h"

/*
assembles a file into the output segment

handle: file handle
errorList: list of errors
handleList: list of handles
segments: list of segments
macroDefs: defined macros
varDefs: defined vars
wordSize: addresses occupied by a 16-bit word
isLittleEndian: if the code is little endian
*/
char assemble(FileHandle* handle, List* errorList, List* handleList, List* segments, StringTable macroDefs, StringTable varDefs, int wordSize, char isLittleEndian) {
    // setup
    List* localVars = newList();
    List* macroVars = newList();
    char line[256];
    unsigned int lineCount = 0;
    long filePos;
    SegmentDef* activeSeg = NULL;
    Stack* includeStack = newStack();
    Stack* ifStack = newStack();
    Stack* segStack = newStack();
    Stack* macroStack = newStack();
    StringTable defines = newStringTable();
    StringTable instTable = newInstructionTable();

    // reset the segment counters and allocate the outputs
    for (Node* node = segments->head; node != NULL; node = node->next) {
        SegmentDef* curSeg = (SegmentDef*)(node->dataptr);
        curSeg->writeAddr = 0;
        if (curSeg->accessType != bss) {
            curSeg->outputArr = (uint8_t*)malloc(curSeg->size * (wordSize == 1 ? 2 : 1));
            for (int i = 0; i < (curSeg->size * (wordSize == 1 ? 2 : 1)); i++) {
                curSeg->outputArr[i] = 0;
            }
        }
    }
    // get first set of local vars
    readLocalVars(handle, errorList, handleList, segments, macroDefs, varDefs, defines, wordSize, localVars, activeSeg, lineCount, includeStack, ifStack, segStack, macroStack);

    // reset the segment counters
    for (Node* node = segments->head; node != NULL; node = node->next) {
        SegmentDef* curSeg = (SegmentDef*)(node->dataptr);
        curSeg->writeAddr = 0;
    }

    // no errors
    if (errorList->size > 0) {
        // cleanup
        for (Node* node = localVars->head; node != NULL; node = node->next) {
            free(*(char**)(node->dataptr));
        }
        deleteList(localVars);
        deleteStack(includeStack);
        deleteStack(ifStack);
        deleteStack(macroStack);
        deleteStack(segStack);
        deleteStringTable(defines);
        return 0;
    }

    while (1) {
        // handle new line eof
        filePos = ftell(handle->fptr);
        if (filePos == handle->length) {
            if (fgets(line, 256, handle->fptr) == NULL && !feof(handle->fptr)) {return 1;}
        }

        // handle end of return
        if (feof(handle->fptr) && includeStack->size > 0) {
            line[0] = '\0';
            handle = includeReturn(includeStack, &lineCount);
            lineCount++;
        }

        // kill on eof
        if (feof(handle->fptr)) {break;}

        if (fgets(line, 256, handle->fptr) == NULL && !feof(handle->fptr)) {return 1;}

        // empty line

        // reset local vars
        if (!isspace(line[0]) && line[0] != '.' && line[0] != '@' && line[0] != ';') {
            unsigned int errorCount = errorList->size;
            List* segWriteRes = newList();
            for (Node* node = segments->head; node != NULL; node = node->next) {
                uint16_t writeAddr = ((SegmentDef*)(node->dataptr))->writeAddr;
                appendList(segWriteRes, &writeAddr, 2);
                if (wordSize == 1) {((SegmentDef*)(node->dataptr))->writeAddr /= 2;}
            }
            readLocalVars(handle, errorList, handleList, segments, macroDefs, varDefs, defines, wordSize, localVars, activeSeg, lineCount + 1, includeStack, ifStack, segStack, macroStack);
            Node* nodei = segWriteRes->head;
            for (Node* nodej = segments->head; nodej != NULL; nodej = nodej->next) {
                uint16_t writeAddr = *(uint16_t*)(nodei->dataptr);
                ((SegmentDef*)(nodej->dataptr))->writeAddr = writeAddr;
                nodei = nodei->next;
            }
            deleteList(segWriteRes);
            if (errorList->size > errorCount) {break;}
        }

        // handle the case of label: code
        int i = 0;
        if (!isspace(line[0]) && line[0] != '.' && line[0] != ';') {
            char* afterLabel = line;
            char* label = extractVar(&afterLabel, 256);
            free(label);
            if (afterLabel[0] == ':') {i = (afterLabel + 1 - line);}
            if (isValidLineEnding(afterLabel + 1, strlen(afterLabel + 1))) {lineCount++; continue;}
        }

        if (line[i] == '.') {
            handle = executeType3Macro(handle, errorList, handleList, line, strlen(line), &lineCount, i, includeStack, ifStack, segStack, macroStack, defines, &activeSeg, segments, macroDefs, wordSize, isLittleEndian, macroVars, varDefs);

        } else if (isspace(line[i])) {
            i += countWhitespaceChars(line + i, strlen(line + i));
            if (line[i] == '.') {
                handle = executeType3Macro(handle, errorList, handleList, line + i, strlen(line + i), &lineCount, i, includeStack, ifStack, segStack, macroStack, defines, &activeSeg, segments, macroDefs, wordSize, isLittleEndian, macroVars, varDefs);
            } else if (!isValidLineEnding(line, strlen(line))) {
                char* afterName = line + i;
                char* name = extractVar(&afterName, strlen(afterName));
                MacroDefData* macroData = (MacroDefData*)(readStringTable(macroDefs, name, strlen(name) + 1));
                if (macroData != NULL) {
                    // validate macro args
                    int argCount = countArgs(afterName, strlen(afterName));
                    if (isValidLineEnding(afterName, strlen(afterName))) {argCount = 0;}
                    if (argCount != macroData->vars->size) {
                        char* errorStr = (char*)malloc(25 * sizeof(char));
                        sprintf(errorStr, "Incorrect argument count");
                        ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName) + 1, handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                        free(name);
                        lineCount++;
                        continue;
                    }

                    // define the vars
                    char hasError = 0;
                    char* errorMessage1;
                    List* args = extractArgs(afterName, strlen(afterName));
                    Node* argNode = args->head;
                    for (Node* node = macroData->vars->head; node != NULL; node = node->next) {
                        char* expr = *(char**)(argNode->dataptr);
                        char* name = *(char**)(node->dataptr);
                        ExprErrorShort exprOut = evalShortExpr(expr, strlen(expr), varDefs, defines);
                        if (exprOut.errorMessage == NULL) {
                            uint16_t val = exprOut.val;
                            appendList(macroVars, &name, sizeof(char*));
                            setStringTableValue(varDefs, name, strlen(name) + 1, &val, 2);
                        } else {
                            if (!hasError) {
                                errorMessage1 = (char*)malloc((strlen(exprOut.errorMessage) + 1) * sizeof(char));
                                strcpy(errorMessage1, exprOut.errorMessage);
                            }
                            free(exprOut.errorMessage);
                            hasError = 1;
                        }

                        argNode = argNode->next;
                    }
                    deleteList(args);
                    if (hasError) {
                        char* errorStr = (char*)malloc(30 * sizeof(char) + strlen(errorMessage1) * sizeof(char));
                        sprintf(errorStr, "Could not parse arguments: %s", errorMessage1);
                        ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                        free(name);
                        free(errorMessage1);
                        lineCount++;
                        continue;
                    }

                    // push return data
                    IncludeReturnData retData = {handle, 0, lineCount, errorList->size};
                    retData.filePosition = ftell(handle->fptr);
                    pushStack(macroStack, &retData, sizeof(IncludeReturnData));

                    // go to the macro
                    handle = macroData->handle;
                    lineCount = macroData->line;
                    fseek(handle->fptr, macroData->start, SEEK_SET);

                    // read in the macroVars
                    List* tempMacroVars = newList();
                    unsigned int errorCount = errorList->size;
                    List* segWriteRes = newList();
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        uint16_t writeAddr = ((SegmentDef*)(node->dataptr))->writeAddr;
                        appendList(segWriteRes, &writeAddr, 2);
                        if (wordSize == 1) {((SegmentDef*)(node->dataptr))->writeAddr /= 2;}
                    }
                    readLocalVars(handle, errorList, handleList, segments, macroDefs, varDefs, defines, wordSize, tempMacroVars, activeSeg, lineCount + 1, includeStack, ifStack, segStack, macroStack);
                    Node* nodei = segWriteRes->head;
                    for (Node* nodej = segments->head; nodej != NULL; nodej = nodej->next) {
                        uint16_t writeAddr = *(uint16_t*)(nodei->dataptr);
                        ((SegmentDef*)(nodej->dataptr))->writeAddr = writeAddr;
                        nodei = nodei->next;
                    }
                    deleteList(segWriteRes);
                    
                    // stitch to macroVars
                    if (macroVars->tail) {macroVars->tail->next = tempMacroVars->head;}
                    if (tempMacroVars->head) {tempMacroVars->head->prev = macroVars->tail;}
                    if (!(macroVars->head)) {macroVars->head = tempMacroVars->head;}
                    macroVars->tail = tempMacroVars->tail;
                    macroVars->size += tempMacroVars->size;
                    free(tempMacroVars);
                    if (errorList->size > errorCount) {break;}
                } else {
                    // get the instruction
                    InstData* instData = readStringTable(instTable, name, strlen(name) + 1);
                    if (instData == NULL) {
                        char* errorStr = (char*)malloc((23 + strlen(name)) * sizeof(char));
                        sprintf(errorStr, "Invalid instruction: %s", name);
                        ErrorData errorData = {errorStr, lineCount, i, strlen(name), handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                        free(name);
                        lineCount++;
                        continue;
                    }

                    // get the args
                    char hasError = 0;
                    char* errorMessage1;
                    List* args = extractArgs(afterName, strlen(afterName));
                    List* argEvals = newList();
                    for (Node* node = args->head; node != NULL; node = node->next) {
                        char* expr = *(char**)(node->dataptr);
                        ExprErrorShort exprOut = evalShortExpr(expr, strlen(expr), varDefs, defines);
                        if (exprOut.errorMessage == NULL) {
                            appendList(argEvals, &(exprOut.val), 2);
                        } else if (args->size >= 1 && !isValidLineEnding(expr, strlen(expr) + 1)) {
                            if (!hasError) {
                                errorMessage1 = (char*)malloc((strlen(exprOut.errorMessage) + 1) * sizeof(char));
                                strcpy(errorMessage1, exprOut.errorMessage);
                            }
                            hasError = 1;
                            free(exprOut.errorMessage);
                        }
                    }
                    deleteList(args);
                    if (hasError) {
                        char* errorStr = (char*)malloc(30 * sizeof(char) + strlen(errorMessage1) * sizeof(char));
                        sprintf(errorStr, "Could not parse arguments: %s", errorMessage1);
                        ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                        appendList(errorList, &errorData, sizeof(ErrorData));
                        free(name);
                        free(errorMessage1);
                        deleteList(argEvals);
                        lineCount++;
                        continue;
                    }

                    // assemble the instruction
                    uint16_t inst;
                    if (instData->type == reg || instData->type == imm || instData->type == sft) {
                        // make sure that the arg count is correct
                        if (argEvals->size != 2) {
                            char* errorStr = (char*)malloc(40 * sizeof(char));
                            sprintf(errorStr, "Invalid number of arguments: expected 2");
                            ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                            appendList(errorList, &errorData, sizeof(ErrorData));
                            lineCount++;
                            free(name);
                            deleteList(argEvals);
                            continue;
                        }

                        // get args
                        uint16_t arg1 = *(uint16_t*)(argEvals->head->dataptr);
                        uint16_t arg2 = *(uint16_t*)(argEvals->tail->dataptr);

                        // error on size
                        hasError = 0;
                        if (instData->type == reg && arg1 > 15) {
                            hasError = 1;
                            char* errorStr = (char*)malloc(33 * sizeof(char));
                            sprintf(errorStr, "Argument 1 exceeds range 0 to 15");
                            ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                            appendList(errorList, &errorData, sizeof(ErrorData));
                        } else if (instData->type == imm && ((arg1 & 0xff00) != 0x0000 && (arg1 & 0xff00) != 0xff00)) {
                            hasError = 1;
                            char* errorStr = (char*)malloc(49 * sizeof(char));
                            sprintf(errorStr, "Argument 1 exceeds range -128 to 127 or 0 to 255");
                            ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                            appendList(errorList, &errorData, sizeof(ErrorData));
                        } else if (instData->type == sft && ((arg1 & 0xffe0) != 0x0000 && (arg1 & 0xffe0) != 0xffe0)) {
                            hasError = 1;
                            char* errorStr = (char*)malloc(35 * sizeof(char));
                            sprintf(errorStr, "Argument 1 exceeds range -16 to 15");
                            ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                            appendList(errorList, &errorData, sizeof(ErrorData));
                        }
                        if (instData->type == imm && arg2 > 255) {
                            hasError = 1;
                            char* errorStr = (char*)malloc(49 * sizeof(char));
                            sprintf(errorStr, "Argument 2 exceeds range -128 to 127 or 0 to 255");
                            ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                            appendList(errorList, &errorData, sizeof(ErrorData));
                        } else if (instData->type != imm && arg2 > 15) {
                            hasError = 1;
                            char* errorStr = (char*)malloc(33 * sizeof(char));
                            sprintf(errorStr, "Argument 2 exceeds range 0 to 15");
                            ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                            appendList(errorList, &errorData, sizeof(ErrorData));
                        }
                        if (hasError) {
                            deleteList(argEvals);
                            free(name);
                            lineCount++;
                            continue;
                        }

                        // get the instruction
                        if (instData->type == reg) {arg1 &= 0x000f;}
                        else if (instData->type == imm) {arg1 &= 0x00ff;}
                        else {arg1 &= 0x001f;}
                        arg2 &= 0x000f;
                        inst = instData->param2(arg1, arg2);
                    } else if (instData->type == brc || instData->type == jrg) {
                        // make sure that the arg count is correct
                        if (argEvals->size != 1) {
                            char* errorStr = (char*)malloc(40 * sizeof(char));
                            sprintf(errorStr, "Invalid number of arguments: expected 1");
                            ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                            appendList(errorList, &errorData, sizeof(ErrorData));
                            lineCount++;
                            free(name);
                            deleteList(argEvals);
                            continue;
                        }

                        // get the argument
                        uint16_t arg = *(uint16_t*)(argEvals->head->dataptr);
                        hasError = 0;
                        if (instData->type == brc) {
                            uint16_t curAddr = (activeSeg->writeAddr / (wordSize == 1 ? 2 : 1)) + activeSeg->startAddr;
                            arg = arg - curAddr - wordSize;
                            if ((arg & 0xff80) != 0x0000 && (arg & 0xff80) != 0xff80) {
                                hasError = 1;
                                char* errorStr = (char*)malloc(46 * sizeof(char));
                                sprintf(errorStr, "Branch displacement exceeds range -128 to 127");
                                ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                                appendList(errorList, &errorData, sizeof(ErrorData));
                            }
                            arg &= 0x00ff;
                        } else {
                            if (arg > 15) {
                                hasError = 1;
                                char* errorStr = (char*)malloc(31 * sizeof(char));
                                sprintf(errorStr, "Argument exceeds range 0 to 15");
                                ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                                appendList(errorList, &errorData, sizeof(ErrorData));
                            }
                            arg &= 0x000f;
                        }
                        if (hasError) {
                            lineCount++;
                            deleteList(argEvals);
                            free(name);
                            continue;
                        }

                        // generate the instruction
                        inst = instData->param1(arg);
                    } else {
                        // make sure that the arg count is correct
                        if (argEvals->size != 0) {
                            char* errorStr = (char*)malloc(40 * sizeof(char));
                            sprintf(errorStr, "Invalid number of arguments: expected 0");
                            ErrorData errorData = {errorStr, lineCount, (afterName - line), strlen(afterName), handle};
                            appendList(errorList, &errorData, sizeof(ErrorData));
                            lineCount++;
                            free(name);
                            deleteList(argEvals);
                            continue;
                        }

                        // generate instruction
                        inst = instData->param0();
                    }

                    // write the instruction
                    uint8_t upperByte = (uint8_t)(inst >> 8);
                    uint8_t lowerByte = (uint8_t)(inst & 0x00ff);
                    if (isLittleEndian) {
                        activeSeg->outputArr[activeSeg->writeAddr] = lowerByte;
                        activeSeg->outputArr[activeSeg->writeAddr + 1] = upperByte;
                    } else {
                        activeSeg->outputArr[activeSeg->writeAddr] = upperByte;
                        activeSeg->outputArr[activeSeg->writeAddr + 1] = lowerByte;
                    }
                    deleteList(argEvals);
                    activeSeg->writeAddr += 2;
                }

                free(name);
            }
        }

        lineCount++;
    }

    // cleanup
    for (Node* node = localVars->head; node != NULL; node = node->next) {
        free(*(char**)(node->dataptr));
    }
    deleteList(macroVars);
    deleteList(localVars);
    deleteStack(includeStack);
    deleteStack(ifStack);
    deleteStack(macroStack);
    deleteStack(segStack);
    deleteStringTable(defines);
    deleteStringTable(instTable);
    return 0;
}