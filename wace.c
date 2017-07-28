#include <stdint.h>
#include <math.h>

// Call table/trapping table lookups/execution
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/ucontext.h>

#include "util.h"
#include "wa.h"

void usage(char *prog) {
    fprintf(stderr, "%s [--debug] WASM_FILE [-- ARG...]\n", prog);
    exit(2);
}

/////////////////////////////////////////////////////////
// emscripten memory

#define TOTAL_MEMORY  0x1000000 // 16MB
#define TOTAL_TABLE   65536

uint8_t  *_env__memory_ = 0;
uint8_t  *_env__memoryBase_;

uint32_t *_env__table_ = 0;
uint32_t *_env__tableBase_;

double    _global__NaN_         = NAN;
double    _global__Infinity_    = INFINITY;

uint32_t **_env__DYNAMICTOP_PTR_;
uint32_t *_env__tempDoublePtr_;


uint32_t _env__STACKTOP_ = 0;
uint32_t _env__STACK_MAX_ = STACK_SIZE;  // CALLSTACK_SIZE?



// Initialize memory globals and function/jump table
void emscripten_init() {
    _env__memoryBase_ = calloc(TOTAL_MEMORY, sizeof(uint8_t));
    _env__memory_ = _env__memoryBase_;

    //_env__tableBase_ = calloc(TOTAL_TABLE, sizeof(uint32_t));

    _env__table_ = calloc(TOTAL_TABLE, sizeof(uint32_t));
    _env__tableBase_ = _env__table_;

    // This arrangement correlates to the module mangle_table_offset option
    if (posix_memalign((void **)&_env__table_, sysconf(_SC_PAGESIZE),
                       TOTAL_TABLE*sizeof(uint32_t))) {
        perror("posix_memalign");
        exit(1);
    }
    _env__tableBase_ = _env__table_;

    _env__tempDoublePtr_ = (uint32_t*)_env__memoryBase_;
    _env__DYNAMICTOP_PTR_ = (uint32_t**)(_env__memoryBase_ + 16);

    *_env__DYNAMICTOP_PTR_ = (uint32_t*)(_env__memoryBase_ + TOTAL_MEMORY);


    info("emscripten_init results:\n");
    info("  _env__memory_: %p (0x%x)\n", _env__memory_, _env__memory_);
    info("  _env__memoryBase_: %p\n", _env__memoryBase_);
    info("  _env__DYNAMIC_TOP_PTR_: %p\n", _env__DYNAMICTOP_PTR_);
    info("  *_env__DYNAMIC_TOP_PTR_: %p\n", *_env__DYNAMICTOP_PTR_);
    info("  _env__table_: %p\n", _env__table_);
    info("  _env__tableBase_: 0x%x\n", _env__tableBase_);


}

