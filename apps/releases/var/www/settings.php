<?PHP
    include_once './include/Authentication.php';

	if (isset($_POST['submit']))
	{
		//var_dump($_POST);

		switch($_POST['submit'])
		{
			case 'Apply':
				if (empty($_POST['oldPassword']) or empty($_POST['newPassword1']) or empty($_POST['newPassword2']))
				{
					header("Location: ./index.php?link=settings&message=".urlencode("Content blank"));
					die();		
				}
				elseif ($_POST['newPassword1'] <> $_POST['newPassword2'])
				{
					header("Location: ./index.php?link=settings&message=".urlencode("Passwords don't match"));
					die();							
				}
				else
				{
					if (!$login->userChangePassword($_POST['oldPassword'],$_POST['newPassword1']))
					{
						header("Location: ./index.php?link=settings&message=".urlencode("Incorrect password"));
						die();	
					}
					else
					{
						header("Location: ./index.php?link=logout&message=".urlencode("Password changed"));
						die();	
					}
				}
				break;

			case 'Upload':
				
				$target_file = FIRMWARE_FILE;

				// Time this script could last
				$timeEstimated = 1*5*60; // allocate 5 min per device
				set_time_limit($timeEstimated); 	

				$fwFileReceived = file_get_contents($_FILES["fileToUpload"]["tmp_name"]);

				// Validate the firmware to be uploaded
				//$fwFileData = base64_decode($fwFileReceived);
				$fwFileData = $fwFileReceived;
								
				$fwHeader = substr($fwFileData, 0, 12);
				$fwData = substr($fwFileData, 12);
				
				$version_received = substr($fwHeader,0,1) ."." .substr($fwHeader,1,1) ."." .substr($fwHeader,2,1);
				$productModel_Received = substr($fwHeader,3,9);

				if ($version_received == null)
				{
					header("Location: ./index.php?link=settings&message=".urlencode("Invalid Firmware!"));
					die();	
				}
				else if ($productModel_Received != DEVICE_NAME)
				{
					header("Location: ./index.php?link=settings&message=".urlencode("Firmware is not compatible!"));
					die();	
				}
				else
				{
					// Save the firmware to a file
					$fp = fopen($target_file, 'w');
					fwrite($fp,$fwData);
					fclose($fp);
					//sleep(3); //sleep for 3 seconds

					if (GlobalFunctions::isDeviceRX()) 	header("Location: ./index.php?link=info&message=".urlencode("Uploaded. Rebooting...")."&reboot=0");
					else 								header("Location: ./index.php?link=info&message=".urlencode("Uploaded. Rebooting...")."&reboot=0");
					die();	
				}				
				break;

			case 'Reset':
			    $currentMessage = new MUXProtocol((string)$login->json_username, (string)$login->json_password);
				$test = $currentMessage->cmd_reset();
				//$test = $currentMessage->cmd_reboot();
				header("Location: ./index.php?link=info&message=".urlencode("Reseting and Rebooting...")."&reboot=1");				
				die();
				break;

			case 'Reboot':
			    //$currentMessage = new MUXProtocol((string)$login->json_username, (string)$login->json_password);
				//$test = $currentMessage->cmd_reboot();
				header("Location: ./index.php?link=info&message=".urlencode("Rebooting...")."&reboot=1");
				die();				
				break;

			default:
				break;
		}

		//header("Location: ./index.php?link=encoder&message=".urlencode("Changes made."));
		//die();
	}

