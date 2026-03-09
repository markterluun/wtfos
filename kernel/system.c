#include <stdint.h>

#include "io.h"
#include "system.h"

#define GDT_ENTRIES 6u
#define IDT_ENTRIES 256u

#define KERNEL_CODE_SEL 0x08u
#define KERNEL_DATA_SEL 0x10u
#define USER_CODE_SEL   0x1Bu
#define USER_DATA_SEL   0x23u
#define TSS_SEL         0x28u

#define PAGE_PRESENT 0x001u
#define PAGE_RW      0x002u
#define PAGE_USER    0x004u

#define USER_CODE_PADDR 0x00030000u
#define USER_STACK_PADDR 0x00031000u
#define PAGE_SIZE 4096u

struct __attribute__((packed)) gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t gran;
    uint8_t base_high;
};

struct __attribute__((packed)) gdt_ptr {
    uint16_t limit;
    uint32_t base;
};

struct __attribute__((packed)) idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t offset_high;
};

struct __attribute__((packed)) idt_ptr {
    uint16_t limit;
    uint32_t base;
};

struct __attribute__((packed)) tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
};

extern void arch_load_gdt(const struct gdt_ptr *ptr);
extern void arch_load_idt(const struct idt_ptr *ptr);
extern void arch_load_tss(uint16_t selector);
extern void arch_enable_paging(uint32_t page_directory_phys);
extern void isr80_stub(void);
extern void isr13_stub(void);
extern void isr14_stub(void);
extern uint8_t _binary_build_userprog_bin_start[];
extern uint8_t _binary_build_userprog_bin_end[];

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdtr;

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtr;

static struct tss_entry tss;
static uint8_t kernel_stack[4096] __attribute__((aligned(16)));

static uint32_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t kernel_page_table[1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t user_page_table[1024] __attribute__((aligned(PAGE_SIZE)));

static void memzero(void *dst, uint32_t n) {
    uint8_t *d = (uint8_t *)dst;
    for (uint32_t i = 0; i < n; ++i) {
        d[i] = 0;
    }
}

static void memcopy(void *dst, const void *src, uint32_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (uint32_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
}

static void set_gdt_entry(uint32_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[idx].limit_low = (uint16_t)(limit & 0xFFFFu);
    gdt[idx].base_low = (uint16_t)(base & 0xFFFFu);
    gdt[idx].base_mid = (uint8_t)((base >> 16) & 0xFFu);
    gdt[idx].access = access;
    gdt[idx].gran = (uint8_t)(((limit >> 16) & 0x0Fu) | ((flags & 0x0Fu) << 4));
    gdt[idx].base_high = (uint8_t)((base >> 24) & 0xFFu);
}

static void set_idt_gate(uint32_t vec, uint32_t handler, uint16_t selector, uint8_t flags) {
    idt[vec].offset_low = (uint16_t)(handler & 0xFFFFu);
    idt[vec].selector = selector;
    idt[vec].zero = 0;
    idt[vec].flags = flags;
    idt[vec].offset_high = (uint16_t)((handler >> 16) & 0xFFFFu);
}

static void init_gdt_and_tss(void) {
    memzero(gdt, sizeof(gdt));

    set_gdt_entry(0, 0, 0, 0x00, 0x0);
    set_gdt_entry(1, 0, 0xFFFFFu, 0x9A, 0xC);
    set_gdt_entry(2, 0, 0xFFFFFu, 0x92, 0xC);
    set_gdt_entry(3, 0, 0xFFFFFu, 0xFA, 0xC);
    set_gdt_entry(4, 0, 0xFFFFFu, 0xF2, 0xC);

    memzero(&tss, sizeof(tss));
    tss.ss0 = KERNEL_DATA_SEL;
    tss.esp0 = (uint32_t)(kernel_stack + sizeof(kernel_stack));
    tss.iomap_base = (uint16_t)sizeof(tss);

    set_gdt_entry(5, (uint32_t)&tss, (uint32_t)sizeof(tss) - 1u, 0x89, 0x0);

    gdtr.limit = (uint16_t)(sizeof(gdt) - 1u);
    gdtr.base = (uint32_t)&gdt[0];

    arch_load_gdt(&gdtr);
    arch_load_tss(TSS_SEL);
}

static void init_idt(void) {
    memzero(idt, sizeof(idt));

    set_idt_gate(13, (uint32_t)isr13_stub, KERNEL_CODE_SEL, 0x8E);
    set_idt_gate(14, (uint32_t)isr14_stub, KERNEL_CODE_SEL, 0x8E);
    set_idt_gate(0x80, (uint32_t)isr80_stub, KERNEL_CODE_SEL, 0xEE);

    idtr.limit = (uint16_t)(sizeof(idt) - 1u);
    idtr.base = (uint32_t)&idt[0];

    arch_load_idt(&idtr);
}

static void init_paging(void) {
    const uint32_t user_prog_size = (uint32_t)(_binary_build_userprog_bin_end - _binary_build_userprog_bin_start);

    if (user_prog_size > PAGE_SIZE) {
        kio_println("User program too large");
        for (;;) {
            __asm__ volatile ("hlt");
        }
    }

    for (uint32_t i = 0; i < 1024u; ++i) {
        page_directory[i] = 0;
        kernel_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
        user_page_table[i] = 0;
    }

    memcopy((void *)USER_CODE_PADDR, _binary_build_userprog_bin_start, user_prog_size);
    memzero((void *)USER_STACK_PADDR, PAGE_SIZE);

    user_page_table[0] = USER_CODE_PADDR | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    user_page_table[1] = USER_STACK_PADDR | PAGE_PRESENT | PAGE_RW | PAGE_USER;

    page_directory[0] = ((uint32_t)&kernel_page_table[0]) | PAGE_PRESENT | PAGE_RW;
    page_directory[1] = ((uint32_t)&user_page_table[0]) | PAGE_PRESENT | PAGE_RW | PAGE_USER;

    arch_enable_paging((uint32_t)&page_directory[0]);
}

void kernel_arch_init(void) {
    init_gdt_and_tss();
    init_idt();
    init_paging();
}

static int is_user_range(uint32_t ptr) {
    return ptr >= USER_CODE_VADDR && ptr < USER_STACK_TOP;
}

uint32_t syscall_dispatch(uint32_t num, uint32_t arg0) {
    if (num == 1u) {
        const char *s = (const char *)arg0;
        uint32_t count = 0;

        if (!is_user_range(arg0)) {
            return 0xFFFFFFFFu;
        }

        while (count < 512u) {
            uint32_t addr = arg0 + count;
            char c;

            if (!is_user_range(addr)) {
                return 0xFFFFFFFEu;
            }

            c = s[count];
            if (c == '\0') {
                return count;
            }

            kio_putc(c);
            ++count;
        }

        return count;
    }

    return 0xFFFFFFFFu;
}
