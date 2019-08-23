#!/bin/bash

DIR_SRC=`dirname "$0"`

source $DIR_SRC/hwConfig.sh

BOOTSTRAP_BIN=at91bootstrap-sama5d2_xplained.bin
BOOTSTRAP_BIN=muxlab_500768-dataflashboot--3.8.10.bin
BOOTSTRAP_BIN=Mux7XX-bootStrap-3.8.10.bin
BOOTSTRAP_BIN=../Binaries/xpld/7Sama5d2Xpld-bootStrap-3.8.10.bin

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_SERIAL::::20 -c erase::$BOOTSTRAP_SIZE -c writeboot:$BOOTSTRAP_BIN"
echo "program bootstrap $BOOTSTRAP_BIN to $APPLET_SERIAL..."
echo "$CMD"
echo ""

$CMD

