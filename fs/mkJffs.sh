#!/bin/bash

## Script to make jffs2 filesystem
## Jack Lee, Apr.3rd, 2009

# if [[ $# -lt 2 ]]
# then
#    echo "Usage: $0 FSNAME.JFFS2 MOUNTPOINT [ERASEBLOCK_SIZE]"
#    exit 1
# fi

DIR_SRC=`dirname "$0"`
source $DIR_SRC/params.sh


# erase block
SIZE_64K=0x10000
SIZE_32K=0x8000
SIZE_4K=0x1000

if [[ "$3" == "" ]]
then
    # PAD_SIZE=$SIZE_16M
	#PAD_SIZE=$SIZE_32M
	# PAD_SIZE=$SIZE_48M
	PAD_SIZE=$SIZE_55M
    # PAD_SIZE=$SIZE_64M
    OUT_FILE=$SRC_FILE.$BUILDTIME
else
    PAD_SIZE=$SIZE_4M
    OUT_FILE=App.jffs2.$BUILDTIME
fi


ERASE_BLOCK_SIZE=$SIZE_64K

if [[ ! -d $SRC_DIR ]]
then
    echo "$SRC_DIR is not a valid directory"
    exit 1
fi


#--no-cleanmarkers: Don't add a cleanmarker to every eraseblock
echo "   make JFFS2 file system into '$OUT_FILE' from '$SRC_DIR', size $PAD_SIZE..."

mkfs.jffs2 -r $SRC_DIR -e $ERASE_BLOCK_SIZE -p $PAD_SIZE --no-cleanmarkers --little-endian -o $OUT_FILE || exit 1

umount -r $SRC_DIR|| exit 1

rm -r $SRC_DIR

rm $MTD_BLOCK_DEV

echo "Successfully make new file system $OUT_FILE and umount $SRC_DIR"
exit 0
