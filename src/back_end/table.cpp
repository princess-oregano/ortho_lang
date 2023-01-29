#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h" 
#include "../log.h"

static int
sym_alloc(int cap, table_t *table)
{
        if (cap < 0) {
                log("Error: Capacity must be larger or equal to 0.\n");
                return TBL_BAD_CAP;
        }

        var_t *tmp = nullptr;
        tmp = (var_t *) realloc(table->vars, (size_t) cap * sizeof(var_t));

        if (tmp == nullptr) {
                log("Error: Couldn't allocate memory for tokens.\n");
                return TBL_ALLOC;
        }

        table->vars = tmp;
        table->cap = cap;

        return TBL_NO_ERR;
}

int
sym_ctor(int cap, table_t *table)
{
        int err = 0;
        if ((err = sym_alloc(cap, table)) != TBL_NO_ERR)
                return err;

        return TBL_NO_ERR;
}

int
sym_insert(char *name, table_t *table, int ram)
{
        int err = 0; 
        if (table->size >= table->cap - 1) {
                if ((err = sym_alloc(2 * table->cap, table)) != TBL_NO_ERR)
                        return err;
        }

        if (sym_lookup(name, table)) {
                log("Error: variable already exists.\n");
                return TBL_INSERT;
        }

        table->vars[table->size].name = name;
        table->vars[table->size].ram = ram;

        table->size++;

        return TBL_NO_ERR;
}

[[nodiscard]] int
sym_find(char *name, table_t *table)
{
        for (int i = 0; i < table->size; i++) {
                if (strcmp(table->vars[i].name, name) == 0) {
                        return i;
                }
        }

        return -1;
}

[[nodiscard]] bool
sym_lookup(char *name, table_t *table)
{
        for (int i = 0; i < table->size; i++) {
                if (strcmp(table->vars[i].name, name) == 0) {
                        return true;
                }
        }

        return false;
}

int
sym_dtor(table_t *table)
{
        free(table->vars); 

        return 0;
}
