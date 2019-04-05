<?PHP
    include_once './include/Authentication.php';

	if (isset($_POST['submit']))
	{
		$arrNewValues = $currentDevice->save_changes($_POST);

		//var_dump($arrNewValues);
		//die;

		if ($arrNewValues)
		{
			$currentMessage = new MUXProtocol((string)$login->json_username, (string)$login->json_password);

			$test = $currentMessage->cmd_set_param_player($arrNewValues);
			
			$test = $currentMessage->cmd_set_param($arrNewValues);						
			if ( $test <> true )
			{
				$arrParameters = $currentMessage->hex_command_to_text($test[0]);				
			}
						
			if ($arrParameters['login_ack'] == 'OK' or $test == true )
			{
				sleep(1); // give it a second to find the resolution/fps
				header("Location: ./index.php?link=decoder&message=".urlencode("Settings saved"));
				die();
			}
			else
			{
				//Logger::getInstance()->lwrite('ERROR: ' . print_r($arrParameters) );
				header("Location: ./index.php?link=decoder&message=".urlencode("Error. (". $arrParameters['login_ack'].")"));
				die();				
			}
		}
		else
		{
			;
		}

		//echo "<SCRIPT type='text/javascript'>submit_main('decoder.php','No changes made');</script>";
		header("Location: ./index.php?link=decoder&message=".urlencode("No changes made"));
		die();
	}
	else
	{
		;
	}
?>

<?PHP
	//var_dump($currentDevice);
?>

<DIV id="forms">
	<FORM method="post" id="formDecoder" action="decoder.php">
		<DIV class="form">
			<DIV class="title">
				<H2>Protocol</H2>
			</DIV>
			<DIV class="fields">

				<DIV class="field">
					<?php echo $currentDevice->htmlVideoSizeSelectRX();?>					
					<span class="input_label_right">Input: <?php echo ($currentDevice->get('vid_width_I')) ? $currentDevice->get('vid_width_I')."x".$currentDevice->get('vid_Height_I')." @ ".$currentDevice->get('vid_fps_I') : "Off"; ?></span>
				</DIV>

				<DIV class="field">
					<LABEL for="deep_color">Deep Color:</LABEL> 
					<SELECT id="deep_color" name="deep_color">
						<OPTION value="3" <?PHP if ($currentDevice->get('deep_color') == "3") echo "selected";?>>AUTO</OPTION>
						<OPTION value="2" <?PHP if ($currentDevice->get('deep_color') == "2") echo "selected";?>>12</OPTION>
						<OPTION value="1" <?PHP if ($currentDevice->get('deep_color') == "1") echo "selected";?>>10</OPTION>
						<OPTION value="0" <?PHP if ($currentDevice->get('deep_color') == "0") echo "selected";?>>8</OPTION>
					</SELECT>
				</DIV>				

				<DIV class="field, tooltip">
					<LABEL for="src_onplaying">URL:</LABEL> 
					<INPUT id="src_onplaying" name="src_onplaying" maxlength="128" size="40" type="text" autocomplete=off class="input_text" value="<?php echo $currentDevice->get('src_onplaying'); ?>"/> 
					<SELECT id="play_state" name="play_state">
						<OPTION value="0" <?PHP if ($currentDevice->get('play_state') == "0") echo "selected";?>>Disabled</OPTION>
						<OPTION value="1" <?PHP if ($currentDevice->get('play_state') == "1") echo "selected";?>>Enabled</OPTION>
					</SELECT>
					<BR />
					<SPAN class="tooltiptext">
						The following examples show valid URL formats: <BR />
						<BR />
						TS URL:  &nbsp;&nbsp; http://192.168.168.1:8083/video1.ts <BR />
						HLS URL:  &nbsp;&nbsp;http://192.168.168.1:8083/video1.m3u8 <BR />
						FLV URL:  &nbsp;&nbsp;http://192.168.168.1:8083/video1.flv <BR />
						RTSP URL: &nbsp;rtsp://192.168.168.1:10001/video1.rtsp <BR />
						RTMP URL: &nbsp;rtmp://192.168.1.50/video1.rtmp <BR />
						Multicast: udp://@239.100.0.17:37000<BR />
					</SPAN>

				</DIV>

				<DIV class="field">
					<LABEL for="low_delay">Low Delay:</LABEL> 
					<SELECT id="low_delay" name="low_delay">
						<OPTION value="0" <?PHP if ($currentDevice->get('low_delay') == "0") echo "selected";?>>Off</OPTION>
						<OPTION value="1" <?PHP if ($currentDevice->get('low_delay') == "1") echo "selected";?>>On</OPTION>						
					</SELECT>
				</DIV>

				<!--
				<DIV class="field">
					<LABEL for="repeatable">Repeatable:</LABEL> 
					<SELECT id="repeatable" name="repeatable">
						<OPTION value="0" <?PHP if ($currentDevice->get('repeatable') == "0") echo "selected";?>>Disabled</OPTION>
						<OPTION value="1" <?PHP if ($currentDevice->get('repeatable') == "1") echo "selected";?>>Enabled</OPTION>
					</SELECT>
				</DIV>
				-->

				<DIV class="field">
					<LABEL for="isDipOn">Dipswitch:</LABEL> 
					<SELECT id="isDipOn" name="isDipOn">
						<OPTION value="0" <?PHP if ($currentDevice->get('isDipOn') == 0) echo "selected";?>>Disabled</OPTION>
						<OPTION value="1" <?PHP if ($currentDevice->get('isDipOn') == 1) echo "selected";?>>Enabled</OPTION>
					</SELECT>
				</DIV>
				
				<BR /><BR />

				<DIV class="field">
					<INPUT name="submit" type="submit" class="btnSubmit" value="Apply" />
					<INPUT name="ResetButton" type="reset" class="btnReset" value="Reset" id="ResetButton" />
				</DIV>


				<BR /><BR />

			</DIV>
		</DIV>
	</FORM>
