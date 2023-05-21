#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include "encode.h"

/* TODO: change name to emitter */

int 
en_code_resize(code_t *code, long long new_cap)
{
        assert(code);
        assert(new_cap > 0);

        uint8_t *code_ptr = (uint8_t *) realloc(code->code, new_cap * sizeof(uint8_t));
        if (code_ptr == nullptr) {
                fprintf(stderr, "Could not allocate memory for code_t.\n");
                return EN_ALLOC;
        }
        
        code->code = code_ptr;
        code->cap = new_cap;  

        return EN_NO_ERR;
}

int
en_code_ctor(code_t *code, long long cap)
{
        assert(code);

        if (en_code_resize(code, cap) != EN_NO_ERR)
                return EN_ALLOC;

        return EN_NO_ERR;
}

static int
en_add_byte(code_t *code, uint8_t byte)
{
        assert(code);

        if (code->size >= code->cap) {
                if (en_code_resize(code, 2 * code->cap) != EN_NO_ERR)
                        return EN_ALLOC;
        }

        code->code[code->size++] = byte;

        return EN_NO_ERR;
}

void
en_code_dtor(code_t *code)
{
        assert(code);

        free(code->code);
        code->cap = -1;
        code->size = -1;
}

/// Simple opcode, calculated as OPCODE + REG_NUM.
static int
en_simple(code_t *code, cmd_token_t cmd)
{
        assert(code);
        assert(cmd.arg1.type == ARG_REG);

        uint8_t opcode = 0;
        switch (cmd.instr) {
                case INSTR_PUSH:
                        opcode = PUSH_REG;
                        break;
                case INSTR_POP:
                        opcode = POP_REG;
                        break;
                default:
                        assert(0 && "Invalid instruction type.");
                        break;
        }

        en_add_byte(code, opcode + cmd.arg1.val.reg);

        return EN_NO_ERR;
}

static int
en_add_imm32(code_t *code, uint32_t *num32)
{
        assert(code);

        uint8_t *num8 = (uint8_t *) num32;

        for (int i = 0; i < 4; i++) {
                en_add_byte(code, num8[i]); 
        }

        return EN_NO_ERR;
}

/// Handle SIB byte.
static int
en_sib(code_t *code, mem_t *mem)
{
        assert(code);
        assert(mem->sib_on); 

        uint8_t sib = 0;
        
        switch (mem->scale) {
                case 1:
                        sib |= SIB_SCALE1;
                        break;
                case 2:
                        sib |= SIB_SCALE2;
                        break;
                case 4:
                        sib |= SIB_SCALE4;
                        break;
                case 8:
                        sib |= SIB_SCALE8;
                        break;
                default:
                        assert(0 && "Invalid scale value.");
                        break;
        }

        sib |= (mem->index << 3);
        sib |=  mem->reg;

        en_add_byte(code, sib);

        return EN_NO_ERR;
}

/// Handle memory addressing.
static int
en_mem(code_t *code, arg_t *mem, arg_t *non_mem)
{
        assert(code);

        uint8_t rm = 0;

        if (mem->val.mem.disp_on) {
                rm |= MOD_DISP8;
        }

        if (mem->val.mem.reg_on) {
                rm |= (mem->val.reg << 3);
                rm |=  non_mem->val.reg;
        } else if (mem->val.mem.sib_on) {
                rm |= (non_mem->val.reg << 3);
                rm |= REG_ESP;
                en_add_byte(code, rm);
                en_sib(code, &mem->val.mem);
        }

        if (mem->val.mem.disp_on) {
                en_add_byte(code, mem->val.mem.disp);
        }

        return EN_NO_ERR;
}
/// Handle instuction arguments.
static int
en_args(code_t *code, arg_t arg1, arg_t arg2)
{
        assert(code);

        uint8_t rm = 0;

        switch (arg2.type) {
                case ARG_REG:
                        if (arg1.type == ARG_REG) {
                                rm |= MOD_REG;
                                rm |= (arg2.val.reg << 3);
                                rm |=  arg1.val.reg;
                                en_add_byte(code, rm);
                        break;
                        } else if (arg1.type == ARG_IMM) {
                                /* TODO: Immediate -- exception + mask for prev instr*/
                        } else if (arg1.type == ARG_MEM) {
                                en_mem(code, &arg1, &arg2);
                        }
                case ARG_MEM:
                        en_mem(code, &arg2, &arg1);
                        break;
                case ARG_IMM:
                // Exception of push/pop, where arg2 is absent.
                case ARG_INV:
                default:
                        assert(0 && "Invalid arg1 type.");
                        break;
        }

        return EN_NO_ERR;
}

// Push/pop have one argument only, so handle them with separate function.
static int
en_push_pop(code_t *code, cmd_token_t cmd)
{
        assert(code);

        switch (cmd.arg1.type) {
                case ARG_REG:
                        en_simple(code, cmd);
                        break;
                case ARG_IMM:
                        en_add_byte(code, PUSH_IMM);
                        en_add_imm32(code, &cmd.arg1.val.imm);
                        break;
                case ARG_MEM:
                        en_add_byte(code, PUSH_MEM);
                        // TODO: en_mem();
                        break;
                case ARG_INV:
                default:
                        assert(0 && "Invalid argument type.");
                        break;
        }

        return EN_NO_ERR;
}

int
encode(code_t *code, cmd_token_t cmd)
{
        assert(code);

        // First byte is opcode.
        switch (cmd.instr) {
                case INSTR_PUSH:
                case INSTR_POP:
                        en_push_pop(code, cmd);
                        break;
                case INSTR_ADD:
                        en_add_byte(code, ADD | SIZE_MASK);
                        en_args(code, cmd.arg1, cmd.arg2);
                        break;
                default:
                        assert(0 && "Invalid instruction.");
                        break;
        }
        
        return EN_NO_ERR;
}

