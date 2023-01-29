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
gen_declare(tree_t *ast, int *pos, FILE *stream)
{
        int tmp = *pos;
        *pos = ast->nodes[tmp].right;

        if (sym_find(ast->nodes[*pos].data.val.var, &table) != -1) {
                log("Error: Variable has been already declared.\n");
                return GEN_VAR_DECL;
        }

        sym_insert(ast->nodes[*pos].data.val.var, &table, RAM_OFFSET);
        RAM_OFFSET++;

        gen_write_asm(ast, &ast->nodes[tmp].right, stream);

        return GEN_NO_ERR;
}

static void
gen_assign(tree_t *ast, int *pos, FILE *stream)
{
        gen_write_asm(ast, &ast->nodes[*pos].right, stream);
        int tmp = ast->nodes[*pos].left;

        int table_num = sym_find(ast->nodes[tmp].data.val.var, &table);
        int offset = table.vars[table_num].ram;

        fprintf(stream, "pop [rax + %d]", offset);

        gen_write_asm(ast, &ast->nodes[tmp].left, stream);
}

// Generates ASM code from AST.
static int
gen_write_asm(tree_t *ast, int *pos, FILE *stream)
{
        switch (ast->nodes[*pos].data.type) {
                case TOK_POISON:
                        assert(0 && "Poison node encountered.");
                        break;
                case TOK_DECL:
                        gen_declare(ast, pos, stream);
                        break;
                case TOK_VAR: {
                        int table_num = sym_find(ast->nodes[*pos].data.val.var, &table);
                        if (table_num == -1) {
                                log("Error: Undeclared variable.\n");
                                return GEN_UNDECL;
                        }
                        int offset = table.vars[table_num].ram;
                        fprintf(stream, "push [rax + %d]\n", offset);
                        break;
                }
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
                default:
                        assert(0 && "Invalid type encountered.");
                        break;
        }

        return GEN_NO_ERR;
}

// Restores node from description from the buffer.
static void
gen_restore(tree_t *tree, char *buf, int *pos)
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
                                assert(0 && "Error: Poison node encountered.\n");
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
                        default: 
                                assert(0 && "Invalid token type.\n");
                                break;
                }

                free(type);
                free(val);

                node_insert(tree, pos, data);

                gen_restore(tree, buffer, &tree->nodes[*pos].left);
                gen_restore(tree, buffer, &tree->nodes[*pos].right);

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

        gen_restore(&ast, ast_buffer, &ast.root);
        include_graph(tree_graph_dump(&ast, VAR_INFO(ast)));

        gen_write_asm(&ast, &ast.root, asm_stream);
        fprintf(asm_stream, "out\nhlt\n\n");
        
        sym_dtor(&table);
        tree_dtor(&ast);
        return GEN_NO_ERR;
}

