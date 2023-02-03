#ifndef TYPES_H
#define TYPES_H

// Enum of available puntuators.
#define DEF_PUNC(NAME, SIGN) PUNC_##NAME,
enum punc_t {
        #include "../punctuators.inc"
};
#undef DEF_PUNC

// Enum of available operations.
#define DEF_OP(NAME, SIGN) OP_##NAME,
enum op_t {
        #include "../operations.inc"
};
#undef DEF_OP

// Enum of available keywords.
#define DEF_KW(NAME, SIGN) KW_##NAME,
enum kword_t {
        #include "../keywords.inc"
};
#undef DEF_KW

// Enum of available objects that can be differentiated.
enum tok_type_t {
        TOK_POISON = 0,   // Empty token.
        TOK_DECL   = 1,   // Declaration operator.
        TOK_VAR    = 2,   // Variable.
        TOK_NUM    = 3,   // Number.
        TOK_OP     = 4,   // Operation.
        TOK_KW     = 5,   // Keyword.
        TOK_PUNC   = 6,   // Punctuator.
        TOK_EOF    = 7,   // End of file.
        TOK_EXP    = 666, // Expression token.
        TOK_BLOCK  = 777, // Block token.
};

// Union with object
union value_t {
        punc_t punc;           // Punctuator.
        char *var;             // Variable. Later extend this.
        double num;            // Number value.
        op_t op;               // Operation.
        kword_t kw;            // Keyword.
};

#endif // TYPES_H

