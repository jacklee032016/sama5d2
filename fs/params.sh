#!/bin/bash

## common parameters for file system


SRC_DIR=mnt

SRC_FILE=root.jffs2

MTD_BLOCK_DEV=/tmp/mtdblock0


BUILDTIME=`TZ=CN date -u "+%Y_%m_%d"`

# app partition
SIZE_4M=0x400000

# root file system
SIZE_16M=0x1000000
