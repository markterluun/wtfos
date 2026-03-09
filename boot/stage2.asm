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

    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode_start

print_string:
.next:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .next
.done:
    ret

enable_a20:
    in al, 0x92
    or al, 00000010b
    out 0x92, al
    ret

msg_stage2 db 'Stage 2 loaded', 13, 10, 0
msg_a20    db 'A20 enabled', 13, 10, 0

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

clear_screen:
    mov edi, 0xB8000
    mov ecx, 80 * 25
    mov ax, 0x0720
.clear_loop:
    mov word [edi], ax
    add edi, 2
    loop .clear_loop

    mov dword [cursor_pos], 0
    ret

print_string_pm:
.next:
    lodsb
    test al, al
    jz .done
    call print_char_pm
    jmp .next
.done:
    ret

print_char_pm:
    push ebx
    push edi

    mov ebx, [cursor_pos]
    mov edi, 0xB8000
    add edi, ebx

    mov ah, 0x07
    mov [edi], ax

    add dword [cursor_pos], 2

    pop edi
    pop ebx
    ret

cursor_pos dd 0
msg_pm32 db 'PM32 OK', 0
