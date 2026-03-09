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

pause:
    xor ax, ax
    int 0x16
    ret

delay:
    push ax
    push cx
    push dx

    mov dx, [si]
    mov cx, [si+2]
    mov ah, 0x86
    int 0x15

    pop dx
    pop cx
    pop ax
    ret
