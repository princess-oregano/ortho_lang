#include <stdio.h>
#include <stdlib.h>
#include "args.h"
#include "front_end/lexer.h"
#include "front_end/parser.h"
#include "tree/tree.h"
#include "file.h"
#include "log.h"

int
main(int argc, char *argv[])
{
        open_log("log.html");
        file_t file {};

        params_t params {};
        process_args(argc, argv, &params);

        get_file(params.filename.src_code, &file, "r");

        char *buffer = nullptr;
        read_file(&buffer, &file);

        tok_arr_t tok_arr {};
        lexer(buffer, &tok_arr);

        tree_t ast {};
        tree_ctor(&ast, 200);
        parser(&tok_arr, &ast);

        free(tok_arr.tok);
        clean_args(&params);
        tree_dtor(&ast);
        close_log();

        return 0;
}

