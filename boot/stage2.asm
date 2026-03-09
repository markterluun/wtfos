BITS 16
ORG 0x8000

KERNEL_LOAD_SEGMENT equ 0x1000
KERNEL_SECTORS      equ 8
KERNEL_START_SECTOR equ 6
STAGE2_SECTORS      equ 4

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

    call load_kernel

    mov si, msg_kernel_loaded
    call print_string

    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode_start

load_kernel:
    mov ax, KERNEL_LOAD_SEGMENT
    mov es, ax
    xor bx, bx

    mov ah, 0x02               ; BIOS read sectors
    mov al, KERNEL_SECTORS
    mov ch, 0
    mov cl, KERNEL_START_SECTOR
    mov dh, 0
    int 0x13
    jc kernel_load_error
    ret

kernel_load_error:
    mov si, msg_kernel_error
    call print_string

.hang:
    cli
    hlt
    jmp .hang

enable_a20:
    in al, 0x92
    or al, 00000010b
    out 0x92, al
    ret

%include "io16.inc"

msg_stage2          db 'Stage 2 loaded', 13, 10, 0
msg_a20             db 'A20 enabled', 13, 10, 0
msg_kernel_loaded   db 'Kernel loaded', 13, 10, 0
msg_kernel_error    db 'Kernel read error', 13, 10, 0

align 8
gdt_start:
gdt_null:
    dq 0x0000000000000000
gdt_code:
    dq 0x00CF9A000000FFFF
gdt_data:
    dq 0x00CF92000000FFFF
gdt_user_code:
    dq 0x00CFFA000000FFFF
gdt_user_data:
    dq 0x00CFF2000000FFFF
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

    jmp 0x10000

%include "io32.inc"

times STAGE2_SECTORS * 512 - ($ - $$) db 0
