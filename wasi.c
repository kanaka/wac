#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <string.h>

#include "util.h"
#include "wa.h"
#include "wasi.h"

//
// WASI implemenation (https://wasi.dev)
//

// Memory and utilities
Memory *wasi_memory = NULL;

int           host_argc = 0;
char        **host_argv = NULL;
char        **host_envp = NULL;

#define MAX_IOV 128
struct iovec host_iov[MAX_IOV];

#define PREOPEN_CNT 7
Preopen      preopen[PREOPEN_CNT] = {
    { .path = "<stdin>",  .path_len = 7, },
    { .path = "<stdout>", .path_len = 8, },
    { .path = "<stderr>", .path_len = 8, },
    { .path = "./",       .path_len = 2, },
    { .path = "../",      .path_len = 3, },
    { .path = "/",        .path_len = 1, },
    { .path = "/tmp",     .path_len = 4, },
};

__wasi_errno_t errno_to_wasi(int errnum) {
    switch (errnum) {
    case EPERM:   return __WASI_EPERM;   break;
    case ENOENT:  return __WASI_ENOENT;  break;
    case ESRCH:   return __WASI_ESRCH;   break;
    case EINTR:   return __WASI_EINTR;   break;
    case EIO:     return __WASI_EIO;     break;
    case ENXIO:   return __WASI_ENXIO;   break;
    case E2BIG:   return __WASI_E2BIG;   break;
    case ENOEXEC: return __WASI_ENOEXEC; break;
    case EBADF:   return __WASI_EBADF;   break;
    case ECHILD:  return __WASI_ECHILD;  break;
    case EAGAIN:  return __WASI_EAGAIN;  break;
    case ENOMEM:  return __WASI_ENOMEM;  break;
    case EACCES:  return __WASI_EACCES;  break;
    case EFAULT:  return __WASI_EFAULT;  break;
    case EBUSY:   return __WASI_EBUSY;   break;
    case EEXIST:  return __WASI_EEXIST;  break;
    case EXDEV:   return __WASI_EXDEV;   break;
    case ENODEV:  return __WASI_ENODEV;  break;
    case ENOTDIR: return __WASI_ENOTDIR; break;
    case EISDIR:  return __WASI_EISDIR;  break;
    case EINVAL:  return __WASI_EINVAL;  break;
    case ENFILE:  return __WASI_ENFILE;  break;
    case EMFILE:  return __WASI_EMFILE;  break;
    case ENOTTY:  return __WASI_ENOTTY;  break;
    case ETXTBSY: return __WASI_ETXTBSY; break;
    case EFBIG:   return __WASI_EFBIG;   break;
    case ENOSPC:  return __WASI_ENOSPC;  break;
    case ESPIPE:  return __WASI_ESPIPE;  break;
    case EROFS:   return __WASI_EROFS;   break;
    case EMLINK:  return __WASI_EMLINK;  break;
    case EPIPE:   return __WASI_EPIPE;   break;
    case EDOM:    return __WASI_EDOM;    break;
    case ERANGE:  return __WASI_ERANGE;  break;
    default:      return errno;
    }
}


void init_wasi(Memory *memory, int argc, char **argv, char **envp) {
    // Save memory
    wasi_memory = memory;

    // save argc/argv for WASI args_* calls
    host_argc = argc;
    host_argv = argv;
    host_envp = envp;

    // Close dir fds that overlap with where preopen fds need to land (>= 3)
    for (int fd = 3; fd < PREOPEN_CNT; fd++) {
        if (fcntl(fd, F_GETFD, 0) >= 0) {
            close(fd);
        }
    }
    // Open default preopened dirfds
    for (int fd = 3; fd < PREOPEN_CNT; fd++) {
        int tfd = open(preopen[fd].path, O_RDONLY);
        if (tfd < 0) {
            FATAL("opening '%s': %s\n", "./", strerror(errno));
        }
        if (tfd != fd) {
            //char ppath[1024];
            //char path3[1024];
            //sprintf(ppath, "/proc/self/fd/%d", fd);
            //int ret = readlink(ppath, path3, 1023);
            //FATAL("fd 3 is already open to '%s' (%d)\n", path3, ret);
            FATAL("fd %d could not be freed up before preopen\n", fd);
        }
    }
}

uint32_t min(uint32_t a, uint32_t b) {
    return a <= b ? a : b;
}

uint32_t addr2offset(void *addr) {
    return addr - (void *)wasi_memory->bytes;
}

void *offset2addr(uint32_t offset) {
    return wasi_memory->bytes+offset;
}

struct iovec *copy_iov_to_host(uint32_t iov_offset, uint32_t iovs_len) {
    if (iovs_len > MAX_IOV) {
        FATAL("copy_iov_to_host called with iovs_len > 128\n");
    }

