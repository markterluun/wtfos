#include "io.h"
#include "disk.h"
#include "memory.h"
#include "string.h"

void init_disk_io();
void test_disk_io();
void test_mem_io();
void test_string_io();

void kmain(void) {
    kio_init();
    kio_println("Hello World from C");
    kio_println("Basic kernel IO online");

    init_disk_io();

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void init_disk_io() {
    // Initialize disk
    if (!disk_init()) {
        kio_println("Disk initialization failed");
    } else {
        kio_println("Disk initialized successfully");
    }

    // Test disk identify
    uint16_t identify_buffer[256];
    if (disk_identify(identify_buffer)) {
        kio_println("Disk identify successful");
    } else {
        kio_println("Disk identify failed");
    }
}

void test_disk_io() {
    uint8_t buffer[512];

    // First, write some data to sector 0
    for (int i = 0; i < 512; i++) {
        buffer[i] = (uint8_t)i;
    }
    if (disk_write_sector(0, buffer)) {
        kio_println("Disk write successful");
    } else {
        kio_println("Disk write failed");
    }

    // Now read it back
    if (disk_read_sector(0, buffer)) {
        kio_println("Disk read successful");
        // Check if data matches
        int match = 1;
        for (int i = 0; i < 512; i++) {
            if (buffer[i] != (uint8_t)i) {
                match = 0;
                break;
            }
        }
        if (match) {
            kio_println("Data matches - disk I/O working!");
        } else {
            kio_println("Data does not match");
        }
    } else {
        kio_println("Disk read failed");
    }
}

void test_mem_io() {
    // Test memory allocation
    void *ptr1 = kmalloc(100);
    void *ptr2 = kmalloc(200);
    if (ptr1 && ptr2) {
        kio_println("Memory allocation successful");
    } else {
        kio_println("Memory allocation failed");
    }
}

void test_string_io() {
    // Test string functions
    char *str1 = (char *)kmalloc(20);
    char *str2 = (char *)kmalloc(20);
    strcpy(str1, "Hello");
    strcpy(str2, "World");
    if (strcmp(str1, "Hello") == 0 && strcmp(str2, "World") == 0) {
        kio_println("String functions working");
    } else {
        kio_println("String functions failed");
    }
}
