BITS 16
ORG 0x8000

start_stage2:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000

    mov si, msg_stage2
    call print_string

    call enable_a20

    mov si, msg_a20
    call print_string

    mov si, delay_2s
    call delay

    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode_start

enable_a20:
    in al, 0x92
    or al, 00000010b
    out 0x92, al
    ret

%include "io16.inc"

msg_stage2  db 'Stage 2 loaded', 13, 10, 0
msg_a20     db 'A20 enabled', 13, 10, 0

delay_2s    dd 2000000

align 8
gdt_start:
gdt_null:
    dq 0x0000000000000000
gdt_code:
    dq 0x00CF9A000000FFFF
gdt_data:
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

BITS 32

protected_mode_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9FC00

    call clear_screen

    mov esi, msg_pm32
    call print_string_pm

.hang:
    cli
    hlt
    jmp .hang

%include "io32.inc"

cursor_pos dd 0
msg_pm32 db 'PM32 OK', 0
