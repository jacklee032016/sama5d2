#!/bin/bash

## Script execute after tar has been loaded into minimal
## Jack Lee, Oct.31st, 2009

echo "Remove not used files before pack FS..."
echo ""

FILE_LIST="minimal/usr/bin/phpdbg "

i=0
for file in $FILE_LIST;
do
	let i+=+1
	echo "    removing File #$i $file"
	rm -rf $file
	
done
