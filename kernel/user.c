#include <stdint.h>

#include "system.h"
#include "user.h"

#define USER_CODE_SEL 0x1B
#define USER_DATA_SEL 0x23

__attribute__((noreturn)) void enter_user_mode(void (*entry)(void)) {
    const uint32_t user_stack = USER_STACK_TOP;

    __asm__ volatile (
        "movw %[user_data], %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        "pushl %[user_data]\n\t"
        "pushl %[user_stack]\n\t"
        "pushfl\n\t"
        "pushl %[user_code]\n\t"
        "pushl %[entry]\n\t"
        "iret\n\t"
        :
        : [entry] "r" (entry),
          [user_stack] "r" (user_stack),
          [user_code] "i" (USER_CODE_SEL),
          [user_data] "i" (USER_DATA_SEL)
        : "ax", "memory"
    );

    __builtin_unreachable();
}
