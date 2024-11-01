/*
main help infromation for the assembler

Written by Adam Billings
*/

#ifndef MainHelp_h
#define MainHelp_h

#include <stdio.h>

// include all other help files
#include "ExprHelp.h"
#include "VarHelp.h"
#include "ConfigHelp.h"
#include "DefaultConfigHelp.h"
#include "MacroHelp.h"
#include "InstructionHelp.h"
#include "SampleHelp.h"

#define HELP_STRING "\
\n\
  -- ACE3710 Help --\n\
\n\
    - Options -\n\
      -h <option>, --help <option> : open help page\n\
      -v, --version                : get version information\n\
      -c <file>, --config <file>   : set configuration file\n\
      -d, --default-config         : set default configuration\n\
      -b, --byte                   : addresses store bytes\n\
      -w, --word                   : addresses store words\n\
      -L, --little-endian          : write output file in little-endian format\n\
      -B, --big-endian             : write output file in big-endian format\n\
      -r, --raw                    : output raw binary\n\
      -t, --text-byte              : output hex as text bytes\n\
      -T, --text-word              : output hex as words\n\
      -o <file>, --output <file>   : set output file name\n\
\n\
    - Help Pages -\n\
      expressions\n\
      macros\n\
      instructions\n\
      variables\n\
      config\n\
      default-config\n\
      sample-assembly\n\
\n\
  For more information, use \"--help <help_page>\".\n\
\n\
"

/*
print the help infromation
*/
void printHelp() {
    printf(HELP_STRING);
}

#undef HELP_STRING

#endif