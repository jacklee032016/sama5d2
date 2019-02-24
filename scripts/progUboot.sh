#!/bin/bash
 
source hwConfig.sh

UBOOT_BIN=u-boot.bin

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_QSPI -c erase:$UBOOT_START:$UBOOT_SIZE -c write:$UBOOT_BIN:$UBOOT_START"

echo "program u-boot $UBOOT_BIN to APPLET_QSPI..."

echo "$CMD"
echo ""

$CMD