#include <stdint.h>
#include <math.h>
#include <string.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

#include "util.h"
#include "wa.h"


/////////////////////////////////////////////////////////
// emscripten memory globals

#define TOTAL_STACK   0x500000   // 5242880 bytes
//#define TOTAL_MEMORY  0xffffffff // 4GB-1
//#define TOTAL_MEMORY  0xc0000000 // 3GB
//#define TOTAL_MEMORY  0x80000000 // 2GB
#define TOTAL_MEMORY  0x1000000 // 16MB
#define TOTAL_TABLE   256
//#define STATIC_BASE   1024
#define STATIC_BASE   0
#define STATIC_BUMP   50256
#define STATIC_TOP    STATIC_BASE + STATIC_BUMP

/////////////////////////////////////////////////////////
// emscripten global variables

double    _global__NaN_         = NAN;
double    _global__Infinity_    = INFINITY;

uint8_t  *_env__memory_ = 0;
uint8_t  *_env__memoryBase_;

uint8_t  *_env__DYNAMICTOP_PTR_;
uint8_t  *_env__tempDoublePtr_;
uint8_t  *_env__STACKTOP_;
uint8_t  *_env__STACK_MAX_;


uint32_t  _REAL_TABLE_[TOTAL_TABLE];
uint32_t *_env__table_;
uint32_t *_env__tableBase_;

uint8_t  *gb;
uint32_t fb;


/////////////////////////////////////////////////////////
// Memory utility functions

int emscripten_init() {
    _env__memoryBase_ = calloc(TOTAL_MEMORY, 1);
    gb = _env__memoryBase_+STATIC_BASE;

    _env__table_ = _REAL_TABLE_;
    _env__tableBase_ = 0;
    fb = 0;

    _env__tempDoublePtr_ = _env__memoryBase_ + STATIC_TOP;

    _env__DYNAMICTOP_PTR_ = _env__memoryBase_ + STATIC_TOP + 16;


    _env__STACKTOP_ = _env__memoryBase_ + STATIC_TOP + 32;
    _env__STACK_MAX_ = _env__STACKTOP_ + TOTAL_STACK;

    *(uint32_t *)_env__DYNAMICTOP_PTR_ = (uint32_t)_env__STACK_MAX_;

    info("emscripten_init results:\n");
    info("  _env__memory_: %p (0x%x)\n", _env__memory_, _env__memory_);
    info("  _env__memoryBase_: %p\n", _env__memoryBase_);
    info("  _env__table_: %p, _env__tableBase_: %p\n", _env__table_, _env__tableBase_);
    info("  STACK_MAX: %p\n", _env__STACK_MAX_);
    info("  DYNAMICTOP_PTR: %p, *DYNAMICTOP_PTR: %p\n",
            _env__DYNAMICTOP_PTR_, *(uint32_t *)_env__DYNAMICTOP_PTR_);
    info("  gb: 0x%x (%d), fb: 0x%x (%d)\n", gb, gb, fb, fb);

}

/////////////////////////////////////////////////////////

// Some general wrappers

void pthread_cleanup_push(uint32_t routine, uint32_t arg) {
    FATAL("pthread_cleanup_push unimplemented\n");
}

void pthread_cleanup_pop(uint32_t _) {
    FATAL("pthread_cleanup_pop unimplemented\n");
}

void ___lock(uint32_t _) {
    return; // nop
}

void ___unlock(uint32_t _) {
    return; // nop
}


void lock(uint32_t _) {
    return; // nop
}

void unlock(uint32_t _) {
    return; // nop
}

void _abort() {
    FATAL("_abort");
}

#include <time.h>
uint32_t _env___clock_gettime_(uint32_t clk_id, struct timespec *tp) {
    if (TRACE) {
        warn("    _clock_gettime clk_id: %d, tp: %p\n", clk_id, tp);
    }
    return clock_gettime(clk_id, tp);
}

#include <signal.h>
uint32_t _signal(uint32_t a, uint32_t b) {
    FATAL("_signal unimplemented\n");
}

int _sigaction(int signum, const struct sigaction *act,
               struct sigaction *oldact) {
    if (TRACE) { warn("    _sigaction signum: %d, act: %p, oldact: %p\n",
                      signum, act, oldact); }
    return sigaction(signum, act, oldact);
}

