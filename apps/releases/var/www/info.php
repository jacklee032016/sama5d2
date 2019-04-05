<?PHP
    include_once './include/Authentication.php';
	//echo '<pre>'; print_r($arrParameters); echo '</pre>';

	if (GlobalFunctions::isDeviceTX())
	{
		if ($currentDevice->Memory_Usage <> "")
		{
			list($Memory_Free, $Memory_Total) = explode("/",$currentDevice->Memory_Usage);
			$Memory_Used = $Memory_Total - $Memory_Free;

			$Memory_Free = round($Memory_Free / 1024, 2);
			$Memory_Used = round($Memory_Used / 1024, 2);
			$Memory_Total = round($Memory_Total / 1024, 2);
		}
		else
		{
			$Memory_Free = 0;
			$Memory_Used = 0;
			$Memory_Total = 0;
		}
	}


	if (GlobalFunctions::isDeviceRX())
	{
		if (file_exists(NETWORK_MONITOR_FILE))
        {
			if (!$file = fopen(NETWORK_MONITOR_FILE, "r"))
			{
				Logger::getInstance()->lwrite('NETWORK MONITOR: File cannot be opened/created');				
			}
			else
			{
				$netmon_text = fread($file, filesize(NETWORK_MONITOR_FILE));
	
				$netmon_rows = preg_split( "/(\\n|\\e|\[2K|\[)/", $netmon_text );
				
				foreach($netmon_rows as $netmon_row)
				{
					$netmon_row = trim($netmon_row);
					if ($netmon_row != "")
					{
						if ( strpos($netmon_row,"packets sampled in") ) $netmon_packets = $netmon_row;
						if ( substr($netmon_row, 0, 2) == "rx" ) $netmon_rx = trim(explode("rx ",$netmon_row)[1]);
						if ( substr($netmon_row, 0, 2) == "tx" ) $netmon_tx = trim(explode("tx ",$netmon_row)[1]);
					}
				}				
			}		
			fclose($file);
        }
	}


	if (IS_SECURITY_CHECK)
	{
		$result = $currentMessage->cmd_security_get_status();
		$arrSecurityCheck = $currentMessage->hex_command_to_text($result[0]);
		$Security_Check = ($arrSecurityCheck["login_ack"] == 'OK') ? "PASS" : "NO PASS";
	}
	else
	{
		$Security_Check = "NO";
	}

	

//	echo '<pre>'; print_r($arrSecurityCheck); echo '</pre>';
?>

<?php if (GlobalFunctions::isDeviceTX()) : ?>

<DIV class="title">
	<H2>Input</H2>
</DIV>

<DIV class="fields-info">
	<DIV class="field">
		<LABEL for="CPU_Usage">CPU:</LABEL>
		<DIV class="label" id="CPU_Usage"><?php echo $currentDevice->CPU_Usage." %"; ?></DIV>
	</DIV>

	<DIV class="field">
		<LABEL for="Memory_Used_Memory_Total">Memory Used/Total:</LABEL>
		<DIV class="label" id="Memory_Used_Memory_Total"><?php echo (string)$Memory_Used."M / ".(string)$Memory_Total."M" ?></DIV>
	</DIV>	

	<DIV class="field">
		<LABEL for="VidResI_1">Resolution:</LABEL>
		<DIV class="label" id="VidResI_1"><?php echo $currentDevice->VidResI_1; ?></DIV>
	</DIV>
	
	<DIV class="field">
		<LABEL for="VidFpsI_1">FPS:</LABEL>
		<DIV class="label" id="VidFpsI_1"><?php echo $currentDevice->VidFpsI_1; ?></DIV>
	</DIV>

	<DIV class="field">
		<LABEL for="SecurityCheck">Security Check:</LABEL>
		<DIV class="label" id="SecurityCheck"><?php echo $Security_Check; ?></DIV>
	</DIV>	

</DIV>

<DIV class="title">
	<H2>Encoder</H2>
</DIV>

