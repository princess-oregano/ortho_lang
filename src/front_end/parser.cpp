#include <stdio.h>
#include <assert.h>
#include "parser.h"
#include "../tree/tree.h"
#include "../tree/tree_dump.h"
#include "../log.h"

#define TOK arr->tok[*t_count]
#define IS_OP(NAME) (TOK.type == TOK_OP && TOK.val.op == OP_##NAME)
#define IS_PUNC(NAME) (TOK.type == TOK_PUNC && TOK.val.punc == (PUNC_##NAME))
#define INSERT node_insert(ast, pos, {.type = TOK.type, .val = TOK.val})

static int
general(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
primary_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
brace_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
mul_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
add_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);

static int
primary_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        if (TOK.type != TOK_NUM) {
                log("Error: Expected number.\n");
                return PAR_NUMBER;
        }

        INSERT;
        (*t_count)++;

        return PAR_NO_ERR;
}

static int
brace_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        if (IS_PUNC(OPBRACE)) {
                (*t_count)++;
                add_expr(arr, t_count, ast, pos);
                if (!IS_PUNC(CLBRACE)) {
                        log("Error: Expected closing brace.\n");
                        return PAR_BRACE;
                }
                (*t_count)++;
        } else {
                primary_expr(arr, t_count, ast, pos);
        }

        return PAR_NO_ERR;
}

static int
mul_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        brace_expr(arr, t_count, ast, pos);
        while (IS_OP(DIV) || IS_OP(MUL)) {
                int tmp = *pos; 
                INSERT;
                node_bound(&ast->nodes[*pos].left, tmp);
                (*t_count)++;
                brace_expr(arr, t_count, ast, &ast->nodes[*pos].right);
        }

        return PAR_NO_ERR;
}

static int
add_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);
        
        mul_expr(arr, t_count, ast, pos);
        while (IS_OP(ADD) || IS_OP(SUB)) {
                int tmp = *pos; 
                INSERT;
                node_bound(&ast->nodes[*pos].left, tmp);
                (*t_count)++;
                mul_expr(arr, t_count, ast, &ast->nodes[*pos].right);
        }

        return PAR_NO_ERR;
}

static int
general(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        add_expr(arr, t_count, ast, pos);
        if (!IS_PUNC(COLON)) {
                log("Error: Expected end of statement ';'.\n");
                return -1;
        }

        return PAR_NO_ERR;
}

int
parser(tok_arr_t *arr, tree_t *ast)
{
        int t_count = 0;
        general(arr, &t_count, ast, &ast->root); 
        include_graph(tree_graph_dump(ast, VAR_INFO(ast)));

        return PAR_NO_ERR;
}

static void
print_node(tree_t *tree, int pos, FILE *stream, int level)
{
        log("Entering %s.\n", __PRETTY_FUNCTION__);

        assert(tree);
        assert(stream);

        if (pos < 0)
                return;

        for (int i = 0; i < level; i++)
                fprintf(stream, "        ");

        level++;

        fprintf(stream, "{\'%d\'", tree->nodes[pos].data.type);

        switch (tree->nodes[pos].data.type) {
                case TOK_POISON:
                        assert(0 && "Error: Poison node encountered.\n");
                        break;
                case TOK_VAR:
                        fprintf(stream, " \'%s\'", tree->nodes[pos].data.val.var);
                        break;
                case TOK_NUM:
                        fprintf(stream, " \'%lg\'", tree->nodes[pos].data.val.num);
                        break;
                case TOK_OP:
                        fprintf(stream, " \'%d\'", tree->nodes[pos].data.val.op);
                        break;
                case TOK_KWORD:
                        assert(0 && "Keywords are not supported yet.\n");
                        break;
                case TOK_PUNC:
                        assert(0 && "Punctuators should not be in AST.\n");
                default: 
                        assert(0 && "Invalid token type.\n");
                        break;
        }

        if (tree->nodes[pos].left  == -1 &&
            tree->nodes[pos].right == -1) {
                fprintf(stream, "}\n");
        } else {
                fprintf(stream, "\n");
                print_node(tree, tree->nodes[pos].left,  stream, level);
                print_node(tree, tree->nodes[pos].right, stream, level);
                for (int i = 0; i < level - 1; i++)
                        fprintf(stream, "        ");
                fprintf(stream, "}\n");
        }

        log("Exiting %s.\n", __PRETTY_FUNCTION__);
}

int
par_write_ast(tree_t *ast, FILE *stream)
{
        setvbuf(stream, nullptr, _IONBF, 0);

        print_node(ast, ast->root, stream, 0);

        fclose(stream);
       
        return PAR_NO_ERR;
}
