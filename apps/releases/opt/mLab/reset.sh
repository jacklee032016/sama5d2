#! /bin/sh
### BEGIN INIT INFO
# Provides:          MuxLab Script
# script for reset factory configuration
### END INIT INFO


FLAGS_FILE=/var/factory.sh
NET_INTERFACE=/etc/network/interfaces

if [ ! -f "$FLAGS_FILE" ]; then
    exit
fi

echo "Reset to factory default..."

echo ""
echo "    STEP 1: mount backup package..."
mount -t jffs2 /dev/mtdblock7 /mnt/

source $FLAGS_FILE
# echo " $FLAGS_FILE $MAC_ADDRESS"


PACKAGES=`find /mnt -name "apps_install*.zip"`
echo ""
echo "    STEP 2: find files"
echo "      $PACKAGES"


for FILE in $PACKAGES
do
    /usr/bin/unzip -l $FILE > /dev/null #check validation of zip file
    if [ $? -eq 0 ]; then
         break
    fi
done


if [ -z "$FILE" ]; then
    echo ""
    echo "     No Install package can be found, reset failed"
    exit
fi

echo ""
echo "    STEP 3: extract install package $FILE..."
/usr/bin/unzip -o $FILE -d /


echo ""
echo "    STEP 4: umount file system..."
umount /mnt/


echo ""
echo "    STEP 5: restore MAC address $MAC_ADDRESS"
if [ -z "$MAC_ADDRESS" ]
then
    echo "       MAC_ADDRESS is not configured"
else
    echo "       MAC_ADDRESS restore as $MAC_ADDRESS"
    sed -i "/hwaddress/c\        hwaddress ether $MAC_ADDRESS" $NET_INTERFACE
fi 

echo ""
echo "    clear reset flags..."
rm $FLAGS_FILE  

echo ""
echo "    clear old configuration data..."
rm /etc/mLab/muxConfig.dat


echo "    reboot now..."
reboot


if [ -f "$FLAGS_FILE" ]; then	#double-check if upgrade has not been removed
    rm $FLAGS_FILE
fi


