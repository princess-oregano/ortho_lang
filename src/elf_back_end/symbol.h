#ifndef SYMBOL_H
#define SYMBOL_H

#include "../stack/stack.h"

enum sym_err_t {
        SYM_NO_ERR  = 0,
        SYM_ALLOC   = 1,
        SYM_BAD_CAP = 2,
        SYM_INSERT  = 3,
};

/// Constructs symbol table with given capacity.
int
sym_ctor(int cap, table_t *table);

/// Makes an entry in table.
int
sym_insert(char *name, table_t *table, int ram);

/// Finds variable in table.
int
sym_lookup(char *name, table_t *table);

/// Removes element from symbol table.
int
sym_remove(char *name, table_t *table);

/// Removes table from stack.
int
sym_remove_table(stack_t *tbl_stack);

/// Constructs new table and pushes to stack.
int
sym_new_table(stack_t *tbl_stack);

/// Finds variable in stack. Returns RAM cell or -1 if error.
int
sym_find(char *name, stack_t *tbl_stack);

/// Destroys symbol table with all entries.
int
sym_dtor(table_t *table);

#endif // SYMBOL_H

