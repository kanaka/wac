#include <stdio.h>
#include <stdlib.h>

#include <limits.h>

#include "wa.h"

/////////////////////////////////////////////////////////
// memory layout

#define PAGE_COUNT   256 // 64K * 256 = 16MB
#define TOTAL_PAGES  ULONG_MAX / PAGE_SIZE
#define TABLE_COUNT  256

Memory memory = {TOTAL_PAGES, TOTAL_PAGES, TOTAL_PAGES, 0};
uint8_t  *memoryBase;

// Initialize memory globals
void init_wace() {
    memoryBase = calloc(PAGE_COUNT, PAGE_SIZE);
    //printf("memory: %p\n", memory);
    //printf("memoryBase: %p\n", memoryBase);
}

