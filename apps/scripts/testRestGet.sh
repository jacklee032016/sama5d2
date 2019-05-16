# /bin/sh

# read-only REST API tests

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

echo "start REST API read-only services tests..."

if [ -z "$1" ]
  then
    echo "Define board IP address as '$0 IP'"
    exit 0
fi

if [ -z "$2" ]
  then
	REST_ROOT=/api
else
	REST_ROOT=:5000
fi

ROOT_URL=$REST_ROOT

i=0
for uri in $URLS
	do
		let i+=1
		REQ_URI=$REST_ROOT$uri
		echo "Case#$i: $REQ_URI..."
		curl  -H "Content-Type:application/json" -X GET -d '{"username":"xyz","passwd":"123"}' http://$1$REQ_URI
		echo ""
		echo ""
done



