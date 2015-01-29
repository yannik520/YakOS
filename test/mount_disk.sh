#!/bin/sh

hdiutil attach -quiet -imagekey diskimage-class=CRawDiskImage -mount required -mountpoint $2 $1
echo "\033[32mmount success! \033[0m"
