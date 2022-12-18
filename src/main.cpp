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
        file_t src_file {};

        params_t params {};
        process_args(argc, argv, &params);

        get_file(params.filename.src_code, &src_file, "r");

        char *buffer = nullptr;
        read_file(&buffer, &src_file);

        tok_arr_t tok_arr {};
        lexer(buffer, &tok_arr);

        tree_t ast {};
        tree_ctor(&ast, 200);
        parser(&tok_arr, &ast);

        file_t ast_file {};
        get_file(params.filename.ast_code, &ast_file, "w");
        par_write_ast(&ast, ast_file.stream);

        free(tok_arr.tok);
        clean_args(&params);
        tree_dtor(&ast);
        close_log();

        return 0;
}

