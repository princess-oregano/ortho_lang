#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "lexer.h"
#include "../log.h"
#include "../types.h"

static int
get_word(char **word, char *start, size_t n)
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

#define DEF_OP(NAME, SIGN) if (strcmp(str, SIGN) == 0) { \
                                token->type = TOK_OP;    \
                                token->val.op = OP_##NAME;  \
                           } else 
#define DEF_KW(NAME, SIGN) if (strcmp(str, SIGN) == 0) { \
                                token->type = TOK_KW;    \
                                token->val.kw = KW_##NAME;  \
                           } else 
#define DEF_PUNC(NAME, SIGN) if (strcmp(str, SIGN) == 0) { \
                                token->type = TOK_PUNC;    \
                                token->val.punc = PUNC_##NAME;  \
                           } else 
#define DEF_EMBED(NAME, SIGN) if (strcmp(str, SIGN) == 0) { \
                                token->type = TOK_EMBED;    \
                                token->val.em = EMBED_##NAME;  \
                           } else 

static int
lex_token(token_t *token, char *str)
{
        char *tmp = nullptr;
        double num = strtod(str, &tmp);
        if (str != tmp) {
                token->type = TOK_NUM;
                token->val.num = num;
                return LEX_NO_ERR;
        }

        if (strcmp(str, "int") == 0) {
                token->type = TOK_DECL;
        } else
        #include "../punctuators.inc"
        #include "../operations.inc"
        #include "../keywords.inc"
        #include "../embedded.inc"
        {
                log("Unknown command: '%s'. Treated as variable.\n", str);
                token->type = TOK_VAR;
                token->val.var = str;
        }

        return LEX_NO_ERR;
}
#undef DEF_OP
#undef DEF_KW
#undef DEF_PUNC
#undef DEF_EMBED

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
lexer(char *buffer, tok_arr_t *arr, iden_t *id)
{
        assert(buffer);
        assert(arr);

        int err = 0;

        if ((err = lex_alloc(arr, 200)) != LEX_NO_ERR)
                return err;
        
        if ((err = id_alloc(id, 200)) != LEX_NO_ERR)
                return err;

        int tok_count = 0;
        char *word = nullptr;
        while (*(buffer + 1)) {
                while (isspace(*buffer))
                        buffer++;

                char *new_buffer = strpbrk(buffer, BREAKSET);

                if (buffer == new_buffer)
                        new_buffer++;

                if (get_word(&word, buffer, (size_t) (new_buffer - buffer)) 
                                                                == LEX_ALLOC) {
                        return LEX_ALLOC;
                }

                lex_token(&arr->tok[tok_count], word);

                if (arr->tok[tok_count].type != TOK_VAR) {
                        free(word);
                } else {
                        id->ptrs[id->size] = word;
                        id->size++;
                        if (id->cap < id->size + 1) {
                                if ((err = id_alloc(id, id->cap * 2)) !=
                                                        LEX_NO_ERR)
                                        return err;
                        }
                }

                tok_count++;

                buffer = new_buffer;

                if (arr->cap < tok_count + 1) {
                        if ((err = lex_alloc(arr, arr->cap * 2)) !=
                                                LEX_NO_ERR)
                                return err;
                }

                while (isspace(*buffer))
                        buffer++;
        }

        arr->tok[tok_count].type = TOK_EOF;

        return LEX_NO_ERR;
}

