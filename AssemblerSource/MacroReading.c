/*
code to read in the macro definitions

Written by Adam Billings
*/

#include "GeneralMacros.h"
#include "ExpressionEvaluation.h"
#include "ProcessMacros.h"
#include "MacroReading.h"

/*
reads macros defined in a file

handle: handle to the file
errorList: list of errors
handleList: list of open handles
macroDeleteTracker: lsit to track values to delete

returns: StringTable to lookup macro information
*/
StringTable readMacros(FileHandle* handle, List* errorList, List* handleList, List* macroDeleteTracker) {
    // setup
    char line[256];
    char isInMacro = 0;
    unsigned int lineCount = 0;
    long curPos;
    PosData macroLocation;
    StringTable macroTable = newStringTable();
    StringTable defines = newStringTable();
    Stack* ifStack = newStack();
    Stack* incStack = newStack();

    // read file
    while (!feof(handle->fptr)) {
        // read a line
        if (fgets(line, 256, handle->fptr) < 0) {return NULL;}

        // ignore leading space
        int i = countWhitespaceChars(line, 256);

        // only concern is macors
        if (line[i] == '.') {
            // process macros
            handle = executeType1Macro(handle, errorList, handleList, line + i, 256 - i, &lineCount, i, incStack, ifStack, defines, macroTable, &isInMacro, &macroLocation, macroDeleteTracker);
        }

        // handle "troll" line
        curPos = ftell(handle->fptr);
        if (curPos == handle->length) {
            if (fgets(line, 256, handle->fptr) < 0) {return NULL;}
        }

        // handle unmatched if blocks
        if (feof(handle->fptr) && incStack->size == 0) {
            while (ifStack->size > 0) {
                PosData* ifData = popStack(ifStack);
                char* errorStr = (char*)malloc(16 * sizeof(char));
                sprintf(errorStr, "Expected .endif");
                ErrorData errorData = {errorStr, ifData->line, ifData->col, 1, handle};
                appendList(errorList, &errorData, sizeof(ErrorData));
                free(ifData);
            }
        }

        // handle end of return
        if (feof(handle->fptr) && incStack->size > 0) {
            line[0] = '\0';
            handle = includeReturn(incStack, &lineCount);
        }

        lineCount++;
    }

    deleteStringTable(defines);
    deleteStack(ifStack);
    deleteStack(incStack);
    return macroTable;
}