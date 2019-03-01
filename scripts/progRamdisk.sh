#!/bin/bash
 
DIR_SRC=`dirname "$0"`

source $DIR_SRC/hwConfig.sh

RAMDISK_BIN=ramDisk.gz

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_QSPI -c erase:$RAMDISK_START:$RAMDISK_SIZE -c write:$RAMDISK_BIN:$RAMDISK_START"

echo "program RAM Disk $RAMDISK_BIN..."
#echo ""

echo "$CMD"
echo ""

$CMD
