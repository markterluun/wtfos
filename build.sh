#!/usr/bin/env bash

nasm -f bin boot/boot.asm -o build/boot.bin
nasm -f bin boot/stage2.asm -o build/stage2.bin

dd if=/dev/zero of=build/bf.img bs=512 count=2880
dd if=build/boot.bin of=build/bf.img conv=notrunc
dd if=build/stage2.bin of=build/bf.img bs=512 seek=1 conv=notrunc
