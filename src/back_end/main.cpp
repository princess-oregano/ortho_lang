#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../args.h"
#include "generator.h"
#include "../tree/tree.h"
#include "../file.h"
#include "../log.h"

int
main(int argc, char *argv[])
{
        open_log("log.html");

        // Process cmd-line arguments.
        params_t params {};
        process_args(argc, argv, &params);

        // Read source file.
        file_t ast_file {};
        char *buffer = nullptr;
        get_file(params.filename.ast_code, &ast_file, "r");
        read_file(&buffer, &ast_file);
        fclose(ast_file.stream);

        // Generate ASM code from AST(from file).
        file_t asm_file {};
        get_file(params.filename.asm_code, &asm_file, "w");
        generator(buffer, asm_file.stream);

        // Clean-up.
        clean_args(&params);
        close_log();

        return 0;
}

