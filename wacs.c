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

double    _global__NaN_         = NAN;
double    _global__Infinity_    = INFINITY;
uint32_t _env__zeroval_ = 0;
uint8_t *host_memory;
uint8_t *memory_dump;
uint32_t *host_table;

double _temp_double_ = 0;
uint32_t _env__stacktop_;
uint32_t _env__stackmax_;
uint32_t _env__dynamictop_ptr_;


void print_memory(uint8_t* memory, uint32_t start, uint32_t stop) {
    static const uint8_t colsize = 32;
    printf("\n");
    uint32_t m = start;
    while(m < stop) {
        printf("%08x  ", m);
        for(int i=0; i<colsize; i++) {
            printf("%02x ", memory[m + i]);
        }
        for(int i=0; i<colsize; i++) {
            char c = memory[m + i];
            if(c >= 32 && c < 127)
                printf("%c", c);
            else
                printf(".");
        }
        m += colsize;
        printf("\n");
    }
    printf("\n");
}

#define ANSI_COLOR_DIFF    "\x1b[1m\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void memdiff(void *a, void *b, size_t offset, size_t n) {
    static const uint8_t colsize = 16;
    size_t o = offset;
    while(o < n) {
        printf("%08x  ", o);
        for(int i=0; i<colsize; i++) {
            uint8_t m = *(uint8_t*)(a + i);
            if(m == *(uint8_t*)(b + i))
                printf("%02x ", m);
            else
                printf(ANSI_COLOR_DIFF "%02x " ANSI_COLOR_RESET, m);
        }
        for(int i=0; i<colsize; i++) {
            char c = *(char*)(a + i);
            if(c != *(char*)(b + i))
                printf(ANSI_COLOR_DIFF);
            if(c >= 32 && c < 127)
                printf("%c", c);
            else
                printf(".");
            printf(ANSI_COLOR_RESET);
        }
        printf("  ");
        for(int i=0; i<colsize; i++) {
            uint8_t m = *(uint8_t*)(b + i);
            if(m == *(uint8_t*)(a + i))
                printf("%02x ", m);
            else
                printf(ANSI_COLOR_DIFF "%02x " ANSI_COLOR_RESET, m);
        }
        for(int i=0; i<colsize; i++) {
            char c = *(char*)(b + i);
            if(c != *(char*)(a + i))
                printf(ANSI_COLOR_DIFF);
            if(c >= 32 && c < 127)
                printf("%c", c);
            else
                printf(".");
            printf(ANSI_COLOR_RESET);
        }
        o += colsize;
        a += colsize;
        b += colsize;
        printf("\n");
    }
}

void print_globals(Module *m) {
    for(uint32_t i=0; i<m->global_count; i++) {
        printf("global %d: ", i);
        switch(m->globals[i].value_type) {
        case I32: printf("%d\n", m->globals[i].value.int32); break;
        case I64: printf("%lld\n", m->globals[i].value.int64); break;
        case F32: printf("%f\n", m->globals[i].value.f32); break;
        case F64: printf("%f\n", m->globals[i].value.f64); break;
        }
    }
}

void _env__memdump_() {
    memcpy(memory_dump, host_memory, TOTAL_MEMORY);
}

void _env__memdiff_(uint32_t offset, uint32_t n) {
    memdiff(host_memory, memory_dump, offset, n);
}


int32_t syscall54(uint32_t a, uint32_t b) {
    info("syscall54(%d, %d)\n", a, b);
    return 0;
}


