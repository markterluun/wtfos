#!/usr/bin/env bash

# qemu-system-i386 -nographic -drive format=raw,file=build/bf.img -drive format=raw,file=build/hdd.img,if=ide,index=1,media=disk
qemu-system-i386 -drive format=raw,file=build/bf.img -drive format=raw,file=build/hdd.img,if=ide,index=1,media=disk
