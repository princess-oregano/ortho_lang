#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"
#include "../tree/tree.h"

enum par_err_t {
        PAR_NO_ERR = 0,
        PAR_EXP_COLON = 1,
        PAR_NUMBER = 2,
        PAR_BRACE = 3,
        PAR_EXP_EXPR = 4,
        PAR_EXP_DECL = 5,
        PAR_EXP_VAR = 6,
};

// Parses token array to AST.
int
parser(tok_arr_t *arr, tree_t *ast);
// Writes AST to file.
int
par_write_ast(tree_t *ast, FILE *stream);

#endif // PARSER_H