</DIV>


<script type="text/javascript">
$('#formDecoder').submit(function() {
    // DO STUFF...	
    return IsValidated(); // return false to cancel form action
});
</script>

<SCRIPT type="text/javascript">

	//startup
	//SetType();

    document.getElementById('ResetButton').onclick= function() {
		$("#formEncoder")[0].reset();
		//SetType();
 	};

	
	function GetInputAjax() {
		// ShowWifi();
		$.ajax({
			type : "GET",
			url : GET_OUTPUT,
			dataType : "xml",
			cache : false,
			success : function(data) {
				var venc_framerate = $(data).find("venc_framerate").text();
				$("#venc_framerateTxt").attr("value", venc_framerate);

				var venc_gop = $(data).find("venc_gop").text();
				$("#venc_gopTxt").attr("value", venc_gop);
				var venc_width_height_same_as_input = $(data).find(
						"venc_width_height_same_as_input").text();
				var width = $(data).find("vi_cap_width").text();
				var height = $(data).find("vi_cap_height").text();
				var des_width = $(data).find("venc_width").text();
				var des_height = $(data).find("venc_height").text();
				var VideoSizehtml = GetVideoSize(venc_width_height_same_as_input,
						width, height, des_width, des_height);
				$("#Videosize").html(VideoSizehtml);

				var venc_rc_mode = $(data).find("venc_rc_mode").text();
				$("#venc_rc_modeSle").attr("value", venc_rc_mode);

				var venc_bitrate = $(data).find("venc_bitrate").text();
				$("#venc_bitrateTxt").attr("value", venc_bitrate);

				var venc_profile = $(data).find("venc_profile").text();
				$("#venc_profile").attr("value", venc_profile);

				var http_ts_uri = $(data).find("http_ts_uri").text();
				$("#http_ts_uriTxt").attr("value", http_ts_uri);

				var http_ts_enable = $(data).find("http_ts_enable").text();
				$("#http_ts_enableSle").attr("value", http_ts_enable);

				var http_flv_uri = $(data).find("http_flv_uri").text();
				$("#http_flv_uriTxt").attr("value", http_flv_uri);

				var http_flv_enable = $(data).find("http_flv_enable").text();
				$("#http_flv_enableSle").attr("value", http_flv_enable);

				var rtsp_uri = $(data).find("rtsp_uri").text();
				$("#rtsp_uriTxt").attr("value", rtsp_uri);

				var rtsp_enable = $(data).find("rtsp_enable").text();
				$("#rtsp_enableSle").attr("value", rtsp_enable);

				var rtmp_publish_uri = $(data).find("rtmp_publish_uri").text();
				$("#rtmp_publish_uri").attr("value", rtmp_publish_uri);

				var rtmp_enable = $(data).find("rtmp_enable").text();
				$("#rtmp_enableSle").attr("value", rtmp_enable);

				var http_hls_uri = $(data).find("http_hls_uri").text();
				$("#http_hls_uriTxt").attr("value", http_hls_uri);
				var http_hls_enable = $(data).find("http_hls_enable").text();
				$("#http_hls_enableSle").attr("value", http_hls_enable);

				var multicast_enable = $(data).find("multicast_enable").text();
				$("#multicast_enable").attr("value", multicast_enable);
				var multicast_ip = $(data).find("multicast_ip").text();
				$("#multicast_ip").attr("value", multicast_ip);
				var multicast_port = $(data).find("multicast_port").text();
				$("#multicast_port").attr("value", multicast_port);
				var venc_codec = $(data).find("venc_codec").text();
				$("#venc_codec").attr("value", venc_codec);

				SetType();
			}
		});
	}

	function SetType() {
		var venc_codec = $("#venc_codec").val();
		if (venc_codec == "H.264") {
			$("#flv").show();
			$("#rtmp_publish").show();
			$("#venc_profile_div").show();
		} else {
			$("#flv").hide();
			$("#rtmp_publish").hide();
			$("#venc_profile_div").hide();
		}
	}

	function SetInput() {
		var venc_framerate = $("#venc_framerateTxt").val();
		var venc_gop = $("#venc_gopTxt").val();
		var videosize = $("#VideosizeSle").val();
		var venc_width = videosize.split('x')[0];
		var venc_height = videosize.split('x')[1];
		var venc_width_height_same_as_input = 0;
		if (venc_width == "0" && venc_height == "0") {
			venc_width_height_same_as_input = 1;
		}
		var venc_rc_mode = $("#venc_rc_modeSle").val();
		var venc_bitrate = $("#venc_bitrateTxt").val();
		var venc_profile = $("#venc_profile").val();
		var http_ts_uri = $("#http_ts_uriTxt").val();
		var http_flv_uri = $("#http_flv_uriTxt").val();
		var rtsp_uri = $("#rtsp_uriTxt").val();
		var rtmp_publish_uri = $("#rtmp_publish_uri").val();
		var rtmp_enable = $("#rtmp_enableSle").val();
		rtmp_publish_uri = encodeURIComponent(rtmp_publish_uri);
		var http_ts_enable = $("#http_ts_enableSle").val();
		var http_flv_enable = $("#http_flv_enableSle").val();
		var rtsp_enable = $("#rtsp_enableSle").val();
		var http_hls_uri = $("#http_hls_uriTxt").val();
		var http_hls_enable = $("#http_hls_enableSle").val();
		var multicast_enable = $("#multicast_enable").val();
		var multicast_ip = $("#multicast_ip").val();
		var multicast_port = $("#multicast_port").val();
		var venc_codec = $("#venc_codec").val();

		var src_onplaying = $("#src_onplaying").val();

		var validated = IsValidate(venc_framerate, venc_gop, venc_bitrate,
				http_ts_uri, http_flv_uri, rtsp_uri, rtmp_publish_uri,
				http_hls_uri, multicast_ip, multicast_port, src_onplaying);

		if (validated == true) {return true;}
		else {return false;}
	}

	function IsValidated() {
		validated = true;

		//var VidFpsO_1 	= $("#VidFpsO_1").val();
		//var VidBitR_1 	= $("#VidBitR_1").val();
		var TSurl_1 	= $("#TSurl_1").val();
		var isTS_1 		= $("#isTS_1").val();		
		var HLSurl_1 	= $("#HLSurl_1").val();
		var isHLS_1 	= $("#isHLS_1").val();
		var FLVurl_1 	= $("#FLVurl_1").val();
		var isFLV_1 	= $("#isFLV_1").val();
		//var HTTPport_1	= $("#HTTPport_1").val();		
		var RTSPurl_1 	= $("#RTSPurl_1").val();
		//var RTSPport_1	= $("#RTSPport_1").val();
		var isRTSP_1 	= $("#isRTSP_1").val();
		//var RTMPurl_1 	= $("#RTMPurl_1").val();
		//var RTMPport_1 	= $("#RTMPport_1").val();
		//var isRTMP_1 	= $("#isRTMP_1").val();
		var MCASTurl_1 	= $("#MCASTurl_1").val();
		//var MCASTport_1 = $("#MCASTport_1").val();
		var isMCAST_1 	= $("#isMCAST_1").val();

		var src_onplayingurl_1 	= $("#src_onplaying").val();

		//var IntVidFpsO_1 = IsInteger(VidFpsO_1);
		//var IntVidBitR_1 = IsInteger(VidBitR_1);
		//var IntHTTPport_1 = IsInteger(HTTPport_1);		
		//var IntRTSPport_1 = IsInteger(RTSPport_1);
		//var IntRTMPport_1 = IsInteger(RTMPport_1);
		//var IntMCASTport_1 = IsInteger(MCASTport_1);

		if (	TSurl_1 == "" ||
				HLSurl_1 == "" || FLVurl_1 == "" || RTSPurl_1 == "" ||				
				MCASTurl_1 == "" || src_onplayingurl_1  == "")
		{
			alert('Content cannot be empty!');
			validated = false;			
		}	
	
		return validated;
	}	
	
	function IsInteger(interger) {
		var IsInteger = true;
		if (parseInt(interger) == interger) {
			IsInteger = true;
		} else {
			IsInteger = false;
		}
		return IsInteger;
	}

	function checkIp(str) {
		var ss = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
		return ss.test(str);
	}

	function GetVideoSize(venc_width_height_same_as_input, width, height,
			des_width, des_height) {
		var arrWidthTxt = "";
		var IsSlected = 0;
		var result = '<LABEL for="VideosizeSle"> Encoded size:</LABEL><select id="VideosizeSle" name="select_big">';
		width = parseInt(width);
		var arrWidth = new Array('0x0', '1920x1080', '1680x1056', '1280x720',
				'1024x576', '850x480', '720x576', '720x540', '720x480', '720x404',
				'704x576', '640x480', '640x360', '608x448', '544x480', '480x480',
				'480x384', '480x360', '480x320', '480x272', '480x270', '400x320',
				'400x224', '352x480', '352x228', '320x256', '320x240', '320x180',
				'240x180', '176x144');
		var j = 0;
		if (venc_width_height_same_as_input == "1") {
			arrWidthTxt = "Same as input";
			result += '<option value="' + arrWidth[0] + '" selected ="selected">'
					+ arrWidthTxt + '</option>';
			IsSlected = 1;
			j = 1;
		}
		for (var i = j; i < arrWidth.length; i++) {
			if (arrWidth[i] == "0x0") {
				arrWidthTxt = "Same as input";
			} else {
				arrWidthTxt = arrWidth[i];
			}

			if (IsSlected == 1) {
				result += '<option value="' + arrWidth[i] + '">' + arrWidthTxt
						+ '</option>';
			} else {
				var str_before = arrWidth[i].split('x')[0];
				str_before = parseInt(str_before);
				var str_after = arrWidth[i].split('x')[1];
				str_after = parseInt(str_after);
				if (width >= str_before) {
					if (des_width == str_before && des_height == str_after) {
						result += '<option value="' + arrWidth[i]
								+ '" selected ="selected">' + arrWidthTxt
								+ '</option>';
						IsSlected = 1;
					} else {
						result += '<option value="' + arrWidth[i] + '">'
								+ arrWidthTxt + '</option>';
					}
				}
			}
		}

		result += '</select>';
		return result;
	}	
</SCRIPT>
