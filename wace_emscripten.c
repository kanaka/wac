#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <limits.h>
#include <math.h>

// Call table/trapping table lookups/execution
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

#include "util.h"
#include "wa.h"

/////////////////////////////////////////////////////////
// emscripten memory layout

#define PAGE_COUNT   256 // 64K * 256 = 16MB
#define TOTAL_PAGES  ULONG_MAX / PAGE_SIZE
#define TABLE_COUNT  256

Memory _env__memory_ = {TOTAL_PAGES, TOTAL_PAGES, TOTAL_PAGES, NULL};
uint8_t  *_env__memoryBase_;

Table     _env__table_ = {ANYFUNC, TABLE_COUNT, TABLE_COUNT, TABLE_COUNT, 0};
//uint32_t *_env__table_ = 0;
uint32_t *_env__tableBase_;

double    _global__NaN_         = NAN;
double    _global__Infinity_    = INFINITY;

uint32_t **_env__DYNAMICTOP_PTR_;
uint32_t *_env__tempDoublePtr_;


// Initialize function/jump table
void segv_thunk_handler(int cause, siginfo_t * info, void *uap) {
    int index = (info->si_addr - (void *)_env__table_.entries);
    if (info->si_addr >= (void *)_env__table_.entries &&
        (info->si_addr - (void *)_env__table_.entries) < TABLE_COUNT) {
        uint32_t fidx = _env__table_.entries[index];
        ucontext_t *context = uap;
        void (*f)(void);
        f = setup_thunk_in(fidx);
        // Test/debug only (otherwise I/O should be avoided in a signal handlers)
        //printf("SIGSEGV raised at address %p, index: %d, fidx: %d\n",
        //        info->si_addr, index, fidx);

        // On Linux x86, general register 14 is EIP
        context->uc_mcontext.gregs[14] = (unsigned int)f;
    } else {
        // What to do here?
    }
}

void init_thunk_in_trap() {
    // Trap on sigsegv
    struct sigaction sa;
    sa.sa_sigaction = segv_thunk_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset (&sa.sa_mask);
    if (sigaction (SIGSEGV, &sa, 0)) {
	perror ("sigaction");
	exit(1);
    }

    // Allow READ/WRITE but prevent execute. This only works if PROT_EXEC does
    // in fact trap
    debug("mprotect on table at: %p\n", _env__table_.entries);
    if (mprotect(_env__table_.entries, TABLE_COUNT*sizeof(uint32_t),
                 PROT_READ | PROT_WRITE)) {
        perror("mprotect");
        exit(1);
    }
}

// Initialize memory globals and function/jump table
void init_wace() {
    _env__memoryBase_ = calloc(PAGE_COUNT, PAGE_SIZE);

    _env__tempDoublePtr_ = (uint32_t*)_env__memoryBase_;
    _env__DYNAMICTOP_PTR_ = (uint32_t**)(_env__memoryBase_ + 16);

    *_env__DYNAMICTOP_PTR_ = (uint32_t*)(_env__memoryBase_ + PAGE_COUNT * PAGE_SIZE);

    // This arrangement correlates to the module mangle_table_offset option
    if (posix_memalign((void **)&_env__table_.entries, sysconf(_SC_PAGESIZE),
                       TABLE_COUNT*sizeof(uint32_t))) {
        perror("posix_memalign");
        exit(1);
    }
    _env__tableBase_ = _env__table_.entries;

    info("init_mem results:\n");
    info("  _env__memory_.bytes: %p\n", _env__memory_.bytes);
    info("  _env__memoryBase_: %p\n", _env__memoryBase_);
    info("  _env__DYNAMIC_TOP_PTR_: %p\n", _env__DYNAMICTOP_PTR_);
    info("  *_env__DYNAMIC_TOP_PTR_: %p\n", *_env__DYNAMICTOP_PTR_);
    info("  _env__table_.entries: %p\n", _env__table_.entries);
    info("  _env__tableBase_: 0x%x\n", _env__tableBase_);

    init_thunk_in_trap();
}


/////////////////////////////////////////////////////////
// General globals/imports

uint32_t _env__ABORT_ = 0;

#include <stdarg.h>
int _env___printf_(const char * fmt, va_list args) {
    return vprintf(fmt, args);
}

void _env__abortStackOverflow_(uint32_t x) {
    FATAL("_env__abortStackOverflow 0x%x\n", x);
}

void _env__nullFunc_X_(uint32_t x) {
    FATAL("_env__nullFunc_X_ 0x%x\n", x);
}
