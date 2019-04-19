#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>

#include "util.h"
#include "wa.h"
#include "platform.h"
#include "wasi.h"

void _spectest__print_(uint32_t val) {
    //printf("spectest.print 0x%x:i32\n", val);
    printf("0x%x:i32\n", val);
}
void _spectest__print_i32_(uint32_t val) {
    printf("0x%x:i32\n", val);
}
void _spectest__print_i64_(uint64_t val) {
    printf("0x%llx:i64\n", val);
}


void usage(char *prog) {
    fprintf(stderr, "%s WASM_FILE [ARG...]\n", prog);
    exit(2);
}

/////////////////////////////////////////////////////////
// Command line

int main(int argc, char **argv) {
    char     *mod_path;
    int       res = 0;
    uint8_t  *bytes = NULL;
    int       byte_count;

    // Parse arguments
    if (argc < 2) { usage(argv[0]); }
    mod_path = argv[1];

    // Load the module
#if PLATFORM==1
    // open and mmap the WASM module
    bytes = mmap_file(mod_path, &byte_count);
#else
    // read the file via mem_fs or serial method
    byte_count = vfs_file_size(mod_path);
    if (byte_count > 0) {
        bytes = malloc(byte_count);
        vfs_read_file(mod_path, (char *)bytes);
    }
#endif
    if (bytes == NULL) {
        fprintf(stderr, "Could not load %s", mod_path);
        return 2;
    }

    Options opts;
    Module *m = load_module(bytes, byte_count, opts);
    m->path = mod_path;

    Memory *mem = get_export(m, "memory", KIND_MEMORY);
    if (!mem) { FATAL("no exported memory named 'memory'\n"); }
    init_wasi(mem, argc-1, argv+1);

    // Invoke main/_main function and exit
    Block *func = get_export(m, "_start", KIND_FUNCTION);
    if ((!func)) { FATAL("no exported function named '_start'\n"); }
    res = invoke(m, func->fidx);

    if (!res) {
        error("Exception: %s\n", exception);
        return 1;
    }

    if (m->sp >= 0) {
        return (m->stack[m->sp--].value.uint32);
    } else {
        return 0;
    }

}
