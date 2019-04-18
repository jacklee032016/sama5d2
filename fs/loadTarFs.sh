#!/bin/bash

## Script to mount jffs2 image into local directory
## Jack Lee, Apr.3rd, 2009

#if [[ $# -lt 2 ]]
#then
#    echo "Usage: $0 FSNAME.JFFS2 MOUNTPOINT [ERASEBLOCK_SIZE]"
#    exit 1
#fi

DIR_SRC=`dirname "$0"`
source $DIR_SRC/params.sh


#if [ "$(whoami)" != "root" ]
#then
#    echo "$0 must be run as root!"
#    exit 1
#fi

#if [[ ! -e $SRC_FILE ]]
#then
#    echo "$SRC_FILE does not exist"
#    exit 1
#fi

if [[ ! -d $SRC_DIR ]]
then
    echo "$SRC_DIR not exist, create..."
	mkdir -p $SRC_DIR
    # exit 1
fi

cd $SRC_DIR
tar xvzf ../core*.tar.gz 
cd ..
