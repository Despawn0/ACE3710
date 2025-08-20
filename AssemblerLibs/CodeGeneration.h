/*
code to decode and generate instructions

Written by Adam Billings
*/

#ifndef CodeGenerator_h
#define CodeGenerator_h

#include <stdint.h>
#include "MiscAssembler.h"

typedef enum InstType {
    reg, imm, sft, brc, jrg, imp
} InstType;

// hold instruction information
typedef struct InstData {
    union {
        uint16_t (*param2)(uint16_t, uint16_t);
        uint16_t (*param1)(uint16_t);
        uint16_t (*param0)();
    };
    InstType type;
} InstData;

/*
all following functions are instructions taking in 2 args and returning a writable value
*/

uint16_t add(uint16_t rSrc, uint16_t rDst);

uint16_t addi(uint16_t imm, uint16_t rDst);

uint16_t addu(uint16_t rSrc, uint16_t rDst);

uint16_t addui(uint16_t imm, uint16_t rDst);

uint16_t addc(uint16_t rSrc, uint16_t rDst);

uint16_t addci(uint16_t imm, uint16_t rDst);

uint16_t mul(uint16_t rSrc, uint16_t rDst);

uint16_t muli(uint16_t imm, uint16_t rDst);

uint16_t sub(uint16_t rSrc, uint16_t rDst);

uint16_t subi(uint16_t imm, uint16_t rDst);

uint16_t subc(uint16_t rSrc, uint16_t rDst);

uint16_t subci(uint16_t imm, uint16_t rDst);

uint16_t cmp(uint16_t rSrc, uint16_t rDst);

uint16_t cmpi(uint16_t imm, uint16_t rDst);

uint16_t and(uint16_t rSrc, uint16_t rDst);

uint16_t andi(uint16_t imm, uint16_t rDst);

uint16_t or(uint16_t rSrc, uint16_t rDst);

uint16_t nop();

uint16_t ori(uint16_t imm, uint16_t rDst);

uint16_t xor(uint16_t rSrc, uint16_t rDst);

uint16_t xori(uint16_t imm, uint16_t rDst);

uint16_t mov(uint16_t rSrc, uint16_t rDst);

uint16_t movi(uint16_t imm, uint16_t rDst);

uint16_t lsh(uint16_t rAmt, uint16_t rDst);

uint16_t lshi(uint16_t imm, uint16_t rDst);

uint16_t ashu(uint16_t rAmt, uint16_t rDst);

uint16_t ashui(uint16_t imm, uint16_t rDst);

uint16_t lui(uint16_t imm, uint16_t rDst);

uint16_t load(uint16_t rDst, uint16_t rAddr);

uint16_t stor(uint16_t rSrc, uint16_t rAddr);

uint16_t snxb(uint16_t rSrc, uint16_t rDst);

uint16_t zrxb(uint16_t rSrc, uint16_t rDst);

uint16_t seq(uint16_t rDst);

uint16_t sne(uint16_t rDst);

uint16_t scs(uint16_t rDst);

uint16_t scc(uint16_t rDst);

uint16_t shi(uint16_t rDst);

uint16_t sls(uint16_t rDst);

uint16_t sgt(uint16_t rDst);

uint16_t sle(uint16_t rDst);

uint16_t sfs(uint16_t rDst);

uint16_t sfc(uint16_t rDst);

uint16_t slo(uint16_t rDst);

uint16_t shs(uint16_t rDst);

uint16_t slt(uint16_t rDst);

uint16_t sge(uint16_t rDst);

uint16_t suc(uint16_t rDst);

uint16_t beq(uint16_t disp);

uint16_t bne(uint16_t disp);

uint16_t bcs(uint16_t disp);

uint16_t bcc(uint16_t disp);

uint16_t bhi(uint16_t disp);

uint16_t bls(uint16_t disp);

uint16_t bgt(uint16_t disp);

uint16_t ble(uint16_t disp);

uint16_t bfs(uint16_t disp);

uint16_t bfc(uint16_t disp);

uint16_t blo(uint16_t disp);

uint16_t bhs(uint16_t disp);

uint16_t blt(uint16_t disp);

uint16_t bge(uint16_t disp);

uint16_t buc(uint16_t disp);

uint16_t jeq(uint16_t rTgt);

uint16_t jne(uint16_t rTgt);

uint16_t jcs(uint16_t rTgt);

uint16_t jcc(uint16_t rTgt);

uint16_t jhi(uint16_t rTgt);

uint16_t jls(uint16_t rTgt);

uint16_t jgt(uint16_t rTgt);

uint16_t jle(uint16_t rTgt);

uint16_t jfs(uint16_t rTgt);

uint16_t jfc(uint16_t rTgt);

uint16_t jlo(uint16_t rTgt);

uint16_t jhs(uint16_t rTgt);

uint16_t jlt(uint16_t rTgt);

uint16_t jge(uint16_t rTgt);

uint16_t juc(uint16_t rTgt);

uint16_t jal(uint16_t rLnk, uint16_t rTgt);

uint16_t tbit(uint16_t rOst, uint16_t rSrc);

uint16_t tbiti(uint16_t imm, uint16_t rSrc);

uint16_t lpr(uint16_t rSrc, uint16_t rPrc);

uint16_t spr(uint16_t rPrc, uint16_t rScr);

uint16_t di();

uint16_t ei();

uint16_t excp(uint16_t vect);

uint16_t retx();

uint16_t wait_();

/*
create a LUT to read instructions

returns: data about the instruction
*/
StringTable newInstructionTable();

#endif