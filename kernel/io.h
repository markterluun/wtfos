#ifndef KERNEL_IO_H
#define KERNEL_IO_H

#include <stdint.h>

void kio_init(void);
void kio_clear(void);
void kio_set_color(uint8_t fg, uint8_t bg);
void kio_putc(char c);
void kio_print(const char *s);
void kio_println(const char *s);

#endif
