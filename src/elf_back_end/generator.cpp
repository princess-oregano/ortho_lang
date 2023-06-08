#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "encode.h"
#include "generator.h"
#include "symbol.h"
#include "../stack/stack.h"
#include "../types.h"
#include "../file.h"
#include "../log.h"

static int LABEL_COUNT = 0;
static int RAM_OFFSET = 0;
static stack_t var_stack = {};
static table_t func_table = {};

static int
get_delim_buf(char **line, int delim, char *buffer)
{
        assert(line);
        assert(buffer);

        size_t count = 0;
        for ( ; buffer[count] != delim; count++)
                ;

        *line = (char *) calloc (count + 1, sizeof(char));
        if (*line == nullptr) {
                log("Couldn't allocate memory.\n");
                return 0;
        }

        memcpy(*line, buffer, count);

        return (int) count;
}

static void
gen_assign(tree_t *ast, int *pos, code_t *code)
{
        assert(ast);
        assert(pos);
        assert(code);

        int tmp = ast->nodes[*pos].left;

        uint32_t offset = sym_find(ast->nodes[tmp].data.val.var, &var_stack);
        if (offset == -1) {
                sym_insert(ast->nodes[tmp].data.val.var, var_stack.data[var_stack.size - 1], RAM_OFFSET);
                offset = RAM_OFFSET;
                RAM_OFFSET++;
        }

        cmd_token_t cmd = {.instr = INSTR_POP, .arg1 = {.type = ARG_MEM, 
                .val = {.mem = {.disp_on = true, .reg_on = true, .disp = offset * 4, .reg = REG_EBP}}}};
        en_emit(code, &cmd);
        // fprintf(stream, "        pop [rbx + %d]\n", offset * 4);
}

static int
gen_push_args(tree_t *ast, int *pos, code_t *code, uint32_t *num_of_args)
{
        if (ast->nodes[*pos].data.type == TOK_POISON)
                return GEN_NO_ERR;

        gen_write_asm(ast, &ast->nodes[*pos].right, code);

        (*num_of_args)++;
        gen_push_args(ast, &ast->nodes[*pos].left, code, num_of_args);

        return GEN_NO_ERR;
}

int
gen_identifier(tree_t *ast, int *pos, code_t *code)
{
        assert(ast);
        assert(pos);
        assert(code);

        cmd_token_t cmd = {};

        char *name = ast->nodes[*pos].data.val.var;

        int table_num = sym_lookup(name, &func_table);

        int offset = -1;
        if (table_num == -1) {
                offset = sym_find(name, &var_stack);
                if (offset == -1) {
                        log("Error: Undeclared variable.\n");
                        return GEN_UNDECL;
                }
        } else {
                uint32_t num_of_args = 0;
                gen_push_args(ast, &ast->nodes[*pos].left, code, &num_of_args);
                cmd.instr = INSTR_PUSH;
                cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_EBP}};
                en_emit(code, &cmd);

                cmd.instr = INSTR_POP;
                en_emit(code, &cmd);

                cmd.instr = INSTR_CALL;
                cmd.arg1 = {.type = ARG_IMM, .val = {.imm = 0}};
                en_emit(code, &cmd);

                cmd.instr = INSTR_SUB;
                cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_ESP}};
                cmd.arg2 = {.type = ARG_IMM, .val = {.imm = num_of_args * 4}};
                en_emit(code, &cmd);
                /*
                 *fprintf(stream, "\n        push rsp\n"
                 *                  "        push %d\n"
                 *                  "        sub\n"
                 *                  "        pop rsp\n", num_of_args);
                 */

                cmd.instr = INSTR_PUSH;
                cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_EAX}};
                /*
                 *fprintf(stream,   "        push rax\n");
                 */
                return GEN_NO_ERR;
        }

        cmd.instr = INSTR_PUSH;
        cmd.arg1.type = ARG_MEM;
        cmd.arg1.val = {.mem = {.disp_on = true,               .reg_on = true, 
                                .disp = (uint32_t) offset * 4, .reg = REG_EBP}};
        en_emit(code, &cmd);

        return GEN_NO_ERR;
}

