# /bin/sh

# POST tests of System

DEST_IP=192.168.168.101
DEST_PORT=5000

DATA='{"username":"xyz","passwd":"123", "data":[
		{"media": "video", "ip":"192.168.167.55", "port":8090, "uri":"videoSdp5"},
		{"media": "audio", "ip":"192.168.167.65", "port":8090, "uri":"audioSdp5"},
		{"media": "anc", "ip":"192.168.167.75", "port":8090, "uri":"ancSdp5"}]}'

DATA_HEADER='{"username":"xyz","passwd":"123", '
DATA_CMD='"data":[
		{"media": "video", "ip":"192.168.168.101", "port":5000, "uri":"video/sdp"},
		{"media": "audio", "ip":"192.168.168.101", "port":5000, "uri":"audio/sdp"},
		{"media": "anc", "ip":"192.168.168.101", "port":5000, "uri":"anc/sdp"}]'


DATA_TAIL='}'

DATA_I=$DATA_HEADER$DATA_CMD$DATA_TAIL


echo ""
echo "Testing POST of '$DATA_I' on $$DEST_IP:$DEST_PORT..."

curl  -H "Content-Type:application/json" -X POST -d "$DATA_I" \
		http://$DEST_IP:$DEST_PORT/sdp -v
echo ""
echo ""
