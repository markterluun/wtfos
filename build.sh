#!/usr/bin/env bash

set -euo pipefail

STAGE2_SECTORS=4
KERNEL_SECTORS=32

mkdir -p build

nasm -I ./boot/ -f bin boot/boot.asm -o build/boot.bin
nasm -I ./boot/ -f bin boot/stage2.asm -o build/stage2.bin

nasm -f elf32 kernel/entry.asm -o build/entry.o
nasm -f elf32 kernel/arch.asm -o build/arch.o
gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -nostdlib -c kernel/userprog.c -o build/userprog.o
ld -m elf_i386 -T kernel/userprog.ld -o build/userprog.elf build/userprog.o
objcopy -O binary build/userprog.elf build/userprog.bin
ld -m elf_i386 -r -b binary -o build/userprog_blob.o build/userprog.bin
gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -nostdlib -c kernel/io.c -o build/io.o
gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -nostdlib -c kernel/main.c -o build/main.o
gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -nostdlib -c kernel/user.c -o build/user.o
gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -nostdlib -c kernel/system.c -o build/system.o
ld -m elf_i386 -T kernel/linker.ld -o build/kernel.elf build/entry.o build/arch.o build/io.o build/main.o build/user.o build/system.o build/userprog_blob.o
objcopy -O binary build/kernel.elf build/kernel.bin

userprog_size=$(stat -c%s build/userprog.bin)
if (( userprog_size > 4096 )); then
    echo "userprog.bin is ${userprog_size} bytes, max is 4096 bytes (one mapped user code page)" >&2
    exit 1
fi

kernel_size=$(stat -c%s build/kernel.bin)
max_kernel_size=$((KERNEL_SECTORS * 512))
if (( kernel_size > max_kernel_size )); then
    echo "kernel.bin is ${kernel_size} bytes, but stage2 loads only ${max_kernel_size} bytes" >&2
    exit 1
fi

dd if=/dev/zero of=build/bf.img bs=512 count=2880
dd if=build/boot.bin of=build/bf.img conv=notrunc
dd if=build/stage2.bin of=build/bf.img bs=512 seek=1 conv=notrunc
dd if=build/kernel.bin of=build/bf.img bs=512 seek=$((1 + STAGE2_SECTORS)) conv=notrunc