int32_t syscall146(uint32_t what, uint32_t argp) {
    //printf("syscall146(%d, %p)\n", what, (void*)argp);
    //int fd = *(int32_t*)&host_memory[argp];
    uint32_t iovecptr = *(uint32_t*)&host_memory[argp + 4];
    uint32_t iovcnt = *(uint32_t*)&host_memory[argp + 8];
    struct {
        uint32_t iov_base;
        uint32_t iov_len;
    } *iovec;
    //printf("    fd: %p, cnt: %d\n", (void*)fd, iovcnt);
    int32_t total_written = 0;
    for(uint32_t i=0; i<iovcnt; i++, iovecptr+=8) {
        iovec = (void*)&host_memory[iovecptr];
        //printf("    iovec %p, %d: ", (void*)iovec->iov_base, iovec->iov_len);
        for(uint32_t j=0; j<iovec->iov_len; j++) {
            printf("%c", host_memory[iovec->iov_base + j]);
            total_written += 1;
        }
        //printf("\n");
    }

    //FATAL("---")
    //return -1;
    return total_written;
}

uint32_t _env__getTotalMemory_() {
    return TOTAL_MEMORY;
}

//uint32_t _env__memcpy_big_(uint32_t dst, uint32_t src, uint32_t num) {
//    return (uint32_t)memcpy(&_memory_.memory[dst], &_memory_.memory[src], num);
//}

//void _env__nullFunc_(int32_t a) { printf("calling nullFunc(%d)\n", a);}

#define DECLARE_DUMMY0(ret_t, name) ret_t _env__##name##_() { FATAL("Calling undefined function: %s\n", #name); }
#define DECLARE_DUMMY1(ret_t, name, a1_t) ret_t _env__##name##_(a1_t a1) { FATAL("Calling undefined function: %s(%d)\n", #name, a1); }
#define DECLARE_DUMMY2(ret_t, name, a1_t, a2_t) ret_t _env__##name##_(a1_t a1, a2_t a2) { FATAL("Calling undefined function: %s(%d, %d)\n", #name, a1, a2); }
#define DECLARE_DUMMY3(ret_t, name, a1_t, a2_t, a3_t) ret_t _env__##name##_(a1_t a1, a2_t a2, a3_t a3) { FATAL("Calling undefined function: %s(%d, %d, %d)\n", #name, a1, a2, a3); }
#define DECLARE_DUMMY4(ret_t, name, a1_t, a2_t, a3_t, a4_t) ret_t _env__##name##_(a1_t a1, a2_t a2, a3_t a3, a4_t a4) { FATAL("Calling undefined function: %s(%d, %d, %d, %d)\n", #name, a1, a2, a3, a4); }
#define DECLARE_DUMMY5(ret_t, name, a1_t, a2_t, a3_t, a4_t, a5_t) ret_t _env__##name##_(a1_t a1, a2_t a2, a3_t a3, a4_t a4, a5_t a5) { FATAL("Calling undefined function: %s(%d, %d, %d, %d, %d)\n", #name, a1, a2, a3, a4, a5); }



DECLARE_DUMMY0(void, lazy_dummy)

DECLARE_DUMMY1(void, nullFunc, int32_t)
DECLARE_DUMMY2(int32_t, syscall, int32_t, int32_t)
DECLARE_DUMMY0(int32_t, enlargeMemory)
//DECLARE_DUMMY0(int32_t, getTotalMemory)
DECLARE_DUMMY0(int32_t, abortOnCannotGrowMemory)
DECLARE_DUMMY1(void, abortStackOverflow, int32_t)
DECLARE_DUMMY1(void, unlock, int32_t)
DECLARE_DUMMY1(void, lock, int32_t)
DECLARE_DUMMY1(void, setErrNo, int32_t)
DECLARE_DUMMY0(void, abort)
DECLARE_DUMMY1(void, segfault, int32_t);
DECLARE_DUMMY1(void, alignfault, int32_t);
DECLARE_DUMMY1(void, ftfault, int32_t);
DECLARE_DUMMY3(int32_t, memcpy_big, int32_t, int32_t, int32_t)
DECLARE_DUMMY1(int32_t, invoke_i, int32_t)
DECLARE_DUMMY2(int32_t, invoke_ii, int32_t, int32_t)
DECLARE_DUMMY3(int32_t, invoke_iii, int32_t, int32_t, int32_t)
DECLARE_DUMMY4(int32_t, invoke_iiii, int32_t, int32_t, int32_t, int32_t)

