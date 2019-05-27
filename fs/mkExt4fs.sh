#!/bin/bash

## Script to make ext4 filesystem
## Jack Lee, May.26, 2009

# if [[ $# -lt 2 ]]
# then
#    echo "Usage: $0 FSNAME.JFFS2 MOUNTPOINT [ERASEBLOCK_SIZE]"
#    exit 1
# fi

DIR_SRC=`dirname "$0"`
source $DIR_SRC/params.sh


# block size
BLOCK_SIZE_4K=4k
# 4*1024*16 = 64M
SIZE_1M=1024
FS_SIZE=256

BLOCK_COUNT=$( expr "$SIZE_1M" '*' "$FS_SIZE" "/"  4)

# EXT3_FS_SIZE_64M=16384

echo "File System size: $FS_SIZE MB; block count: $BLOCK_COUNT; block size: $BLOCK_SIZE_4K;"


if [[ "$3" == "" ]]
then
#    OUT_FILE=$SRC_FILE.$BUILDTIME
    OUT_FILE=App.ext4.$BUILDTIME
else
    OUT_FILE=App.ext4.$BUILDTIME
fi

if [[ ! -d $SRC_DIR ]]
then
    echo "$SRC_DIR is not a valid directory"
    exit 1
fi


echo "   make ext4 file system into '$OUT_FILE' from '$SRC_DIR', size $PAD_SIZE..."


dd if=/dev/zero of=$OUT_FILE bs=$BLOCK_SIZE_4K count=$BLOCK_COUNT

yes Y | mkfs.ext4 $OUT_FILE 

mount -oloop $OUT_FILE /opt/mnt

echo "copy from $SRC_DIR..."
cp -a $SRC_DIR/. 	/opt/mnt/.

umount -r /opt/mnt|| exit 1

# e2fsck -f $OUT_FILE|| exit 1
# resize2fs -M $OUT_FILE|| exit 1


# rm -r $SRC_DIR

# rm $MTD_BLOCK_DEV

echo "Successfully make new EXT4 file system into image of $OUT_FILE"
exit 0
