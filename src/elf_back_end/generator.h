#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include "../tree/tree.h"
#include "../tree/tree_dump.h"
#include "../identifiers.h"
#include "encode.h"

enum gen_err_t {
        GEN_NO_ERR = 0,
        GEN_VAR_DECL = 1,
        GEN_UNDECL = 2,
};

/// Generates asm code from AST.
int
generator(char *ast_buffer, code_t *code);

/// Restores node from description from the buffer.
void
gen_restore(tree_t *tree, char *buf, int *pos, iden_t *id);

/// Handles identifiers.
int
gen_identifier(tree_t *ast, int *pos, code_t *code);

/// Handles operations.
void
gen_op(tree_t *ast, int *pos, code_t *code);

/// Handles keywords.
void
gen_kw(tree_t *ast, int *pos, code_t *code);

/// Generates ASM code from AST.
int
gen_write_asm(tree_t *ast, int *pos, code_t *code);

#endif // GENERATOR_H

