#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include "util.h"

// Assert calloc
void *acalloc(size_t nmemb, size_t size,  char *name) {
    void *res = calloc(nmemb, size);
    if (res == NULL) {
        FATAL("Could not allocate %ul bytes for %s", nmemb * size, name);
    }
    return res;
}

// Assert realloc/calloc
void *arecalloc(void *ptr, size_t old_nmemb, size_t nmemb,
                size_t size,  char *name) {
    void *res = realloc(ptr, nmemb * size);
    if (res == NULL) {
        FATAL("Could not allocate %ul bytes for %s", nmemb * size, name);
    }
    // Initialize new memory
    memset(res + old_nmemb * size, 0, (nmemb - old_nmemb) * size);
    return res;
}

//
// Some extra lirary routines
//

// open and mmap a file
uint8_t *mmap_file(char *path, int *len) {
    int          fd;
    int          res;
    struct stat  sb;
    uint8_t     *bytes;

    fd = open(path, O_RDONLY);
    if (fd < 0) { FATAL("could not open file '%s'\n", path); }
    res = fstat(fd, &sb);
    if (res < 0) { FATAL("could not stat file '%s' (%d)\n", path, res); }

    bytes = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (len) {
        *len = sb.st_size;  // Return length if requested
    }
    if (bytes == MAP_FAILED) { FATAL("could not mmap file '%s'", path); }
    return bytes;
}

int read_file(char *path, char *buf) {
    int          fd;
    struct stat  sb;
    size_t       len;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("could not open file '%s'\n", path);
        return 0;
    }
    if (fstat(fd, &sb) < 0) {
        printf("could stat file '%s'\n", path);
        return 0;
    }

    len = read(fd, buf, sb.st_size);
    if (len < sb.st_size) {
        printf("failed to read all of '%s'\n", path);
        return 0;
    }

    return len;
}

//
// Dynamic lib resolution
//

// If filename is NULL, a NULL handle will be used
// Returns true if resolution successful
// Return false and sets err if resolution is not successful
bool resolvesym(char *filename, char *symbol, void **val, char **err) {
    void *handle = NULL;
    dlerror(); // clear errors
    //log("filename: %s, symbol: %s\n", filename, symbol);
    if (filename) {
        handle = dlopen(filename, RTLD_LAZY);
        if (!handle) {
            *err = dlerror();
            return false;
        }
    }
    *val = dlsym(handle, symbol);
    //log("    val: 0x%p\n", *val);
    if ((*err = dlerror()) != NULL) {
        return false;
    }
    return true;
}

