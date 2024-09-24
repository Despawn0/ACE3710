/*
help page for configuration help

Written by Adam Billings
*/

#ifndef ConfigHelp_h
#define ConfigHelp_h

#include <stdio.h>

#define HELP_STRING "\
\n\
  -- ACE3710 Configuration Help --\n\
\n\
    - Blocks -\n\
      Configuration files are split into two blocks: MEMORY and SEGMENTS.\n\
      The MEMORY block defines the layout of the address space used by the program.\n\
      The SEGMENTS block defines the segments used by the assembler to generat an output file.\n\
\n\
    - MEMORY Attributes -\n\
      start : start of the address range\n\
      size  : size of the address range\n\
      type  : access type of the address range\n\
\n\
    - SEGMENTS Attributes -\n\
      load  : address range from MEMORY to load to\n\
      align : (optional) alignment in the output file\n\
      fill  : (optional, default yes) if the segment should be filled entirely\n\
\n\
    - Access Types -\n\
      ro  : read only\n\
      rw  : read/write\n\
      bss : uninitialized\n\
\n\
  For a sample configuration file, see the help page for default-config.\n\
\n\
"

/*
prints the variable help message
*/
void printConfigHelp() {
    printf(HELP_STRING);
}

#undef HELP_STRING

#endif