<?php 
	ini_set('display_errors',1);
	ini_set('display_statup_errors',1);
	error_reporting(E_ALL);

	require_once('./include/GlobalConfiguration.inc');
	require_once(MUXPROTOCOL_FILE);
	require_once('./include/UserLoginFile.inc');

	$login = new UserLoginFile();

	if ($login->isAuthenticated())	
	{
		if (IS_TEST_SERVER)
		{
			if (GlobalFunctions::isDeviceTX()) $jsonParameters = file_get_contents('./xml/TX_json.txt');
	    	if (GlobalFunctions::isDeviceRX()) $jsonParameters = file_get_contents('./xml/RX_json.txt');
	            
	        $arrParameters= json_decode($jsonParameters, true);
			$currentDevice = new MuxDevice($arrParameters['data'][0]);    

			echo "<SCRIPT type='text/javascript'>$('#blink').css('visibility', 'visible').css( 'display','block');</script>";						
		}
		else
		{
			Logger::getInstance()->lwrite('AUTH: Username: '. $login->json_username . ' Password: ' . $login->json_password );									

			$currentMessage = new MUXProtocol((string)$login->json_username, (string)$login->json_password);
			
			$parameters = $currentMessage->cmd_get_param();	
						
	        //check if good
	        if ($parameters)
			{
				$arrParameters = $currentMessage->hex_command_to_text($parameters[0]);

				if ($arrParameters['login_ack'] == 'OK')
				{
					$currentDevice = new MuxDevice($arrParameters['data'][0]);								
					//echo "<PRE>".print_r($arrParameters['data'][0])."</PRE>"; // DEBUG

					$version_check =  (int)explode(".", HARDWARE_VERSION)[0]; 
										
					// Phase specific changes
					if ( (DEVICE_NAME == "RX-500768") && ($version_check >= 2) ) //RX Phase 2 
					{
						$parameters = $currentMessage->cmd_get_param_player();
						$player_parameters = $currentMessage->hex_command_to_text($parameters[0]);
			
						//print_r($player_parameters['data'][0]);
			
						$currentDevice->fill_attributes($player_parameters['data'][0]);
					}

					//show blink button
					echo "<SCRIPT type='text/javascript'>$('#blink').css('visibility', 'visible').css( 'display','block');</script>";

				}
				else
				{
					echo "<SCRIPT type='text/javascript'>submit_main('userlogin.php','Unable to communicate correctly');</script>";
    				die();				
				}
			}
			else
			{    				
				echo "<SCRIPT type='text/javascript'>submit_main('userlogin.php','Unable to communicate');</script>";
    			die();
			}
		}
	}
	else
	{
		echo "<SCRIPT type='text/javascript'>submit_main('userlogin.php','Please login');</script>";
		die();			
	}


?>