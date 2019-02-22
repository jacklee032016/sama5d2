#!/bin/bash
 
source hwConfig.sh

DTB_BIN=at91-sama5d2_xplained.dtb

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_QSPI -c erase:$DTB_START:$DTB_SIZE -c write:$DTB_BIN:$DTB_START"

echo "program kernel $KERNEL_BIN..."
#echo ""

echo "$CMD"
echo ""

$CMD
