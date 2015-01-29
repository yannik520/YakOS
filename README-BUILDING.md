HOW TO BUILD
============

In order to make it easy to configure and compile YakOS for different hardware platforms or into a simulation platform, using kconfig to produce configuration file， without having to edit makefiles.

To configure and build the YakOS, use:

    make menuconfig
    make

Load with u-boot and run:
    loadb 0xc0008000   --> load kernel image
    loadb 0xc0200000   --> load ramdisk
    go 0xc0008000      --> run kernel
    mount 0xc0200000   --> mount file system
    insmod /HELLO.KO   --> insmod the module hello

So easy!
