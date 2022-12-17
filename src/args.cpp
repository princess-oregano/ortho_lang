#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "args.h"
#include "log.h"

// Changes file extention.
static int
change_ext(char *src_filename, char **dst_filename, const char *ext)
{
        assert(src_filename);
        assert(ext);

        char *c = strrchr(src_filename, '.');
        size_t c_len = (size_t) (c - src_filename) + strlen(ext);

        char *tmp = (char *) calloc (c_len + 1, sizeof(char));
        if (tmp == nullptr) {
                log("Error: Could't allocate space for filenames.\n");
                return ARG_ALLOC;
        }

        strncpy(tmp, src_filename, c_len - strlen(ext));
        tmp[c_len - strlen(ext)] = '\0';
        strcat(tmp, ext);

        *dst_filename = tmp;

        return ARG_NO_ERR;
}

int
process_args(int argc, char *argv[], params_t *params)
{
        assert(params);

        if (argc != 2) {
                log("Error: Expected one source filename.\n");
                return ARG_INV_USG;
        }

        params->filename.src_code = argv[1];
        change_ext(params->filename.src_code, &params->filename.ast_code, AST_EXT);
        change_ext(params->filename.src_code, &params->filename.asm_code, AST_EXT);

        return ARG_NO_ERR;
}

void
clean_args(params_t *params)
{
        free(params->filename.ast_code);
        free(params->filename.asm_code);
}