<DIV class="fields-info">
	<DIV class="field">
		<LABEL for="GOP">GOP:</LABEL>
		<DIV class="label" id="GOP"><?php echo $currentDevice->GOP_1; ?></DIV>
	</DIV>	

	<DIV class="field">
		<LABEL for="codec0_0">Encoding Type:</LABEL>
		<DIV class="label" id="codec0_0"><?php echo $currentDevice->VidCodec_1; ?></DIV>
	</DIV>

	<DIV class="field">
		<LABEL for="ts0_url0_0">Resolution:</LABEL>
		<DIV class="label" id="ts0_url0_0"><?php echo $currentDevice->VidResO_1; ?></DIV>
	</DIV>
	
	<DIV class="field">
		<LABEL for="whframerate0_0">FPS:</LABEL>
		<DIV class="label" id="whframerate0_0"><?php echo $currentDevice->VidFpsO_1; ?></DIV>
	</DIV>


	<DIV class="field">
		<LABEL for="bitrate0_0">Bitrate(kbit):</LABEL>
		<DIV class="label" id="bitrate0_0"><?php echo $currentDevice->VidBitR_1; ?></DIV>
	</DIV>

	<DIV class="field<?php if ($currentDevice->isTS_1) echo ' highlight';?>">
		<LABEL for="ts0_url0_0">TS URL:</LABEL>
		<!-- <DIV class="label" id="ts0_url0_0"><?PHP echo $currentDevice->TSurl_1; ?></DIV> -->
		<DIV class="label" id="ts0_url0_0"><?PHP echo $currentDevice->TSurl_1; ?></DIV>
	</DIV>

	<DIV class="field<?php if ($currentDevice->isHLS_1) echo ' highlight';?>">
		<LABEL for="hls0_url0_0">HLS URL:</LABEL>
		<!-- <DIV class="label" id="hls0_url0_0"><?PHP echo $currentDevice->HLSurl_1; ?></DIV> -->
		<DIV class="label" id="hls0_url0_0"><?PHP echo $currentDevice->HLSurl_1; ?></DIV>
	</DIV>

	<DIV  id="flv_0" class="field<?php if ($currentDevice->isFLV_1) echo ' highlight';?>">
		<LABEL for="flv0_url0_0">FLV URL:</LABEL>
		<!-- <DIV class="label" id="flv0_url0_0"><?PHP echo $currentDevice->FLVurl_1; ?></DIV> -->
		<DIV class="label" id="flv0_url0_0"><?PHP echo $currentDevice->FLVurl_1; ?></DIV>
	</DIV>

	<DIV  id="rtsp_0" class="field<?php if ($currentDevice->isRTSP_1=='1') echo ' highlight';?>">
		<LABEL for="rtsp0_url0_0">RTSP URL:</LABEL>
		<!-- <DIV class="label" id="rtsp0_url0_0"><?PHP echo $currentDevice->RTSPurl_1; ?></DIV> -->
		<DIV class="label" id="rtsp0_url0_0"><?PHP echo $currentDevice->RTSPurl_1; ?></DIV>
	</DIV>

	<DIV  id="rtmp_0" class="field<?php if ($currentDevice->isRTMP_1) echo ' highlight';?>">
		<LABEL for="rtmp_url0_0">RTMP URL:</LABEL>
		<DIV class="label" id="rtmp_url0_0"><?php echo $currentDevice->RTMPurl_1; ?></DIV>
	</DIV>

	<DIV  class="field<?php if ($currentDevice->isMCAST_1) echo ' highlight';?>">
		<LABEL for="multicast_url0_0">Multicast URL:</LABEL>
		<DIV class="label" id="multicast_url0_0"><?php echo $currentDevice->MCASTurl_1; ?></DIV>
	</DIV>

</DIV>

<SCRIPT type="text/javascript">
	var cpuusage = <?php echo $currentDevice->CPU_Usage; ?>;
	var memoryfree = <?php echo $Memory_Free; ?>;
	var memorytotal = <?php echo $Memory_Total; ?>;

	var memorypercentfree = (memoryfree / memorytotal) * 100;

	if (memorypercentfree < 5) {
		$("#Memory_Used_Memory_Total").css("color", "red");
		$("#Memory_Used_Memory_Total").css("font-weight", "bold");
	}
	if (cpuusage >= 95) {
		$("#CPU_Usage").css("color", "red");
		$("#CPU_Usage").css("font-weight", "bold");
		$("#CPU_Usage").text(cpuusage + "%" + " (Please disable some streams)");
	}
</SCRIPT>

<?php elseif (GlobalFunctions::isDeviceRX()) : ?>

<DIV class="title">
	<H2>Input</H2>
</DIV>

<DIV class="fields-info">

	<DIV class="field">
		<LABEL for="ts0_url0_0">Output Resolution:</LABEL>
		<DIV class="label" id="ts0_url0_0"><?php echo $currentDevice->return_resolution($currentDevice->display_res); ?></DIV>
	</DIV>
	
	<DIV class="field">
		<LABEL for="ts0_url0_0">Input Resolution:</LABEL>
		<DIV class="label" id="ts0_url0_0"><?php echo ($currentDevice->get('vid_width_I')) ? $currentDevice->get('vid_width_I')."x".$currentDevice->get('vid_Height_I')." @ ".$currentDevice->get('vid_fps_I') : "Off"; ?></DIV>
	</DIV>	

	<DIV class="field">
		<LABEL for="SecurityCheck">Security Check:</LABEL>
		<DIV class="label" id="SecurityCheck"><?php echo $Security_Check; ?></DIV>
	</DIV>		
		
</DIV>

<DIV class="title">
	<H2>Decoder</H2>
</DIV>

<DIV class="fields-info">

	<DIV class="field<?php if ($currentDevice->src_onplaying) echo ' highlight';?>">
		<LABEL for="src_onplaying">URL:</LABEL>
		<DIV class="label" id="src_onplaying"><?php echo $currentDevice->src_onplaying; ?></DIV>
	</DIV>
	
</DIV>


<?php if (isset($netmon_packets) || isset($netmon_rx) || isset($netmon_tx)) : ?>

<DIV class="title">
	<H2>Network Monitor</H2>
</DIV>

<DIV class="fields-info">

<?php if (isset($netmon_packets)) : ?>
	<DIV class="field">
		<LABEL for="netmon_packets">&nbsp;</LABEL>
		<DIV class="label" id="netmon_packets"><?php echo $netmon_packets; ?></DIV>
	</DIV>
<?php endif; ?>

<?php if (isset($netmon_rx)) : ?>
	<DIV class="field">
		<LABEL for="netmon_rx">Receiving:</LABEL>
		<DIV class="label" id="netmon_rx"><?php echo $netmon_rx; ?></DIV>
	</DIV>
<?php endif; ?>

<?php if (isset($netmon_tx)) : ?>
	<DIV class="field">
		<LABEL for="netmon_tx">Sending:</LABEL>
		<DIV class="label" id="netmon_tx"><?php echo $netmon_tx; ?></DIV>
	</DIV>
<?php endif; ?>

</DIV>

<?php endif; ?>

<?php else : ?>

<?php endif; ?>

<SCRIPT type="text/javascript">
	var securitycheck = '<?php echo $Security_Check; ?>';
	if (securitycheck != "PASS") {
		$("#SecurityCheck").css("color", "red");
		$("#SecurityCheck").css("font-weight", "bold");
	}
</SCRIPT>

