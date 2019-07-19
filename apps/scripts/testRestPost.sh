# /bin/sh

# POST tests of REST APIs

declare -a URLS="
	/ 
	/system
	/video
	/audio
	/anc
	/sdp
	/rs232
	/IR
	/security
	/others
	/video/sdp
	/audio/sdp
	/anc/sdp
"

echo "start REST API POST services tests..."

if [ -z "$1" ]
  then
    echo "Define board IP address as '$0 IP'"
    exit 0
fi

if [ -z "$3" ]
  then
	REST_ROOT=/api
else
	REST_ROOT=:5000
fi


testVideo()
{
	echo ""
	echo "Testing POST of 'video' on $1$2..."
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"ip": "239.0.19.1", 
		"port": 3343, "sdp": "videoSdp2", "height": 1920, "width":1080,
		"fps":"29.97", "colorSpace": "YCbCr-4:4:4", "depth":12, "intlce":1}}' \
		http://$1$2 -v
	echo ""
	echo ""
}

testAudio()
{
	echo ""
	echo "Testing POST of 'audio' on $1$2..."
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"ip": "239.0.19.2", 
		"port":4454, "sdp":"audioSdp2", "sample":"48000", "channels":24,"depth":16, "pktSize":"125us"}}' \
		http://$1$2 -v
	echo ""
	echo ""
}

testAnc()
{
	echo ""
	echo "Testing POST of 'anc' on $1$2..."
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"ip": "239.0.19.3","port":4445,"sdp":"ancSdp2"}}' \
		http://$1$2 -v
	echo ""
	echo ""
}

testSdp()
{
	echo ""
	echo "Testing POST of 'SDP' on $1$2..."
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":[
		{"media": "video", "ip":"192.168.167.55", "port":8090, "uri":"videoSdp5"},
		{"media": "audio", "ip":"192.168.167.65", "port":8090, "uri":"audioSdp5"},
		{"media": "anc", "ip":"192.168.167.75", "port":8090, "uri":"ancSdp5"}]}' \
		http://$1$2 -v
	echo ""
	echo ""
}

testRs232()
{
	echo ""
	echo "Testing POST of 'RS232' on $1$2..."
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}' \
		http://$1$2 -v
	echo ""
	echo ""
}


testIR()
{
	echo ""
	echo "Testing POST of 'IR' on $1$2..."
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"data":"1234567980abcdef","isFeedback":1, "waitTime":1000}}' \
		http://$1$2 -v
	echo ""
	echo ""
}

testSecurity()
{
	echo ""
	echo "Testing POST of 'Security' on $1$2..."
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"get_id": ""}}' \
		http://$1$2 -v
	echo ""
	echo ""
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"get_status": 0}}' \
		http://$1$2 -v
	echo ""
	echo ""
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":[{"set_key": "fedcba0987654321"}]}' \
		http://$1$2 -v
	echo ""
	echo ""
}

testSystem()
{
	echo ""
	echo "Testing POST of 'System' on $1$2..."
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"model":"744-TXRX", 
				"product":"SdiOverIP-774","name":"774-TX01","ver":"01.01-001", 
      			"ip":"192.168.168.181", "mask":"255.255.255.0","gateway":"192.168.168.1", 
				"mac":"02:01:03:04:05:06", "isDhcp":1, "isDipOn":0, "reset": 0, "reboot": 0, "blinkLed": 0}}' \
		http://$1$2 -v
	echo ""
	echo ""
}

testOthers()
{
	echo ""
	echo "Testing POST of 'Others' on $1$2..."
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"ip": "239.0.19.1", 
		"port": 3343, "sdp": "videoSdp2", "height": 1920, "width":1080,
		"fps":"29.97", "colorSpace": "YCbCr-4:4:4", "depth":12, "intlce":1}}' \
		http://$1$2 -v
	echo ""
	echo ""
}


# REQ_URI=$REST_ROOT$uri

# echo "$#"
if [ $# -le 1 ]; then
		echo "$0: Missing argument, 'video|audio|anc|sdp|rs232|IR|security|system|others'"
		echo "$0: usage: myscript name"
		exit 1
fi

case $2 in
    video)
    testVideo $1 $REST_ROOT/video
    exit 1
    ;;

    audio)
    testAudio $1 $REST_ROOT/audio
    exit 1
    ;;
    
    anc)
    testAnc $1 $REST_ROOT/anc
    exit 1
    ;;
	
    sdp)
    testSdp $1 $REST_ROOT/sdp
    exit 1
    ;;
	
	rs232)
    testRs232 $1 $REST_ROOT/rs232
    exit 1
    ;;
    
    IR)
    testIR $1 $REST_ROOT/IR
    exit 1
    ;;
    
	security)
    testSecurity $1 $REST_ROOT/security
    exit 1
    ;;
    
    system)
    testSystem $1 $REST_ROOT/system
    exit 1
    ;;

	others)
    testOthera $1 $REST_ROOT/others
    exit 1
    ;;
    
    
    *)    # unknown option
    echo "'video|audio|anc|sdp|rs232|IR|security|system|others'"
#    shift # past argument
		exit 1
    ;;
esac