static int
gen_embedded(tree_t *ast, int *pos, code_t *code)
{
        assert(ast);
        assert(pos);
        assert(code);

        int tmp = 0;
        int offset = 0;
        switch(ast->nodes[*pos].data.val.em) {
                case EMBED_PRINT:
                case EMBED_SIN:
                case EMBED_COS:
                case EMBED_SQRT:
                case EMBED_SCAN:
                default:
                        assert(0 && "Invalid embedded function code.");
                        break;
        }

        return GEN_NO_ERR;
}

static int
gen_parameter(tree_t *ast, int *pos, code_t *code)
{
        assert(ast);
        assert(pos);
        assert(code);

        if (ast->nodes[*pos].data.type != TOK_POISON) {
                uint32_t offset = RAM_OFFSET++;
                sym_insert(ast->nodes[*pos].data.val.var, 
                           var_stack.data[var_stack.size - 1], offset);

                cmd_token_t cmd = {.instr = INSTR_POP, .arg1 = {.type = ARG_MEM, 
                        .val = {.mem = {.disp_on = true, .reg_on = true, .disp = offset * 4, .reg = REG_EBP}}}};
                en_emit(code, &cmd);
                // fprintf(stream, "        pop [rbx + %d]\n", offset);
                //
                gen_parameter(ast, &ast->nodes[*pos].left, code);

                cmd.instr = INSTR_PUSH;
                en_emit(code, &cmd);
                // fprintf(stream, "        push [rbx + %d]\n", offset);
        }
        
        return GEN_NO_ERR;
}

int
generator(char *ast_buffer, code_t *code)
{
        assert(ast_buffer);
        assert(code);

        stack_ctor(&var_stack, 10, STK_VAR_INFO(var_stack));
        sym_ctor(100, &func_table);

        tree_t ast {};
        tree_ctor(&ast, 200);

        iden_t id {};
        id_alloc(&id, 200);

        gen_restore(&ast, ast_buffer, &ast.root, &id);
        include_graph(tree_graph_dump(&ast, VAR_INFO(ast)));

        // fprintf(asm_stream, "jmp :.main\n\n");
        gen_write_asm(&ast, &ast.root, code);

        id_free(&id);
        sym_dtor(&func_table);
        stack_dtor(&var_stack);
        tree_dtor(&ast);

        return GEN_NO_ERR;
}

int
gen_write_asm(tree_t *ast, int *pos, code_t *code)
{
        assert(ast);
        assert(pos);
        assert(code);

        cmd_token_t cmd = {};

        switch (ast->nodes[*pos].data.type) {
                case TOK_FUNC:
                        RAM_OFFSET = 0;

                        cmd.instr = INSTR_MOV;
                        cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_ESP}};
                        cmd.arg2 = {.type = ARG_REG, .val = {.reg = REG_EBP}};
                        en_emit(code, &cmd);

                        gen_write_asm(ast, &ast->nodes[*pos].right, code);

                        if (strcmp("main", ast->nodes[*pos].data.val.var) == 0) {
                                cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_EBP}};
                                cmd.arg2 = {.type = ARG_REG, .val = {.reg = REG_ESP}};
                                en_emit(code, &cmd);

                                cmd.instr = INSTR_MOV;
                                cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_EAX}};
                                cmd.arg2 = {.type = ARG_IMM, .val = {.imm = 1}};
                                en_emit(code, &cmd);

                                cmd.arg1.val.reg = REG_EBX;
                                cmd.arg2.val.imm = 0;
                                en_emit(code, &cmd);

                                cmd.instr = INSTR_INT;
                                cmd.arg1 = {.type = ARG_IMM, .val = {.imm = 0x80}};
                                en_emit(code, &cmd);
                        }
                        gen_write_asm(ast, &ast->nodes[*pos].left, code);
                        break;
                case TOK_BLOCK:
                        sym_new_table(&var_stack);
                        if (ast->nodes[ast->nodes[*pos].left].data.type != TOK_POISON) {
                                cmd.instr = INSTR_PUSH;
                                cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_ECX}};
                                en_emit(code, &cmd);

                                cmd.arg1.val.reg = REG_EDX;
                                en_emit(code, &cmd);

                                gen_parameter(ast, &ast->nodes[*pos].left, code);

                                cmd.instr = INSTR_PUSH;
                                en_emit(code, &cmd);

                                cmd.arg1.val.reg  = REG_ECX;
                                en_emit(code, &cmd);
                        }
                        gen_write_asm(ast, &ast->nodes[*pos].right, code);
                        sym_remove_table(&var_stack);
                        break;
                case TOK_EMBED:
                        gen_embedded(ast, pos, code);
                        break;
                case TOK_EXP:
                        gen_write_asm(ast, &ast->nodes[*pos].right, code);
                        gen_write_asm(ast, &ast->nodes[*pos].left, code);
                        break;
                case TOK_POISON:
                        break;
                case TOK_DECL:
                        break;
                case TOK_VAR:
                        if (gen_identifier(ast, pos, code) != GEN_NO_ERR)
                                return GEN_UNDECL;
                        break;
                case TOK_NUM:
                        cmd.instr = INSTR_PUSH;
                        cmd.arg1 = {.type = ARG_IMM, .val = {.imm = ast->nodes[*pos].data.val.num}};
                        en_emit(code, &cmd);
                        break;
                case TOK_OP:
                        gen_op(ast, pos, code);
                        break;
                case TOK_KW:
                        gen_kw(ast, pos, code);
                        break;
                case TOK_PUNC:
                        assert(0 && "Punctuator encountered.");
                        break;
                case TOK_EOF:
                        break;
                default:
                        assert(0 && "Invalid type encountered.");
                        break;
        }

        return GEN_NO_ERR;
}

