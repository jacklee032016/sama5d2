<?PHP
    include_once './include/Authentication.php';

	if (isset($_POST['submit']))
	{
		$arrNewValues = $currentDevice->save_changes($_POST);

		if ($arrNewValues)
		{
		    $currentMessage = new MUXProtocol((string)$login->json_username, (string)$login->json_password);
			$test = $currentMessage->cmd_set_param($arrNewValues);
			
			//check if good

			$arrParameters = $currentMessage->hex_command_to_text($test[0]);				

			if ($arrParameters['login_ack'] == 'OK')
			{
				header("Location: ./index.php?link=encoder&message=".urlencode("Settings saved"));
				die();
			}
			else
			{
				//Logger::getInstance()->lwrite('ERROR: ' . print_r($arrParameters) );
				header("Location: ./index.php?link=encoder&message=".urlencode("Error. (". $arrParameters['login_ack'].")"));
				die();				
			}
		}
		else
		{
			;
		}

		//echo "<SCRIPT type='text/javascript'>submit_main('encoder.php','No changes made');</script>";
		header("Location: ./index.php?link=encoder&message=".urlencode("No changes made"));
		die();
	}

	echo '<pre>'; print_r($currentDevice); echo '</pre>';
?>

<DIV id="forms">
	<FORM method="post" id="formEncoder" action="encoder.php">
		<DIV class="form">
			<DIV class="title">
				<H2>Protocol</H2>
			</DIV>
			<DIV class="fields">
						
				<DIV class="field">
					<!--<DIV class="select" id="Videosize"><LABEL for="VideosizeSle"> Encoded size:</LABEL></DIV>-->
					<?php echo $currentDevice->htmlVideoSizeSelect();?>
					<span class="input_label_right">Input: <?php echo $currentDevice->get('VidResI_1'); ?></span>
				</DIV>

				<DIV class="field">
					<LABEL for="VidFpsO_1">FPS:</LABEL> 
					<INPUT class="input_fpsgop" name="VidFpsO_1" id="VidFpsO_1" type="text" autocomplete=off size="40" value="<?php echo $currentDevice->get('VidFpsO_1'); ?>"><span class="input_label_right"> [5-60]</span>

					<LABEL class="input_fpsgop" for="GOP_1" >GOP:</LABEL> 
					<INPUT class="input_fpsgop" name="GOP_1" id="GOP_1" type="text" autocomplete=off size="40" value="<?php echo $currentDevice->get('GOP_1'); ?>"><span class="input_label_right"> [5-300]</span>
				</DIV>

				<DIV class="field">
					<LABEL for="isVBR_1">Encoding Method:</LABEL> 
					<SELECT name="isVBR_1" id="isVBR_1" onChange="">
						<OPTION <?PHP if ($currentDevice->get('isVBR_1') == '1') echo "selected";?> value="1">VBR</OPTION>
						<OPTION <?PHP if ($currentDevice->get('isVBR_1') == '0') echo "selected";?> value="0">CBR</OPTION>
					</SELECT>
				</DIV>				

				<DIV class="field">
					<LABEL for="MCASTip_1">Multicast IP:</LABEL> 
					<INPUT id="MCASTip_1" name="MCASTip_1" maxlength="128" size="40" type="text" value="<?php echo $currentDevice->get('MCASTip_1'); ?>"/>
					
					<label class="input_port">Port: </label><INPUT id="MCASTport_1" name="MCASTport_1" maxlength="5" size="40" type="text" class="input_port" value="<?php echo $currentDevice->get('MCASTport_1'); ?>"/>					
					 
					<SELECT id="isMCAST_1" name="isMCAST_1" class="input_enabled">
						<OPTION value="0" <?PHP if ($currentDevice->get('isMCAST_1') == 0) echo "selected";?>>Disabled</OPTION>
						<OPTION value="1" <?PHP if ($currentDevice->get('isMCAST_1') == 1) echo "selected";?>>Enabled</OPTION>
					</SELECT>
				</DIV>

				<DIV class="field">
					<LABEL for="isAudin">Audio Input:</LABEL>
					<SELECT id="isAudin" name="isAudin" class="input_port">
						<OPTION value="AUTO" <?PHP if ($currentDevice->get('isAudioAuto') == 1) echo "selected";?>>AUTO</OPTION>
						<OPTION value="HDMI" <?PHP if (($currentDevice->get('isAudin') == 'HDMI') AND ($currentDevice->get('isAudioAuto') == 0)) echo "selected";?>>HDMI</OPTION>
						<OPTION value="ANALOG" <?PHP if (($currentDevice->get('isAudin') == 'ANALOG') AND ($currentDevice->get('isAudioAuto') == 0)) echo "selected";?>>ANALOG</OPTION>
					</SELECT>

					<span class="input_label_right"> [Currently: <?PHP echo (string)$currentDevice->get('isAudin');?>]</span>

					<DIV class="field-right">
						<LABEL for="AudBitR_1">Audio Bitrate:</LABEL> 
						<INPUT id="AudBitR_1" name="AudBitR_1" maxlength="128" size="40" type="text" class="input_audio" value="<?php echo $currentDevice->get('AudBitR_1'); ?>"/>					
					</DIV>
				</DIV>

				<DIV class="field">
					<LABEL class="">Audio Codec:</LABEL>
					<SELECT id="AudCodec_1" name="AudCodec_1" class="">
						<OPTION value="MP3" <?PHP if ($currentDevice->get('AudCodec_1') == 'MP3') echo "selected";?>>MP3</OPTION>
						<OPTION value="AAC" <?PHP if ($currentDevice->get('AudCodec_1') == 'AAC') echo "selected";?>>AAC</OPTION>
					</SELECT>

					<DIV class="field-right">
						<LABEL class="">Audio Frequency:</LABEL>
						<SELECT id="AudFrq_1" name="AudFrq_1" class="">
							<OPTION value="44100" <?PHP if ($currentDevice->get('AudFrq_1') == '44100') echo "selected";?>>44100</OPTION>
							<OPTION value="48000" <?PHP if ($currentDevice->get('AudFrq_1') == '48000') echo "selected";?>>48000</OPTION>
						</SELECT>					
					</DIV>
				</DIV>				

				<DIV class="field">
					<LABEL for="isDipOn">Dipswitch:</LABEL> 
					<SELECT id="isDipOn" name="isDipOn" class="disabled_enabled">
						<OPTION value="0" <?PHP if ($currentDevice->get('isDipOn') == 0) echo "selected";?>>Disabled</OPTION>
						<OPTION value="1" <?PHP if ($currentDevice->get('isDipOn') == 1) echo "selected";?>>Enabled</OPTION>
					</SELECT>
				</DIV>

				<DIV class="field">
					<INPUT name="submit" type="submit" class="btnSubmit" value="Apply" />
					<INPUT name="ResetButton" type="reset" class="btnReset" value="Reset" id="ResetButton" />
				</DIV>

			</DIV>
		</DIV>
	</FORM>
