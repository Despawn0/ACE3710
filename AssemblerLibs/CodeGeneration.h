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

uint16_t add(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x0050 | rSrc;
}

uint16_t addi(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x5000 | rd | imm;
}

uint16_t addu(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x0060 | rSrc;
}

uint16_t addui(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x6000 | rd | imm;
}

uint16_t addc(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x0070 | rSrc;
}

uint16_t addci(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x7000 | rd | imm;
}

uint16_t mul(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x00e0 | rSrc;
}

uint16_t muli(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0xe000 | rd | imm;
}

uint16_t sub(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x0090 | rSrc;
}

uint16_t subi(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x9000 | rd | imm;
}

uint16_t subc(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x00a0 | rSrc;
}

uint16_t subci(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0xa000 | rd | imm;
}

uint16_t cmp(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x00b0 | rSrc;
}

uint16_t cmpi(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0xb000 | rd | imm;
}

uint16_t and(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x0010 | rSrc;
}

uint16_t andi(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x1000 | rd | imm;
}

uint16_t or(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x0020 | rSrc;
}

uint16_t nop() {
    return 0x0020;
}

uint16_t ori(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x2000 | rd | imm;
}

uint16_t xor(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x0030 | rSrc;
}

uint16_t xori(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x3000 | rd | imm;
}

uint16_t mov(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return rd | 0x00d0 | rSrc;
}

uint16_t movi(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0xd000 | rd | imm;
}

uint16_t lsh(uint16_t rAmt, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x8040 | rd | rAmt;
}

uint16_t lshi(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x8000 | rd | imm;
}

uint16_t ashu(uint16_t rAmt, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x8060 | rd | rAmt;
}

uint16_t ashui(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x8020 | rd | imm;
}

uint16_t lui(uint16_t imm, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0xf000 | rd | imm;
}

uint16_t load(uint16_t rDst, uint16_t rAddr) {
    uint16_t rd = rDst << 8;
    return 0x4000 | rd | rAddr;
}

uint16_t stor(uint16_t rSrc, uint16_t rAddr) {
    uint16_t rs = rSrc << 8;
    return 0x4040 | rs | rAddr;
}

uint16_t snxb(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x4020 | rd | rSrc;
}

uint16_t zrxb(uint16_t rSrc, uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x4060 | rd | rSrc;
}

uint16_t seq(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40d0 | rd;
}

uint16_t sne(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40d1 | rd;
}

uint16_t scs(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40d2 | rd;
}

uint16_t scc(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40d3 | rd;
}

uint16_t shi(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40d4 | rd;
}

uint16_t sls(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40d5 | rd;
}

uint16_t sgt(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40d6 | rd;
}

uint16_t sle(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40d7 | rd;
}

uint16_t sfs(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40d8 | rd;
}

uint16_t sfc(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40d9 | rd;
}

uint16_t slo(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40da | rd;
}

uint16_t shs(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40db | rd;
}

uint16_t slt(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40dc | rd;
}

uint16_t sge(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40dd | rd;
}

uint16_t suc(uint16_t rDst) {
    uint16_t rd = rDst << 8;
    return 0x40de | rd;
}

uint16_t beq(uint16_t disp) {
    return 0xc000 | disp;
}

uint16_t bne(uint16_t disp) {
    return 0xc100 | disp;
}

uint16_t bcs(uint16_t disp) {
    return 0xc200 | disp;
}

uint16_t bcc(uint16_t disp) {
    return 0xc300 | disp;
}

uint16_t bhi(uint16_t disp) {
    return 0xc400 | disp;
}

uint16_t bls(uint16_t disp) {
    return 0xc500 | disp;
}

uint16_t bgt(uint16_t disp) {
    return 0xc600 | disp;
}

uint16_t ble(uint16_t disp) {
    return 0xc700 | disp;
}

uint16_t bfs(uint16_t disp) {
    return 0xc800 | disp;
}

uint16_t bfc(uint16_t disp) {
    return 0xc900 | disp;
}

