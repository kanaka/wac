#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

void memdump();
void memdiff(size_t offset, size_t n);

int test() {
    int x = 42;
    //printf("ABCDEFGHIJKLMNOPQRSTOVWXYZ\n", x);
    return x;
}

int main () {
    char *name = malloc(20);
    memset(name, 255, 20);
    strncpy(name, "malloc people", 15);
    printf("hello %s\n", name);
    return 2;
}
