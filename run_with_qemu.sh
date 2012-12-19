#!/bin/sh
#C+a x  exit qemu
#C+a h  show help

QEMU=/home/yannik/work/qemu/mini2440/arm-softmmu/qemu-system-arm

if [ "$1" = "-d" ]; then
    
    $QEMU -M versatilepb -nographic -s -S -kernel bigeye.bin
else
    $QEMU -M versatilepb -nographic -kernel bigeye.bin
fi