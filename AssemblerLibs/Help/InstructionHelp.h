/*
help page for instructions

Written by Adam Billings
*/

#ifndef InstructionHelp_h
#define InstructionHelp_h

#define HELP_STRING "\
  -- ACE3710 Instruction Help --\n\
\n\
    - Instructions -\n\
      nop : no operation\n\
      add <Rsrc>, <Rdest>    : add <Rsrc> to <Rdest>\n\
      addi <imm>, <Rdest>    : add <imm> to <Rdest>\n\
      addu <Rsrc>, <Rdest>   : add <Rsrc> to <Rdest> unsigned\n\
      addui <imm>, <Rdest>   : add <imm> to <Rdest> unsigned\n\
      addc <Rsrc>, <Rdest>   : add <Rsrc> to <Rdest> with carry\n\
      addci <imm>, <Rdest>   : add <imm> to <Rdest> with carry\n\
      mul <Rsrc>, <Rdest>    : multiply <Rdest> by <Rsrc>\n\
      muli <imm>, <Rdest>    : multiply <Rdesr> by <imm>\n\
      sub <Rsrc>, <Rdest>    : subtract <Rsrc> from <Rdest>\n\
      subi <imm>, <Rdest>    : subtract <imm> from <Rdest>\n\
      subc <Rsrc>, <Rdest>   : subtract <Rsrc> from <Rdest> with carry\n\
      subci <imm>, <Rdest>   : subtract <imm> from <Rdest> with carry\n\
      cmp <Rsrc>, <Rdest>    : compare <Rdest> to <Rsrc>\n\
      cmpi <imm>, <Rdest>    : compare <Rdest> to <imm>\n\
      and <Rsrc>, <Rdest>    : bitwise and <Rdest> with <Rsrc>\n\
      andi <imm>, <Rdest>    : bitwise and <Rdest> with <imm>\n\
      or <Rsrc>, <Rdest>     : bitwise or <Rdest> with <Rsrc>\n\
      ori <imm>, <Rdest>     : bitwise or <Rdest> with <imm>\n\
      xor <Rsrc>, <Rdest>    : bitwise or <Rdest> with <Rsrc>\n\
      xori <imm>, <Rdest>    : bitwise or <Rdest> wirh <imm>\n\
      mov <Rsrc>, <Rdest>    : move <Rsrc> to <Rdest>\n\
      movi <imm>, <Rdest>    : move <imm> to <Rdest>\n\
      lsh <Rsrc>, <Rdest>    : logical bitshift <Rdest> left by <Rsrc>\n\
      lshi <imm>, <Rdest>    : logical bitshift <Rdest> left by <imm>\n\
      ashu <Rsrc>, <Rdest>   : arithmetic bitshift <Rdest> left by <Rsrc>\n\
      ashui <imm>, <Rdest>   : arithmetic bitshift <Rdest> left by <imm>\n\
      lui <imm>, <Rdest>     : load upper immediate <imm> into <Rdest>\n\
      load <Rdest>, <Raddr>  : load from address <Raddr> into <Rdest>\n\
      stor <Rsrc>, <Raddr>   : store <Rsrc> into address <Raddr>\n\
      snxb <Rsrc>, <Rdest>   : sign extend <Rsrc> into <Rdest>\n\
      zrxb <Rsrc>, <Rdest>   : zero extend <Rsrc> into <Rdest>\n\
      s<cond> <Rdest>        : set <Rdest> if <cond>\n\
      b<cond> <disp>         : branch <disp> addresses if <cond>\n\
      j<cond> <Rtarget>      : jump to address <Rtarget> if <cond>\n\
      jal <Rlink>, <Rtarget> : jump to address <Rtarget> and link current address in <Rlink>\n\
      tbit <Roffset>, <Rsrc> : test bit <Roffset> of <Rsrc>\n\
      tbiti <offset>, <Rsrc> : test bit <offset> if <Rsrc>\n\
      lpr <Rsrc>, <Rproc>    : load program status register <Rproc> from <Rsrc>\n\
      spr <Rproc>, <Rdest>   : store program status register <Rproc> to <Rdest>\n\
      di                     : disable interrupt\n\
      ei                     : enable interrupt\n\
      excp <vector>          : call exception <vector>\n\
      retx                   : return from exception\n\
      wait                   : wait for interrupt\n\
\n\
    - Conditions -\n\
      eq : equal\n\
      ne : not equal\n\
      cs : carry set\n\
      cc : carry clear\n\
      hi : higher than\n\
      ls : lower than or same as\n\
      gt : greater than\n\
      le : less than or equal to\n\
      fs : flag set\n\
      fc : flag clear\n\
      lo : lower than\n\
      hs : higher than or same as\n\
      lt : less than\n\
      ge : greater than or equal to\n\
      uc : unconditional\n\
\n\
    - Registers -\n\
      r0  : register 0\n\
      r1  : register 1\n\
      r2  : register 2\n\
      r3  : register 3\n\
      r4  : register 4\n\
      r5  : register 5\n\
      r6  : register 6\n\
      r7  : register 7\n\
      r8  : register 8\n\
      r9  : register 9\n\
      r10 : register 10\n\
      r11 : register 11\n\
      r12 : register 12\n\
      r13 : register 13\n\
      r14 : register 14 (return address)\n\
      r15 : register 15 (stack pointer)\n\
      ra  : return address (register 14)\n\
      sp  : stack pointer (register 15)\n\
\n\
"

/*
print instruction help
*/
void printInstructionHelp() {
    printf(HELP_STRING);
}

#undef HELP_STRING

#endif