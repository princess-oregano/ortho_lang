#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "generator.h"
#include "table.h"
#include "../tree/tree.h"
#include "../tree/tree_dump.h"
#include "../front_end/types.h"
#include "../identifiers.h"
#include "../file.h"
#include "../log.h"

static int RAM_OFFSET = 0;
static table_t table {};
static int
gen_write_asm(tree_t *ast, int *pos, FILE *stream);

static int
get_delim_buf(char **line, int delim, char *buffer)
{
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
gen_op(op_t op, FILE *stream)
{
        switch(op) {
                case OP_ADD:
                        fprintf(stream, "add\n");
                        break;
                case OP_SUB:
                        fprintf(stream, "sub\n");
                        break;
                case OP_MUL:
                        fprintf(stream, "mul\n");
                        break;
                case OP_DIV:
                        fprintf(stream, "div\n");
                        break;
                case OP_ASSIGN:
                default:
                        assert(0 && "Invalid operation type.");
        }
}

static int
gen_declare(tree_t *ast, int *pos)
{
        int tmp = *pos;
        *pos = ast->nodes[tmp].right;

        if (sym_find(ast->nodes[*pos].data.val.var, &table) != -1) {
                log("Error: Variable has been already declared.\n");
                return GEN_VAR_DECL;
        }

        sym_insert(ast->nodes[*pos].data.val.var, &table, RAM_OFFSET);
        RAM_OFFSET++;

        return GEN_NO_ERR;
}

static void
gen_assign(tree_t *ast, int *pos, FILE *stream)
{
        gen_write_asm(ast, &ast->nodes[*pos].right, stream);
        int tmp = ast->nodes[*pos].left;

        int table_num = sym_find(ast->nodes[tmp].data.val.var, &table);
        int offset = table.vars[table_num].ram;

        fprintf(stream, "pop [rax + %d]\n", offset);
}

static int
gen_variable(tree_t *ast, int *pos, FILE *stream)
{
        int table_num = sym_find(ast->nodes[*pos].data.val.var, &table);
        if (table_num == -1) {
                log("Error: Undeclared variable.\n");
                return GEN_UNDECL;
        }
        int offset = table.vars[table_num].ram;
        fprintf(stream, "push [rax + %d]\n", offset);

        return GEN_NO_ERR;
}

// Generates ASM code from AST.
static int
gen_write_asm(tree_t *ast, int *pos, FILE *stream)
{
        switch (ast->nodes[*pos].data.type) {
                case TOK_EXP:
                        gen_write_asm(ast, &ast->nodes[*pos].right, stream);
                        gen_write_asm(ast, &ast->nodes[*pos].left, stream);
                        break;
                case TOK_POISON:
                        break;
                case TOK_DECL:
                        if (gen_declare(ast, pos) != GEN_NO_ERR) 
                                return GEN_VAR_DECL;
                        break;
                case TOK_VAR:
                        if (gen_variable(ast, pos, stream) == GEN_UNDECL)
                                return GEN_UNDECL;
                        break;
                case TOK_NUM:
                        fprintf(stream, "push %lg\n", ast->nodes[*pos].data.val.num);
                        break;
                case TOK_OP:
                        if (ast->nodes[*pos].data.val.op == OP_ASSIGN) {
                                gen_assign(ast, pos, stream);
                        } else {
                        gen_write_asm(ast, &ast->nodes[*pos].left, stream);
                        gen_write_asm(ast, &ast->nodes[*pos].right, stream);
                        gen_op(ast->nodes[*pos].data.val.op, stream);
                        }
                        break;
                case TOK_KWORD:
                        assert(0 && "Keywords are not supported yet.");
                        break;
                case TOK_PUNC:
                        assert(0 && "Punctuator encountered.");
                        break;
                case TOK_EOF:
                        fprintf(stream, "out\nhlt\n\n");
                        break;
                default:
                        assert(0 && "Invalid type encountered.");
                        break;
        }

        return GEN_NO_ERR;
}

// Restores node from description from the buffer.
static void
gen_restore(tree_t *tree, char *buf, int *pos, iden_t *id)
{
        assert(tree);
        assert(buf);

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
                        case TOK_KWORD:
                                assert(0 && "Keywords are not supported yet.\n");
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
                if (data.type != TOK_VAR) {
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

int
generator(char *ast_buffer, FILE *asm_stream)
{
        tree_t ast {};
        tree_ctor(&ast, 200);
        sym_ctor(100, &table);

        iden_t id {};
        id_alloc(&id, 200);

        gen_restore(&ast, ast_buffer, &ast.root, &id);
        include_graph(tree_graph_dump(&ast, VAR_INFO(ast)));

        setvbuf(asm_stream, nullptr, _IONBF, 0);
        gen_write_asm(&ast, &ast.root, asm_stream);
        
        id_free(&id);
        sym_dtor(&table);
        tree_dtor(&ast);
        return GEN_NO_ERR;
}

