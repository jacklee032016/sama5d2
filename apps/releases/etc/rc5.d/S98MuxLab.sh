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


#detect if new upgrade firmware present, then reboot 
/opt/mLab/firmware_detect &


echo "install w1 DS28e15 driver..."
insmod $MUX_LAB_HOME/drv/w1Ds28e15.ko
echo ""

echo "start PTP service..."
ptp4l &
echo ""

echo "start REST API service..."
# LOG_DIR=/var/volatile
LOG_DIR=/run

python3 $MUX_LAB_HOME/apiSvr/app.py > $LOG_DIR/restapi.log 2>&1 &
echo ""


echo "start MuxLab service..."
muxMgr
echo ""
