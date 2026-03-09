#ifndef KERNEL_SYSTEM_H
#define KERNEL_SYSTEM_H

#include <stdint.h>

#define USER_CODE_VADDR 0x00400000u
#define USER_STACK_TOP  0x00402000u

void kernel_arch_init(void);
uint32_t syscall_dispatch(uint32_t num, uint32_t arg0);

#endif
