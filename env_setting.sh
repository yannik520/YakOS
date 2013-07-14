#!/bin/sh
echo $HOME
TOOLCHAIN_PATH="./tools/arm-elf-toolchain/bin"
echo $TOOLCHAIN_PATH
export PATH=$TOOLCHAIN_PATH:$PATH
export TOPDIR=`pwd`
