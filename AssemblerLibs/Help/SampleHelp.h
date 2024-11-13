/*
Sample assembly file help

Written by Adam Billings
*/

#ifndef SampleHelp_h
#define SampleHelp_h

#define HELP_STRING "\
  -- ACE3710 Sample Assembly File Help --\n\
\n\
    - About -\n\
      This is a sample file given to provide syntax and feature help.\n\
      This file is not useful code, but is used as a demonstration.\n\
\n\
    - Sample File -\n\
      \e[32m; include guard\n\
      \e[36m.ifndef \e[33msampleFile_s\n\
      \e[36m.define \e[33msampleFile_s\n\
      \n\
      \e[32m; set the active segment to write to\n\
      \e[36m.segment \e[35m\"CODE\"\n\
      \n\
      \e[32m; pushing and poping a segment is a good way to reserve space if the segment is unknown\n\
      \e[36m.pushseg\n\
      \e[32m; addresses can be reserved in an initialize or uninitialized segment\n\
      \e[36m.segment \e[35m\"BSS\"\n\
      \e[33moneWordValue: \e[36m.res \e[0m1 \e[32m; reserve one address\n\
      \e[33moneWordArr: \e[36m.res \e[0m10 \e[32m; reserve 10 addresses for the array\n\
      \e[36m.popseg \e[32m; make sure to remember to pop from the segment stack\n\
      \n\
      ; include an outside file and a binary file\n\
      \e[36m.include \e[35m\"outsideFile.s\"\n\
      \e[36m.incbin \e[35m\"binaryFile\"\n\
      \n\
      \e[32m; define a pseudoinstruction\n\
      \e[36m.macro \e[34mrshi\e[0m shAmt, rDst\n\
        \e[34mlsh\e[0m -shAmt, rDst\n\
      \e[36m.endmacro\n\
      \n\
      \e[32m; define a custom instruction as how the machine code would look\n\
      \e[36m.macro \e[34mldz\e[0m rDst \e[32m; load 0\n\
      \e[33m@opcode\e[0m = $d\n\
        \e[36m.word\e[0m (\e[33m@opcode\e[0m << 12) | (rDst << 8)\n\
      \e[36m.endmacro\n\
      \n\
      \e[32m; define, redefine, and undefine a value\n\
      \e[36m.define \e[33mval \e[0m13\n\
      \e[36m.redef \e[33mval \e[0m4\n\
      \e[36m.undef \e[33mval\n\
      \n\
      \e[32m; write some useless code\n\
      \e[33ma\e[0m = 13\n\
        \e[34mmovi\e[0m \e[33ma\e[0m, r4\n\
      \e[33m@tempLabel:\n\
        \e[34mrshi\e[0m 2, r4 \e[32m; macro calls are just like instructions\n\
        \e[34mbcc \e[33m@tempLabel\n\
      \n\
      \e[32m; put a string in the data segment\n\
      \e[36m.segment \e[35m\"DATA\"\n\
      \e[33mstring: \e[36m.asciiz \e[35m\"Hello, World!\"\n\
      \n\
      \e[32m; end include guard\n\
      \e[36m.endif\n\
\e[0m\n\
"

/*
print the help page
*/
void printSampleHelp() {
    printf(HELP_STRING);
}

#undef HELP_STRING

#endif