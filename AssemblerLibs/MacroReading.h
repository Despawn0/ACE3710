/*
code to read in the macro definitions

Written by Adam Billings
*/

#ifndef MacroSweep_h
#define MacroSweep_h

#include "GeneralMacros.h"
#include "ExpressionEvaluation.h"
#include "ProcessMacros.h"

/*
reads macros defined in a file

handle: handle to the file
errorList: list of errors
handleList: list of open handles
macroDeleteTracker: lsit to track values to delete

returns: StringTable to lookup macro information
*/
StringTable readMacros(FileHandle* handle, List* errorList, List* handleList, List* macroDeleteTracker);

#endif