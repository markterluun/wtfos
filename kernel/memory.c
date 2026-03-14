#include "memory.h"

extern char _end;

static char *heap_start = (char *)&_end;
static char *heap_end = (char *)&_end + HEAP_SIZE;
static char *next_free = (char *)&_end;

void *kmalloc(size_t size) {
    if (next_free + size > heap_end) {
        return NULL;  // Out of memory
    }
    void *ptr = next_free;
    next_free += size;
    return ptr;
}

void kfree(void *ptr) {
    // Bump allocator: no free implementation
    (void)ptr;
}