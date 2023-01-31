#ifndef TYPES_H
#define TYPES_H

// Enum of available puntuators.
enum punc_t {
        PUNC_COLON = 1, 
        PUNC_COMMA = 2,
        PUNC_DOT   = 3,
        PUNC_OPBRACE = 4,
        PUNC_CLBRACE = 5,
};

// Enum of available operations.
enum op_t {
        OP_ADD    = 1,
        OP_SUB    = 2,
        OP_MUL    = 3,
        OP_DIV    = 4,
        OP_ASSIGN = 5,
};

// Enum of available objects that can be differentiated.
enum tok_type_t {
        TOK_POISON = 0, // Empty token.
        TOK_DECL   = 1, // Declaration operator.
        TOK_VAR    = 2, // Variable.
        TOK_NUM    = 3, // Number.
        TOK_OP     = 4, // Operation.
        TOK_KWORD  = 5, // Keyword.
        TOK_PUNC   = 6, // Punctuator.
        TOK_EOF    = 7, // End of file.
        TOK_EXP = 666,
};

// Union with object
union value_t {
        punc_t punc;           // Punctuator.
        char *var;             // Variable. Later extend this.
        double num;            // Number value.
        op_t op;               // Operation.
        // Later will be extended.
};

#endif // TYPES_H

