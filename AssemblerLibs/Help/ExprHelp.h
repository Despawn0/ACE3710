/*
expressing help infromation for the assembler

Written by Adam Billings
*/

#ifndef ExprHelp_h
#define ExprHelp_h

#include <stdio.h>

#define HELP_STRING "\
\n\
  -- ACE3710 Expression Help --\n\
\n\
    - Valid Constants -\n\
      <decimal_integer>\n\
      $<hexadecimal_integer>\n\
      %%<binary_integer>\n\
      '<character>'\n\
      <variable_name>\n\
\n\
    - Valid Expressions -\n\
      ( <expression> )                           : set precedence\n\
      + <expression>                             : expression\n\
      - <expression>                             : integer negate expression\n\
      ~ <expression>                             : bitwise negate expression\n\
      ! <expression>                             : boolean negate expression\n\
      < <expression>                             : lower byte of expression\n\
      > <expression>                             : upper byte of expression\n\
      <expression> + <expression>                : sum of expressions\n\
      <expression> - <expression>                : difference of expressions\n\
      <expression> * <expression>                : product of expressions\n\
      <expression> / <expression>                : integer quotient of expressions\n\
      <expression> // <expression>               : modulo division of expressions\n\
      <expression> & <expression>                : bitwise \"and\" of expressions\n\
      <expression> | <expression>                : bitwise \"or\" of expressions\n\
      <expression> ^ <expression>                : bitwise \"xor\" of expressions\n\
      <expression> && <expression>               : boolean \"and\" of expressions\n\
      <expression> || <expression>               : boolean \"or\" of expressions\n\
      <expression> ^^ <expression>               : boolean \"xor\" of expressions\n\
      <expression> == <expression>               : equality of expressions\n\
      <expression> != <expression>               : inequality of expressions\n\
      <expression> >= <expression>               : comparison of expressions\n\
      <expression> <= <expression>               : comparison of expressions\n\
      <expression> > <expression>                : strict comparison of expressions\n\
      <expression> < <expression>                : strict comparison of expressions\n\
      <expression> ? <expression> : <expression> : \"ternary operator\" with expressions\n\
\n\
  Note: All Expressions evaluate to 16-bit integers.\n\
  Note: Division by zero cannot be parsed.\n\
\n\
"

/*
print the help infromation
*/
void printExprHelp() {
    printf(HELP_STRING);
}

#undef HELP_STRING

#endif