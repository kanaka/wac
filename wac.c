#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <ctype.h>
#include <string.h>
#include <math.h>

#if PLATFORM==2
  #include <vfs.h>
#endif

#include "util.h"
#include "platform.h"
#include "wa.h"

void usage(char *prog) {
    fprintf(stderr, "%s --repl WASM_FILE\n", prog);
    fprintf(stderr, "%s        WASM_FILE CMD [ARG...]\n", prog);
    exit(2);
}

//
// Imports used by specification tests
//

// https://github.com/WebAssembly/spec/blob/master/interpreter/host/spectest.ml
#define PAGE_COUNT   1
#define TABLE_COUNT  20
Memory _spectest__memory_ = {PAGE_COUNT, PAGE_COUNT, PAGE_COUNT, NULL};
Table _spectest__table_ = {ANYFUNC, TABLE_COUNT, TABLE_COUNT, TABLE_COUNT, NULL};
uint32_t _spectest__global_i32_ = 666;

void _spectest__print_(uint32_t val) {
    //printf("spectest.print 0x%x:i32\n", val);
    printf("0x%x:i32\n", val);
}
void _spectest__print_i32_(uint32_t val) {
    printf("0x%x:i32\n", val);
}

//
// Command line parsing functions
//

// Split a space separated string into an argv style array of strings
// Destroys str, sets argc to number of args, return static buffer argv_buf
// Returns 0 on failure
char *argv_buf[100];
char **split_argv(char *str, int *argc) {
    argv_buf[(*argc)++] = str;

    for (int i = 1; str[i] != '\0'; i += 1) {
	if (str[i-1] == ' ') {
	    str[i-1] = '\0';
	    argv_buf[(*argc)++] = str + i;
	}
    }
    argv_buf[(*argc)] = NULL;
    return argv_buf;
}

// Parse and add arguments to the stack
void parse_args(Module *m, Type *type, int argc, char **argv) {
    for (int i=0; i<argc; i++) {
        for (int j=0; argv[i][j]; j++) {
            argv[i][j] = tolower(argv[i][j]); // lowecase
        }
        m->sp++;
        StackValue *sv = &m->stack[m->sp];
       sv->value_type = type->params[i];
        switch (type->params[i]) {
        case I32: sv->value.uint32 = strtoul(argv[i], NULL, 0); break;
        case I64: sv->value.uint64 = strtoull(argv[i], NULL, 0); break;
        case F32: if (strncmp("-nan", argv[i], 4) == 0) {
                      sv->value.f32 = -NAN;
                  } else {
                      sv->value.f32 = atof(argv[i]);
                  }; break;
        case F64: if (strncmp("-nan", argv[i], 4) == 0) {
                      sv->value.f64 = -NAN;
                  } else {
                      sv->value.f64 = atof(argv[i]);
                  }; break;
        }
    }
}


int main(int argc, char **argv) {
    char     *mod_path;
    int       res = 0;
    uint8_t  *bytes = NULL;
    int       byte_count;
    int       optidx, repl = 0;
    char     *line = NULL;

    // Parse arguments
    if (argc < 3) { usage(argv[0]); }
    if (strcmp("--repl", argv[1]) == 0) {
        if (argc > 3) { usage(argv[0]); }
        repl = 1;
        optidx = 2;
    } else {
        optidx = 1;
    }

    mod_path = argv[optidx++];

    // Allocate memory and table used for spec test
    _spectest__memory_.bytes = malloc(PAGE_COUNT * PAGE_SIZE);
    _spectest__table_.entries = malloc(TABLE_COUNT);

    // Load the module
#if PLATFORM==1
    // open and mmap the WASM module
    bytes = mmap_file(mod_path, &byte_count);
#else
    line = malloc(256);

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

    for (;;) {
        if (repl) {
#if PLATFORM==1
            line = readline("webassembly> ");
            if (!line) { break; }
#else
            if (!readline("webassembly> ", line, 1024)) {
                break;
            }
#endif
            argc = 0;
            optidx = 0;
            argv = split_argv(line, &argc);
            if (argc == 0) { continue; }
        }

        // Reset the stacks
        m->sp = -1;
        m->fp = -1;
        m->csp = -1;

        Block *func = get_export(m, argv[optidx], KIND_FUNCTION);
        if (!func) {
            error("no exported function named '%s'\n", argv[optidx]);
            if (repl) { continue; }
            return 1;
        }
        parse_args(m, func->type, argc-optidx-1, argv+optidx+1);
        warn("Running '%s' function 0x%x ('%s')\n", m->path, func->fidx, entry);
        res = invoke(m, func->fidx);
        if (res) {
            if (m->sp >= 0) {
                printf("%s\n", value_repr(&m->stack[m->sp]));
            }
        } else {
            error("Exception: %s\n", exception);
            if (!repl) { return 1; }
        }

        if (repl) {
#if PLATFORM==1
            free(line);
#endif
            continue;
        }
        break;
    }

    return 0;
}
