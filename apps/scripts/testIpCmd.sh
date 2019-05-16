# /bin/sh

# read-only REST API tests


if [ -z "$1" ]
	then
		echo "No parameters defined, so find first board LAN"
		apiClient -c find
		echo "Use IP address and MAC address from this command to test other IP Commands: '$0 IP MAC Type(get|set|rs232|secure|ir)'"
		echo ""
		exit 0
fi

if [ -z "$2" ]
	then 
		echo "Use IP address and MAC address from this command to test other IP Commands: '$0 IP MAC Type(get|set|rs232|secure|ir)'"
		exit 1
fi		


testRs232()
{
	echo ""
	echo "Case#1: Testing Command of 'send_data_rs232'..."
	apiClient -c rs -a $1 -m $2 -d  '{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}'
	echo ""
	echo ""

	echo "Case#2: Test modifying parameters of RS232 with Command of 'set_param'..."
	apiClient -c rs -a $1 -m $2 -d '{"rs232":{"baudrate": 115200, 
    "databit":8, "parity":"none", "stopbit":1, "data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}'
	echo ""
	  
}

testFind()
{
	echo "Testing Command of 'get_param'..."
	apiClient -c get -a $1 -m $2
}

testSecurity()
{
	echo ""
	echo "testing 'secure' command..."
	echo "Testing Command of 'security_check'..."
	echo "   Test 'get_id'..."
	apiClient -c secure -a $1 -m $2 -d  '{"get_id": ""}'
	echo ""
	echo ""

	echo "   Test 'get_status'..."
	apiClient -c secure -a $1 -m $2 -d  '{"get_status": 0}'
	echo ""
	echo ""
	  
	echo "   Test 'set_key'..."
	apiClient -c secure -a $1 -m $2 -d  '{"set_key": "0987654321fedcba"}'
	echo ""
	echo ""
}

testIR()
{
	echo ""
	echo "Testing Command of 'send_data_ir'..."
	apiClient -c ir -a $1 -m $2 -d '{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}'
	echo ""
	echo ""

}

testSystemConfig()
{
	echo ""
	echo "Testing 'set_param' command..."
	echo "   Case#1: Test 'video'..."
	apiClient -c set -a $1 -m $2 -d  '{"video":{"ip": "239.0.10.1", 
		"port": 3333, "sdp": "videoSdp", "height": 1920, "width":1080,
		"fps":"29.97", "colorSpace": "YCbCr-4:4:4", "depth":12, "intlce":1}}'
	echo ""
	echo ""

	echo "   Case#2: Test 'audio'..."
	apiClient -c set -a $1 -m $2 -d '{"audio":{"ip": "239.0.10.2", 
		"port":4444, "sdp":"audioSdp", "sample":"44100", "channels":24,"depth":16, "pktSize":"125us"}}'
	echo ""
	echo ""

	echo "   Case#3: Test 'anc'..."
	apiClient -c set -a $1 -m $2 -d '{"anc":{"ip": "239.0.10.3","port":4444,"sdp":"ancSdp"}}'
	echo ""
	echo ""

	echo "   Case#4: Test 'sdp'..."
	apiClient -c set -a $1 -m $2 -d '{"sdp":[
		{"media": "video", "ip":"192.168.167.155", "port":8080, "uri":"videoSdp3"},
		{"media": "audio", "ip":"192.168.167.165", "port":8080, "uri":"audioSdp3"},
		{"media": "anc", "ip":"192.168.167.175", "port":8080, "uri":"ancSdp3"}]}'
	echo ""
	echo ""

	echo "   Case#5: Test 'system'..."
	apiClient -c set -a $1 -m $2 -d '{"system":{"model": "744-TX/RX", 
		"product": "SdiOverIP-774", "name": "774-TX01", "ver": "01.01-001", 
		"ip":"192.168.168.181", "mask": "255.255.255.0", "gateway": "192.168.168.1", 
		"mac":"02:01:03:04:05:06", "isDhcp":1, "isDipOn": 0, "reset": 0, "reboot":0, "blinkLed":0}}'
	echo ""
	echo ""
}
	

if [ -z "$3" ]
	then 
		testFind $1 $2
		echo "Use IP address and MAC address from this command to test other IP Commands: '$0 IP MAC Type(get|set|rs232|secure|ir)'"
		echo ""
		echo ""
		exit 1
fi		


case $3 in
    get)
    testFind $1 $2
    exit 1
    ;;

    set)
    testSystemConfig $1 $2
    exit 1
    ;;
    
    rs232)
    testRs232 $1 $2
    exit 1
    ;;
    
    secure)
    testSecurity $1 $2
    exit 1
    ;;
    
	ir)
    testIR $1 $2
	exit 1
    ;;
    
    
    *)    # unknown option
    echo "get|set|rs232|secure|ir"
#    shift # past argument
		exit 1
    ;;
esac
