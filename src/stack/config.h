#ifndef CONFIG_STACK_H
#define CONFIG_STACK_H

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

typedef table_t *elem_t;

// #define CANARY

// #define HASH

#endif // CONFIG_STACK_H

