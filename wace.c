#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#if PLATFORM==2
  #include <vfs.h>
#endif

#include "util.h"
#include "platform.h"
#include "wa.h"
#include "thunk.h"

#if PLATFORM==1
  #include "wace_emscripten.h"
  #define MANGLE_TABLE_INDEX true
#elif PLATFORM==2
  #include "wace_fooboot.h"
  #define MANGLE_TABLE_INDEX false
#else
  #error "unknown PLATFORM"
#endif

void usage(char *prog) {
    fprintf(stderr, "%s WASM_FILE [ARG...]\n", prog);
    exit(2);
}

/////////////////////////////////////////////////////////
// Command line

int main(int argc, char **argv) {
    char     *mod_path;
    int       fidx = 0, res = 0;
    uint8_t  *bytes = NULL;
    int       byte_count;

    // Parse arguments
    if (argc < 2) { usage(argv[0]); }
    mod_path = argv[1];

    init_wace();

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

    Options opts = { .disable_memory_bounds = true,
                     .mangle_table_index    = MANGLE_TABLE_INDEX,
                     .dlsym_trim_underscore = true };
    Module *m = load_module(bytes, byte_count, opts);
    m->path = mod_path;

    init_thunk_in(m);

    // emscripten initialization
    Block *func = get_export(m, "__post_instantiate", KIND_FUNCTION);
    if (func) {
        res = invoke(m, func->fidx);
    }

    // setup argc/argv
    m->stack[++m->sp].value_type = I32;
    m->stack[m->sp].value.uint32 = argc-1;
    m->stack[++m->sp].value_type = I32;
    m->stack[m->sp].value.uint32 = (uint32_t)(argv+1);

    // Invoke main/_main function and exit
    func = get_export(m, "main", KIND_FUNCTION);
    if (!func) {
        func = get_export(m, "_main", KIND_FUNCTION);
	if (!func) {
	    FATAL("no exported function named 'main' or '_main'\n");
	}
    }
    res = invoke(m, fidx);

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
