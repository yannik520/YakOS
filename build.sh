#!/bin/sh
echo $HOME
TOOLCHAIN_PATH="$HOME/work/arm-none-eabi-toolchain/bin"
echo $TOOLCHAIN_PATH
export PATH=$TOOLCHAIN_PATH:$PATH
make $1
