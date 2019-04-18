#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <sys/uio.h>
#include <fcntl.h>
#include <time.h>

#include <string.h>

#include "util.h"
#include "wa.h"

//
// WASI implemenation (https://wasi.dev)
//

// Memory and utilities
Memory *wasi_memory = NULL;

int          host_argc = 0;
char       **host_argv = NULL;
int          dirfd_offset = 0;

#define MAX_IOV 128
struct iovec host_iov[MAX_IOV];

void init_wasi(Memory *memory, int argc, char **argv) {
    // Save memory
    wasi_memory = memory;

    // save argc/argv for WASI args_* calls
    host_argc = argc;
    host_argv = argv;

    // open default dirfd file descriptors
    int fd = open("./", O_RDONLY);
    if (fd < 0)                    { FATAL("opening '%s': %s", "./", strerror(errno)); }
    if (open("../", O_RDONLY) < 0) { FATAL("opening '%s': %s", "../", strerror(errno)); }
    if (open("/", O_RDONLY) < 0)   { FATAL("opening '%s': %s", "/", strerror(errno)); }
    dirfd_offset = fd - 3;
}

uint32_t addr2offset(void *addr) {
    return addr - (void *)wasi_memory->bytes;
}

void *offset2addr(uint32_t offset) {
    return wasi_memory->bytes+offset;
}

void copy_iov_to_host(uint32_t iov_offset, uint32_t iovcnt) {
    if (iovcnt > MAX_IOV) {
        FATAL("fd_write called with iovcnt > 128\n");
    }

    // Convert wasi_memory offsets to host addresses
    struct iovec *wasi_iov = offset2addr(iov_offset);
    for (int i = 0; i < iovcnt; i++) {
        host_iov[i].iov_base = offset2addr((uint32_t)wasi_iov[i].iov_base);
        host_iov[i].iov_len = wasi_iov[i].iov_len;
    }
}

// https://github.com/CraneStation/wasmtime/blob/master/docs/WASI-api.md

uint32_t _wasi_unstable__args_get_(uint32_t argv_offset,
                                   uint32_t argv_buf_offset) {
    uint32_t *argv_ptrs = offset2addr(argv_offset);
    char *argv = offset2addr(argv_buf_offset);
    for (int i = 0; i < host_argc; i++) {
        int alen = strlen(host_argv[i])+1;
        memmove(argv, host_argv[i], alen);
        argv_ptrs[i] = addr2offset(argv);
        argv += alen;
    }
    return 0;
}

uint32_t _wasi_unstable__args_sizes_get_(uint32_t argc_offset,
                                         uint32_t argv_size_offset) {
    uint32_t argv_size = 0;
    for (int i = 0; i < host_argc; i++) {
        argv_size += strlen(host_argv[i]);
    }
    //wasi_memory->bytes[argc_offset] = host_argc;
    //wasi_memory->bytes[argv_size_offset] = argv_size;
    *(uint32_t *)offset2addr(argc_offset) = host_argc;
    *(uint32_t *)offset2addr(argv_size_offset) = argv_size;
    return 0;
}

uint32_t _wasi_unstable__clock_time_get_(uint32_t clock_id,
                                         uint64_t precision,
                                         uint32_t time_offset) {
    uint64_t *time = offset2addr(time_offset);
    struct timespec tp;
    clock_gettime(clock_id, &tp);
    *time = (uint64_t)tp.tv_sec * 1000000000 + tp.tv_nsec;
    return 0;
}

uint32_t _wasi_unstable__fd_read_(uint32_t fd,
                                  uint32_t iov, uint32_t iovcnt,
                                  uint32_t nread_offset) {
    copy_iov_to_host(iov, iovcnt);
    ssize_t ret = readv(fd, host_iov, iovcnt);
    if (ret >= 0) {
        *(uint32_t *)offset2addr(nread_offset) = ret;
        return 0;
    } else {
        return 29; // __WASI_EIO
    }
    return 0;
}

uint32_t _wasi_unstable__fd_write_(uint32_t fd,
                                   uint32_t iov, uint32_t iovcnt,
                                   uint32_t nwritten_offset) {
    copy_iov_to_host(iov, iovcnt);

    ssize_t ret = writev(fd, host_iov, iovcnt);
    if (ret >= 0) {
        *(uint32_t *)offset2addr(nwritten_offset) = ret;
        return 0;
    } else {
        return 29; // __WASI_EIO
    }
}

uint32_t _wasi_unstable__path_open_(uint32_t dirfd,
                                    uint32_t dirflags,
                                    uint32_t path_offset,
                                    uint32_t path_len,
                                    uint32_t o_flags,
                                    uint64_t fs_rights_base,
                                    uint64_t fs_rights_inheriting,
                                    uint32_t fs_flags,
                                    uint32_t fd_offset) {
    char path[1024];
    if (path_len > 1023) {
        FATAL("path_open called with path_len > 1023\n");
    }
    memmove(path, offset2addr(path_offset), path_len);
    path[path_len] = '\0'; // NULL terminator
    // TODO: translate o_flags and fs_flags into flags and mode
    int flags = 0;
    int mode = 0;
    int fd = openat(dirfd + dirfd_offset, path, flags, mode);
    if (fd < 0) {
        return 29; // __WASI_EIO
    }
    *(uint32_t *)offset2addr(fd_offset) = fd;
    return 0;
}

uint32_t _wasi_unstable__proc_exit_(uint32_t code) {
    exit(code);
}


