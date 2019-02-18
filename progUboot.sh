#!/bin/bash
 
source hwConfig.sh

UBOOT_BIN=u-boot.bin

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_QSPI -c erase:$BOOTSTRAP_SIZE:$UBOOT_SIZE -c write:$UBOOT_BIN:$BOOTSTRAP_SIZE"

echo "program bootstrap $BOOTSTRAP_BIN..."
#echo ""

# sam-ba -p $HW_PORT -d $HW_DEVICE -a $APPLET_QSPI -c erase:$BOOTSTRAP_SIZE:$UBOOT_SIZE -c write:$UBOOT_BIN:$BOOTSTRAP_SIZE
echo "$CMD"
echo ""

$CMD
