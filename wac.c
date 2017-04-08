#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#ifdef USE_READLINE
  // WARNING: GPL license implications
  #include <readline/readline.h>
  #include <readline/history.h>
#else
  #include <editline/readline.h>
#endif

#include "util.h"
#include "wa.h"

void usage() {
    fprintf(stderr, "./wac [--debug] WASM_FILE [--repl|-- ARG...]\n");
    exit(2);
}


// Special test imports
uint32_t _spectest__global_ = 666;

void _spectest__print_(uint32_t val) {
    //printf("spectest.print 0x%x:i32\n", val);
    printf("0x%x:i32\n", val);
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
        case '?': usage(); break;
        default: usage();
        }
    }
    if (optind >= argc) { usage(); }
    mod_path = argv[optind++];

    if (debug) {
        printf("repl: %d, debug: %d, module path: %s\n",
               repl, debug, mod_path);
    }

    // Load the module
    Module *m = load_module(mod_path);

    if (!repl) {
        // Invoke one function and exit
        res = invoke(m, argc-optind, argv+optind);
    } else {
        // Simple REPL
        if (optind < argc) { usage(); }
        while (line = readline("webassembly> ")) {
            int token_cnt = 0;
            char **tokens = split_string(line, &token_cnt);
            if (token_cnt == 0) { continue; }

            res = invoke(m, token_cnt, tokens);
            free(tokens);
        }
    }
    exit(res);
}
