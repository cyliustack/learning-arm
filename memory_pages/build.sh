#!/bin/bash

CC=aarch64-suse-linux-gcc
LD=aarch64-suse-linux-ld
OBJCOPY=aarch64-suse-linux-objcopy


$CC -c boot.s -o boot.o
$CC -c lib.s -o liba.o
$CC -c handler.s -o handlera.o
$CC -c mmu.s -o mmu.o
$CC -std=c99 -ffreestanding -mgeneral-regs-only  -c main.c
$CC -std=c99 -ffreestanding -mgeneral-regs-only  -c uart.c
$CC -std=c99 -ffreestanding -mgeneral-regs-only  -c print.c
$CC -std=c99 -ffreestanding -mgeneral-regs-only  -c debug.c
$CC -std=c99 -ffreestanding -mgeneral-regs-only  -c handler.c
$CC -std=c99 -ffreestanding -mgeneral-regs-only  -c memory.c
$LD -nostdlib -T link.lds -o kernel boot.o main.o liba.o uart.o print.o debug.o handlera.o handler.o mmu.o  memory.o
$OBJCOPY -O binary kernel kernel8.img