uint32_t *_dlsym(uint32_t *handle, uint32_t *symbol) {
    FATAL("_dlsym unimplemented, handle: %p, symbol: %p\n",
          handle, symbol);
}

uint32_t *_dlerror() {
    FATAL("_dlerror unimplemented\n");
}

uint32_t _dlclose(uint32_t a) {
    FATAL("_dlclose unimplemented\n");
}

uint32_t _nanosleep(uint32_t a, uint32_t b) {
    FATAL("_nanosleep unimplemented\n");
}

uint32_t _gettimeofday(uint32_t a, uint32_t b) {
    FATAL("_gettimeofday unimplemented\n");
}

char *_getenv(const char *name) {
    if (TRACE) { warn("    _getenv name: %s\n", name); }
    return getenv(name);
}


// Syscall wrappers
// /usr/include/asm/unistd_32.h

// open
uint32_t ___syscall5(uint32_t which, uint32_t *varargs) {
    FATAL("    syscall5 which: %d, varargs: 0x%x\n", which, varargs);
}

// close
uint32_t ___syscall6(uint32_t which, uint32_t *varargs) {
    FATAL("    syscall6 which: %d, varargs: 0x%x\n", which, varargs);
}

// ioctl
uint32_t ___syscall54(uint32_t which, uint32_t *varargs) {
    if (TRACE) {
        warn("    syscall54 which: %d, varargs: 0x%x\n", which, varargs);
    }
    return 0;
}

// _llseek
uint32_t ___syscall140(uint32_t which, uint32_t *varargs) {
    if (TRACE) {
        warn("    syscall140 which: %d, varargs: 0x%x\n", which, varargs);
    }
    return syscall(which, varargs);
}

#include <sys/uio.h>
// readv
uint32_t ___syscall145(uint32_t which, uint32_t *varargs) {
    FATAL("    syscall145 which: %d, varargs: 0x%x\n", which, varargs);
}

// writev
uint32_t ___syscall146(uint32_t which, uint32_t *varargs) {
    if (TRACE) {
        warn("    syscall146 which: %d, varargs: 0x%x\n", which, varargs);
    }
    uint32_t  fd = varargs[0];
    void     *iov = (void *)varargs[1];
    uint32_t  iovcnt = varargs[2];
    uint32_t  res;

    if (TRACE) {
        warn("      fd: %d, iov: %p, iovcnt: %d\n", fd, iov, iovcnt);
    }

    res = writev(fd, iov, iovcnt);
    if (TRACE) { warn("      res: %d\n", res); }
    return res;
    //res = syscall(146, fd, iov, iovcnt);
}

// fcntl64
uint32_t ___syscall221(uint32_t which, uint32_t *varargs) {
    FATAL("    syscall221 which: %d, varargs: 0x%x\n", which, varargs);
}

// Some emscripten utilities

uint32_t _asm2wasm__f64_to_int_(double a) {
    return (uint32_t)a;
}

uint32_t getTotalMemory() {
    return TOTAL_MEMORY;
}

uint32_t abortOnCannotGrowMemory() {
    FATAL("Cannot enlarge memory arrays. Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value 0x%x, (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which adjusts the size at runtime but prevents some optimizations, (3) set Module.TOTAL_MEMORY to a higher value before the program runs, or if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0\n",
          TOTAL_MEMORY);
}

uint32_t enlargeMemory() {
    return abortOnCannotGrowMemory();
}


void ___setErrNo(uint32_t value) {
    if (TRACE) {
        warn("    >>><<< ___setErrNo value: %d\n", value);
    }
    return;
}

uint32_t setTempRet0(uint32_t value) {
    FATAL("setTempRet0 unimplemented\n");
}

uint32_t getTempRet0() {
    FATAL("getTempRet0 unimplemented\n");
}

// emscripten/SDL/GL wrappers are in em.c

/////////////////////////////////////////////////////////

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

    emscripten_init();

    // Load the module
    Options opts = { .disable_memory_bounds=true };
    Module *m = load_module(mod_path, opts);

    // TODO: setup argc/argv

    // runPostSets (emscripten initialization)
    res = invoke(m, "runPostSets", 0, 0);

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
