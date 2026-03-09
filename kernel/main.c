#include <stdint.h>

static volatile uint16_t *const VGA = (uint16_t *)0xB8000;

void kmain(void) {
    const char *msg = "Hello World from C";

    for (uint32_t i = 0; msg[i] != '\0'; ++i) {
        VGA[i] = (uint16_t)msg[i] | (uint16_t)(0x07u << 8);
    }

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
