#!/bin/bash

## Script to mount jffs2 image into local directory
## Jack Lee, Apr.3rd, 2009

#if [[ $# -lt 2 ]]
#then
#    echo "Usage: $0 FSNAME.JFFS2 MOUNTPOINT [ERASEBLOCK_SIZE]"
#    exit 1
#fi

DIR_SRC=`dirname "$0"`
source $DIR_SRC/params.sh


if [ "$(whoami)" != "root" ]
then
    echo "$0 must be run as root!"
    exit 1
fi

if [[ ! -e $SRC_FILE ]]
then
    echo "$SRC_FILE does not exist"
    exit 1
fi

if [[ ! -d $SRC_DIR ]]
then
    echo "$SRC_DIR not exist, create..."
	mkdir -p $SRC_DIR
    # exit 1
fi

if [[ "$3" == "" ]]
then
	esize="128"
else
	esize="$3"
fi

# device file
mknod $MTD_BLOCK_DEV b 31 0

# cleanup if necessary
umount $MTD_BLOCK_DEV &>/dev/null

# load drivers
modprobe -r mtdblock &>/dev/null
modprobe -r mtdram &>/dev/null
modprobe -r jffs2

# check drivers are loaded
modprobe mtdblock || exit 1
modprobe mtdram total_size=65536 erase_size=256 || exit 1
# modprobe mtdram total_size=32768 erase_size=$esize || exit 1


dd if="$SRC_FILE" of=$MTD_BLOCK_DEV || exit 1
mount -t jffs2 $MTD_BLOCK_DEV $SRC_DIR || exit 1

echo "Successfully mounted $SRC_FILE on $SRC_DIR"
exit 0