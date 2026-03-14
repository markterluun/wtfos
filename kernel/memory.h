#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <stdint.h>

typedef unsigned int size_t;
#define NULL ((void*)0)

#define HEAP_SIZE 0x100000  // 1MB heap

void *kmalloc(size_t size);
void kfree(void *ptr);

#endif