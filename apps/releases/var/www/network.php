<?PHP
    include_once './include/Authentication.php';

	if (isset($_POST['submit']))
	{
		$changesMade = false;

		$login = new UserLoginFile();		
		if ( $login->get('dns1') <> $_POST['dns1'] OR $login->get('dns2') <> $_POST['dns2']) 
		{
			$login->set_DNS($_POST['dns1'],$_POST['dns2']);
            $login->fileWrite();
			$changesMade = true;
		}

		$arrNewValues = $currentDevice->save_changes($_POST);

		if ($arrNewValues)
		{
			$changesMade = true;

		    $currentMessage = new MUXProtocol((string)$login->json_username, (string)$login->json_password);
			$test = $currentMessage->cmd_set_param($arrNewValues);
			
			//check if good

			$arrParameters = $currentMessage->hex_command_to_text($test[0]);				

			if ($arrParameters['login_ack'] == 'OK')
			{

			    $currentMessage = new MUXProtocol((string)$login->json_username, (string)$login->json_password);
				
				header("Location: ./index.php?link=network&message=".urlencode("Settings saved. Rebooting..."));
				die();
			}
			else
			{				
				header("Location: ./index.php?link=network&message=".urlencode("Error. (". $arrParameters['login_ack'].")"));
				die();				
			}

		}
		
		if ($changesMade)
		{
			//echo "<SCRIPT type='text/javascript'>submit_main('network.php','No changes made');</script>";
			//header("Location: ./index.php?link=network&message=".urlencode("Settings saved."));
			header("Location: ./index.php?link=info&message=".urlencode("Settings saved. Rebooting...")."&reboot=1");
			die();
		}		
		else
		{
			//echo "<SCRIPT type='text/javascript'>submit_main('network.php','No changes made');</script>";
			header("Location: ./index.php?link=network&message=".urlencode("No changes made"));
			die();
		}
	}
?>

<DIV id="forms">
	<FORM method="post" id="formNetwork" action="network.php">
		<DIV class="form">
			<DIV class="title">
				<H2>Network</H2>
			</DIV>
			<DIV class="fields">
				<DIV class="field">
					<DIV class="label">
						<LABEL for="isDhcp">DHCP:</LABEL>
					</DIV>
					<DIV class="select">
						<SELECT id="isDhcp" name="isDhcp" onchange="dhcpEnable()">
							<OPTION <?PHP if ($currentDevice->get('isDhcp') == '1') echo "selected";?> value="1">Enabled</OPTION>
							<OPTION <?PHP if ($currentDevice->get('isDhcp') == '0') echo "selected";?> value="0">Disabled</OPTION>
						</SELECT>
					</DIV>
				</DIV>
				<DIV class="field">
					<DIV class="label">
						<LABEL for="ip">IP Address:</LABEL>
					</DIV>
					<DIV class="input">
						<INPUT id="ip" name="ip" maxlength="18" type="text" autocomplete=off value="<?php echo $currentDevice->get('ip'); ?>" />
					</DIV>
				</DIV>

				<DIV class="field">
					<DIV class="label">
						<LABEL for="mask">Netmask:</LABEL>
					</DIV>
					<DIV class="input">
						<INPUT id="mask" name="mask" maxlength="18" type="text" autocomplete=off value="<?php echo $currentDevice->get('mask'); ?>"/>
					</DIV>
				</DIV>

				<DIV class="field">
					<DIV class="label">
						<LABEL for="gateway">Gateway:</LABEL>
					</DIV>
					<DIV class="input">
						<INPUT id="gateway" name="gateway" maxlength="18" type="text" autocomplete=off value="<?php echo $currentDevice->get('gateway'); ?>"/>
					</DIV>
				</DIV>

				<DIV class="field" <?php if (GlobalFunctions::isDeviceTX()) : ?> style="visibility:hidden;display:none;"<?php endif;?>>
					<DIV class="label">
						<LABEL for="dns1">DNS 1:</LABEL>
					</DIV>
					<DIV class="input">
						<INPUT id="dns1" name="dns1" maxlength="20" type="text" autocomplete=off value="<?php echo $login->get('dns1'); ?>"/>
					</DIV>
				</DIV>

				<DIV class="field" <?php if (GlobalFunctions::isDeviceTX()) : ?> style="visibility:hidden;display:none;"<?php endif;?>>
					<DIV class="label">
						<LABEL for="dns2">DNS 2:</LABEL>
					</DIV>
					<DIV class="input">
						<INPUT id="dns2" name="dns2" maxlength="20" type="text" autocomplete=off value="<?php echo $login->get('dns2'); ?>"/>
					</DIV>
				</DIV>

				<DIV class="field">
					<DIV class="label">
						<LABEL for="mac">MAC Address:</LABEL>
					</DIV>
					<DIV class="input">
						<INPUT id="mac" name="mac" maxlength="20" type="text" autocomplete=off value="<?php echo $currentDevice->get('mac'); ?>" disabled />
					</DIV>
				</DIV>
			</DIV>

			<DIV class="fields">
				<DIV class="field">
					<DIV class="buttons">
						<INPUT name="submit" type="submit" class="btnSubmit" value="Apply" />
						<INPUT name="ResetButton" type="reset" class="btnReset" value="Reset" id="ResetButton" />						
					</DIV>
				</DIV>
			</DIV>
		</DIV>
	</FORM>