void segv_thunk_handler(int cause, siginfo_t * info, void *uap) {
    int index = (info->si_addr - (void *)_env__table_);
    if (info->si_addr >= (void *)_env__table_ &&
        (info->si_addr - (void *)_env__table_) < TOTAL_TABLE) {
        uint32_t fidx = _env__table_[index];
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

void thunk_in_trap_init(Module *m) {
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
    debug("mprotect on table at: %p\n", _env__table_);
    if (mprotect(_env__table_, TOTAL_TABLE*sizeof(uint32_t),
                 PROT_READ | PROT_WRITE)) {
        perror("mprotect");
        exit(1);
    }
}


/////////////////////////////////////////////////////////
// General globals/imports

uint32_t _env__ABORT_ = 0;

#include <stdarg.h>
int _env___printf_(const char * fmt, va_list args) {
    vprintf(fmt, args);
}

void _env__abortStackOverflow_(uint32_t x) {
    FATAL("_env__abortStackOverflow 0x%x\n", x);
}

void _env__nullFunc_X_(uint32_t x) {
    FATAL("_env__nullFunc_X_ 0x%x\n", x);
}


void* _env___malloc_(uint32_t size) {
    void *m = malloc(size);
    printf("malloc(%d) => %d \n", size, m);
    return m;
}

#define DECLARE_DUMMY0(ret_t, name) ret_t _env__##name##_() { FATAL("Calling undefined function: %s", #name); }
#define DECLARE_DUMMY1(ret_t, name, a1_t) ret_t _env__##name##_(a1_t a1) { FATAL("Calling undefined function: %s(%d)", #name, a1); }
#define DECLARE_DUMMY2(ret_t, name, a1_t, a2_t) ret_t _env__##name##_(a1_t a1, a2_t a2) { FATAL("Calling undefined function: %s(%d, %d)", #name, a1, a2); }
#define DECLARE_DUMMY3(ret_t, name, a1_t, a2_t, a3_t) ret_t _env__##name##_(a1_t a1, a2_t a2, a3_t a3) { FATAL("Calling undefined function: %s(%d, %d, %d)", #name, a1, a2, a3); }
#define DECLARE_DUMMY4(ret_t, name, a1_t, a2_t, a3_t, a4_t) ret_t _env__##name##_(a1_t a1, a2_t a2, a3_t a3, a4_t a4) { FATAL("Calling undefined function: %s(%d, %d, %d, %d)", #name, a1, a2, a3, a4); }
#define DECLARE_DUMMY5(ret_t, name, a1_t, a2_t, a3_t, a4_t, a5_t) ret_t _env__##name##_(a1_t a1, a2_t a2, a3_t a3, a4_t a4, a5_t a5) { FATAL("Calling undefined function: %s(%d, %d, %d, %d, %d)", #name, a1, a2, a3, a4, a5); }

DECLARE_DUMMY0(int32_t, enlargeMemory)
DECLARE_DUMMY0(int32_t, getTotalMemory)
DECLARE_DUMMY0(int32_t, abortOnCannotGrowMemory)
//DECLARE_DUMMYv1(abortStackOverflow, int32_t)

DECLARE_DUMMY1(int32_t, invoke_i, int32_t)
DECLARE_DUMMY2(int32_t, invoke_ii, int32_t, int32_t)
DECLARE_DUMMY3(int32_t, invoke_iii, int32_t, int32_t, int32_t)
DECLARE_DUMMY4(int32_t, invoke_iiii, int32_t, int32_t, int32_t, int32_t)
DECLARE_DUMMY1(void, invoke_v, int32_t)
DECLARE_DUMMY2(void, invoke_vi, int32_t, int32_t)
DECLARE_DUMMY3(void, invoke_vii, int32_t, int32_t, int32_t)
DECLARE_DUMMY4(void, invoke_viii, int32_t, int32_t, int32_t, int32_t)
DECLARE_DUMMY5(void, invoke_viiii, int32_t, int32_t, int32_t, int32_t, int32_t)

DECLARE_DUMMY2(int32_t, _pthread_cond_wait, int32_t, int32_t)
DECLARE_DUMMY2(int32_t, _pthread_key_create, int32_t, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_rwlock_unlock, int32_t)
DECLARE_DUMMY2(int32_t, _pthread_cond_init, int32_t, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_mutexattr_destroy, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_key_delete, int32_t)
DECLARE_DUMMY2(int32_t, _pthread_condattr_setclock, int32_t, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_getspecific, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_rwlock_rdlock, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_cond_signal, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_mutex_destroy, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_condattr_init, int32_t)
DECLARE_DUMMY2(int32_t, _pthread_mutexattr_settype, int32_t, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_condattr_destroy, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_mutexattr_init, int32_t)
DECLARE_DUMMY2(int32_t, _pthread_setspecific, int32_t, int32_t)
DECLARE_DUMMY1(int32_t, _pthread_cond_destroy, int32_t)
DECLARE_DUMMY2(int32_t, _pthread_mutex_init, int32_t, int32_t)

DECLARE_DUMMY1(void, _abort, int32_t)
DECLARE_DUMMY3(int32_t, _emscripten_memcpy_big, int32_t, int32_t, int32_t)
DECLARE_DUMMY1(int32_t, _getenv, int32_t)
DECLARE_DUMMY2(int32_t, _dladdr, int32_t, int32_t)
DECLARE_DUMMY0(void, _llvm_trap)

DECLARE_DUMMY1(int32_t, __Unwind_FindEnclosingFunction, int32_t)
DECLARE_DUMMY2(int32_t, __Unwind_GetIPInfo, int32_t, int32_t)
DECLARE_DUMMY2(int32_t, __Unwind_Backtrace, int32_t, int32_t)

//DECLARE_DUMMY5(int32_t, ___gxx_personality_v0, int32_t, int32_t, int64_t, int32_t, int32_t)
DECLARE_DUMMY5(int32_t, ___gxx_personality_v0, int32_t, int32_t, int32_t, int32_t, int32_t)
DECLARE_DUMMY0(int32_t, ___cxa_find_matching_catch_2)
DECLARE_DUMMY1(int32_t, ___cxa_find_matching_catch_3, int32_t)
DECLARE_DUMMY1(void, ___cxa_free_exception, int32_t)
DECLARE_DUMMY3(void, ___cxa_throw, int32_t, int32_t, int32_t)
DECLARE_DUMMY1(void, ___setErrNo, int32_t)
DECLARE_DUMMY1(int32_t, ___cxa_allocate_exception, int32_t)
DECLARE_DUMMY1(void, ___resumeException, int32_t)
DECLARE_DUMMY1(void, ___unlock, int32_t)
DECLARE_DUMMY1(void, ___lock, int32_t)

DECLARE_DUMMY2(int32_t, ___syscall4, int32_t, int32_t)
DECLARE_DUMMY2(int32_t, ___syscall6, int32_t, int32_t)
DECLARE_DUMMY2(int32_t, ___syscall54, int32_t, int32_t)
DECLARE_DUMMY2(int32_t, ___syscall140, int32_t, int32_t)
DECLARE_DUMMY2(int32_t, ___syscall146, int32_t, int32_t)


#define EXPORT(name) h = declare_host_export(h, "env." #name, &_env__##name##_)


HostExport* exports_init() {
    HostExport *h = NULL;
    // This are the minimum exports to make the hello world examples run.
    // Of course, SDL and other stuff is broken without explicit exports.
    EXPORT(memory);
    EXPORT(table);
    EXPORT(memoryBase);
    EXPORT(tableBase);
    //EXPORT(_puts);
    h = declare_host_export(h, "env._puts", &puts);
    EXPORT(_malloc);
    EXPORT(_printf);

    // Exports required for emscripten...
    // much todo...
    EXPORT(DYNAMICTOP_PTR);
    EXPORT(tempDoublePtr);
    EXPORT(ABORT);
    EXPORT(STACKTOP);
    EXPORT(STACK_MAX);

    EXPORT(enlargeMemory);
    EXPORT(getTotalMemory);
    EXPORT(abortOnCannotGrowMemory);
    EXPORT(abortStackOverflow);

    h = declare_host_export(h, "env.nullFunc_i", &_env__nullFunc_X_);
    h = declare_host_export(h, "env.nullFunc_ii", &_env__nullFunc_X_);
    h = declare_host_export(h, "env.nullFunc_iii", &_env__nullFunc_X_);
    h = declare_host_export(h, "env.nullFunc_iiii", &_env__nullFunc_X_);
    h = declare_host_export(h, "env.nullFunc_v", &_env__nullFunc_X_);
    h = declare_host_export(h, "env.nullFunc_vi", &_env__nullFunc_X_);
    h = declare_host_export(h, "env.nullFunc_vii", &_env__nullFunc_X_);
    h = declare_host_export(h, "env.nullFunc_viii", &_env__nullFunc_X_);
    h = declare_host_export(h, "env.nullFunc_viiii", &_env__nullFunc_X_);
    h = declare_host_export(h, "env.nullFunc_ji", &_env__nullFunc_X_);

    EXPORT(invoke_i);
    EXPORT(invoke_ii);
    EXPORT(invoke_iii);
    EXPORT(invoke_iiii);
    EXPORT(invoke_v);
    EXPORT(invoke_vi);
    EXPORT(invoke_vii);
    EXPORT(invoke_viii);
    EXPORT(invoke_viiii);

    EXPORT(_pthread_cond_wait);
    EXPORT(_pthread_key_create);
    EXPORT(_pthread_rwlock_unlock);
    EXPORT(_pthread_cond_init);
    EXPORT(_pthread_mutexattr_destroy);
    EXPORT(_pthread_key_delete);
    EXPORT(_pthread_condattr_setclock);
    EXPORT(_pthread_getspecific);
    EXPORT(_pthread_rwlock_rdlock);
    EXPORT(_pthread_cond_signal);
    EXPORT(_pthread_mutex_destroy);
    EXPORT(_pthread_condattr_init);
    EXPORT(_pthread_mutexattr_settype);
    EXPORT(_pthread_condattr_destroy);
    EXPORT(_pthread_mutexattr_init);
    EXPORT(_pthread_setspecific);
    EXPORT(_pthread_cond_destroy);
    EXPORT(_pthread_mutex_init);

    EXPORT(_abort);
    EXPORT(_emscripten_memcpy_big);
    EXPORT(_getenv);
    EXPORT(_dladdr);
    EXPORT(_llvm_trap);

    EXPORT(__Unwind_FindEnclosingFunction);
    EXPORT(__Unwind_GetIPInfo);
    EXPORT(__Unwind_Backtrace);

    EXPORT(___gxx_personality_v0);
    EXPORT(___cxa_find_matching_catch_2);
    EXPORT(___cxa_find_matching_catch_3);
    EXPORT(___cxa_free_exception);
    EXPORT(___cxa_throw);
    EXPORT(___setErrNo);
    EXPORT(___cxa_allocate_exception);
    EXPORT(___resumeException);
    EXPORT(___unlock);
    EXPORT(___lock);

    EXPORT(___syscall4);
    EXPORT(___syscall6);
    EXPORT(___syscall54);
    EXPORT(___syscall140);
    EXPORT(___syscall146);

    h = declare_host_export(h, "global.NaN", &_global__NaN_);
    h = declare_host_export(h, "global.Infinity", &_global__Infinity_);
    return h;
}

/////////////////////////////////////////////////////////
// Command line

int main(int argc, char **argv) {
    char   *mod_path;
    int     res = 0;

    if (argc < 2) { usage(argv[0]); }
    mod_path = argv[1];

    HostExport *exports = exports_init();
    emscripten_init();

    // Load the module
    Options opts = { .disable_memory_bounds = true,
                     .mangle_table_index    = true,
                     .dlsym_trim_underscore = true };
    Module *m = load_module(mod_path, opts, exports);

    thunk_in_trap_init(m);

    // emscripten initialization
    res = invoke(m, "__post_instantiate", 0, 0);

    // setup argc/argv
    m->stack[++m->sp].value_type = I32;
    m->stack[m->sp].value.uint32 = argc-1;
    m->stack[++m->sp].value_type = I32;
    m->stack[m->sp].value.uint32 = (uint32_t)(argv+1);

    // Invoke main/_main function and exit
    res = invoke(m, NULL, 0, 0);

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
