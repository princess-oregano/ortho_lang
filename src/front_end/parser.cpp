#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "parser.h"
#include "../tree/tree.h"
#include "../tree/tree_dump.h"
#include "../log.h"

#define TOK arr->tok[*t_count]
#define IS_OP(NAME) (TOK.type == TOK_OP && TOK.val.op == OP_##NAME)
#define IS_PUNC(NAME) (TOK.type == TOK_PUNC && TOK.val.punc == (PUNC_##NAME))
#define IS_KW(NAME) (TOK.type == TOK_KW && TOK.val.kw == (KW_##NAME))
#define INSERT node_insert(ast, pos, {.type = TOK.type, .val = TOK.val})
#define INS_EXP node_insert(ast, pos, {.type = TOK_EXP, .val = {}})
#define INS_BLOCK node_insert(ast, pos, {.type = TOK_BLOCK, .val = {}})
#define INS_POISON node_insert(ast, pos, {.type = TOK_POISON, .val = {}})

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
assign_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
relational_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
expression(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
declaration(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
iteration(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
selection(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
statement(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
block_item(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);
static int
compound_statement(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos);

static int
primary_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        if (TOK.type != TOK_NUM && TOK.type != TOK_VAR) {
                log("Error: Expected number or variable.\n");
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

        if (IS_PUNC(OPROUND)) {
                (*t_count)++;
                add_expr(arr, t_count, ast, pos);
                if (!IS_PUNC(CLROUND)) {
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
assign_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        (*t_count)++;
        if (IS_OP(ASSIGN)) {
                INSERT;

                (*t_count)--;
                node_insert(ast, &ast->nodes[*pos].left, {.type = TOK.type, .val = TOK.val});
                (*t_count) += 2;

                relational_expr(arr, t_count, ast, &ast->nodes[*pos].right);
        } else {
                (*t_count)--;
                relational_expr(arr, t_count, ast, pos);
        }

        return PAR_NO_ERR;
}

static int
relational_expr(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        add_expr(arr, t_count, ast, pos);
        if (IS_OP(EQ) || IS_OP(NEQ) || IS_OP(LEQ) || IS_OP(GEQ)
                                    || IS_OP(LESSER) || IS_OP(GREATER)) {
                int tmp = *pos;
                INSERT;
                node_bound(&ast->nodes[*pos].left, tmp);
                (*t_count)++;
                add_expr(arr, t_count, ast, &ast->nodes[*pos].right);
        }

        return PAR_NO_ERR;
}

static int
expression(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        assign_expr(arr, t_count, ast, pos);
        if (!IS_PUNC(COLON)) {
                log("Error: Expected end of statement ';'.\n");
                return PAR_EXP_COLON;
        }
        (*t_count)++;

        return PAR_NO_ERR;
}

static int
declaration(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        if (TOK.type != TOK_DECL) {
                log("Error: Expected declarator.\n");
                return PAR_EXP_DECL;
        }
        INSERT;
        (*t_count)++;

        if (TOK.type != TOK_VAR) {
                log("Error: Expected variable.\n");
                return PAR_EXP_VAR;
        }
        primary_expr(arr, t_count, ast, &ast->nodes[*pos].right);
        node_insert(ast, &ast->nodes[*pos].left, {.type = TOK_POISON, .val = {}});

        if (!IS_PUNC(COLON)) {
                log("Error: Expected end of statement ';'.\n");
                return PAR_EXP_COLON;
        }
        (*t_count)++;

        return PAR_NO_ERR;
}

static int
iteration(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        if (!IS_KW(WHILE)) {
                log("Error: Expected keyword 'while'.\n");
                return PAR_EXP_EXPR;
        } 
        INSERT;
        (*t_count)++;

        if (!IS_PUNC(OPROUND)) {
                log("Error: Expected expression.\n");         
                return PAR_EXP_EXPR;
        }
        (*t_count)++;

        relational_expr(arr, t_count, ast, &ast->nodes[*pos].right);
        if (!IS_PUNC(CLROUND)) {
                log("Error: Expected expression.\n");         
                return PAR_EXP_EXPR;
        }
        (*t_count)++;
        compound_statement(arr, t_count, ast, &ast->nodes[*pos].left);

        return PAR_NO_ERR;
}

static int
selection(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        if (!IS_KW(IF)) {
                log("Error: Expected keyword 'if'.\n");
                return PAR_EXP_EXPR;
        } 
        INSERT;
        (*t_count)++;

        if (!IS_PUNC(OPROUND)) {
                log("Error: Expected expression.\n");         
                return PAR_EXP_EXPR;
        }
        (*t_count)++;

        relational_expr(arr, t_count, ast, &ast->nodes[*pos].right);
        if (!IS_PUNC(CLROUND)) {
                log("Error: Expected expression.\n");         
                return PAR_EXP_EXPR;
        }
        (*t_count)++;
        compound_statement(arr, t_count, ast, &ast->nodes[*pos].left);

        if (IS_KW(ELSE)) {
                int tmp = *pos;
                INSERT;
                node_bound(&ast->nodes[*pos].right, tmp);
                (*t_count)++;
                compound_statement(arr, t_count, ast, &ast->nodes[*pos].left);
        }

        return PAR_NO_ERR;
}

static int
statement(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        if (IS_PUNC(CLFIG)) {
                compound_statement(arr, t_count, ast, pos);
        } else if (IS_KW(IF)) {
                selection(arr, t_count, ast, pos);
        } else if (IS_KW(WHILE)) {
                iteration(arr, t_count, ast, pos);
        } else {
                expression(arr, t_count, ast, pos);
        }

        return PAR_NO_ERR;
}

static int
block_item(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        INS_EXP;
        int tmp = *pos;
        pos = &ast->nodes[*pos].right;

        if (TOK.type == TOK_DECL) {
                declaration(arr, t_count, ast, pos);
        } else {
                statement(arr, t_count, ast, pos);
        }

        pos = &ast->nodes[tmp].left;
        INS_POISON;

        return PAR_NO_ERR;
}

static int
compound_statement(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        INS_BLOCK;
        int tmp = *pos;
        pos = &ast->nodes[*pos].right;

        if (!IS_PUNC(OPFIG)) {
                log("Error: Expected '{'.\n");
                return PAR_BRACE;
        }
        (*t_count)++;

        while (!IS_PUNC(CLFIG)) {
                block_item(arr, t_count, ast, pos);

                include_graph(tree_graph_dump(ast, VAR_INFO(ast)));
                pos = &ast->nodes[*pos].left;
        }
        (*t_count)++;

        pos = &ast->nodes[tmp].left;
        INS_POISON;

        return PAR_NO_ERR;
}

static int
general(tok_arr_t *arr, int *t_count, tree_t *ast, int *pos)
{
        assert(arr);
        assert(t_count);
        assert(ast);
        assert(*t_count < arr->cap);

        while (TOK.type != TOK_EOF) {
                compound_statement(arr, t_count, ast, pos);

                include_graph(tree_graph_dump(ast, VAR_INFO(ast)));
                pos = &ast->nodes[*pos].left;
        }
        INSERT;
        (*t_count)++;

        return PAR_NO_ERR;
}

int
parser(tok_arr_t *arr, tree_t *ast)
{
        assert(arr);
        assert(ast);

        int t_count = 0;
        general(arr, &t_count, ast, &ast->root);
        include_graph(tree_graph_dump(ast, VAR_INFO(ast)));

        return PAR_NO_ERR;
}

static void
print_node(tree_t *tree, int pos, FILE *stream, int level)
{
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
                        fprintf(stream, " \'VOID\'");
                        break;
                case TOK_BLOCK:
                        fprintf(stream, " \'BLOCK\'");
                        break;
                case TOK_EXP:
                        fprintf(stream, " \'EXP\'");
                        break;
                case TOK_DECL:
                        fprintf(stream, " \'DECL\'");
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
                case TOK_KW:
                        fprintf(stream, " \'%d\'", tree->nodes[pos].data.val.kw);
                        break;
                case TOK_PUNC:
                        assert(0 && "Punctuators should not be in AST.\n");
                        break;
                case TOK_EOF:
                        fprintf(stream, " \'EOF\'");
                        break;
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
}

int
par_write_ast(tree_t *ast, FILE *stream)
{
        setvbuf(stream, nullptr, _IONBF, 0);

        print_node(ast, ast->root, stream, 0);

        return PAR_NO_ERR;
}
