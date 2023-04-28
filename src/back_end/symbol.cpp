#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symbol.h" 
#include "../stack/stack.h"
#include "../log.h"

static int
sym_alloc(int cap, table_t *table)
{
        assert(table);

        if (cap < 0) {
                log("Error: Capacity must be larger or equal to 0.\n");
                return SYM_BAD_CAP;
        }

        var_t *tmp = nullptr;
        tmp = (var_t *) realloc(table->vars, (size_t) cap * sizeof(var_t));

        if (tmp == nullptr) {
                log("Error: Couldn't allocate memory for tokens.\n");
                return SYM_ALLOC;
        }

        table->vars = tmp;
        table->cap = cap;

        return SYM_NO_ERR;
}

int
sym_ctor(int cap, table_t *table)
{
        assert(table);

        int err = 0;
        if ((err = sym_alloc(cap, table)) != SYM_NO_ERR)
                return err;

        return SYM_NO_ERR;
}

int
sym_insert(char *name, table_t *table, int ram)
{
        assert(name);
        assert(table);

        int err = 0; 
        if (table->size >= table->cap - 1) {
                if ((err = sym_alloc(2 * table->cap, table)) != SYM_NO_ERR)
                        return err;
        }

        if (sym_lookup(name, table) != -1) {
                log("Error: variable already exists.\n");
                return SYM_INSERT;
        }

        table->vars[table->size].name = name;
        table->vars[table->size].ram = ram;

        table->size++;

        return SYM_NO_ERR;
}

[[nodiscard]] int
sym_lookup(char *name, table_t *table)
{
        assert(name);
        assert(table);

        for (int i = 0; i < table->size; i++) {
                if (strcmp(table->vars[i].name, name) == 0) {
                        return i;
                }
        }

        return -1;
}

int
sym_remove_table(stack_t *tbl_stack)
{
        assert(tbl_stack);

        table_t *tmp = nullptr;

        stack_pop(tbl_stack, &tmp);
        sym_dtor(tmp);
        free(tmp);

        return SYM_NO_ERR;
}

int
sym_new_table(stack_t *tbl_stack)
{
        assert(tbl_stack);

        table_t *tmp = (table_t *) calloc(1, sizeof(table_t));
        sym_ctor(10, tmp); 

        stack_push(tbl_stack, tmp);

        return SYM_NO_ERR;
}

int
sym_find(char *name, stack_t *tbl_stack)
{
        assert(name);
        assert(tbl_stack);

        int table_num = -1;

        for (int i = (int) tbl_stack->size - 1; i >= 0; i--) {
                table_num = sym_lookup(name, tbl_stack->data[i]);
                if (table_num != -1) {
                        return tbl_stack->data[i]->vars[table_num].ram;
                }
        }

        return -1;
}

int
sym_dtor(table_t *table)
{
        assert(table);

        free(table->vars); 

        return 0;
}