</DIV>

<script type="text/javascript">

	dhcpEnable();

	$('#formNetwork').submit(function() {
		return IsValidated(); // return false to cancel form action
	});

    document.getElementById('ResetButton').onclick= function() {
		$("#formNetwork")[0].reset();
		dhcpEnable();
 	};

	function IsValidated() {

		var validated = true;

		var isDhcp = $("#isDhcp").val();
		var ip = $("#ip").val();
		var mask = $("#mask").val();
		var gateway = $("#gateway").val();
		var dns1 = $("#dns1").val();
		var dns2 = $("#dns2").val();

		if (isDhcp == "" || ip == "" || mask == "" || gateway == "" || dns1 == "" || dns2 == "")
		{
			alert('Content cannot be empty!');
			validated = false;
		} 
		else if (!checkIp(ip)) 
		{
			alert('IP format is not correct!');
			validated = false;
		} 
		else if (!checkNetmask(mask)) 
		{
			alert('Netmask format is not correct!');
			validated = false;
		} 
		else if (!checkIp(gateway)) 
		{
			alert('Gateway format is not correct!');
			validated = false;
		}
		else if (!checkIp(dns1)) 
		{
			alert('DNS IP format is not correct!');
			validated = false;
		} 
		else if (!checkIp(dns2)) 
		{
			alert('DNS IP format is not correct!');
			validated = false;
		} 
		return validated;
	}

	//var hostip = window.location.host;
	
	function dhcpEnable() 
	{
		var dhcp = $("#isDhcp").val();
		if (dhcp == 0) {
			$("#ip").removeAttr("disabled");
			$("#mask").removeAttr("disabled");
			$("#gateway").removeAttr("disabled");
		} else {
			$("#ip").attr("disabled", "disabled");
			$("#mask").attr("disabled", "disabled");
			$("#gateway").attr("disabled", "disabled");
		}
	}

	function IsValidate() {

		var validated = true;
		var dhcp_enable = $("#dhcp_enable").val();
		var ip0 = $("#ipTxt").val();
		var netmask0 = $("#netmaskTxt").val();
		var gateway0 = $("#gatewayTxt").val();
		var dns0 = $("#dns0Txt").val();
		var mac0 = $("#macTxt").val();
		var dns1 = $("#dns1Txt").val();
		var http_port = $("#http_portTxt").val();
		var Inthttp_port = IsInteger(http_port);
		var rtsp_port = $("#rtsp_portTxt").val();
		var Intrtsp_port = IsInteger(rtsp_port);

		if (ip0 == "" || netmask0 == "" || gateway0 == "" || dns0 == ""
				|| mac0 == "" || dns1 == "" || http_port == ""
				|| rtsp_port == "") {
			alert('Content cannot be empty!');
			validated = false;
		} else if (!checkIp(ip0)) {
			alert('IP format is not correct!');
			validated = false;
		} else if (!checkNetmask(netmask0)) {
			alert('Netmask format is not correct!');
			validated = false;
		} else if (!checkIp(gateway0)) {
			alert('Gateway format is not correct!');
			validated = false;
		} else if (!checkIp(dns0)) {
			alert('Preferred DNS format is not correct!');
			validated = false;
		} else if (!checkIp(dns1)) {
			alert('Reserve DNS format is not correct!');
			validated = false;
		} else if (!CheckMac(mac0)) {
			alert('MAC format is not correct!');
			validated = false;
		} else if (http_port<1 || http_port>65500 || Inthttp_port == false) {
			alert('HTTP Port value should be an integer in 1-65500!');
			validated = false;
		} else if (rtsp_port<1 || rtsp_port>65500 || Intrtsp_port == false) {
			alert('RTSP Port value should be an integer in 1-65500!');
			validated = false;
		}

		return validated;
	}

	function checkIp(str) {
		var ss = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
		return ss.test(str);
	}

	function CheckMac(str) {
		var reg_name = /^[a-fA-F\d]{2}:[a-fA-F\d]{2}:[a-fA-F\d]{2}:[a-fA-F\d]{2}:[a-fA-F\d]{2}:[a-fA-F\d]{2}$/;
		return reg_name.test(str);
	}

	function checkNetmask(str) {
		var ss = /^(254|252|248|240|224|192|128|0)\.0\.0\.0|255\.(254|252|248|240|224|192|128|0)\.0\.0|255\.255\.(254|252|248|240|224|192|128|0)\.0|255\.255\.255\.(254|252|248|240|224|192|128|0)$/;
		return ss.test(str);
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
</SCRIPT>