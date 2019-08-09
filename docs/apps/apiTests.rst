REST API Testing
##############################################
Jack Lee, 04.22, 2019


blink:

apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"system":{"blinkLed":1}}'


Commands tests
================================

system 
---------------------------------

::

    apiClient -c set -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  "{\"system\":{\"model\": \"744-TX/RX\", 
				\"product\": \"SdiOverIP-774\", \"name\": \"774-TX01\", \"ver\": \"01.01-001\", 
      			\"ip\": \"192.168.168.181\", \"mask\": \"255.255.255.0\", \"gateway\": \"192.168.168.1\", 
				\"mac\":\"02:01:03:04:05:06\", \"isDhcp\" : 1, \"isDipOn\": 0, \"reset\": 0, \"reboot\": 0, \"blinkLed\": 0}}"

apiClient -c set -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"system":{"model": "744-TX/RX", 
"product": "SdiOverIP-774", "name": "774-TX01", "ver": "01.01-001", 
"ip":"192.168.168.181", "mask": "255.255.255.0", "gateway": "192.168.168.1", 
"mac":"02:01:03:04:05:06", "isDhcp":1, "isDipOn": 0, "reset": 0, "reboot":0, "blinkLed":0}}'


apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"system":{"model": "744-TX/RX", 
"product": "SdiOverIP-774", "name": "774-TX01", "ver": "01.01-001", 
"ip":"192.168.168.181", "mask": "255.255.255.0", "gateway": "192.168.168.1", 
"mac":"02:01:03:04:05:06", "isDhcp":1, "isDipOn": 0, "reset": 0, "reboot":0, "blinkLed":0}}'

video 
---------------------------------

::

with command of 'set_param'
++++++++++++++++++++++++++++++++++++
  apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"video":{"ip": "239.0.10.1", 
    "port": 3333, "sdp": "videoSdp", "height": 1920, "width":1080,
    "fps":"29.97", "colorSpace": "YCbCr-4:4:4", "depth":12, "intlce":1}}'

with REST API
++++++++++++++++++++++++++++++++++++
curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"ip": "239.0.19.1", 
    "port": 3343, "sdp": "videoSdp2", "height": 1920, "width":1080,
    "fps":"29.97", "colorSpace": "YCbCr-4:4:4", "depth":12, "intlce":1}}' \
	http://192.168.168.101:5000/video -v
	
	
audio 
---------------------------------

::

with command of 'set_param'
++++++++++++++++++++++++++++++++++++
  apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"audio":{"ip": "239.0.10.2", 
    "port":4444, "sdp":"audioSdp", "sample":"44100", "channels":24,"depth":16, "pktSize":"125us"}}'

with REST API
++++++++++++++++++++++++++++++++++++
curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"ip": "239.0.19.2", 
    "port":4454, "sdp":"audioSdp2", "sample":"48000", "channels":24,"depth":16, "pktSize":"125us"}}' \
	http://192.168.168.101:5000/audio --trace-ascii  /dev/stdout  

	
ANC 
---------------------------------

::

with command of 'set_param'
++++++++++++++++++++++++++++++++++++
  apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"anc":{"ip": "239.0.10.3","port":4444,"sdp":"ancSdp"}}'

  
with REST API
++++++++++++++++++++++++++++++++++++
curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"ip": "239.0.19.3","port":4445,"sdp":"ancSdp2"}}' \
	http://192.168.168.101:5000/anc --trace-ascii  /dev/stdout  

SDP 
---------------------------------

::

with command of 'set_param'
++++++++++++++++++++++++++++++++++++
  apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"sdp":[
    {"media": "video", "ip":"192.168.167.155", "port":8080, "uri":"videoSdp3"},
    {"media": "audio", "ip":"192.168.167.165", "port":8080, "uri":"audioSdp3"},
    {"media": "anc", "ip":"192.168.167.175", "port":8080, "uri":"ancSdp3"}]}'

	
