IP Command progressive testing
##############################################
Jack Lee, 08.07, 2019

Following commands using set/get:

system command:

LED: off --> blink; blink-->off;

blink:
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"system":{"blinkLed":1}}'

# error format: no section of "system"
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"blinkLed":1}'

apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"system":{"blinkLed":1}}'

stop blink
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"system":{"blinkLed":0}}'

Reboot
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"system":{"reboot":1}}'

Reset
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"system":{"reset":1}}'

MAC address
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"system":{"mac":"02:01:03:04:05:06"}}'
wrong format of MAC address:
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"system":{"mac":"02:01:03:04:05"}}'

media mode: "Auto|Manual|SDP"
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"system":{"mediaMode":"Manual"}}'

IP Address

DHCP enable:
apiClient -c set -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"system":{"isDhcp":1}}'

DHCP disable and IP address must also be provided:
apiClient -c set -a 192.168.168.102 -m "08:00:27:E7:B7:63" -d  '{"system":{"ip":"192.168.168.181", "mask": "255.255.255.0", "gateway": "192.168.168.1", "isDhcp":0}}'


Video:

apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"intlce":1}}'

apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"video":{"ip": "239.0.10.1", 
    "port": 3333, "sdp": "videoSdp", "height": 1920, "width":1080,
    "fps":"29.97", "colorSpace": "YCbCr-4:4:4", "depth":12, "intlce":1}}'

# only one field is wrong: OK
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"video":{"ip": "239.0.10.1", 
    "port": 3333, "sdp": "videoSdp", "height": 1920, "width":1080,
    "fps":"29.97", "colorSpace": "YCbCr-4:4:4", "depth":12, "intlce2":1}}'

apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"video":{"ip": "239.0.10.1"}}'
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"video":{"ip2": "239.0.10.1"}}'


Audio
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"audio":{"ip": "239.0.10.2", 
    "port":4444, "sdp":"audioSdp", "sample":"44100", "channels":24,"depth":16, "pktSize":"125us"}}'

apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"audio":{"sample":"44100"}}'

Error value:
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"audio":{"sample":"44109"}}'

Error format:
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"audio":{"sample2":"44100"}}'


ANC
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"anc":{"ip": "239.0.10.3","port":4444,"sdp":"ancSdp"}}'

Error IP address:
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"anc":{"ip": "239.0.10.311"}}'

Error field name:
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"anc":{"sdp2":"ancSdp"}}'


SDP:
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"sdp":[
    {"media": "video", "ip":"192.168.167.155", "port":8080, "uri":"videoSdp3"},
    {"media": "audio", "ip":"192.168.167.165", "port":8080, "uri":"audioSdp3"},
    {"media": "anc", "ip":"192.168.167.175", "port":8080, "uri":"ancSdp3"}]}'


security chip:

apiClient -c secure -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"get_id": ""}'

apiClient -c secure -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"get_status": 0}'

# correct key: 64 bytes  
apiClient -c secure -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"set_key": "0987654321fedcbaABCDED12345678900987654321fedcbaABCDED1234567890"}'

# key size is not enough
apiClient -c secure -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"set_key": "0987654321fedcba"}'
 
# wrong key : some letters can be changed into digit
apiClient -c secure -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"set_key": "0987654321fedcbaABCDED12345678900987654321fedcbaABCDED12345678QW"}'



RS232
---------------------------------

::
with command of 'set_param'
++++++++++++++++++++++++++++++++++++
Configure RS232 and send data to RS232:

apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"rs232":{"baudrate": 115200, 
    "databit":8, "parity":"none", "stopbit":1, "data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}'

apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"rs232":{"baudrate": 115200, 
    "databit":7, "parity":"none", "stopbit":1, "data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}'

	
with command of 'send_data_rs232'
++++++++++++++++++++++++++++++++++++
Only send data to RS232(as specs): 

apiClient -c rs -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}'

error data: invalidate code
apiClient -c rs -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"data":"1234567980abcdefGV","isFeedback":1, "waitTime":1000}'

error data: odd number of letters, is not even number
apiClient -c rs -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"data":"1234567980abcdeff","isFeedback":1, "waitTime":1000}'
	

with REST API
++++++++++++++++++++++++++++++++++++
curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}' \
	http://192.168.168.101:5000/rs232 -v 
	

IR
---------------------------------

::
with command of 'set_param'
++++++++++++++++++++++++++++++++++++
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"IR":{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}'

	
with command of 'send_data_ir'
++++++++++++++++++++++++++++++++++++
apiClient -c ir -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}'

error data: invalidate code
apiClient -c ir -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"data":"1234567980abcdefgh","isFeedback":1, "waitTime":1000}'

error data: odd number of letters, is not even number
apiClient -c ir -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"data":"1234567980abcdef345","isFeedback":1, "waitTime":1000}'


other command:

apiClient -c status

apiClient -c status -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"authen": 1}'

apiClient -c status -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"authen": 0}'


apiClient -c status -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"debugRest": 1}'

apiClient -c status -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"debugRest": 0}'


apiClient -c status -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"debugCmd": 1}'

apiClient -c status -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"debugCmd": 0}'


apiClient -c status -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"debugSdp": 1}'

apiClient -c status -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"debugSdp": 0}'

