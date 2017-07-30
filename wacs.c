#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>

#ifdef USE_READLINE
  // WARNING: GPL license implications
  #include <readline/readline.h>
  #include <readline/history.h>
#else
  #include <editline/readline.h>
#endif

#include "util.h"
#include "wa.h"

void usage(char *prog) {
    fprintf(stderr, "%s [--debug] WASM_FILE [--repl|-- ARG...]\n", prog);
    exit(2);
}

// Special test imports
uint32_t _spectest__global_ = 666;

void _spectest__print_(uint32_t val) {
    //printf("spectest.print 0x%x:i32\n", val);
    printf("0x%x:i32\n", val);
}

#define TOTAL_MEMORY  0x1000000 // 16MB
#define TOTAL_TABLE   65536

/*  memory layout
 *
 *  +-------------------+
 *  | Stack             |
 *  | ...               |
 *  | STACK_TOP         |
 *  | ...               |
 *  | STACK_MAX         |
 *  +-------------------+
 *  | ...               |
 *  | DYNAMIC_TOP       |
*/

typedef struct HostMemory {
    char *memory;
    uint32_t stack_top;
    uint32_t stack_max;
    uint32_t dynamic_top;

    uint32_t *table;
} HostMemory;

double    _global__NaN_         = NAN;
double    _global__Infinity_    = INFINITY;
uint32_t _env__zeroval_ = 0;
HostMemory _memory_;
double _temp_double_ = 0;

#define DECLARE_DUMMY0(ret_t, name) ret_t _env__##name##_() { FATAL("Calling undefined function: %s", #name); }
#define DECLARE_DUMMY1(ret_t, name, a1_t) ret_t _env__##name##_(a1_t a1) { FATAL("Calling undefined function: %s(%d)", #name, a1); }
#define DECLARE_DUMMY2(ret_t, name, a1_t, a2_t) ret_t _env__##name##_(a1_t a1, a2_t a2) { FATAL("Calling undefined function: %s(%d, %d)", #name, a1, a2); }
#define DECLARE_DUMMY3(ret_t, name, a1_t, a2_t, a3_t) ret_t _env__##name##_(a1_t a1, a2_t a2, a3_t a3) { FATAL("Calling undefined function: %s(%d, %d, %d)", #name, a1, a2, a3); }

DECLARE_DUMMY1(void, nullFunc, int32_t)
DECLARE_DUMMY2(int32_t, syscall, int32_t, int32_t)
DECLARE_DUMMY0(int32_t, enlargeMemory)
DECLARE_DUMMY0(int32_t, getTotalMemory)
DECLARE_DUMMY0(int32_t, abortOnCannotGrowMemory)
DECLARE_DUMMY1(void, abortStackOverflow, int32_t)
DECLARE_DUMMY1(void, unlock, int32_t)
DECLARE_DUMMY1(void, lock, int32_t)
DECLARE_DUMMY1(void, setErrNo, int32_t)
DECLARE_DUMMY0(void, abort)
DECLARE_DUMMY3(int32_t, memcpy_big, int32_t, int32_t, int32_t)

#define EXPORT(field, obj) if(strcmp(name, field) == 0) return &obj

void *exports(char *module, char *name) {
    // This is rather crude... a hashtable would be nicer.
    // But then, performance is unlikely to matter, here.
    if(strcmp("env", module) == 0) {
        EXPORT("memory", _memory_.memory);
        EXPORT("table", _memory_.table);

        EXPORT("STACKTOP", _memory_.stack_top);
        EXPORT("STACK_MAX", _memory_.stack_max);
        EXPORT("DYNAMICTOP_PTR", _memory_.dynamic_top);
        EXPORT("tempDoublePtr", _temp_double_);

        EXPORT("enlargeMemory", _env__enlargeMemory_);
        EXPORT("getTotalMemory", _env__getTotalMemory_);
        EXPORT("abortOnCannotGrowMemory", _env__abortOnCannotGrowMemory_);
        EXPORT("abortStackOverflow", _env__abortStackOverflow_);
        EXPORT("_emscripten_memcpy_big", _env__memcpy_big_);

        EXPORT("___syscall6", _env__syscall_);
        EXPORT("___syscall54", _env__syscall_);
        EXPORT("___syscall140", _env__syscall_);
        EXPORT("___syscall146", _env__syscall_);
        EXPORT("___lock", _env__lock_);
        EXPORT("___unlock", _env__lock_);
        EXPORT("___setErrNo", _env__setErrNo_);
        EXPORT("_abort", _env__abort_);

        EXPORT("nullFunc_i", _env__nullFunc_);
        EXPORT("nullFunc_ii", _env__nullFunc_);
        EXPORT("nullFunc_iii", _env__nullFunc_);
        EXPORT("nullFunc_iiii", _env__nullFunc_);

        // these seem to be unused
        EXPORT("ABORT", _env__zeroval_);
        EXPORT("memoryBase", _env__zeroval_);
        EXPORT("tableBase", _env__zeroval_);
    }
    if(strcmp("global", module) == 0) {
        EXPORT("NaN", _global__NaN_);
        EXPORT("Infinity", _global__Infinity_);
    }
    return NULL;
}


void init_memory() {
    _memory_.memory = calloc(TOTAL_MEMORY, sizeof(char));
    _memory_.stack_top = 0;
    _memory_.stack_max = STACK_SIZE;
    _memory_.dynamic_top = STACK_SIZE + 1;

    _memory_.table = calloc(TOTAL_TABLE, sizeof(uint32_t));
}


int main(int argc, char **argv) {
    char   *mod_path, *entry, *line;
    int     repl = 0, debug = 0, res = 0;

    // Parse arguments
    int option_index = 0, c;
    struct option long_options[] = {
        {"repl",  no_argument, &repl,  1},
        {"debug", no_argument, &debug, 1},
        {0,       0,           0,      0}
    };
    while ((c = getopt_long (argc, argv, "",
                             long_options, &option_index)) != -1) {
        switch (c) {
        case 0: break;
        case '?': usage(argv[0]); break;
        default: usage(argv[0]);
        }
    }
    if (optind >= argc) { usage(argv[0]); }
    mod_path = argv[optind++];

    if (debug) {
        printf("repl: %d, debug: %d, module path: %s\n",
               repl, debug, mod_path);
    }

    // Load the module
    Options opts;
    Module *m = load_module(mod_path, opts, &exports);

    if (!repl) {
        // Invoke one function and exit
        res = invoke(m, argv[optind], argc-optind-1, argv+optind+1);
        if (res) {
	    if (m->sp >= 0) {
		printf("%s\n", value_repr(&m->stack[m->sp]));
	    }
        } else {
	    error("Exception: %s\n", exception);
	    exit(1);
	}
    } else {
        // Simple REPL
        if (optind < argc) { usage(argv[0]); }
        while (line = readline("webassembly> ")) {
            int token_cnt = 0;
            char **tokens = split_string(line, &token_cnt);
            if (token_cnt == 0) { continue; }

            // Reset the stacks
            m->sp = -1;
            m->fp = -1;
            m->csp = -1;
            res = invoke(m, tokens[0], token_cnt-1, tokens+1);
	    if (res) {
		if (m->sp >= 0) {
		    printf("%s\n", value_repr(&m->stack[m->sp]));
		}
	    } else {
		error("Exception: %s\n", exception);
	    }
            free(tokens);
        }
    }
    exit(0);
}
