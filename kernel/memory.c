#include "memory.h"

static char heap_buffer[HEAP_SIZE];
static char *heap_start = heap_buffer;
static char *heap_end = heap_buffer + HEAP_SIZE;
static char *next_free = heap_buffer;

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