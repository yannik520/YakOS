#!/bin/sh

PWD=`pwd`

export PATH=$PWD/tools/arm-cs-tools/bin:$PATH
make $1