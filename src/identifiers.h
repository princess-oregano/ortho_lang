#ifndef IDEN_T
#define IDEN_T

enum iden_err {
        IDEN_NO_ERR  = 0,
        IDEN_ALLOC   = 1,
        IDEN_BAD_CAP = 2,
};

struct iden_t {
        char **ptrs = nullptr;
        int size = 0;
        int cap = 0;
};

// Allocates memory to hols identifiers.
int
id_alloc(iden_t *id, int cap);
// Frees all allocated for identifiers space.
void
id_free(iden_t *id);

#endif // IDEN_T

