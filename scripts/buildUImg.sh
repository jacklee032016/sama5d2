#!/bin/bash

# -A: arch
# -O: os
# -C: compress

# -a: load address to 'addr' (hex)
# -e: set entry point to 'ep' (hex)
# -n: image name

MEM_ADDRESS=22000000
NAME=MuxLab-AN767

mkimage -A arm -O linux -C none -T kernel -a $MEM_ADDRESS -e $MEM_ADDRESS -n $NAME -d zImage uImage
