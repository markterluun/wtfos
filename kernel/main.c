#include "io.h"
#include "user.h"

static void user_main(void);

void kmain(void) {
    kio_init();
    kio_println("Basic kernel IO online");
    kio_println("Switching to user space...");
    
    enter_user_mode(user_main);
}

static void user_main(void) {
    kio_println("Entered user space (ring 3)");

    for (;;) {
        __asm__ volatile ("jmp .");
    }
}