uint16_t blo(uint16_t disp) {
    return 0xca00 | disp;
}

uint16_t bhs(uint16_t disp) {
    return 0xcb00 | disp;
}

uint16_t blt(uint16_t disp) {
    return 0xcc00 | disp;
}

uint16_t bge(uint16_t disp) {
    return 0xcd00 | disp;
}

uint16_t buc(uint16_t disp) {
    return 0xce00 | disp;
}

uint16_t jeq(uint16_t rTgt) {
    return 0x40c0 | rTgt;
}

uint16_t jne(uint16_t rTgt) {
    return 0x41c0 | rTgt;
}

uint16_t jcs(uint16_t rTgt) {
    return 0x42c0 | rTgt;
}

uint16_t jcc(uint16_t rTgt) {
    return 0x43c0 | rTgt;
}

uint16_t jhi(uint16_t rTgt) {
    return 0x44c0 | rTgt;
}

uint16_t jls(uint16_t rTgt) {
    return 0x45c0 | rTgt;
}

uint16_t jgt(uint16_t rTgt) {
    return 0x46c0 | rTgt;
}

uint16_t jle(uint16_t rTgt) {
    return 0x47c0 | rTgt;
}

uint16_t jfs(uint16_t rTgt) {
    return 0x48c0 | rTgt;
}

uint16_t jfc(uint16_t rTgt) {
    return 0x49c0 | rTgt;
}

uint16_t jlo(uint16_t rTgt) {
    return 0x4ac0 | rTgt;
}

uint16_t jhs(uint16_t rTgt) {
    return 0x4bc0 | rTgt;
}

uint16_t jlt(uint16_t rTgt) {
    return 0x4cc0 | rTgt;
}

uint16_t jge(uint16_t rTgt) {
    return 0x4dc0 | rTgt;
}

uint16_t juc(uint16_t rTgt) {
    return 0x4ec0 | rTgt;
}

uint16_t jal(uint16_t rLnk, uint16_t rTgt) {
    uint16_t rl = rLnk << 8;
    return 0x4080 | rl | rTgt;
}

uint16_t tbit(uint16_t rOst, uint16_t rSrc) {
    uint16_t rs = rSrc << 8;
    return 0x40a0 | rs | rOst;
}

uint16_t tbiti(uint16_t imm, uint16_t rSrc) {
    uint16_t rs = rSrc << 8;
    return 0x40e0 | rs | imm;
}

uint16_t lpr(uint16_t rSrc, uint16_t rPrc) {
    uint16_t rs = rSrc << 8;
    return 0x4010 | rs | rPrc;
}

uint16_t spr(uint16_t rPrc, uint16_t rScr) {
    uint16_t rp = rPrc << 8;
    return 0x4050 | rp | rScr;
}

uint16_t di() {
    return 0x4030;
}

uint16_t ei() {
    return 0x4070;
}

uint16_t excp(uint16_t vect) {
    return 0x40b0 | vect;
}

uint16_t retx() {
    return 0x4090;
}

uint16_t wait_() {
    return 0x0000;
}

