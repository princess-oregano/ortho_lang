#include <stdlib.h>
#include "identifiers.h"
#include "log.h"

int
id_alloc(iden_t *id, int cap)
{
        if (cap < 0) {
                log("Error: Capacity must be larger or equal to 0.\n");
                return IDEN_BAD_CAP;
        }

        char **tmp = nullptr;
        tmp = (char **) realloc(id->ptrs, (size_t) cap * sizeof(char *));

        if (tmp == nullptr) {
                log("Error: Couldn't allocate memory for tokens.\n");
                return IDEN_ALLOC;
        }

        id->ptrs = tmp;
        id->cap = cap;

        return IDEN_NO_ERR;
}

void
id_free(iden_t *id)
{
        for (int i = 0; i < id->size; i++)
                free(id->ptrs[i]);

        free(id->ptrs);
}

