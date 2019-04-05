<?PHP
    require_once './include/Logger.inc';
    require_once('./include/GlobalConfiguration.inc');

    Logger::getInstance()->lwrite('BACKGROUND: Started');

    if (isset($_POST['value']))
    {
        $value = ($_POST['value'] == 'true')? '1' : '0';
    }
    else
    {
        print 'FALSE';
        die();		        
    }

    if (IS_TEST_SERVER)
    {
        Logger::getInstance()->lwrite('BACKGROUND: '.print_r('VALUE = '.(string)$value,true));
        if ($value == '1') PRINT 'TRUE';
        else PRINT 'FALSE';
        die();		 
    }
    else
    {
        require_once(MUXPROTOCOL_FILE);

        $currentMessage = new MUXProtocol((string)$login->json_username, (string)$login->json_password);
        $result = $currentMessage->cmd_link_led($value);

        //check if good
        $arrParameters = $currentMessage->hex_command_to_text($result[0]);				

        if ($arrParameters['login_ack'] == 'OK')
        {
            $version_check =  (int)explode(".", HARDWARE_VERSION)[0]; 

            if ( (DEVICE_NAME == "RX-500768") && ($version_check >= 2) ) //RX Phase 2 
            {
                //Logger::getInstance()->lwrite('BACKGROUND: '.'RX VALUE = '.print_r($arrParameters['data'][0]['blink_led'],true));
                if ($arrParameters['data'][0]['blink_led'] == "1") print 'TRUE';
                else print 'FALSE';                
            }
            else
            {
                //Logger::getInstance()->lwrite('BACKGROUND: '.'VALUE = '.print_r($arrParameters['data'][0]['value'],true));
                if ($arrParameters['data'][0]['value'] == '1') print 'TRUE';
                else print 'FALSE';
            }
            die();
        }
        else
        {
            print 'FALSE';
            die();				
        }

    }

?>