#!/bin/sh

ARMEMU_PATH=/home/yannik/work/armemu/armemu-master/
echo $ARMEMU_PATH
cd $ARMEMU_PATH
./build-generic/armemu
cd -