DECLARE_DUMMY1(void, invoke_v, int32_t)
DECLARE_DUMMY2(void, invoke_vi, int32_t, int32_t)
DECLARE_DUMMY3(void, invoke_vii, int32_t, int32_t, int32_t)
DECLARE_DUMMY4(void, invoke_viii, int32_t, int32_t, int32_t, int32_t)
DECLARE_DUMMY5(void, invoke_viiii, int32_t, int32_t, int32_t, int32_t, int32_t)

DECLARE_DUMMY4(void, assert_fail, int32_t, int32_t, int32_t, int32_t)
DECLARE_DUMMY5(int32_t, strftime_l, int32_t, int32_t, int32_t, int32_t, int32_t)
DECLARE_DUMMY0(int32_t, _ZSt18uncaught_exceptionv)
DECLARE_DUMMY1(int32_t, getenv, int32_t)
DECLARE_DUMMY2(int32_t, __map_file, int32_t, int32_t)

DECLARE_DUMMY2(int32_t, _pthread_stuff, int32_t, int32_t)

#define EXPORT(field, obj) if(strcmp(name, field) == 0) return &obj

void *exports(char *module, char *name) {
    // This is rather crude... a hashtable would be nicer.
    // But then, performance is unlikely to matter, here.
    if(strcmp("env", module) == 0) {
        EXPORT("_memdump", _env__memdump_);
        EXPORT("_memdiff", _env__memdiff_);

        EXPORT("memory", host_memory);
        EXPORT("table",  host_table);

        EXPORT("STACKTOP", _env__stacktop_);
        EXPORT("STACK_MAX", _env__stackmax_);
        EXPORT("DYNAMICTOP_PTR", _env__dynamictop_ptr_);
        EXPORT("tempDoublePtr", _temp_double_);

        EXPORT("enlargeMemory", _env__enlargeMemory_);
        EXPORT("getTotalMemory", _env__getTotalMemory_);
        EXPORT("abortOnCannotGrowMemory", _env__abortOnCannotGrowMemory_);
        EXPORT("abortStackOverflow", _env__abortStackOverflow_);
        EXPORT("_emscripten_memcpy_big", _env__memcpy_big_);

        EXPORT("___syscall4", _env__syscall_);
        EXPORT("___syscall6", _env__syscall_);
        EXPORT("___syscall54", syscall54);
        EXPORT("___syscall91", _env__syscall_);
        EXPORT("___syscall140", _env__syscall_);
        EXPORT("___syscall145", _env__syscall_);
        EXPORT("___syscall146", syscall146);
        EXPORT("___lock", _env__lock_);
        EXPORT("___unlock", _env__lock_);
        EXPORT("___setErrNo", _env__setErrNo_);
        EXPORT("_abort", _env__abort_);
        EXPORT("abort", _env__abort_);
        EXPORT("segfault", _env__segfault_);
        EXPORT("alignfault", _env__alignfault_);
        EXPORT("ftfault", _env__ftfault_);

        EXPORT("nullFunc_i", _env__nullFunc_);
        EXPORT("nullFunc_ii", _env__nullFunc_);
        EXPORT("nullFunc_iii", _env__nullFunc_);
        EXPORT("nullFunc_iiii", _env__nullFunc_);
        EXPORT("nullFunc_ji", _env__nullFunc_);
        EXPORT("nullFunc_v", _env__nullFunc_);
        EXPORT("nullFunc_vi", _env__nullFunc_);
        EXPORT("nullFunc_vii", _env__nullFunc_);
        EXPORT("nullFunc_viii", _env__nullFunc_);
        EXPORT("nullFunc_viiii", _env__nullFunc_);

        EXPORT("invoke_i", _env__invoke_i_);
        EXPORT("invoke_ii", _env__invoke_ii_);
        EXPORT("invoke_iii", _env__invoke_iii_);
        EXPORT("invoke_iiii", _env__invoke_iiii_);
        EXPORT("invoke_v", _env__invoke_v_);
        EXPORT("invoke_vi", _env__invoke_vi_);
        EXPORT("invoke_vii", _env__invoke_vii_);
        EXPORT("invoke_viii", _env__invoke_viii_);
        EXPORT("invoke_viiii", _env__invoke_viiii_);

        EXPORT("___assert_fail", _env__assert_fail_);
        EXPORT("_strftime_l", _env__strftime_l_);
        EXPORT("_getenv", _env__getenv_);
        EXPORT("___map_file", _env____map_file_);

        EXPORT("_pthread_cond_wait", _env___pthread_stuff_);
        EXPORT("_pthread_key_create", _env___pthread_stuff_);
        EXPORT("_pthread_getspecific", _env___pthread_stuff_);
        EXPORT("_pthread_once", _env___pthread_stuff_);
        EXPORT("_pthread_setspecific", _env___pthread_stuff_);
        EXPORT("_pthread_rwlock_unlock", _env___pthread_stuff_);
        EXPORT("_pthread_cond_init", _env___pthread_stuff_);
        EXPORT("_pthread_cond_destroy", _env___pthread_stuff_);
        EXPORT("_pthread_mutexattr_destroy", _env___pthread_stuff_);
        EXPORT("_pthread_key_delete", _env___pthread_stuff_);
        EXPORT("_pthread_condattr_setclock", _env___pthread_stuff_);
        EXPORT("_pthread_getspecific", _env___pthread_stuff_);
        EXPORT("_pthread_rwlock_rdlock", _env___pthread_stuff_);
        EXPORT("_pthread_cond_signal", _env___pthread_stuff_);
        EXPORT("_pthread_mutex_destroy", _env___pthread_stuff_);
        EXPORT("_pthread_condattr_init", _env___pthread_stuff_);
        EXPORT("_pthread_condattr_destroy", _env___pthread_stuff_);
        EXPORT("_pthread_mutexattr_settype", _env___pthread_stuff_);
        EXPORT("_pthread_mutexattr_init", _env___pthread_stuff_);
        EXPORT("_pthread_mutex_init", _env___pthread_stuff_);

        // C++ name mangled
        EXPORT("__ZSt18uncaught_exceptionv", _env___ZSt18uncaught_exceptionv_);

        // Needed for Rust?
        EXPORT("__Unwind_Backtrace", _env__lazy_dummy_);
        EXPORT("__Unwind_FindEnclosingFunction", _env__lazy_dummy_);
        EXPORT("__Unwind_GetIPInfo", _env__lazy_dummy_);
        EXPORT("___gxx_personality_v0", _env__lazy_dummy_);
        EXPORT("___cxa_find_matching_catch_2", _env__lazy_dummy_);
        EXPORT("___cxa_find_matching_catch_3", _env__lazy_dummy_);
        EXPORT("___cxa_free_exception", _env__lazy_dummy_);
        EXPORT("___cxa_allocate_exception", _env__lazy_dummy_);
        EXPORT("___cxa_throw", _env__lazy_dummy_);
        EXPORT("___resumeException", _env__lazy_dummy_);
        EXPORT("_dladdr", _env__lazy_dummy_);
        EXPORT("_llvm_trap", _env__lazy_dummy_);


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
    host_memory = calloc(TOTAL_MEMORY, sizeof(char));
    host_table = calloc(TOTAL_TABLE, sizeof(uint32_t));
    memory_dump = calloc(TOTAL_MEMORY, sizeof(char));

    _env__stacktop_ = 0;
    _env__stackmax_ = STACK_SIZE;
    _env__dynamictop_ptr_ = 0;  // is the first memory address a good position for the heap pointer?
}


int main(int argc, char **argv) {
    char   *mod_path, *line;
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

    init_memory();

    // Load the module
    Options opts;
    Module *m = load_module(mod_path, opts, &exports);

    // set heap memory right after data section initialized by module (aligned to 16 bytes)
    *(uint32_t*)&host_memory[_env__dynamictop_ptr_] = (m->data_size + 15) & -16;

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