    // Convert wasi_memory offsets to host addresses
    struct iovec *wasi_iov = offset2addr(iov_offset);
    for (int i = 0; i < iovs_len; i++) {
        host_iov[i].iov_base = offset2addr((uint32_t)wasi_iov[i].iov_base);
        host_iov[i].iov_len = wasi_iov[i].iov_len;
    }
    return host_iov;
}

void copy_string_array(char     **src,
                       uint32_t   cnt,
                       char     **dst_ptrs,
                       char      *dst_buf,
                       bool       in) {
    for (int i = 0; i < cnt; i++) {
        dst_ptrs[i] = in ? (char *)addr2offset(dst_buf) : dst_buf;
        int alen = strlen(src[i])+1;
        memmove(dst_buf, src[i], alen);
        dst_buf[alen] = '\0';
        dst_buf += alen;
    }
}

// https://github.com/CraneStation/wasmtime/blob/master/docs/WASI-api.md

uint32_t _wasi_unstable__args_get_(uint32_t argv_offset,
                                   uint32_t argv_buf_offset) {
    copy_string_array(host_argv, host_argc,
                      offset2addr(argv_offset),
                      offset2addr(argv_buf_offset),
                      true);
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__args_sizes_get_(uint32_t argc_offset,
                                         uint32_t argv_buf_size_offset) {
    size_t *argc          = offset2addr(argc_offset);
    size_t *argv_buf_size = offset2addr(argv_buf_size_offset);

    *argc = host_argc;
    *argv_buf_size = 0;
    for (int i = 0; i < host_argc; i++) {
        *argv_buf_size += strlen(host_argv[i])+1;
    }
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__environ_get_(uint32_t environ_ptrs_offset,
                                      uint32_t environ_strs_offset) {
    int environ_count = 0;
    while (host_envp[environ_count]) { environ_count += 1; }

    copy_string_array(host_envp, environ_count,
                      offset2addr(environ_ptrs_offset),
                      offset2addr(environ_strs_offset),
                      true);
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__environ_sizes_get_(uint32_t environ_count_offset,
                                            uint32_t environ_buf_size_offset) {
    size_t *environ_count    = offset2addr(environ_count_offset);
    size_t *environ_buf_size = offset2addr(environ_buf_size_offset);
    *environ_count = 0;
    *environ_buf_size = 0;
    while (host_envp[*environ_count]) {
        //printf("e: %d -> '%s'\n", *environ_count, host_envp[*environ_count]);
        *environ_buf_size += strlen(host_envp[*environ_count])+1;
        *environ_count += 1;
    }
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__clock_time_get_(uint32_t clock_id,
                                         uint64_t precision,
                                         uint32_t time_offset) {
    uint64_t *time = offset2addr(time_offset);
    struct timespec tp;
    clock_gettime(clock_id, &tp);
    *time = (uint64_t)tp.tv_sec * 1000000000 + tp.tv_nsec;
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__fd_close_(uint32_t fd) {
    int ret = close(fd);
    return ret == 0 ? __WASI_ESUCCESS : ret;
}

uint32_t _wasi_unstable__fd_datasync_(uint32_t fd) {
    int ret = fdatasync(fd);
    return ret == 0 ? __WASI_ESUCCESS : ret;
}

uint32_t _wasi_unstable__fd_fdstat_get_(__wasi_fd_t fd,
                                        uint32_t fdstat_offset) {
    struct stat fd_stat;
    __wasi_fdstat_t *fdstat = offset2addr(fdstat_offset);
    int fl = fcntl(fd, F_GETFL);
    if (fl < 0) { return errno_to_wasi(errno); }
    fstat(fd, &fd_stat);
    int mode = fd_stat.st_mode;
    fdstat->fs_filetype = (S_ISBLK(mode)   ? __WASI_FILETYPE_BLOCK_DEVICE     : 0) |
                          (S_ISCHR(mode)   ? __WASI_FILETYPE_CHARACTER_DEVICE : 0) |
                          (S_ISDIR(mode)   ? __WASI_FILETYPE_DIRECTORY        : 0) |
                          (S_ISREG(mode)   ? __WASI_FILETYPE_REGULAR_FILE     : 0) |
                          (S_ISSOCK(mode)  ? __WASI_FILETYPE_SOCKET_STREAM    : 0) |
                          (S_ISLNK(mode)   ? __WASI_FILETYPE_SYMBOLIC_LINK    : 0);
    fdstat->fs_flags = ((fl & O_APPEND)    ? __WASI_FDFLAG_APPEND    : 0) |
                       ((fl & O_DSYNC)     ? __WASI_FDFLAG_DSYNC     : 0) |
                       ((fl & O_NONBLOCK)  ? __WASI_FDFLAG_NONBLOCK  : 0) |
                       ((fl & O_RSYNC)     ? __WASI_FDFLAG_RSYNC     : 0) |
                       ((fl & O_SYNC)      ? __WASI_FDFLAG_SYNC      : 0);
    fdstat->fs_rights_base = (uint64_t)-1; // all rights
    fdstat->fs_rights_inheriting = (uint64_t)-1; // all rights
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__fd_prestat_dir_name_(uint32_t fd,
                                              uint32_t path_offset,
                                              uint32_t path_len) {
    if (fd < 3 || fd >= PREOPEN_CNT) { return __WASI_EBADF; }
    memmove((char *)offset2addr(path_offset), preopen[fd].path,
            min(preopen[fd].path_len, path_len));
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__fd_prestat_get_(uint32_t fd,
                                         uint32_t buf_offset) {
    if (fd < 3 || fd >= PREOPEN_CNT) { return __WASI_EBADF; }
    *(uint32_t *)offset2addr(buf_offset) = __WASI_PREOPENTYPE_DIR;
    *(uint32_t *)offset2addr(buf_offset+4) = preopen[fd].path_len;
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__fd_read_(__wasi_fd_t  fd,
                                   uint32_t    iovs_offset,
                                   size_t      iovs_len,
                                   uint32_t    nread_offset) {
    struct iovec *iovs = copy_iov_to_host(iovs_offset, iovs_len);
    size_t *nread      = offset2addr(nread_offset);

    ssize_t ret = readv(fd, iovs, iovs_len);
    if (ret < 0) { return errno_to_wasi(errno); }
    *nread = ret;
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__fd_seek_(__wasi_fd_t         fd,
                                  __wasi_filedelta_t  offset,
                                  __wasi_whence_t     whence,
                                  uint32_t            newoffset_offset) {
    __wasi_filesize_t *filesize = offset2addr(newoffset_offset);

    int wasi_whence = whence == __WASI_WHENCE_END ? SEEK_END :
                                __WASI_WHENCE_CUR ? SEEK_CUR : 0;
    int64_t ret = lseek(fd, offset, wasi_whence);
    if (ret < 0) { return errno_to_wasi(errno); }
    *filesize = ret;
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__fd_write_(__wasi_fd_t  fd,
                                   uint32_t     iovs_offset,
                                   size_t       iovs_len,
                                   uint32_t     nwritten_offset) {
    struct iovec *iovs = copy_iov_to_host(iovs_offset, iovs_len);
    size_t *nwritten   = offset2addr(nwritten_offset);

    ssize_t ret = writev(fd, iovs, iovs_len);
    if (ret < 0) { return errno_to_wasi(errno); }
    *nwritten = ret;
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__path_open_(__wasi_fd_t           dirfd,
                                    __wasi_lookupflags_t  dirflags,
                                    uint32_t              path_offset,
                                    size_t                path_len,
                                    __wasi_oflags_t       oflags,
                                    __wasi_rights_t       fs_rights_base,
                                    __wasi_rights_t       fs_rights_inheriting,
                                    __wasi_fdflags_t      fs_flags,
                                    uint32_t              fd_offset) {
    char *path      = offset2addr(path_offset);
    __wasi_fd_t *fd = offset2addr(fd_offset);

    // copy path so we can ensure it is NULL terminated
    char host_path[1024];
    if (path_len > 1023) {
        FATAL("path_open called with path_len > 1023\n");
    }
    memmove(host_path, path, path_len);
    host_path[path_len] = '\0'; // NULL terminator

    // translate o_flags and fs_flags into flags and mode
    int flags = ((oflags & __WASI_O_CREAT)             ? O_CREAT     : 0) |
                ((oflags & __WASI_O_DIRECTORY)         ? O_DIRECTORY : 0) |
                ((oflags & __WASI_O_EXCL)              ? O_EXCL      : 0) |
                ((oflags & __WASI_O_TRUNC)             ? O_TRUNC     : 0) |
                ((fs_flags & __WASI_FDFLAG_APPEND)     ? O_APPEND    : 0) |
                ((fs_flags & __WASI_FDFLAG_DSYNC)      ? O_DSYNC     : 0) |
                ((fs_flags & __WASI_FDFLAG_NONBLOCK)   ? O_NONBLOCK  : 0) |
                ((fs_flags & __WASI_FDFLAG_RSYNC)      ? O_RSYNC     : 0) |
                ((fs_flags & __WASI_FDFLAG_SYNC)       ? O_SYNC      : 0);
    if ((fs_rights_base & __WASI_RIGHT_FD_READ) &&
        (fs_rights_base & __WASI_RIGHT_FD_WRITE)) {
        flags |= O_RDWR;
    } else if ((fs_rights_base & __WASI_RIGHT_FD_WRITE)) {
        flags |= O_WRONLY;
    } else if ((fs_rights_base & __WASI_RIGHT_FD_READ)) {
        flags |= O_RDONLY; // no-op because O_RDONLY is 0
    }
    int mode = 0644;
    int host_fd = openat(dirfd, host_path, flags, mode);
    if (host_fd < 0) { return errno_to_wasi(errno); }

    *fd = host_fd;
    return __WASI_ESUCCESS;
}

uint32_t _wasi_unstable__proc_exit_(uint32_t code) {
    exit(code);
}


