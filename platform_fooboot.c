#include <console.h>
#include <serial.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>

#include <vfs.h>
#include <sym_table.h>

#include "util.h"

extern uint8_t  *memory;
extern uint8_t  *memoryBase;


// Assert calloc
void *acalloc(size_t nmemb, size_t size,  char *name) {
    void *res = calloc(nmemb, size);
    if (res == NULL) {
        FATAL("Could not allocate %d bytes for %s", (int)nmemb * size, name);
    }
    //printf("<<< acalloc res: %p\n", res);
    return res;
}

// Assert realloc/calloc
void *arecalloc(void *ptr, size_t old_nmemb, size_t nmemb,
                size_t size,  char *name) {
    void *res = calloc(nmemb, size);
    if (res == NULL) {
        FATAL("Could not allocate %d bytes for %s", (int)nmemb * size, name);
    }
    memmove(res, ptr, old_nmemb * size);
    // Initialize new memory
    memset(res + old_nmemb * size, 0, (nmemb - old_nmemb) * size);
    //printf("<<< arecalloc res: %p\n", res);
    return res;
}

//
// Some extra library routines
//

// Returns true if line successfully read into buf, false for EOF
bool readline(const char *prompt, char *buf, int cnt) {
    //printf(">>> _readline prompt: '%s' (%p), buf: %p\n", prompt, prompt, buf);
    char c;
    int idx = 0;
    console_write(prompt);
    while (idx < cnt) {
        c = console_getc();
        if (c == 0x4) {
            if (idx == 0) return false;
            continue;
        }
        // Backspace (0x08 from FB console, 0x7f from serial console)
        if (c == 0x08 || c == 0x7f) {
            if (idx > 0) {
                idx--;
                console_putc(0x08);
                console_putc(' ');
                console_putc(0x08);
            }
            continue;
        }
        console_putc(c);
        if (c == '\n') {
            break;
        }
        buf[idx++] = c;
    };
    buf[idx] = '\0';
    //printf("<<< _readline buf: '%s' (%p)\n", buf, buf);
    return true;
}

void add_history(char *line) {
    return;
}

//int _gettimeofday(timeval *tv, struct timezone *tz) {
int _gettimeofday(void *tv, void *tz) {
    // TODO: implement
    return -1;
}

int read_file(char *path, char *buf) {
    return vfs_read_file(path, buf);
}

//
// Dynamic lib resolution
//

bool resolvesym(char *filename, char *symbol, void **val, char **err) {
    //printf("resolvesym filename: '%s', symbol: '%s'\n", filename, symbol);
    if (filename && strcmp(filename, "env") != 0) {
        *err = "resolvesym with filename unimplmented";
        return false;
    }
    *val = sym_table_lookup(symbol);
    //printf("           *val: %p\n", *val);
    return true;
}

