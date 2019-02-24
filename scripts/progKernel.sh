#!/bin/bash
 
source hwConfig.sh

KERNEL_BIN=uImage

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_QSPI -c erase:$KERNEL_START:$KERNEL_SIZE -c write:$KERNEL_BIN:$KERNEL_START"

echo "program kernel $KERNEL_BIN..."
#echo ""

echo "$CMD"
echo ""

$CMD