with REST API
++++++++++++++++++++++++++++++++++++
curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":[
    {"media": "video", "ip":"192.168.167.55", "port":8090, "uri":"videoSdp5"},
    {"media": "audio", "ip":"192.168.167.65", "port":8090, "uri":"audioSdp5"},
    {"media": "anc", "ip":"192.168.167.75", "port":8090, "uri":"ancSdp5"}]}' \
	http://192.168.168.101:5000/sdp --trace-ascii  /dev/stdout
	

RS232
---------------------------------

::
with command of 'set_param'
++++++++++++++++++++++++++++++++++++
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"rs232":{"baudrate": 115200, 
    "databit":8, "parity":"none", "stopbit":1, "data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}'

	
with command of 'send_data_rs232'
++++++++++++++++++++++++++++++++++++
apiClient -c rs -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}'
	

with REST API
++++++++++++++++++++++++++++++++++++
curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}' \
	http://192.168.168.101:5000/rs232 -v 
	

IR
---------------------------------

::
with command of 'set_param'
++++++++++++++++++++++++++++++++++++
apiClient -c set -a 192.168.168.101 -m "02:01:03:04:05:06" -d  '{"rs232":{"baudrate": 115200, 
    "databit":8, "parity":"none", "stopbit":1, "data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}'

	
with command of 'send_data_ir'
++++++++++++++++++++++++++++++++++++
apiClient -c ir -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}'
	

with REST API
++++++++++++++++++++++++++++++++++++
curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}' \
	http://192.168.168.101:5000/IR -v 
	

	
	
security
---------------------------------

::

with command of 'set_param'
++++++++++++++++++++++++++++++++++++
apiClient -c set -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"security":{"get_id": ""}}'

apiClient -c set -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"security":{"get_status": 0}}'
  
apiClient -c set -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"security":{"set_key": "0987654321fedcbaABCDED12345678900987654321fedcbaABCDED1234567890"}}'


with command of 'security_check'
++++++++++++++++++++++++++++++++++++
apiClient -c secure -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"get_id": ""}'

apiClient -c secure -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"get_status": 0}'
  
apiClient -c secure -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"set_key": "0987654321fedcba"}'


with REST API
++++++++++++++++++++++++++++++++++++
curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"get_id": ""}}' \
	http://192.168.168.101:5000/security -v 

curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"get_status": 0}}' \
	http://192.168.168.101:5000/security -v 
	
curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":[{"set_key": "0987654321fedcbaABCDED12345678900987654321fedcbaABCDED1234567890"}]}' \
	http://192.168.168.101:5000/security --trace-ascii  /dev/stdout

curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":[{"set_key": "0987654321fedcbaABCDED12345678900987654321fedcbaABCDED12345678QW"}]}' \
	http://192.168.168.101:5000/security --trace-ascii  /dev/stdout

curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":[{"set_key": "0987654321fedcbaABCDED12345678900987654321fedcbaABCDED12345"}]}' \
	http://192.168.168.101:5000/security --trace-ascii  /dev/stdout
	
Notes:

* `--trace-ascii  /dev/stdout` used to debug uploading data of curl

	
get_param
---------------------------------

::

   apiClient -c find 
	

String format in shell script
---------------------------------

::
	
    apiClient -c set -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"system":{"model":"744-TX/RX"}}'

    apiClient -c set -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"system": 
	  {"model":"744-TX/RX"}}'

    apiClient -c set -a 192.168.168.101 -m "08:00:27:E7:B7:63" -d  '{"system": \
	  {"model":"744-TX/RX"}}'
	
Show threads of one process
================================
::

   curl  -H "Content-Type:application/json" -X GET -d '{"username":"xyz","passwd":"123"}' http://localhost:5000/

   curl  -H "Content-Type:application/json" -X POST -d '{"username":"admin","passwd":"admin", "data":[{"ip":"192.168.168.191"}]}' \
	http://localhost:5000/system -v 

	
   curl  -H "Content-Type:application/json" -X POST -d '{"username":"admin","passwd":"admin", "data":{"model": "744-TX/RX",
	"ip":"192.168.168.191"}}' \
	http://localhost:5000/system -v 
