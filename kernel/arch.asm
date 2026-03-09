BITS 32

global arch_load_gdt
global arch_load_idt
global arch_load_tss
global arch_enable_paging
global isr80_stub
global isr13_stub
global isr14_stub

extern syscall_dispatch

arch_load_gdt:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush_cs
.flush_cs:
    ret

arch_load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret

arch_load_tss:
    mov ax, [esp + 4]
    ltr ax
    ret

arch_enable_paging:
    mov eax, [esp + 4]
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

isr80_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, [esp + 44]
    mov ebx, [esp + 32]
    push ebx
    push eax
    call syscall_dispatch
    add esp, 8
    mov [esp + 44], eax

    pop gs
    pop fs
    pop es
    pop ds
    popa
    iretd

isr13_stub:
    cli
.hang13:
    hlt
    jmp .hang13

isr14_stub:
    cli
.hang14:
    hlt
    jmp .hang14
