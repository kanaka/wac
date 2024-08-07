#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//
// Some extra lirary routines
//

// Returns true if line successfully read into buf, false for EOF
extern char * __real_readline (const char *prompt);
bool __wrap_readline(const char *prompt, char *buf, int cnt) {
    // Use GNU Readline to get the input with prompt
    char *input = __real_readline(prompt);

    if (input == NULL) {
        return false; // EOF
    }

    // Copy it into buf and free input
    snprintf(buf, cnt, "%s", input);
    free(input);

    return true;
}

