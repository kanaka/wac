#include <stdint.h>
#include <math.h>
#include <string.h>

#include "util.h"
#include "wa.h"

#define ALLOC_NORMAL  0 // Tries to use _malloc()
#define ALLOC_STACK   1 // Lives for the duration of the current function call
#define ALLOC_STATIC  2 // Cannot be freed
#define ALLOC_DYNAMIC 3 // Cannot be freed except through sbrk
#define ALLOC_NONE    4 // Do not allocate


/////////////////////////////////////////////////////////
// emscripten memory globals

uint8_t   _env__memory_[16777216];  // 256*(2**16)
uint32_t *HEAP32 = (uint32_t *)_env__memory_;

uint32_t  TOTAL_STACK  = 5242880;
uint32_t  TOTAL_MEMORY = 16777216;
uint32_t  STATIC_BASE  = 0;
uint32_t  STATIC_BUMP;
uint32_t  DYNAMIC_BASE = 0;
uint32_t  STACK_BASE   = 0;


/////////////////////////////////////////////////////////
// emscripten global variables

double    _global__NaN_         = NAN;
double    _global__Infinity_    = INFINITY;

uint32_t  _env__DYNAMICTOP_PTR_ = 0;
uint32_t  _env__tempDoublePtr_;
uint32_t  _env__STATICTOP_      = 0;
uint32_t  _env__STACKTOP_       = 0;
uint32_t  _env__STACK_MAX_      = 0;
uint32_t  _env__ABORT_          = 0;

uint32_t  _env__memoryBase_     = 1024;

void     *_env__table_[18];
uint32_t  _env__tableBase_      = 0;


/////////////////////////////////////////////////////////
// Memory utility functions

uint32_t _malloc_fidx           = -1;

uint32_t heap_get_i32(uint32_t addr) {
    uint32_t res;
    memcpy(&res, _env__memory_+addr, 4);
    return res;
}

uint32_t alignMemory(uint32_t size) {
    return ceil((size)/16)*16;
}

uint32_t staticAlloc(uint32_t size) {
    uint32_t ret = _env__STATICTOP_;
    _env__STATICTOP_ = _env__STATICTOP_ + size;
    _env__STATICTOP_ = (_env__STATICTOP_+15)&-16;
    return ret;
}

uint32_t allocate(Module *m,
                  uint8_t *slab, uint32_t size,
                  int item_size, int allocator) {
    bool singleType = slab == NULL;
    uint32_t ret;

    //ASSERT(allocator == ALLOC_NORMAL || allocator == ALLOC_STATIC,
    ASSERT(allocator == ALLOC_STATIC,
           "allocator mode %d unsupported\n", allocator);

    if (m && _malloc_fidx >= 0) {
        ASSERT(call_function32(m, _malloc_fidx, &ret),
               "_malloc failed");
    } else {
        // No link to internal _malloc setup yet
        ret = staticAlloc(size);
    }
    return ret;
}

int emscripten_init() {
    STATIC_BASE = 1024;
    _env__STATICTOP_ = STATIC_BASE + 5168;
    STATIC_BUMP = 5168;
    _env__tempDoublePtr_ = _env__STATICTOP_; _env__STATICTOP_ += 16;

    _env__DYNAMICTOP_PTR_ = allocate(NULL, NULL, 1, 4, ALLOC_STATIC);
    STACK_BASE = _env__STACKTOP_ = alignMemory(_env__STATICTOP_);
    _env__STACK_MAX_ = STACK_BASE + TOTAL_STACK;

    DYNAMIC_BASE = alignMemory(_env__STACK_MAX_);

    HEAP32[_env__DYNAMICTOP_PTR_>>2] = DYNAMIC_BASE;
}

/////////////////////////////////////////////////////////


uint32_t _env__abortOnCannotGrowMemory_();

uint32_t _env__enlargeMemory_() {
    return _env__abortOnCannotGrowMemory_();
}

uint32_t _env__getTotalMemory_() {
    return TOTAL_MEMORY;
}

uint32_t _env__abortOnCannotGrowMemory_() {
    FATAL("Cannot enlarge memory arrays.\n");
}

void _env__abortStackOverflow_(uint32_t allocSize) {
    FATAL("Stack overflow! Attempted to allocate 0x%x bytes on the stack\n",
          allocSize);
}

void _env__nullFunc_ii_(uint32_t x) {
    FATAL("Invalid function pointer called with signature 'ii'\n");
}

void _env__nullFunc_iiii_(uint32_t x) {
    FATAL("Invalid function pointer called with signature 'iiii'\n");
}

void _env__nullFunc_vi_(uint32_t x) {
    FATAL("Invalid function pointer called with signature 'vi'\n");
}

