

Simplest POST and GET API
-------------------------------------------

curl  -H "Content-Type:application/json" -X POST \
	-d '{"username":"xyz","passwd":"123", "data":[{"media": "video", "ip":"192.168.168.101", "port":5000, "uri":"video/sdp"}]}' \
	http://192.168.168.101:5000/others -v
	
curl http://192.168.168.101:5000/others -v

curl  -H "Content-Type:application/json" -X GET -d '{"username":"xyz","passwd":"123"}' http://192.168.168.101:5000/others -v
