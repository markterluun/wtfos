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