?>

	<style>
		.grid {}

		.one-one {
			display: inline-block;
			float: left;
			width: 500px;
			height: 200px;
		}

		.one-two {
			display: inline-block;
			float: left;
			width: 500px;
			height: 200px;
		}

		.two-one {
			display: inline-block;
			float: left;
			margin-top: 10px;
			width: 500px;
		}

		.two-two {
			display: inline-block;
			float: left;
			margin-top: 10px;
			width: 500px;
		}
	</style>

	<div class="grid">
		<div class="one-one">
			<DIV id="forms">
				<FORM method="post" id="formPassword" action="settings.php">
					<DIV class="form">
						<DIV class="title">
							<H2>Change Password</H2>
						</DIV>
						<DIV class="fields">

							<DIV class="field">
								<DIV class="label">
									<LABEL for="textbox">Old password:</LABEL>
								</DIV>
								<DIV class="input">
									<input name="oldPassword" type="password" value="" maxlength="32" id="oldPassword">
								</DIV>
							</DIV>
							<DIV class="field">
								<DIV class="label">
									<LABEL for="textbox">New password:</LABEL>
								</DIV>
								<DIV class="input">
									<input name="newPassword1" type="password" value="" maxlength="32" id="newPassword1">
								</DIV>
							</DIV>
							<DIV class="field">
								<DIV class="label">
									<LABEL for="textbox">Confirm new password:</LABEL>
								</DIV>
								<DIV class="input">
									<input name="newPassword2" type="password" value="" maxlength="32" id="newPassword2">
								</DIV>
							</DIV>
							<DIV class="buttons">
								<INPUT name="submit" type="submit" class="btnSubmit" value="Apply" />
							</DIV>
						</DIV>
					</DIV>
				</FORM>
			</DiV>
		</div>

		<div class="one-two">
			<DIV id="forms">
				<FORM method="post" id="formFirmware" action="settings.php" enctype="multipart/form-data">
					<DIV class="title">
						<H2>Upgrade Firmware</H2>
					</DIV>
					<DIV class="fields">
						<DIV class="field">
							<LABEL for="fileToUpload">Select File to Upload:</LABEL>
							<INPUT type="file" name="fileToUpload" id="fileToUpload">
						</DIV>
						<DIV class="buttons">
							<!--<input type="button" value="Reboot" class="btnSubmit" onClick="" id="subBut3" />-->
							<INPUT name="submit" type="submit" class="btnSubmit" value="Upload" />
						</DIV>
					</DIV>
				</FORM>
			</DIV>
		</div>

		<div class="two-one">
			<DIV id="forms">
				<FORM method="post" id="formReset" action="settings.php">
					<DIV class="form">
						<DIV class="title">
							<H2>Reset</H2>
						</DIV>
						<DIV class="fields">
							<DIV class="field">
								<DIV style="margin-left: 5px;">Restore the device to factory settings.</DIV>
							</DIV>
							<DIV class="buttons">
								<!-- <input type="button" value="Reset" class="btnSubmit" onClick="" id="subBut3" /> -->
								<INPUT name="submit" type="submit" class="btnSubmit" value="Reset" />
							</DIV>
						</DIV>
					</DIV>
				</FORM>
			</DIV>
		</div>

		<div class="two-two">
			<DIV id="forms">
				<FORM method="post" id="formReboot" action="settings.php">
					<DIV class="title">
						<H2>Reboot</H2>
					</DIV>
					<DIV class="fields">
						<DIV class="field">
							<DIV style="margin-left: 5px;">Please wait one minute after rebooting.</DIV>
						</DIV>
						<DIV class="buttons">
							<!--<input type="button" value="Reboot" class="btnSubmit" onClick="" id="subBut3" />-->
							<INPUT name="submit" type="submit" class="btnSubmit" value="Reboot" />
						</DIV>
					</DIV>
				</FORM>
			</DIV>
		</div>
	</div>

	<script type="text/javascript">
		$('#formPassword').submit(function () {
			// DO STUFF...
			return changepw(); // return false to cancel form action
		});

		$('#formReset').submit(function () {
			// DO STUFF...
			if (confirm("Are you sure you want to reset?")) {
				//alert('Reset! Please reboot your device.');
				return true; // return false to cancel form action
			} else {
				return false;
			}
		});

		$('#formReboot').submit(function () {
			// DO STUFF...
			if (confirm("Are you sure you want to reboot?")) {
				//alert('Rebooting! Please wait one minute before refreshing.');
				return true; // return false to cancel form action
			} else {
				return false;
			}
		});
	</script>


	<SCRIPT type="text/javascript">
		var hostip = window.location.host;

		function changepw() {
			var newPassword1 = $("#newPassword1").val();
			var newPassword2 = $("#newPassword2").val();
			var old_password = $("#oldPassword").val();
			if (newPassword1 == "" || newPassword2 == "" || old_password == "") {
				alert("Password cannot be empty!");
				return false;
			} else {
				if (newPassword1 == newPassword2) {
					return true;
				} else {
					alert("Passwords do not match!");
					return false;
				}
			}
		}
	</SCRIPT>


