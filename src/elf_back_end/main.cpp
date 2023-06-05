#include <stdio.h>
#include "encode.h"
#include "generator.h"
#include "elf_file.h"

int
main()
{
        // Generator module: construct code_t struct, according to AST and using 
        // encoding module fill an array of uint8_t.
        code_t code = {};
        en_code_ctor(&code, 1000);

        arg_t arg1 = {.type = ARG_MEM, .val = {.mem = {.sib_on = true, .disp_on = true, .reg_on = true, 
                                        .scale = 8, .index = REG_EBX, .disp = 8, .reg = REG_EAX}}};
        arg_t arg2 = {.type = ARG_IMM, .val = {.imm = 21}};
        cmd_token_t cmd = {.instr = INSTR_MOV, .arg1 = arg1, .arg2 = arg2};
        en_emit(&code, &cmd);

        for (int i = 0; i < code.size; i++) {
                fprintf(stderr, "%02x ", code.code[i]);
        }

        en_code_dtor(&code);
        
        // Elf module: using code_t size and contents build ELF exec.
        
        return 0;
}
