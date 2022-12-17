#include <stdio.h>
#include <stdlib.h>
#include "front_end/lexer.h"
#include "front_end/parser.h"
#include "tree/tree.h"
#include "file.h"
#include "log.h"

int
main()
{
        open_log("log.html");
        file_t file {};

        get_file("test.txt", &file, "r");

        char *buffer = nullptr;
        read_file(&buffer, &file);

        tok_arr_t tok_arr {};
        lexer(buffer, file.stats.st_size - 1, &tok_arr);

        tree_t ast {};
        tree_ctor(&ast, 200);
        parser(&tok_arr, &ast);
        free(tok_arr.tok);
        tree_dtor(&ast);
        close_log();

        return 0;
}

