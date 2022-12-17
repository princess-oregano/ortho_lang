#ifndef ARGS_H
#define ARGS_H

const char * const AST_EXT = ".ast";
const char * const ASM_EXT = ".asm";

enum arg_err_t {
        ARG_NO_ERR = 0,
        ARG_ALLOC = 1,
        ARG_INV_USG = 2,
};

struct filename_t {
        char *src_code = nullptr;
        char *ast_code = nullptr;
        char *asm_code = nullptr;
};

struct params_t {
        filename_t filename;
};

// Processes command line arguments.
int
process_args(int argc, char *argv[], params_t *params);
// Cleans all allocated space for filenames.
void
clean_args(params_t *params);

#endif // ARGS_H

