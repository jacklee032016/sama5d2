#!/bin/bash
 
source hwConfig.sh

BOOTSTRAP_BIN=767samaBootstrap.bin

CMD="sam-ba -p $HW_PORT $ATMEL_DEVICE -a $APPLET_SERIAL::::20 -c erase::$BOOTSTRAP_SIZE -c writeboot:$BOOTSTRAP_BIN"
echo "program bootstrap $BOOTSTRAP_BIN..."
echo "$CMD"
echo ""

$CMD

