#ifndef KERNEL_USER_H
#define KERNEL_USER_H

__attribute__((noreturn)) void enter_user_mode(void (*entry)(void));

#endif
