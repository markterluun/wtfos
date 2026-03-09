BITS 32

global _start
extern kmain

_start:
    cli
    mov esp, 0x9F000

    call kmain

.hang:
    hlt
    jmp .hang
