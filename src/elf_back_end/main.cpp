#include <stdio.h>
#include "encode.h"
#include "generator.h"
#include "elf_file.h"
#include "../file.h"
#include "../args.h"
#include "../log.h"

int
main(int argc, char *argv[])
{
        open_log("log.html", "a");

        // Process cmd-line arguments.
        params_t params {};
        process_args(argc, argv, &params);

        // Read source file.
        file_t ast_file {};
        char *buffer = nullptr;
        get_file(params.filename.ast_code, &ast_file, "r");
        read_file(&buffer, &ast_file);
        fclose(ast_file.stream);

        // Generator module: construct code_t struct, according to AST and using 
        // encoding module fill an array of uint8_t.
        code_t code = {};
        en_code_ctor(&code, 1000);
        generator(buffer, &code);

        // Print code.
        for (int i = 0; i < code.size; i++) {
                fprintf(stderr, "%02x ", code.code[i]);
        }

        // Clean-up.
        clean_args(&params);
        close_log();
        en_code_dtor(&code);
        
        return 0;
}
