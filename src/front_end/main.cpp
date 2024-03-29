#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../args.h"
#include "lexer.h"
#include "parser.h"
#include "../tree/tree.h"
#include "../file.h"
#include "../log.h"

int
main(int argc, char *argv[])
{
        open_log("log.html", "w");

        // Process cmd-line arguments.
        params_t params {};
        process_args(argc, argv, &params);

        // Read source file.
        file_t src_file {};
        char *buffer = nullptr;
        get_file(params.filename.src_code, &src_file, "r");
        read_file(&buffer, &src_file);

        // Make tokens of source code. Also save all pointers to identifiers.
        iden_t id {};
        tok_arr_t tok_arr {};
        lexer(buffer, &tok_arr, &id);

        // Build AST from tokens.
        tree_t ast {};
        tree_ctor(&ast, 300);
        parser(&tok_arr, &ast);

        // Write AST to file.
        file_t ast_file {};
        get_file(params.filename.ast_code, &ast_file, "w");
        par_write_ast(&ast, ast_file.stream);
        fclose(ast_file.stream);

        // Clean-up.
        id_free(&id);
        free(tok_arr.tok);
        clean_args(&params);
        tree_dtor(&ast);
        close_log();

        return 0;
}

