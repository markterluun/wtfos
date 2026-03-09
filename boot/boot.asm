BITS 16
ORG 0x7C00

STAGE2_SECTORS equ 4

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    ; Wait for 2 seconds
    mov si, msg_wait
    call print_string

    ; Load stage2.bin to 0000:8000
    xor ax, ax
    mov es, ax
    mov bx, 0x8000

    mov ah, 0x02            ; BIOS read sectors
    mov al, STAGE2_SECTORS  ; number of sectors to read
    mov ch, 0               ; cylinder 0
    mov cl, 2               ; sector 2
    mov dh, 0               ; head 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    mov dl, [boot_drive]
    jmp 0x0000:0x8000

disk_error:
    mov si, msg_disk_error
    call print_string

.hang:
    cli
    hlt
    jmp .hang

%include "io16.inc"

boot_drive db 0

msg_wait        db 'Booting WTFOS...', 13, 10, 0
msg_disk_error  db 'Disk read error', 0

times 510 - ($ - $$) db 0
dw 0xAA55
