#include "io.h"

#define VGA_WIDTH 80u
#define VGA_HEIGHT 25u
#define VGA_MEMORY ((volatile uint16_t *)0xB8000)

static uint32_t cursor_x;
static uint32_t cursor_y;
static uint8_t color = 0x07;

static uint16_t make_cell(char c) {
    return (uint16_t)((uint8_t)c) | (uint16_t)((uint16_t)color << 8);
}

static void scroll_if_needed(void) {
    if (cursor_y < VGA_HEIGHT) {
        return;
    }

    for (uint32_t y = 1; y < VGA_HEIGHT; ++y) {
        for (uint32_t x = 0; x < VGA_WIDTH; ++x) {
            VGA_MEMORY[(y - 1u) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
        }
    }

    for (uint32_t x = 0; x < VGA_WIDTH; ++x) {
        VGA_MEMORY[(VGA_HEIGHT - 1u) * VGA_WIDTH + x] = make_cell(' ');
    }

    cursor_y = VGA_HEIGHT - 1u;
}

void kio_clear(void) {
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        VGA_MEMORY[i] = make_cell(' ');
    }

    cursor_x = 0;
    cursor_y = 0;
}

void kio_set_color(uint8_t fg, uint8_t bg) {
    color = (uint8_t)((bg << 4) | (fg & 0x0F));
}

void kio_putc(char c) {
    if (c == '\n') {
        cursor_x = 0;
        ++cursor_y;
        scroll_if_needed();
        return;
    }

    VGA_MEMORY[cursor_y * VGA_WIDTH + cursor_x] = make_cell(c);
    ++cursor_x;

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        ++cursor_y;
        scroll_if_needed();
    }
}

void kio_print(const char *s) {
    if (s == 0) {
        return;
    }

    while (*s != '\0') {
        kio_putc(*s++);
    }
}

void kio_println(const char *s) {
    kio_print(s);
    kio_putc('\n');
}

void kio_init(void) {
    kio_set_color(7, 0);
    kio_clear();
}
