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
FileHandle* executeType1Macro(FileHandle* handle, List* errorList, List* handleList, char* line, int lineLength, unsigned int* lineCount, unsigned int curCol, Stack* includeStack, Stack* ifStack, StringTable defines, StringTable macroDefs, char* isInMacro, PosData* macroData, List* macroDeleteTracker);

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
FileHandle* executeType2Macro(FileHandle* handle, List* errorList, List* handleList, char* line, int lineLength, unsigned int* lineCount, unsigned int curCol, Stack* includeStack, Stack* ifStack, Stack* segStack, Stack* macroStack, StringTable defines, SegmentDef** activeSeg, List* segments, StringTable macroDefs, int wordSize);

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
FileHandle* executeType3Macro(FileHandle* handle, List* errorList, List* handleList, char* line, int lineLength, unsigned int* lineCount, unsigned int curCol, Stack* includeStack, Stack* ifStack, Stack* segStack, Stack* macroStack, StringTable defines, SegmentDef** activeSeg, List* segments, StringTable macroDefs, int wordSize, char isLittleEndian, List* macroVars, StringTable vars);

#endif