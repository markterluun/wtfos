#include "io.h"

void kmain(void) {
    kio_init();
    kio_println("Hello World from C");
    kio_println("Basic kernel IO online");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
