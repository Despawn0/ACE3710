/*
code to assemble

Written by Adam Billings
*/


#ifndef Assemble_h
#define Assemble_h

#include "GeneralMacros.h"
#include "VarEvaluation.h"
#include "ExpressionEvaluation.h"
#include "CodeGeneration.h"

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
char assemble(FileHandle* handle, List* errorList, List* handleList, List* segments, StringTable macroDefs, StringTable varDefs, int wordSize, char isLittleEndian);

#endif