/*
create a LUT to read instructions

returns: data about the instruction
*/
StringTable newInstructionTable() {
    // create instruction data
    const InstData add_ = {.param2 = &add, reg};
    const InstData addi_ = {.param2 = &addi, imm};
    const InstData addu_ = {.param2 = &addu, reg};
    const InstData addui_ = {.param2 = &addui, imm};
    const InstData addc_ = {.param2 = &addc, reg};
    const InstData addci_ = {.param2 = &addci, imm};
    const InstData mul_ = {.param2 = &mul, reg};
    const InstData muli_ = {.param2 = &muli, imm};
    const InstData sub_ = {.param2 = &sub, reg};
    const InstData subi_ = {.param2 = &subi, imm};
    const InstData subc_ = {.param2 = &subc, reg};
    const InstData subci_ = {.param2 = &subci, imm};
    const InstData cmp_ = {.param2 = &cmp, reg};
    const InstData cmpi_ = {.param2 = &cmpi, imm};
    const InstData and_ = {.param2 = &and, reg};
    const InstData andi_ = {.param2 = &andi, imm};
    const InstData or_ = {.param2 = &or, reg};
    const InstData nop_ = {.param0 = &nop, imp};
    const InstData ori_ = {.param2 = &ori, imm};
    const InstData xor_ = {.param2 = &xor, reg};
    const InstData xori_ = {.param2 = &xori, imm};
    const InstData mov_ = {.param2 = &mov, reg};
    const InstData movi_ = {.param2 = &movi, imm};
    const InstData lsh_ = {.param2 = &lsh, reg};
    const InstData lshi_ = {.param2 = &lshi, sft};
    const InstData ashu_ = {.param2 = &ashu, reg};
    const InstData ashui_ = {.param2 = &ashui, sft};
    const InstData lui_ = {.param2 = &lui, imm};
    const InstData load_ = {.param2 = &load, reg};
    const InstData stor_ = {.param2 = &stor, reg};
    const InstData snxb_ = {.param2 = &snxb, reg};
    const InstData zrxb_ = {.param2 = &zrxb, reg};
    const InstData seq_ = {.param1 = &seq, jrg};
    const InstData sne_ = {.param1 = &sne, jrg};
    const InstData scs_ = {.param1 = &scs, jrg};
    const InstData scc_ = {.param1 = &scc, jrg};
    const InstData shi_ = {.param1 = &shi, jrg};
    const InstData sls_ = {.param1 = &sls, jrg};
    const InstData sgt_ = {.param1 = &sgt, jrg};
    const InstData sle_ = {.param1 = &sle, jrg};
    const InstData sfs_ = {.param1 = &sfs, jrg};
    const InstData sfc_ = {.param1 = &sfc, jrg};
    const InstData slo_ = {.param1 = &slo, jrg};
    const InstData shs_ = {.param1 = &shs, jrg};
    const InstData slt_ = {.param1 = &slt, jrg};
    const InstData sge_ = {.param1 = &sge, jrg};
    const InstData suc_ = {.param1 = &suc, jrg};
    const InstData beq_ = {.param1 = &beq, brc};
    const InstData bne_ = {.param1 = &bne, brc};
    const InstData bcs_ = {.param1 = &bcs, brc};
    const InstData bcc_ = {.param1 = &bcc, brc};
    const InstData bhi_ = {.param1 = &bhi, brc};
    const InstData bls_ = {.param1 = &bls, brc};
    const InstData bgt_ = {.param1 = &bgt, brc};
    const InstData ble_ = {.param1 = &ble, brc};
    const InstData bfs_ = {.param1 = &bfs, brc};
    const InstData bfc_ = {.param1 = &bfc, brc};
    const InstData blo_ = {.param1 = &blo, brc};
    const InstData bhs_ = {.param1 = &bhs, brc};
    const InstData blt_ = {.param1 = &blt, brc};
    const InstData bge_ = {.param1 = &bge, brc};
    const InstData buc_ = {.param1 = &buc, brc};
    const InstData jeq_ = {.param1 = &jeq, jrg};
    const InstData jne_ = {.param1 = &jne, jrg};
    const InstData jcs_ = {.param1 = &jcs, jrg};
    const InstData jcc_ = {.param1 = &jcc, jrg};
    const InstData jhi_ = {.param1 = &jhi, jrg};
    const InstData jls_ = {.param1 = &jls, jrg};
    const InstData jgt_ = {.param1 = &jgt, jrg};
    const InstData jle_ = {.param1 = &jle, jrg};
    const InstData jfs_ = {.param1 = &jfs, jrg};
    const InstData jfc_ = {.param1 = &jfc, jrg};
    const InstData jlo_ = {.param1 = &jlo, jrg};
    const InstData jhs_ = {.param1 = &jhs, jrg};
    const InstData jlt_ = {.param1 = &jlt, jrg};
    const InstData jge_ = {.param1 = &jge, jrg};
    const InstData juc_ = {.param1 = &juc, jrg};
    const InstData jal_ = {.param2 = &jal, reg};
    const InstData tbit_ = {.param2 = &tbit, reg};
    const InstData tbiti_ = {.param2 = &tbiti, reg};
    const InstData lpr_ = {.param2 = &lpr, reg};
    const InstData spr_ = {.param2 = &spr, reg};
    const InstData di_ = {.param0 = &di, imp};
    const InstData ei_ = {.param0 = &ei, imp};
    const InstData excp_ = {.param1 = excp, jrg};
    const InstData retx_ = {.param0 = retx, imp};
    const InstData wait__ = {.param0 = wait_, imp};

    // place all values in a LUT
    StringTable out = newStringTable();
    setStringTableValue(out, "add", 4, &add_, sizeof(InstData));
    setStringTableValue(out, "addi", 5, &addi_, sizeof(InstData));
    setStringTableValue(out, "addu", 5, &addu_, sizeof(InstData));
    setStringTableValue(out, "addui", 6, &addui_, sizeof(InstData));
    setStringTableValue(out, "addc", 5, &addc_, sizeof(InstData));
    setStringTableValue(out, "addci", 6, &addci_, sizeof(InstData));
    setStringTableValue(out, "mul", 4, &mul_, sizeof(InstData));
    setStringTableValue(out, "muli", 5, &muli_, sizeof(InstData));
    setStringTableValue(out, "sub", 4, &sub_, sizeof(InstData));
    setStringTableValue(out, "subi", 5, &subi_, sizeof(InstData));
    setStringTableValue(out, "subc", 5, &subc_, sizeof(InstData));
    setStringTableValue(out, "subci", 6, &subci_, sizeof(InstData));
    setStringTableValue(out, "cmp", 4, &cmp_, sizeof(InstData));
    setStringTableValue(out, "cmpi", 5, &cmpi_, sizeof(InstData));
    setStringTableValue(out, "and", 4, &and_, sizeof(InstData));
    setStringTableValue(out, "andi", 5, &andi_, sizeof(InstData));
    setStringTableValue(out, "or", 3, &or_, sizeof(InstData));
    setStringTableValue(out, "nop", 4, &nop_, sizeof(InstData));
    setStringTableValue(out, "ori", 4, &ori_, sizeof(InstData));
    setStringTableValue(out, "xor", 4, &xor_, sizeof(InstData));
    setStringTableValue(out, "xori", 5, &xori_, sizeof(InstData));
    setStringTableValue(out, "mov", 4, &mov_, sizeof(InstData));
    setStringTableValue(out, "movi", 5, &movi_, sizeof(InstData));
    setStringTableValue(out, "lsh", 4, &lsh_, sizeof(InstData));
    setStringTableValue(out, "lshi", 5, &lshi_, sizeof(InstData));
    setStringTableValue(out, "ashu", 5, &ashu_, sizeof(InstData));
    setStringTableValue(out, "ashui", 6, &ashui_, sizeof(InstData));
    setStringTableValue(out, "lui", 4, &lui_, sizeof(InstData));
    setStringTableValue(out, "load", 5, &load_, sizeof(InstData));
    setStringTableValue(out, "stor", 5, &stor_, sizeof(InstData));
    setStringTableValue(out, "snxb", 5, &snxb_, sizeof(InstData));
    setStringTableValue(out, "zrxb", 5, &zrxb_, sizeof(InstData));
    setStringTableValue(out, "seq", 4, &seq_, sizeof(InstData));
    setStringTableValue(out, "sne", 4, &sne_, sizeof(InstData));
    setStringTableValue(out, "scs", 4, &scs_, sizeof(InstData));
    setStringTableValue(out, "scc", 4, &scc_, sizeof(InstData));
    setStringTableValue(out, "shi", 4, &shi_, sizeof(InstData));
    setStringTableValue(out, "sls", 4, &sls_, sizeof(InstData));
    setStringTableValue(out, "sgt", 4, &sgt_, sizeof(InstData));
    setStringTableValue(out, "sle", 4, &sle_, sizeof(InstData));
    setStringTableValue(out, "sfs", 4, &sfs_, sizeof(InstData));
    setStringTableValue(out, "sfc", 4, &sfc_, sizeof(InstData));
    setStringTableValue(out, "slo", 4, &slo_, sizeof(InstData));
    setStringTableValue(out, "shs", 4, &shs_, sizeof(InstData));
    setStringTableValue(out, "slt", 4, &slt_, sizeof(InstData));
    setStringTableValue(out, "sge", 4, &sge_, sizeof(InstData));
    setStringTableValue(out, "suc", 4, &suc_, sizeof(InstData));
    setStringTableValue(out, "beq", 4, &beq_, sizeof(InstData));
    setStringTableValue(out, "bne", 4, &bne_, sizeof(InstData));
    setStringTableValue(out, "bcs", 4, &bcs_, sizeof(InstData));
    setStringTableValue(out, "bcc", 4, &bcc_, sizeof(InstData));
    setStringTableValue(out, "bhi", 4, &bhi_, sizeof(InstData));
    setStringTableValue(out, "bls", 4, &bls_, sizeof(InstData));
    setStringTableValue(out, "bgt", 4, &bgt_, sizeof(InstData));
    setStringTableValue(out, "ble", 4, &ble_, sizeof(InstData));
    setStringTableValue(out, "bfs", 4, &bfs_, sizeof(InstData));
    setStringTableValue(out, "bfc", 4, &bfc_, sizeof(InstData));
    setStringTableValue(out, "blo", 4, &blo_, sizeof(InstData));
    setStringTableValue(out, "bhs", 4, &bhs_, sizeof(InstData));
    setStringTableValue(out, "blt", 4, &blt_, sizeof(InstData));
    setStringTableValue(out, "bge", 4, &bge_, sizeof(InstData));
    setStringTableValue(out, "buc", 4, &buc_, sizeof(InstData));
    setStringTableValue(out, "jeq", 4, &jeq_, sizeof(InstData));
    setStringTableValue(out, "jne", 4, &jne_, sizeof(InstData));
    setStringTableValue(out, "jcs", 4, &jcs_, sizeof(InstData));
    setStringTableValue(out, "jcc", 4, &jcc_, sizeof(InstData));
    setStringTableValue(out, "jhi", 4, &jhi_, sizeof(InstData));
    setStringTableValue(out, "jls", 4, &jls_, sizeof(InstData));
    setStringTableValue(out, "jgt", 4, &jgt_, sizeof(InstData));
    setStringTableValue(out, "jle", 4, &jle_, sizeof(InstData));
    setStringTableValue(out, "jfs", 4, &jfs_, sizeof(InstData));
    setStringTableValue(out, "jfc", 4, &jfc_, sizeof(InstData));
    setStringTableValue(out, "jlo", 4, &jlo_, sizeof(InstData));
    setStringTableValue(out, "jhs", 4, &jhs_, sizeof(InstData));
    setStringTableValue(out, "jlt", 4, &jlt_, sizeof(InstData));
    setStringTableValue(out, "jge", 4, &jge_, sizeof(InstData));
    setStringTableValue(out, "juc", 4, &juc_, sizeof(InstData));
    setStringTableValue(out, "jal", 4, &jal_, sizeof(InstData));
    setStringTableValue(out, "tbit", 5, &tbit_, sizeof(InstData));
    setStringTableValue(out, "tbiti", 6, &tbiti_, sizeof(InstData));
    setStringTableValue(out, "lpr", 4, &lpr_, sizeof(InstData));
    setStringTableValue(out, "spr", 4, &spr_, sizeof(InstData));
    setStringTableValue(out, "di", 3, &di_, sizeof(InstData));
    setStringTableValue(out, "ei", 3, &ei_, sizeof(InstData));
    setStringTableValue(out, "excp", 5, &excp_, sizeof(InstData));
    setStringTableValue(out, "retx", 5, &retx_, sizeof(InstData));
    setStringTableValue(out, "wait", 5, &wait__, sizeof(InstData));
    return out;
}

#endif