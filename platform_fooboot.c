#include <console.h>
#include <serial.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>

#include <vfs.h>
#include <sym_table.h>

#include "readline_buf.h"
#include "util.h"

extern uint8_t  *memory;
extern uint8_t  *memoryBase;


// Assert calloc
void *acalloc(size_t nmemb, size_t size,  char *name) {
    void *res = calloc(nmemb, size);
    if (res == NULL) {
        FATAL("Could not allocate %d bytes for %s", (int)(nmemb * size), name);
    }
    //printf("<<< acalloc res: %p\n", res);
    return res;
}

// Assert realloc/calloc
void *arecalloc(void *ptr, size_t old_nmemb, size_t nmemb,
                size_t size,  char *name) {
    void *res = calloc(nmemb, size);
    if (res == NULL) {
        FATAL("Could not allocate %d bytes for %s", (int)(nmemb * size), name);
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
    return readline_buf(prompt, buf, cnt);
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

