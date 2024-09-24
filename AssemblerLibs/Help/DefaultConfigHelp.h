/*
help page for default configuration help

Written by Adam Billings
*/

#ifndef DefaultConfigHelp_h
#define DefaultConfigHelp_h

#include <stdio.h>

#define HELP_STRING "\
\n\
  -- ACE3710 Default Configuration Help --\n\
\n\
    - About -\n\
      This help page shows the configuration code for the default configutation.\n\
\n\
    - Configuration -\n\
      \e[32m# Define the memory mapping\n\
      \e[34mMEMORY\e[0m {\n\
        \e[32m# Assembler defined\n\
        \e[34mCODE:              start\e[0m = $0000, \e[34msize\e[0m = $8000, \e[34mtype\e[0m = ro;\n\
        \e[34mDATA:              start\e[0m = $8000, \e[34msize\e[0m = $1000, \e[34mtype\e[0m = rw;\n\
        \e[34mDISPATCH_TABLE:    start\e[0m = $9000, \e[34msize\e[0m = $1000, \e[34mtype\e[0m = rw;\n\
        \e[34mBSS:               start\e[0m = $a000, \e[34msize\e[0m = $4000, \e[34mtype\e[0m = bss;\n\
        \e[34mSTACK:             start\e[0m = $e000, \e[34msize\e[0m = $0800, \e[34mtype\e[0m = bss;\n\
        \e[34mINTERRUPT_STACK:   start\e[0m = $e800, \e[34msize\e[0m = $0800, \e[34mtype\e[0m = bss;\n\
\n\
        \e[32m# Hardware expectation\n\
        \e[34mINTERRUPT_CONTROL: start\e[0m = $fc00, \e[34msize\e[0m = $0400, \e[34mtype\e[0m = r0;\n\
      }\n\
\n\
      \e[32m# Define the assembler segments\n\
      \e[34mSEGMENTS\e[0m {\n\
        \e[32m# Output file\n\
        \e[34mCODE:            load\e[0m = CODE,           \e[34mfill\e[0m = yes;\n\
        \e[34mDATA:            load\e[0m = DATA,           \e[34mfill\e[0m = yes;\n\
        \e[34mDISPATCH_TABLE:  load\e[0m = DISPATCH_TABLE, \e[34mfill\e[0m = yes;\n\
\n\
        \e[32m# Allow RAM reservations\n\
        \e[34mBSS:             load\e[0m = BSS;\n\
        \e[34mSTACK:           load\e[0m = STACK;\n\
        \e[34mINTERRUPT_STACK: load\e[0m = INTERRUPT_STACK;\n\
      }\n\
\n\
"

/*
prints the variable help message
*/
void printDefaultConfigHelp() {
    printf(HELP_STRING);
}

#undef HELP_STRING

#endif