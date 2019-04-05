var isDevelopment = true;
var HTTP_PORT = 80; //80 or 8080

if (isDevelopment == true) {
	var GET_STATUS = "get_status.xml";
	var GET_OUTPUT = "get_output.xml";
	var GET_SYS = "get_sys.xml";
	var SET_SYS = "set_sys";
	var SET_OUTPUT = "set_output";
	var REBOOT = "reboot";
	var RESET = "reset";
} else { // production
	var hostip = window.location.host + ":" + HTTP_PORT;
	var GET_STATUS = "http://" + hostip + "/get_status";
	var GET_OUTPUT = "http://" + hostip + "/get_output";
	var GET_SYS = "http://" + hostip + "/get_sys";
	var SET_SYS = "http://" + hostip + "/set_sys";
	var SET_OUTPUT = "http://" + hostip + "/set_output";
	var REBOOT = "http://" + hostip + "/reboot";
	var RESET = "http://" + hostip + "/reset";
}

function ShowWifi() {
	$.ajax({
		type : "GET",
		url : GET_SYS,
		dataType : "xml",
		cache : false,
		success : function(data) {
			var wifi_dev_exist = $(data).find("sys").children("wifi_dev_exist")
					.text();
			if (wifi_dev_exist == 0) {
				$("#wifi").hide();
			} else {
				$("#wifi").show();
			}
		}
	});
}

function GetHTTPPort() {
	
	$.ajax({
		type : "GET",
		url : GET_SYS,
		dataType : "xml",
		cache : false,
		success : function(data) {
			var http_port = $(data).find("http_port").text();

			if (http_port == "") {
				return (false);
			} else {
				return (http_port);
			}						
		}
	});		
}
