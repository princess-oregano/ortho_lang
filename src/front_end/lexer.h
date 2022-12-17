#ifndef LEXER_H
#define LEXER_H

#include "types.h"

const char* const BREAKSET = " ;+-*/()\n\t\0";

enum lex_err {
        LEX_NO_ERR  = 0,
        LEX_ALLOC   = 1,
        LEX_BAD_CAP = 2,
        LEX_INV_USG  = 3,
};

struct token_t {
        tok_type_t type = TOK_POISON;
        value_t val = {};
};

struct tok_arr_t {
        token_t *tok = nullptr;
        int cap = 0;
};

// Builds an array of tokens.
int
lexer(char *buffer, tok_arr_t *arr);

#endif // LEXER_H

