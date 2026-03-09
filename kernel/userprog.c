#include <stdint.h>

static uint32_t sys_write(const char *s) {
    uint32_t ret;

    __asm__ volatile (
        "int $0x80"
        : "=a" (ret)
        : "a" (1u), "b" (s)
        : "memory"
    );

    return ret;
}

void user_entry(void) {
    static const char msg[] = "Hello from C user space via syscall";

    (void)sys_write(msg);

    for (;;) {
        __asm__ volatile ("jmp .");
    }
}
