# /bin/sh

# test commands of REST APIs

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

echo "Begin POST command tests..."

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


cmdReset()
{
	echo ""
	echo "Testing POST of 'video' on $1$2..."
	curl  -H "Content-Type:application/json" -X POST -d '{"username":"xyz","passwd":"123", "data":{"reset": 1}}' \
		http://$1$REST_ROOT/system -v
	echo ""
	echo ""
}


# REQ_URI=$REST_ROOT$uri

# echo "$#"
if [ $# -le 1 ]; then
		echo "$0: IP 'reset|reboot|blink'"
		echo "  usage: $0 IP 'reset|reboot|blink'"
		exit 1
fi

case $2 in
	reset)
	cmdReset $1 $REST_ROOT/system
    exit 1
    ;;

    
    *)    # unknown option
    echo "'reset|reboot|blink'"
#    shift # past argument
		exit 1
    ;;
esac