</DIV>

<SCRIPT type="text/javascript">

	//startup
	SetType();

    document.getElementById('ResetButton').onclick= function() {
		$("#formEncoder")[0].reset();
		SetType();
 	};

 	$('#formEncoder').submit(function() {
 	    // DO STUFF...
 	    return IsValidated(); // return false to cancel form action
 	});

	
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
		var VidCodec_1 	= $("#VidCodec_1").val();		
		if (VidCodec_1 == "H.264-HighProfile" || VidCodec_1 == "H.264-MainProfile" || VidCodec_1 == "H.264-BaselineProfile") {
			$("#flv_publish").show();
			$("#rtmp_publish").show();
			//$("#Encoding_level").show();
		} else {
			$("#flv_publish").hide();
			$("#rtmp_publish").hide();
			//$("#Encoding_level").hide();
		}
	}

	function SetInput() {
		var validated = IsValidate();
		if (validated == true) {return true;}
		else {return false;}
	}

/*	
	function IsValidate(venc_framerate, venc_gop, venc_bitrate, http_ts_uri,
			http_flv_uri, rtsp_uri, rtmp_publish_uri, http_hls_uri, multicast_ip,
			multicast_port) {
		validated = true;
		var IntVenc_framerate = IsInteger(venc_framerate);
		var IntVenc_gop = IsInteger(venc_gop);
		var IntVenc_bitrate = IsInteger(venc_bitrate);
		var IntMulticast_port = IsInteger(multicast_port);
		if (venc_framerate == "" || venc_gop == "" || venc_bitrate == ""
				|| http_ts_uri == "" || http_flv_uri == "" || rtsp_uri == ""
				|| rtmp_publish_uri == "" || http_hls_uri == ""
				|| multicast_ip == "" || multicast_port == "") {
			alert('Content cannot be empty!');
			validated = false;
		} else if (venc_framerate < 5 || venc_framerate > 60
				|| IntVenc_framerate == false) {
			alert('Frames Per Second should be between 5-60!');
			validated = false;
		} else if (venc_gop < 5 || venc_gop > 300 || IntVenc_gop == false) {
			alert('Key Interval (frames) value should be between 5-300!');
			validated = false;
		} else if (venc_bitrate < 32 || venc_bitrate > 32000
				|| IntVenc_bitrate == false) {
			alert('Bitrate(kbit) value should be between 32-32000!');
			validated = false;
		} else if (!checkIp(multicast_ip)) {
			alert('Multicast IP format is incorrect!');
			validated = false;
		} else if (multicast_port < 1 || multicast_port > 65535
				|| IntMulticast_port == false) {
			alert('Multicast port should be between 1-65535!');
			return false;
		}
		return validated;
	}
*/
	function IsValidated() {
		validated = true;

		var VidFpsO_1 	= $("#VidFpsO_1").val();
		var GOP_1 		= $("#GOP_1").val();
		var VidBitR_1 	= $("#VidBitR_1").val();
		var TSurl_1 	= $("#TSurl_1").val();
		var isTS_1 		= $("#isTS_1").val();		
		var HLSurl_1 	= $("#HLSurl_1").val();
		var isHLS_1 	= $("#isHLS_1").val();
		var FLVurl_1 	= $("#FLVurl_1").val();
		var isFLV_1 	= $("#isFLV_1").val();
		var HTTPport_1	= $("#HTTPport_1").val();		
		var RTSPurl_1 	= $("#RTSPurl_1").val();
		var RTSPport_1	= $("#RTSPport_1").val();
		var isRTSP_1 	= $("#isRTSP_1").val();
		var RTMPurl_1 	= $("#RTMPurl_1").val();
		var RTMPport_1 	= $("#RTMPport_1").val();
		var isRTMP_1 	= $("#isRTMP_1").val();
		var MCASTip_1 	= $("#MCASTip_1").val();
		var MCASTport_1 = $("#MCASTport_1").val();
		var isMCAST_1 	= $("#isMCAST_1").val();
		var AudBitR_1 	= $("#AudBitR_1").val();	
		
		var IntVidFpsO_1 = IsInteger(VidFpsO_1);
		var IntGOP_1 = IsInteger(GOP_1);
		var IntVidBitR_1 = IsInteger(VidBitR_1);
		var IntHTTPport_1 = IsInteger(HTTPport_1);		
		var IntRTSPport_1 = IsInteger(RTSPport_1);
		var IntRTMPport_1 = IsInteger(RTMPport_1);
		var IntMCASTport_1 = IsInteger(MCASTport_1);
		var IntAudBitR_1 = IsInteger(AudBitR_1);

		if (	VidFpsO_1 == "" || VidBitR_1 == "" || TSurl_1 == "" ||
				HLSurl_1 == "" || FLVurl_1 == "" || RTSPurl_1 == "" ||
				RTSPport_1 == "" || RTMPurl_1 == "" || RTMPport_1 == "" ||
				MCASTip_1 == "" || MCASTport_1 == "" || HTTPport_1 == "" ||
				GOP_1 == "" || AudBitR_1 == "")
		{
			alert('Content cannot be empty!');
			validated = false;			
		}
		else if (VidFpsO_1 < 5 || VidFpsO_1 > 60 || IntVidFpsO_1 == false)
		{
			alert('Frames Per Second should be between 5-60!');
			validated = false;
		}
		else if (GOP_1 < 5 || GOP_1 > 300 || IntVidFpsO_1 == false)
		{
			alert('GOP should be between 5-300!');
			validated = false;
		}
		else if (VidBitR_1 < 32 || VidBitR_1 > 32000 || IntVidBitR_1 == false) 
		{
			alert('Bitrate(kbit) value should be between 32-32000!');
			validated = false;
		} 
		else if (!checkIp(MCASTip_1)) 
		{
			alert('Multicast IP format is incorrect!');
			validated = false;
		}
		else if (HTTPport_1 < 1 || HTTPport_1 > 65535 || IntHTTPport_1 == false) 
		{
			alert('HTTP port should be between 1-65535!');
			validated = false;
		}		
		else if (RTSPport_1 < 1 || RTSPport_1 > 65535 || IntRTSPport_1 == false) 
		{
			alert('RTSP port should be between 1-65535!');
			validated = false;
		}
		else if (RTMPport_1 < 1 || RTMPport_1 > 65535 || IntRTMPport_1 == false) 
		{
			alert('RTMP port should be between 1-65535!');
			validated = false;
		}		
		else if (MCASTport_1 < 1 || MCASTport_1 > 65535 || IntMCASTport_1 == false) 
		{
			alert('Multicast port should be between 1-65535!');
			validated = false;
		}
		else if (AudBitR_1 < 48000 || AudBitR_1 > 320000 || IntAudBitR_1 == false)
		{
			alert('Audio Bitrate should be between 48000-320000!');
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
