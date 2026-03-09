BITS 16
ORG 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    ; Load stage2.bin to 0000:8000
    xor ax, ax
    mov es, ax
    mov bx, 0x8000

    mov ah, 0x02            ; BIOS read sectors
    mov al, 4               ; number of sectors to read
    mov ch, 0               ; cylinder 0
    mov cl, 2               ; sector 2
    mov dh, 0               ; head 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    jmp 0x0000:0x8000

disk_error:
    mov si, msg_disk_error
.print:
    lodsb
    test al, al
    jz .hang
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .print

.hang:
    cli
    hlt
    jmp .hang

boot_drive db 0
msg_disk_error db 'Disk read error', 0

times 510 - ($ - $$) db 0
dw 0xAA55
