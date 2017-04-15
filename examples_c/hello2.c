#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main () {
    char *name;
    name = malloc(20);
    strncpy(name, "people", 7);
    printf("hello %s\n", name);
    return 2;
}
