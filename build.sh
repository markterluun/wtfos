#!/usr/bin/env bash

set -euo pipefail

STAGE2_SECTORS=4
KERNEL_SECTORS=8

mkdir -p build

nasm -I ./boot/ -f bin boot/boot.asm -o build/boot.bin
nasm -I ./boot/ -f bin boot/stage2.asm -o build/stage2.bin

nasm -f elf32 kernel/entry.asm -o build/entry.o
gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -nostdlib -c kernel/io.c -o build/io.o
gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -nostdlib -c kernel/disk.c -o build/disk.o
gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -nostdlib -c kernel/memory.c -o build/memory.o
gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-pie -nostdlib -c kernel/main.c -o build/main.o
ld -m elf_i386 -T kernel/linker.ld -o build/kernel.elf build/entry.o build/io.o build/disk.o build/memory.o build/main.o
objcopy -O binary build/kernel.elf build/kernel.bin

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

# Create a small hard disk image for testing
dd if=/dev/zero of=build/hdd.img bs=1M count=10
