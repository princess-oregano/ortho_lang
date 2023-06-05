#ifndef ENCODE_H
#define ENCODE_H

#include <cstdint>
#include <stdint.h>
#include <sys/types.h>

// Prefixes.
const uint8_t PUSH_POP_MEM = 0x66;
const uint8_t PREFIX       = 0x0F;

// Op-codes for instructions.
const uint8_t PUSH_IMM    = 0x68;
const uint8_t PUSH_REG    = 0x50;
const uint8_t PUSH_MEM    = 0xFF;
const uint8_t POP_REG     = 0x58;
const uint8_t MOV         = 0x88;
const uint8_t MOV_REG_IMM = 0xB8;
const uint8_t MOV_MEM_IMM = 0xC6;
const uint8_t XCHG        = 0x86;

const uint8_t ADD         = 0x00;
const uint8_t SUB         = 0x28;
const uint8_t MUL         = 0xF6;
const uint8_t DIV         = 0xF6;
const uint8_t OR          = 0x08;
const uint8_t AND         = 0x20;
const uint8_t XOR         = 0x30;
const uint8_t CMP         = 0x38;
const uint8_t JMP         = 0xE9;
const uint8_t CALL        = 0xE8;

// Immediate arg exception.
const uint8_t IMM_EXC     = 0x80;

// Op-code masks.
const uint8_t IMM_MASK  = 0x80;
const uint8_t DEST_MASK = 0x2;
const uint8_t SIZE_MASK = 0x1;

// MOD-R/M masks.
const uint8_t MOD_DISP0  = 0x00;
const uint8_t MOD_DISP8  = 0x40;
const uint8_t MOD_DISP32 = 0x80;
const uint8_t MOD_REG    = 0xC0;

// SIB masks.
const uint8_t SIB_SCALE1 = 0x00;
const uint8_t SIB_SCALE2 = 0x40;
const uint8_t SIB_SCALE4 = 0x80;
const uint8_t SIB_SCALE8 = 0xC0;

// Register masks.
const uint8_t REG_EAX = 0x00;
const uint8_t REG_ECX = 0x01;
const uint8_t REG_EDX = 0x02;
const uint8_t REG_EBX = 0x03;
const uint8_t REG_ESP = 0x04;
const uint8_t REG_EBP = 0x05;
const uint8_t REG_ESI = 0x06;
const uint8_t REG_EDI = 0x07;

enum en_err_t {
        EN_NO_ERR = 0,
        EN_ALLOC  = 1,
};

/// MOD-R/M struct. 
struct mem_t {
        bool  sib_on = false;   /// Scale-index-base mode on.
        bool disp_on = false;   /// Displacement mode on.
        bool  reg_on = false;   /// [REG] mode on.

        uint8_t  scale = 0;
        uint8_t  index = 0;
        uint32_t disp  = 0;
        uint8_t  reg   = 0;     /// If SIB on, then reg == base.
};

enum arg_type_t {
        ARG_INV = -1,
        ARG_REG =  0,
        ARG_MEM =  1,
        ARG_IMM =  2,
};

union arg_val_t {
        mem_t mem;      /// Memory.
        uint32_t imm;   /// Immediate.
        uint8_t reg;    /// Register.
};

struct arg_t {
        int type = ARG_INV;
        arg_val_t val = {};
};

enum instr_t {
        INSTR_ADD  = 0,
        INSTR_SUB  = 1,
        INSTR_DIV  = 2,
        INSTR_MUL  = 3,
        INSTR_PUSH = 4,
        INSTR_POP  = 5,
        INSTR_MOV  = 6,
        INSTR_CALL = 7,
        INSTR_AND  = 8,
        INSTR_OR   = 9,
        INSTR_XOR  = 10,
        INSTR_CMP  = 11,
        INSTR_JMP  = 12,
};

/// Instruction token.
struct cmd_token_t {
        int instr  = 0;  /// Instruction enum.
        arg_t arg1 = {}; /// Arguments; may be unused.
        arg_t arg2 = {};
};

struct code_t {
        uint8_t *code  = nullptr;
        long long size = 0;
        long long cap  = 0;
};

/// Constructs code_t structure.
int
en_code_ctor(code_t *code, long long cap);

/// Encodes given token and places it to array with code.
int
en_emit(code_t *code, cmd_token_t *cmd);

/// Destructs code_t structure.
void
en_code_dtor(code_t *code);

#endif // ENCODE_H

