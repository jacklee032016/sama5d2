# /bin/sh

# POST tests of System

DEST_IP=192.168.168.101
DEST_PORT=5000

DATA='{"username":"xyz","passwd":"123", "data":{"model":"744-TXRX", 
				"product":"SdiOverIP-774","name":"774-TX01","ver":"01.01-001", 
      			"ip":"192.168.168.181", "mask":"255.255.255.0","gateway":"192.168.168.1", 
				"mac":"02:01:03:04:05:06", "isDhcp":1, "isDipOn":0, "reset":1, "reboot": 0, "blinkLed": 0}}'

DATA_HEADER='{"username":"xyz","passwd":"123", '
DATA_CMD='"data":{"model":"744-TXRX", 
				"product":"SdiOverIP-774","name":"774-TX01","ver":"01.01-001", 
      			"ip":"192.168.168.181", "mask":"255.255.255.0","gateway":"192.168.168.1", 
				"mac":"02:01:03:04:05:06", "isDhcp":1, "isDipOn":0, "reset":1, "reboot": 0, "blinkLed": 0}'

DATA_CMD='"data":{"reset":1}'

DATA_TAIL='}'

DATA_I=$DATA_HEADER$DATA_CMD$DATA_TAIL


echo ""
echo "Testing POST of '$DATA_I' on $$DEST_IP:$DEST_PORT..."

curl  -H "Content-Type:application/json" -X POST -d "$DATA_I" \
		http://$DEST_IP:$DEST_PORT/system -v
echo ""
echo ""
