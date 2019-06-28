# /bin/sh
echo "start REST API service..."
LOG_DIR=/var/log
LOG_DIR=/tmp

python3 app.py > $LOG_DIR/restapi.log 2>&1 &

