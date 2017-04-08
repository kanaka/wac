#include <stdint.h>
#include <math.h>

#include "util.h"

uint32_t TOTAL_MEMORY = 16777216;

uint32_t _env__DYNAMICTOP_PTR_ = 0;
uint32_t _env__tempDoublePtr_  = 0;
uint32_t _env__ABORT_          = 0;
uint32_t _env__STACKTOP_       = 0;
uint32_t _env__STACK_MAX_      = 0;
double   _global__NaN_         = NAN;
double   _global__Infinity_    = INFINITY;

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
    FATAL("abort");
}

void _env_____setErrNo_(uint32_t value) {
    return;
}

uint32_t _env_____syscall6_(uint32_t which, uint32_t varargs) {
    FATAL("___syscall6 unimplemented\n");
}

uint32_t _env_____syscall140_(uint32_t which, uint32_t varargs) {
    FATAL("___syscall140 unimplemented\n");
}

uint32_t _env_____syscall54_(uint32_t which, uint32_t varargs) {
    FATAL("___syscall154 unimplemented\n");
}

uint32_t _env_____syscall146_(uint32_t which, uint32_t varargs) {
    FATAL("___syscall146 unimplemented\n");
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

uint8_t _env__memory_[256*65536]; // 2**16

/*
_env__table_ (table (;0;) 18 18 anyfunc))
*/

uint32_t _env__memoryBase_   = 0;
uint32_t _env__tableBase_    = 0;

