#ifndef TABLE_H
#define TABLE_H

enum table_err_t {
        TBL_NO_ERR = 0,
        TBL_ALLOC  = 1,
        TBL_BAD_CAP = 2,
        TBL_INSERT = 3,
};

// Structure for cell of symbol table.
struct var_t {
        char *name = nullptr;  // Variable name.
        int ram = 0;           // Memory location of the variable.
};

struct table_t {
        var_t *vars = nullptr;  // All table entries.
        int cap = 0;            // Table capacity.
        int size = 0;           // Current table size.
};

// Constructs symbol table with given capacity.
int
sym_ctor(int cap, table_t *table);
// Makes an entry in table.
int
sym_insert(char *name, table_t *table, int ram);
// Finds variable entry in table.
int
sym_find(char *name, table_t *table);
// Checks if variable exists in table.
bool
sym_lookup(char *name, table_t *table);
// Removes element from symbol table.
int
sym_remove(char *name, table_t *table);
// Destroys symbol table with all entries.
int
sym_dtor(table_t *table);

#endif // TABLE_H

