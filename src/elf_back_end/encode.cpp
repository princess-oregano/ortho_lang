#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include "encode.h"

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

        if (mem->val.mem.sib_on) {
                rm |= (non_mem->val.reg << 3);
                rm |= REG_ESP;
                en_add_byte(code, rm);
                en_sib(code, &mem->val.mem);
        } else if (mem->val.mem.reg_on) {
                rm |= (non_mem->val.reg << 3);
                rm |=  mem->val.mem.reg;
                en_add_byte(code, rm);
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
                        } else if (arg1.type == ARG_IMM) {
                                // Invalid, as arg1 == imm is possible only in 
                                // exceptions such as int, push/pop, call, etc.
                                assert(0 && "Invalid arg1 value.");
                        } else if (arg1.type == ARG_MEM) {
                                en_mem(code, &arg1, &arg2);
                        }
                        break;
                case ARG_MEM:
                        code->code[code->size - 1] |= DEST_MASK;
                        en_mem(code, &arg2, &arg1);
                        break;
                // arg2 imm is exception.
                case ARG_IMM:
                case ARG_INV:
                default:
                        assert(0 && "Invalid arg1 type.");
                        break;
        }

        return EN_NO_ERR;
}

// Push/pop have one argument only, so handle them with separate function.
static int
en_push_pop(code_t *code, cmd_token_t *cmd)
{
        assert(code);

        arg_t mem_push_pop = {.type = ARG_REG, .val = {.reg = REG_ESI}};

        switch (cmd->arg1.type) {
                case ARG_REG:
                        en_simple(code, *cmd);
                        break;
                case ARG_IMM:
                        en_add_byte(code, PUSH_IMM);
                        en_add_imm32(code, &cmd->arg1.val.imm);
                        break;
                case ARG_MEM:
                        en_add_byte(code, PUSH_MEM);
                        en_mem(code, &cmd->arg1, &mem_push_pop);
                        break;
                case ARG_INV:
                default:
                        assert(0 && "Invalid argument type.");
                        break;
        }

        return EN_NO_ERR;
}

int 
en_imm(code_t *code, cmd_token_t *cmd)
{
        arg_t tmp_arg = {.type = ARG_REG};
        
        switch (cmd->instr) {
                case INSTR_MOV:
                        if (cmd->arg1.type == ARG_REG) {
                                en_add_byte(code, MOV_REG_IMM + cmd->arg1.val.reg);
                                en_add_imm32(code, &cmd->arg2.val.imm);
                        } else {
                                en_add_byte(code, MOV_MEM_IMM | SIZE_MASK);
                                tmp_arg.val.reg = REG_EAX;
                                en_args(code, cmd->arg1, tmp_arg);
                                en_add_imm32(code, &cmd->arg2.val.imm);
                        }
                        break;
                case INSTR_PUSH:
                case INSTR_POP:
                        en_push_pop(code, cmd);
                        break;
                case INSTR_ADD:
                        en_add_byte(code, IMM_EXC | SIZE_MASK | DEST_MASK);
                        tmp_arg.val.reg = REG_EAX;
                        en_args(code, cmd->arg1, tmp_arg);
                        en_add_imm32(code, &cmd->arg2.val.imm);
                        break;
                case INSTR_SUB:
                        en_add_byte(code, IMM_EXC | SIZE_MASK | DEST_MASK);
                        tmp_arg.val.reg = REG_EBP;
                        en_args(code, cmd->arg1, tmp_arg);
                        en_add_imm32(code, &cmd->arg2.val.imm);
                        break;
                case INSTR_OR:
                        en_add_byte(code, IMM_EXC | SIZE_MASK | DEST_MASK);
                        tmp_arg.val.reg = REG_ECX;
                        en_args(code, cmd->arg1, tmp_arg);
                        en_add_imm32(code, &cmd->arg2.val.imm);
                        break;
                case INSTR_AND:
                        en_add_byte(code, IMM_EXC | SIZE_MASK | DEST_MASK);
                        tmp_arg.val.reg = REG_EBX;
                        en_args(code, cmd->arg1, tmp_arg);
                        en_add_imm32(code, &cmd->arg2.val.imm);
                        break;
                case INSTR_XOR:
                        en_add_byte(code, IMM_EXC | SIZE_MASK | DEST_MASK);
                        tmp_arg.val.reg = REG_ESI;
                        en_args(code, cmd->arg1, tmp_arg);
                        en_add_imm32(code, &cmd->arg2.val.imm);
                        break;
                case INSTR_CMP:
                        en_add_byte(code, IMM_EXC | SIZE_MASK | DEST_MASK);
                        tmp_arg.val.reg = REG_EDI;
                        en_args(code, cmd->arg1, tmp_arg);
                        en_add_imm32(code, &cmd->arg2.val.imm);
                        break;
                case INSTR_JMP:
                        /* TODO: Same question */
                        en_add_byte(code, JMP);
                        en_add_imm32(code, &cmd->arg1.val.imm);
                        break;
                case INSTR_INT:
                        en_add_byte(code, INT);
                        en_add_byte(code, cmd->arg1.val.imm);
                        break;
                case INSTR_RET:
                        en_add_byte(code, RET);
                        break;
                case INSTR_CALL:
                        en_add_byte(code, CALL);
                        en_add_imm32(code, &cmd->arg1.val.imm);
                        break;
                case INSTR_DIV:
                case INSTR_MUL:
                default:
                        assert(0 && "Invalid instruction.");
                        break;
        }

        return EN_NO_ERR;
}

int
en_emit(code_t *code, cmd_token_t *cmd)
{
        assert(code);

        if (cmd->arg2.type == ARG_IMM) {
                en_imm(code, cmd);
                return EN_NO_ERR;
        }

        // First byte is opcode.
        switch (cmd->instr) {
                case INSTR_MOV:
                        en_add_byte(code, MOV | SIZE_MASK);
                        en_args(code, cmd->arg1, cmd->arg2);
                        break;
                case INSTR_PUSH:
                case INSTR_POP:
                        en_push_pop(code, cmd);
                        break;
                case INSTR_ADD:
                        en_add_byte(code, ADD | SIZE_MASK);
                        en_args(code, cmd->arg1, cmd->arg2);
                        break;
                case INSTR_SUB:
                        en_add_byte(code, SUB | SIZE_MASK);
                        en_args(code, cmd->arg1, cmd->arg2);
                        break;
                case INSTR_DIV:
                        en_add_byte(code, DIV | SIZE_MASK);
                        break;
                case INSTR_MUL:
                        /* TODO: Mul/div: handle somehow */
                        break;
                case INSTR_OR:
                        en_add_byte(code, OR | SIZE_MASK);
                        en_args(code, cmd->arg1, cmd->arg2);
                        break;
                case INSTR_AND:
                        en_add_byte(code, AND | SIZE_MASK);
                        en_args(code, cmd->arg1, cmd->arg2);
                        break;
                case INSTR_XOR:
                        en_add_byte(code, XOR | SIZE_MASK);
                        en_args(code, cmd->arg1, cmd->arg2);
                        break;
                        break;
                case INSTR_CMP:
                        en_add_byte(code, CMP | DEST_MASK);
                        en_args(code, cmd->arg1, cmd->arg2);
                        break;
                case INSTR_CALL:
                case INSTR_JMP:
                case INSTR_INT:
                case INSTR_RET:
                default:
                        assert(0 && "Invalid instruction.");
                        break;
        }
        
        return EN_NO_ERR;
}

