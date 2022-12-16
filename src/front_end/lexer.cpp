#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "lexer.h"
#include "../log.h"

static int
get_word(char **word, char *start, int n)
{
        char *tmp = (char *) calloc(n + 1, sizeof(char));

        if (tmp == nullptr) {
                log("Error: Couldn't allocate memory for tokens.\n");
                return LEX_ALLOC;
        }

        memcpy(tmp, start, n);

        *word = tmp;

        return LEX_NO_ERR;
}

static int
lex_token(token_t *token, char *str)
{
        char *tmp = nullptr;
        int num = strtod(str, &tmp);
        if (str != tmp) {
                token->type = TOK_NUM;
                token->val.num = num;
                return LEX_NO_ERR;
        }

        if (strcmp(str, ";")) {
                token->type = TOK_PUNC;
                token->val.punc = PUNC_COLON;
        }else if (strcmp(str, "+")) {
                token->type = TOK_PUNC;
                token->val.op = OP_ADD;
        }else if (strcmp(str, "-")) {
                token->type = TOK_PUNC;
                token->val.op = OP_SUB;
        }else if (strcmp(str, "*")) {
                token->type = TOK_PUNC;
                token->val.op = OP_MUL;
        }else if (strcmp(str, "/")) {
                token->type = TOK_PUNC;
                token->val.op = OP_DIV;
        } else {
                log("Error: Invalid command '%s'", str);
                return LEX_INV_USG;
        }

        return LEX_NO_ERR;
}

static int
lex_alloc(tok_arr_t *arr, int cap)
{
        if (cap < 0) {
                log("Error: Capacity must be larger or equal to 0.\n");
                return LEX_BAD_CAP;
        }

        token_t *tmp = nullptr;
        tmp = (token_t *) realloc(arr->tok, (size_t) cap * sizeof(token_t));

        if (tmp == nullptr) {
                log("Error: Couldn't allocate memory for tokens.\n");
                return LEX_ALLOC;
        }

        arr->tok = tmp;
        arr->cap = cap;

        return LEX_NO_ERR;
}

int
lexer(char *buffer, int size, tok_arr_t *arr)
{
        assert(buffer);
        assert(arr);

        int err = 0;

        if ((err = lex_alloc(arr, 200)) != LEX_NO_ERR)
                return err;

        int i = 0;
        ssize_t lex_ret = 0;
        int tok_count = 0;
        char *word = nullptr;
        while (*buffer) {
                while (isspace(*buffer))
                        buffer++;

                char *new_buffer = strpbrk(buffer, BREAKSET);

                if (buffer == new_buffer)
                        new_buffer++;

                if (get_word(&word, buffer, 
                                new_buffer - buffer) == LEX_ALLOC) {
                        return LEX_ALLOC;
                }

                lex_token(&arr->tok[tok_count], word);
                tok_count++;

                buffer = new_buffer;

                if (arr->cap < tok_count + 1) {
                        if ((err = lex_alloc(arr, arr->cap * 2)) !=
                                        LEX_NO_ERR)
                                return err;
                }
        }

        return LEX_NO_ERR;
}

