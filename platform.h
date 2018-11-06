#ifndef FS_H
#define FS_H

#include <stdbool.h>

void *acalloc(size_t nmemb, size_t size,  char *name);
void *arecalloc(void *ptr, size_t old_nmemb, size_t nmemb,
                size_t size,  char *name);

#if PLATFORM==1

  #ifdef USE_READLINE
    // WARNING: GPL license implications
    #include <readline/readline.h>
    #include <readline/history.h>
  #else
    #include <editline/readline.h>
  #endif
  uint8_t *mmap_file(char *path, int *len);

#else

  bool readline(const char *prompt, char *buf, int cnt);

#endif

int read_file(char *path, char *buf);

// Dynamic lib resolution
bool resolvesym(char *filename, char *symbol, void **val, char **err);

#endif // of FS_H
