#!/bin/bash
arm-none-eabi-as bootstrap.s -o bootstrap.o
arm-none-eabi-ld -T lscript bootstrap.o notmain.o -o hello.elf
arm-none-eabi-objcopy hello.elf -O binary kernel.img
P=arm_disassembler make
ctags -R .