void _env___pthread_cleanup_pop_(uint32_t _) {
    FATAL("pthread_cleanup_pop unimplemented\n");
}

void _env_____lock_(uint32_t _) {
    return; // nop
}

void _env_____unlock_(uint32_t _) {
    return; // nop
}

void _env___abort_() {
    if (TRACE) {
        warn("    >>><<< __abort\n");
    }
    FATAL("abort");
}

void _env_____setErrNo_(uint32_t value) {
    if (TRACE) {
        warn("    >>><<< ___setErrNo value: %d\n", value);
    }
    return;
}

uint32_t _env_____syscall6_(uint32_t which, uint32_t varargs) {
    if (TRACE) {
        warn("    >>><<< ___syscall6 which: %d, varargs: %d\n", which, varargs);
    }
    FATAL("___syscall6 unimplemented\n");
}

uint32_t _env_____syscall140_(uint32_t which, uint32_t varargs) {
    if (TRACE) {
        warn("    >>><<< ___syscall140 which: %d, varargs: %d\n", which, varargs);
    }
    FATAL("___syscall140 unimplemented\n");
}

uint32_t _env_____syscall54_(uint32_t which, uint32_t varargs) {
    // ioctl
    if (TRACE) {
        warn("    >>><<< ___syscall54 which: %d, varargs: %d\n", which, varargs);
    }
    return 0; // ignore
    //FATAL("___syscall154 unimplemented\n");
}

uint32_t _env_____syscall146_(uint32_t which, uint32_t varargs) {
    uint32_t stream = heap_get_i32(varargs),
             iov = heap_get_i32(varargs+4),
             iovcnt = heap_get_i32(varargs+8),
             ret = 0;
    varargs += 12;
    if (TRACE) {
        warn("    >>> ___syscall146 which: %d, varargs: %d, stream: %d, iov: %d, iovcnt: %d\n", which, varargs, stream, iov, iovcnt);
    }
    for (uint32_t i=0; i<iovcnt; i++) {
        uint32_t ptr = heap_get_i32(iov + i*8);
        uint32_t len = heap_get_i32(iov + i*8 + 4);
        for (uint32_t j=0; j<len; j++) {
            putchar(_env__memory_[ptr+j]);
        }
        ret += len;
    }
    if (TRACE) { warn("    <<< ___syscall146 ret: %d\n", ret); }
    return ret;
}

void _env___pthread_cleanup_push_(uint32_t routine, uint32_t arg) {
    FATAL("_pthread_cleanup_push unimplemented\n");
}

uint32_t _env___emscripten_memcpy_big_(uint32_t dest, uint32_t src,
                                       uint32_t num) {
    FATAL("_emscripten_memcpy_big unimplemented\n");
    return dest;
}

uint32_t _asm2wasm__f64_to_int_(double x) {
    return (uint32_t)x;
}

// Command line

void usage(char *prog) {
    fprintf(stderr, "%s [--debug] WASM_FILE [-- ARG...]\n", prog);
    exit(2);
}

extern char OPERATOR_INFO[][20];

int main(int argc, char **argv) {
    char   *mod_path;
    int     res = 0;

    if (argc < 2) { usage(argv[0]); }
    mod_path = argv[1];

    debug("module path: %s\n", mod_path);

    emscripten_init();

    debug("_env__DYNAMICTOP_PTR_: 0x%x (%d)\n", _env__DYNAMICTOP_PTR_, _env__DYNAMICTOP_PTR_);
    debug("_env__tempDoublePtr_: 0x%x (%d)\n", _env__tempDoublePtr_, _env__tempDoublePtr_);
    debug("_env__STATICTOP_: 0x%x (%d)\n", _env__STATICTOP_, _env__STATICTOP_);
    debug("_env__STACKTOP_: 0x%x (%d)\n", _env__STACKTOP_, _env__STACKTOP_);
    debug("_env__STACK_MAX_: 0x%x (%d)\n", _env__STACK_MAX_, _env__STACK_MAX_);
    debug("STACK_BASE: 0x%x (%d)\n", STACK_BASE, STACK_BASE);
    debug("_env__memory_: %p\n", _env__memory_);

    // Load the module
    Module *m = load_module(mod_path);

    // point to internal malloc
    _malloc_fidx = get_export_fidx(m, "_malloc");
    debug("_malloc_fidx: 0x%x (%d)\n", _malloc_fidx, _malloc_fidx);

    // setup argc/argv

    // Invoke main/_main function and exit
    res = invoke(m, NULL, argc-2, argv+2);

    if (!res) {
        error("Exception: %s\n", exception);
        exit(1);
    }

    if (m->sp >= 0) {
        exit(m->stack[m->sp--].value.uint32);
    } else {
        exit(0);
    }

}
