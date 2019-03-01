#!/bin/bash
 
DIR_SRC=`dirname "$0"`

source $DIR_SRC/hwConfig.sh

DTB_BIN=at91-sama5d2_xplained.dtb

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_QSPI -c erase:$ENV_START:$ENV_SIZE "

echo "clear Environment of u-boot ..."
#echo ""

echo "$CMD"
echo ""

$CMD