void
gen_op(tree_t *ast, int *pos, code_t *code)
{
        assert(ast);
        assert(pos);
        assert(code);

        if (ast->nodes[*pos].data.val.op != OP_ASSIGN)
                gen_write_asm(ast, &ast->nodes[*pos].left, code);

        gen_write_asm(ast, &ast->nodes[*pos].right, code);

        cmd_token_t cmd = {.instr = INSTR_POP, .arg1 = {.type = ARG_REG}};

        cmd.arg1.val.reg = REG_EAX;
        en_emit(code, &cmd);
        cmd.arg1.val.reg = REG_EBX;
        en_emit(code, &cmd);

        cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_EAX}};
        cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_EBX}};

        switch(ast->nodes[*pos].data.val.op) {
                case OP_ADD:
                        cmd.instr = INSTR_ADD;
                        break;
                case OP_SUB:
                        cmd.instr = INSTR_SUB;
                        break;
                // TODO: handle MUL/DIV.
                case OP_MUL:
                        cmd.instr = INSTR_MUL;
                        break;
                case OP_DIV:
                        cmd.instr = INSTR_DIV;
                        break;
                case OP_ASSIGN:
                        gen_assign(ast, pos, code);
                        break;
                case OP_EQ:
                        cmd.instr = INSTR_CMP;
                        break;
                case OP_NEQ:
                        break;
                case OP_LEQ:
                        break;
                case OP_GEQ:
                        break;
                case OP_LESSER:
                        break;
                case OP_GREATER:
                        break;
                default:
                        assert(0 && "Invalid operation type.");
                        break;
        }

        cmd.instr = INSTR_PUSH;
        cmd.arg1.type = ARG_REG;
        cmd.arg1.val.reg = REG_EAX;
        en_emit(code, &cmd);
}

