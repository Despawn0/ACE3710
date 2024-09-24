/*
help page for variable help

Written by Adam Billings
*/

#ifndef VarHelp_h
#define VarHelp_h

#include <stdio.h>

#define HELP_STRING "\
\n\
  -- ACE3710 Variable Help --\n\
\n\
    - Global Variables -\n\
      By default, all variables are global.\n\
      Variables names are defined by a letter or underscore followed by zero or more letters, numbers, or underscores.\n\
      All variables are constant and must be assigned at declaration.\n\
      Variables may either be assigned by expression (\"variable = <expr>\") or as labels (\"label:\").\n\
      All variable and label declarations must be left-aligned in the file (no leading whitespace).\n\
\n\
"

/*
prints the variable help message
*/
void printVarHelp() {
    printf(HELP_STRING);
}

#undef HELP_STRING

#endif