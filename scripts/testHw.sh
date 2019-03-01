#!/bin/bash
 
DIR_SRC=`dirname "$0"`

source $DIR_SRC/hwConfig.sh

# CMD_READ=readcfg:fuse
CMD_READ=readcfg:bscr

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_INFO -c $CMD_READ"
echo "read boot configuration from $HW_DEVICE..."
echo "$CMD"
echo ""

$CMD
# sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_INFO -c $CMD_READ
