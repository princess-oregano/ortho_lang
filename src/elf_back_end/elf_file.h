#ifndef ELF_FILE
#define ELF_FILE

#include <stdio.h>
#include "encode.h"

/// Constructs ELF-header for specific machine.
void
write_header(FILE *stream);

#endif // ELF_FILE

