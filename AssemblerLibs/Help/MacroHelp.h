/*
help page for macros

Written by Adam Billings
*/

#ifndef MacroHelp_h
#define MacroHelp_h

#include <stdio.h>

#define HELP_STRING "\
\n\
  -- ACE3710 Macro Help --\n\
\n\
    - Macros -\n\
      .if <expression>                      : include code if the expression is true\n\
      .ifdef <define>                       : include code if defined\n\
      .ifndef <define>                      : include code if not defined\n\
      .elseif <expression>                  : else include code if the expression is true\n\
      .elseifdef <define>                   : else include code if defined\n\
      .elseifndef <define>                  : else include code if not defined\n\
      .else                                 : else include code\n\
      .endif                                : end .if-.elseif-.else chain\n\
      .macro <param>, <param>, ...          : define a macro (pseudoinstruction)\n\
      .endmacro                             : end macro definition\n\
      .define <define> <expression>         : define a constant\n\
      .redef <define> <expression>          : redefine a constant\n\
      .undef <define>                       : undefine constant\n\
      .align <expression>                   : align to multiple of <expression>\n\
      .res <expression>                     : reserve <expression> addresses\n\
      .byte <expression>, <expression>, ... : write byte(s)\n\
      .word <expression>, <expression>, ... : write word(s)\n\
      .ascii \"<string>\"                     : write string\n\
      .asciiz \"<string>\"                    : write zero-terminated string\n\
      .segment \"<segment_name>\"             : change to <segment_name>\n\
      .pushseg                              : push segment\n\
      .popseg                               : pop segment\n\
      .include \"<file_name>\"                : include code from file\n\
      .incbin \"<file_name>\"                 : include raw binary from file\n\
      .warning \"<string>\"                   : assembler warning\n\
      .error \"<string>\"                     : assembler error\n\
\n\
  Note: expressions can only referenced macro-defined values.\n\
\n\
"

/*
print the help infromation
*/
void printMacroHelp() {
    printf(HELP_STRING);
}

#undef HELP_STRING

#endif