#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// Enum of available puntuators.
#define DEF_PUNC(NAME, SIGN) PUNC_##NAME,
enum punc_t {
        #include "punctuators.inc"
};
#undef DEF_PUNC

// Enum of available operations.
#define DEF_OP(NAME, SIGN) OP_##NAME,
enum op_t {
        #include "operations.inc"
};
#undef DEF_OP

// Enum of available keywords.
#define DEF_KW(NAME, SIGN) KW_##NAME,
enum kword_t {
        #include "keywords.inc"
};
#undef DEF_KW

// Enum of available embedded functions.
#define DEF_EMBED(NAME, SIGN) EMBED_##NAME,
enum embed_t {
        #include "embedded.inc"
};
#undef DEF_EMBED

// Enum of available objects that can be differentiated.
enum tok_type_t {
        TOK_POISON = 0,   // Empty token.
        TOK_DECL   = 1,   // Declaration operator.
        TOK_VAR    = 2,   // Variable.
        TOK_NUM    = 3,   // Number.
        TOK_OP     = 4,   // Operation.
        TOK_KW     = 5,   // Keyword.
        TOK_PUNC   = 6,   // Punctuator.
        TOK_EMBED  = 7,   // Embedded function.
        TOK_EOF    = 8,   // End of file.
        TOK_EXP    = 666, // Expression token.
        TOK_BLOCK  = 777, // Block token.
        TOK_FUNC   = 888, // Function token.
};

// Union with object
union value_t {
        punc_t punc;           // Punctuator.
        char *var;             // Variable. Later extend this.
        uint32_t num;            // Number value.
        op_t op;               // Operation.
        kword_t kw;            // Keyword.
        embed_t em;            // Embedded function.
};

#endif // TYPES_H