void
gen_kw(tree_t *ast, int *pos, code_t *code)
{
        assert(ast);
        assert(pos);
        assert(code);

        cmd_token_t cmd = {};

        int label1 = LABEL_COUNT++;
        int label2 = 0;
        switch(ast->nodes[*pos].data.val.kw) {
                case KW_WHILE:
                        //fprintf(stream, "L%d:\n", label1);
                        //gen_write_asm(ast, &ast->nodes[*pos].right, code);
                        //label2 = LABEL_COUNT++;
                        //fprintf(stream, "        push 0\n"
                                        //"        je :L%d\n", label2);
                        //gen_write_asm(ast, &ast->nodes[*pos].left, code);
                        //fprintf(stream, "        jmp :L%d\n"
                                        //"L%d:\n", label1, label2);
                        //break;
                case KW_IF:
                        //gen_write_asm(ast, &ast->nodes[*pos].right, code);
                        //fprintf(stream, "        push 0\n"
                                        //"        je :L%d\n", label1);
                        //gen_write_asm(ast, &ast->nodes[*pos].left, code);
                        //fprintf(stream, "L%d:\n", label1);
                        //break;
                case KW_ELSE:
                        //gen_write_asm(ast, &ast->nodes[ast->nodes[*pos].right].right, code);
                        //label2 = LABEL_COUNT++;
                        //fprintf(stream, "        push 0\n"
                                        //"        je :L%d\n", label1);
                        //gen_write_asm(ast, &ast->nodes[ast->nodes[*pos].right].left, code);
                        //fprintf(stream, "        jmp :L%d\n", label2);
                        //fprintf(stream, "L%d:\n", label1);
                        //gen_write_asm(ast, &ast->nodes[*pos].left, code);
                        //fprintf(stream, "L%d:\n", label2);
                        //break;
                case KW_RETURN:
                        gen_write_asm(ast, &ast->nodes[*pos].right, code);
                        cmd.instr = INSTR_POP;
                        cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_EAX}};
                        en_emit(code, &cmd);

                        cmd.instr = INSTR_MOV;
                        cmd.arg1 = {.type = ARG_REG, .val = {.reg = REG_ESP}};
                        cmd.arg2 = {.type = ARG_REG, .val = {.reg = REG_EBP}};
                        en_emit(code, &cmd);

                        cmd.instr = INSTR_RET;
                        en_emit(code, &cmd);
                        break;
                default:
                        assert(0 && "Invalid keyword type.");
                        break;
        }
}

void
gen_restore(tree_t *tree, char *buf, int *pos, iden_t *id)
{
        assert(tree);
        assert(buf);
        assert(pos);
        assert(id);

        static char *buffer = buf;
        char *type = nullptr;
        char *val = nullptr;
        int i = 0;

        for ( ; isspace(*buffer); buffer++)
                ;

        if (*buffer == '{') {
                buffer++;
                for ( ; isspace(*buffer); buffer++)
                        ;

                if (*buffer != '\'') {
                        log("Invalid usage.\n");
                        return;
                }

                buffer++;
                i += get_delim_buf(&type, '\'', buffer) + 1;
                buffer += i;

                for ( ; isspace(*buffer); buffer++)
                        ;

                if (*buffer != '\'') {
                        log("Invalid usage.\n");
                        return;
                }

                buffer++;
                i += get_delim_buf(&val, '\'', buffer);
                buffer += i - 1;

                tree_data_t data {};
                data.type = (tok_type_t) atoi(type);
                switch (data.type) {
                        case TOK_POISON:
                                break;
                        case TOK_FUNC:
                                data.val.var = val;
                                sym_insert(val, &func_table, 0);
                                break;
                        case TOK_BLOCK:
                                break;
                        case TOK_EMBED:
                                data.val.em = (embed_t) atoi(val);
                                break;
                        case TOK_EXP:
                                break;
                        case TOK_DECL:
                                break;
                        case TOK_VAR:
                                data.val.var = val;
                                break;
                        case TOK_NUM:
                                data.val.num = atof(val);
                                break;
                        case TOK_OP:
                                data.val.op = (op_t) atoi(val);
                                break;
                        case TOK_KW:
                                data.val.kw = (kword_t) atoi(val);
                                break;
                        case TOK_PUNC:
                                assert(0 && "Punctuators should not be in AST.\n");
                                break;
                        case TOK_EOF:
                                break;
                        default:
                                assert(0 && "Invalid token type.\n");
                                break;
                }

                free(type);
                if (data.type != TOK_VAR && data.type != TOK_FUNC) {
                        free(val);
                } else {
                        id->ptrs[id->size] = val;
                        id->size++;
                        if (id->cap < id->size + 1) {
                                id_alloc(id, id->cap * 2);
                        }
                }

                node_insert(tree, pos, data);

                gen_restore(tree, buffer, &tree->nodes[*pos].left, id);
                gen_restore(tree, buffer, &tree->nodes[*pos].right, id);

                for ( ; isspace(*buffer); buffer++)
                        ;

                if (*buffer == '}') {
                        buffer++;
                        return;
                }
        }
}

