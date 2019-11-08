#! /bin/sh
### BEGIN INIT INFO
# Provides:          MuxLab Script
# Short-Description: One of the first scripts to be executed. Starts or stops
#               the bootlogd log program. If this script is called as
#               "stop-bootlogd", it will stop the daemon instead of
#               starting it even when called with the "start" argument.
#
### END INIT INFO

MUX_LAB_HOME=/opt/mLab

chown www-data /var/www

if [ -f /var/www/upgrade ]; then
    sleep 1
    /usr/bin/unzip -l /var/www/upgrade > /dev/null #check validation of zip file
    if [ $? -eq 0 ]; then
        /usr/bin/unzip -o /var/www/upgrade -d /
        rm /var/www/upgrade
        reboot
    fi
    if [ -f /var/www/upgrade ]; then			#double-check if upgrade has not been removed
        rm /var/www/upgrade
    fi
fi

if [ -f /var/www/upgradeFPGA ]; then
  sleep 1
  chmod 777 /var/www/upgradeFPGA
  fwUpdate -c w -f /var/www/upgradeFPGA
  rm /var/www/upgradeFPGA
  reboot
fi

echo "install w1 DS28e15 driver..."
insmod $MUX_LAB_HOME/drv/w1Ds28e15.ko
echo ""

echo "start PTP service..."
ptp4l &
echo ""

echo "start REST API service..."
# LOG_DIR=/var/volatile
# LOG_DIR=/run
LOG_DIR=/var/log

python3 $MUX_LAB_HOME/apiSvr/app.py > $LOG_DIR/restapi.log 2>&1 &
echo ""


echo "start MuxLab service..."
muxMgr
echo ""
