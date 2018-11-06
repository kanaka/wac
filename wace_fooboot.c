#include <stdio.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////
// memory layout

#define TOTAL_MEMORY  0x1000000 // 16MB
#define TOTAL_TABLE   256

uint8_t  *memory = 0;
uint8_t  *memoryBase;

// Initialize memory globals
void init_wace() {
    memoryBase = calloc(TOTAL_MEMORY, sizeof(uint8_t));
    //printf("memory: %p\n", memory);
    //printf("memoryBase: %p\n", memoryBase);
}

