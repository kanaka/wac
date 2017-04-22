#include <stdint.h>

#include "util.h"
#include "wa.h"

/////////////////////////////////////////////////////////
// emscripten memory

#define TOTAL_MEMORY  0x1000000 // 16MB
#define TOTAL_TABLE   256

uint8_t  *_env__memory_ = 0;
uint8_t  *_env__memoryBase_;

uint32_t  _WA_REAL_TABLE_[TOTAL_TABLE];
uint32_t *_env__table_;
uint32_t *_env__tableBase_;


// Initialize memory variables
int emscripten_init() {
    _env__memoryBase_ = calloc(TOTAL_MEMORY, 1);

    _env__table_ = _WA_REAL_TABLE_;
    _env__tableBase_ = 0;

    info("emscripten_init results:\n");
    info("  _env__memory_: %p (0x%x)\n", _env__memory_, _env__memory_);
    info("  _env__memoryBase_: %p\n", _env__memoryBase_);
    info("  _env__table_: %p\n", _env__table_);
    info("  _env__tableBase_: 0x%x\n", _env__tableBase_);

}

/////////////////////////////////////////////////////////
// General wrappers

#include <stdarg.h>
int _env___printf_(const char * fmt, va_list args) {
    vprintf(fmt, args);
}

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
    Options opts = { .disable_memory_bounds=true,
                     .dlsym_trim_underscore=true };
    Module *m = load_module(mod_path, opts);

    // TODO: setup argc/argv

    // emscripten initialization
    res = invoke(m, "__post_instantiate", 0, 0);

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
