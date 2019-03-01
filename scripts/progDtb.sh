#!/bin/bash
 
DIR_SRC=`dirname "$0"`

source $DIR_SRC/hwConfig.sh

DTB_BIN=at91-sama5d2_xplained.dtb

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_QSPI -c erase:$DTB_START:$DTB_SIZE -c write:$DTB_BIN:$DTB_START"

echo "program Device Tree $DTB_BIN..."
#echo ""

echo "$CMD"
echo ""

$CMD
