#!/bin/bash
 
DIR_SRC=`dirname "$0"`

source $DIR_SRC/hwConfig.sh

UBOOT_BIN=Binaries/xpld/Sama5d2Xpld.u-boot.bin

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_QSPI -c erase:$UBOOT_START:$UBOOT_SIZE -c write:$UBOOT_BIN:$UBOOT_START"

echo "program u-boot $UBOOT_BIN to APPLET_QSPI..."

echo "$CMD"
echo ""

$CMD
