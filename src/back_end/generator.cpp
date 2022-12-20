#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "generator.h"
#include "../tree/tree.h"
#include "../tree/tree_dump.h"
#include "../front_end/types.h"
#include "../file.h"
#include "../log.h"

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

// Generates ASM code from AST.
static void
gen_write_asm(tree_t *ast, FILE *stream)
{
        
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
        char ch = '\0';
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
                i += get_delim_buf(&val, '\'', buffer) + 1;
                buffer += i;

                tree_data_t data {};
                data.type = (tok_type_t) atoi(type);
                switch (data.type) {
                        case TOK_POISON:
                                assert(0 && "Error: Poison node encountered.\n");
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

                if (ch == '}') {
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

        gen_restore(&ast, ast_buffer, &ast.root);
        include_graph(tree_graph_dump(&ast, VAR_INFO(ast)));

        gen_write_asm(&ast, asm_stream);
        
        tree_dtor(&ast);
        return GEN_NO_ERR;
}

