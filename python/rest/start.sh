# /bin/sh
echo "start REST API service..."
python3 app.py > /var/log/restapi.log 2>&1 &

