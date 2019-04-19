#ifndef WASI_H
#define WASI_H

#define __wasi__
#include "wasi_core.h"

///////////////////////////////////////////////////////////

typedef struct Preopen {
    char *path;
    int   path_len;
} Preopen;

void init_wasi(Memory *memory, int argc, char **argv, char **envp);

#endif // WASI_H
