#include "io.h"
#include "system.h"
#include "user.h"

void kmain(void) {
    kio_init();
    kio_println("Kernel online");
    kio_println("Init GDT/TSS/IDT/Paging...");

    kernel_arch_init();

    kio_println("Switching to isolated user process...");
    enter_user_mode((void (*)(void))USER_CODE_VADDR);
}
