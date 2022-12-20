#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>

enum gen_err_t {
        GEN_NO_ERR = 0,
};

int
generator(char *ast_buffer, FILE *asm_stream);

#endif // GENERATOR_H

