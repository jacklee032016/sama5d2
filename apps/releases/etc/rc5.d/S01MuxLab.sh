#! /bin/sh
### BEGIN INIT INFO
### END INIT INFO


echo "Configure SFP interface..."
LOG_DIR=/var/log

sfpCtrl >> $LOG_DIR/sfp.log
