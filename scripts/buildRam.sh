#!/bin/bash

SIZE_IN_BLOCK=40960
RAM_DISK_NAME=ramDisk
ROOT_DIR=tmp

echo "Build ram disk..."
echo " from $ROOT_DIR to ram disk $RAM_DISK_NAME..."
genext2fs --size-in-blocks $SIZE_IN_BLOCK -d $ROOT_DIR $RAM_DISK_NAME

gzip -9 -f $RAM_DISK_NAME
