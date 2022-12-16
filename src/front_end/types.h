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
        OP_ADD = 1,
        OP_SUB = 2,
        OP_MUL = 3,
        OP_DIV = 4,
};

// Enum of available objects that can be differentiated.
enum tok_type_t {
        TOK_POISON = 0, // Empty token.
        TOK_VAR    = 1, // Variable.
        TOK_NUM    = 2, // Number.
        TOK_OP     = 3, // Operation.
        TOK_KWORD  = 5, // Keyword.
        TOK_PUNC   = 6, // Punctuator.
};

// Union with object
union value_t {
        punc_t punc;             // Punctuator.
        char *var;             // Variable. Later extend this.
        double num;            // Number value.
        op_t op;               // Operation.
        // Later will be extended.
};

#endif // TYPES_H

