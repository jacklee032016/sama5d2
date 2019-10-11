<?PHP

namespace App\Base;

use App\Helpers\StringHelper;
use App\Params;

class MUXProtocol
{
	const TAG_FOR_CMD = "faa0";
	const TAG_FOR_RESP = "fba0";
	
	private $username = null;
	private $password = null;

	private $lastErrorMessage = '';	
 
	
	function __construct($username=null, $password=null)
	{
	    $this->username = $username;
	    $this->password = $password;
	}
	
	function text_to_hex_command($textCommand)
	{
    	$jsonCmdMessage = $textCommand;
    	$jsonCmdMessageCRC = StringHelper::string_addPadding(sprintf("%x", crc32($jsonCmdMessage)), true, '0', 8); 
    	
    	$tag = self::TAG_FOR_CMD;
    	$length = StringHelper::string_dec2HexStr( strlen($jsonCmdMessage) + 4, 4);
    	$value =  StringHelper::string_str2Hex($jsonCmdMessage) .$jsonCmdMessageCRC;

    	$hexStringPacket = $tag .$length.$value;
		return($hexStringPacket);
	}

	function hex_command_to_text($hexCommand)
	{
		return($this->parseRspPacket($hexCommand));
    }

    public function parseRspPacket($currentMessage)
	{
	    //==============================================
		// STEP #1 : Extract and Check the response tag
		$cmdTag = substr($currentMessage, 0, 2*2 );

		// Skip if tag received is not the one expected
		if (strcasecmp($cmdTag, self::TAG_FOR_RESP) !== 0)//"fab0") !== 0)
			return;
			
		//==============================================
		// STEP #2 : Extract and Check the response length and value
		$cmdLength = hexdec(substr($currentMessage, 4, 2*2 ));
		$cmdValue = substr($currentMessage, 8);
		    
		$actualDataLength = strlen($cmdValue)/2;
		    
		if ($actualDataLength > $cmdLength)
		{
			$cmdValue = substr($currentMessage, 8, $cmdLength*2 );
			$actualDataLength = strlen($cmdValue)/2;
		}
		        
		// Skip if data length received is not the one expected
		if ($cmdLength != $actualDataLength)
			return;		        
 
		//==============================================
		// STEP #3 : Check CRC od the data	  
		$jsonCmdCrcInHex = substr($cmdValue, -8 ); // To get the last 4 bytes corresponding to the CRC
		
		$jsonData = StringHelper::string_hex2str(substr($cmdValue, 0, $actualDataLength*2 - 8));
		
		$actualJsonCmdCrcInHex = StringHelper::string_addPadding(sprintf("%x", crc32($jsonData)), true, '0', 8);
		
		if (strcasecmp($jsonCmdCrcInHex, $actualJsonCmdCrcInHex) !== 0)
		{
		    Logger::getInstance()->addError('packet CRC Error, can be a socket buffer size problem');
			return;
		}
		//==============================================
		// STEP #4 : The packet is Good, we can now process the command
		$cmdArray = json_decode($jsonData, true);
		return($cmdArray);
	}    

    public function cmd_link_led($value)
    {
		//{"targ":"86:01:FA:56:28:71","cmd":"link_led","login_ack":"","pwd_msg":"","data":[{"value":"1"}]}

		if ($value) $value = '1';
		else $value = '0';

		$arrNewValues = array();
		$arrNewValues['blink_led'] = (string)$value;
		$arrValues = array("targ"=>'',"cmd"=>"set_cmd","login_ack"=>$this->username,"pwd_msg"=>$this->password,"data"=>array($arrNewValues));

		$jsonNewValues = json_encode($arrValues);

        return($this->send_UDP_command($jsonNewValues));
    }

    public function cmd_reset()
    {
		//{"targ":"","cmd":"set_param","login_ack":"","pwd_msg":"","data":[{"isRTMP_1":"0","isRTMPph_1":"0","isDipOn":'1',"RTMPport_1":"36004","MCASTurl_1":"udp://@239.111.101.254:37110"}]}

		$arrNewValues = array();
		$arrNewValues['reset'] = '';        
		$arrValues = array("targ"=>'',"cmd"=>"set_cmd","login_ack"=>$this->username,"pwd_msg"=>$this->password,"data"=>array($arrNewValues));

		$jsonNewValues = json_encode($arrValues);

        return($this->send_UDP_command($jsonNewValues));
    }

    public function cmd_reboot()
    {
		//$arrValues = array("targ"=>'',"cmd"=>"set_param","login_ack"=>(string)$this->username,"pwd_msg"=>(string)$this->username,"data"=>array('isReboot'=>1));

		$arrNewValues = array();
		$arrNewValues['reboot'] = '';       
		$arrValues = array("targ"=>'',"cmd"=>"set_cmd","login_ack"=>$this->username,"pwd_msg"=>$this->password,"data"=>array($arrNewValues));

		$jsonNewValues = json_encode($arrValues);
		
        return($this->send_UDP_command($jsonNewValues));
    }
    
    public function cmd_check_usb()
    {
        $arrNewValues = array('check_usb' => "");
        $arrValues = array("targ"=>'',"cmd"=>"set_cmd","login_ack"=>$this->username,"pwd_msg"=>$this->password,"data"=>array($arrNewValues));
        
        $jsonNewValues = json_encode($arrValues);
        
        return($this->send_UDP_command($jsonNewValues));
    }

    public function cmd_check_sd()
    {
        $arrNewValues = array('check_sd' => "");
        $arrValues = array("targ"=>'',"cmd"=>"set_cmd","login_ack"=>$this->username,"pwd_msg"=>$this->password,"data"=>array($arrNewValues));
        
        $jsonNewValues = json_encode($arrValues);
        
        return($this->send_UDP_command($jsonNewValues));
    }
    
    public function cmd_ir($data)
    {
        //$arrValues = array("targ"=>'',"cmd"=>"set_param","login_ack"=>(string)$this->username,"pwd_msg"=>(string)$this->username,"data"=>array('isReboot'=>1));
        
        $arrNewValues = array();
        $arrNewValues['hexdata']    = $data['hexdata'];
        $arrNewValues['isFeedback'] = $data['isFeedback'];
        $arrNewValues['waitTime']   = $data['waitTime'];
        
        $arrValues = array("targ"=>'',"cmd"=>"send_data_ir","login_ack"=>$this->username,"pwd_msg"=>$this->password,"data"=>array($arrNewValues));
        
        $jsonNewValues = json_encode($arrValues);
        
        return($this->send_UDP_command($jsonNewValues));
    }

    public function cmd_rs232($data)
    {
        //$arrValues = array("targ"=>'',"cmd"=>"set_param","login_ack"=>(string)$this->username,"pwd_msg"=>(string)$this->username,"data"=>array('isReboot'=>1));
        $arrValues = array("targ"=>'',"cmd"=>"send_data_rs232","login-ack"=>$this->username,"pwd-msg"=>$this->password,"data"=>array($data));
        $jsonNewValues = json_encode($arrValues);
        
        return($this->send_UDP_command($jsonNewValues));
    }
    
    public function cmd_set_param($arrNewValues)
    {
		//{"targ":"","cmd":"set_param","login_ack":"","pwd_msg":"","data":[{"isRTMP_1":"0","isRTMPph_1":"0","isDipOn":'1',"RTMPport_1":"36004","MCASTurl_1":"udp://@239.111.101.254:37110"}]}

        $arrValues = array("targ"=>'',"cmd"=>"set_param","login-ack"=>$this->username,"pwd-msg"=>$this->password,"data"=>array($arrNewValues));
        $jsonNewValues = json_encode($arrValues);

        return($this->send_UDP_command($jsonNewValues));
    }

    public function cmd_security_status()
    {
        //{"targ":"","cmd":"security_check","login_ack":"Angelica","pwd_msg":"500762","data":[{"get_status":""}]}
        //$arrNewValues['get_status'] = "";
        $arrValues = array("targ"=>'',"cmd"=>"security_check","login-ack"=>$this->username,"pwd-msg"=>$this->password,"data"=>array(array("get_status" => 0)));
        $jsonNewValues = json_encode($arrValues);
        
        return($this->send_UDP_command($jsonNewValues));
    }
    
    public function cmd_get_param()
    {
        $arrValues = array("targ"=>"", "cmd"=>"get_param","login-ack"=>(string)$this->username,"pwd-msg"=>(string)$this->password);
        $jsonNewValues = json_encode($arrValues);
		
        return($this->send_UDP_command($jsonNewValues));
    }

    public function cmd_cec($command)
    {
        $arrValues = array(
                "comment"   => "",
                "targ"      => "",
                "cmd"       => "play_media",
                "login_ack" => $this->username,
                "pwd_msg"   => $this->password,
                "data" => array(array(
                        "comment" => "",
                        "action"  => $command,
                        "objects" => array(array())
                ))
        );
        
        $jsonNewValues = json_encode($arrValues, JSON_UNESCAPED_SLASHES);
        
        //return($this->send_TCP_command($jsonNewValues));
        return($this->send_UDP_command($jsonNewValues, Params::MEDIA_CONTROLLER_UDP_PORT));
    }

    public function cmd_cec_info()
    {
        //just work properly using json directly
        $jsonCommand = '{"comment":"","targ":"","cmd":"play_media","login_ack":"","pwd_msg":"","data":[{"comment":"","action":"cecInfo","objects":[{}]}]}';
        
        //return($this->send_TCP_command($jsonCommand));
        return($this->send_UDP_command($jsonCommand, Params::MEDIA_CONTROLLER_UDP_PORT));
    }
    
    public function cmd_fw_ver()
    {
        $arrValues = array(
                "comment"   => "",
                "targ"      => "",
                "cmd"       => "sys_admin",
                "login_ack" => $this->username,
                "pwd_msg"   => $this->password,
                "data" => array(array(
                        "comment" => "",
                        "action"  => "verInfo",
                        "objects" => array()
                ))
        );
        $jsonNewValues = json_encode($arrValues, JSON_UNESCAPED_SLASHES);
        
        //return($this->send_TCP_command($jsonNewValues));
        return($this->send_UDP_command($jsonNewValues, Params::MEDIA_CONTROLLER_UDP_PORT));
    }
    
    public function cmd_play_media_control($action, $objects)
    {
        $arrValues = array(
                "comment"   => "",
                "targ"      => "",
                "cmd"       => "play_media",
                "login_ack" => $this->username,
                "pwd_msg"   => $this->password,
                "data" => array(array(
                        "comment" => "",
                        "action"  => $action,
                        "objects" => $objects
                ))
        );
        
        $jsonNewValues = json_encode($arrValues, JSON_UNESCAPED_SLASHES);
        
        //return($this->send_TCP_command($jsonNewValues));
        return($this->send_UDP_command($jsonNewValues, Params::MEDIA_CONTROLLER_UDP_PORT));
    }

    public function cmd_playlist_add($media, $objects)
    {
        $arrValues = array(
                "comment"   => "",
                "targ"      => "",
                "cmd"       => "set_media",
                "login_ack" => $this->username,
                "pwd_msg"   => $this->password,
                "data" => array(array(
                        "comment" => "",
                        "action"  => "playlistAdd",
                        "media"   => $media,
                        "objects" => $objects
                ))
        );
        
        $jsonNewValues = json_encode($arrValues, JSON_UNESCAPED_SLASHES);
        
        //return($this->send_TCP_command($jsonNewValues));
        return($this->send_UDP_command($jsonNewValues, Params::MEDIA_CONTROLLER_UDP_PORT));
    }
    
    public function cmd_get_media_control($action, $objects, $comment = true)
    {
        $data = array(array(
                "action"  => $action,
                "objects" => $objects
        ));
        
        if ($comment)
        {
            $data = array(array(
                    "comment" => "",
                    "action"  => $action,
                    "objects" => $objects
            ));
        } 
        
        $arrValues = array(
                "comment"   => "",
                "targ"      => "",
                "cmd"       => "get_media",
                "login_ack" => $this->username,
                "pwd_msg"   => $this->password,
                "data"      => $data
        );
        
        $jsonNewValues = json_encode($arrValues, JSON_UNESCAPED_SLASHES);
        
        //return($this->send_TCP_command($jsonNewValues, Params::TCP_SOCKET_BIG_TIMEOUT));
        return($this->send_UDP_command($jsonNewValues, Params::MEDIA_CONTROLLER_UDP_PORT, Params::TCP_SOCKET_BIG_TIMEOUT));
    }

    public function cmd_get_media_control_no_comment($action, $objects)
    {
        $arrValues = array(
                "comment"   => "",
                "targ"      => "",
                "cmd"       => "get_media",
                "login_ack" => $this->username,
                "pwd_msg"   => $this->password,
                "data" => array(array(
                        "action"  => $action,
                        "objects" => $objects
                ))
        );
        
        $jsonNewValues = json_encode($arrValues, JSON_UNESCAPED_SLASHES);
        
        //return($this->send_TCP_command($jsonNewValues));
        return($this->send_UDP_command($jsonNewValues, Params::MEDIA_CONTROLLER_UDP_PORT));
    }
    
    public function cmd_capture($action, $objects = array(array()))
    {
        $arrValues = array(
                "comment"   => "",
                "targ"      => "",
                "cmd"       => "record_media",
                "login_ack" => $this->username,
                "pwd_msg"   => $this->password,
                "data" => array(array(
                        "comment" => "",
                        "action"  => $action,
                        "objects" => $objects
                ))
        );
        
        $jsonNewValues = json_encode($arrValues, JSON_UNESCAPED_SLASHES);
        
        //return($this->send_TCP_command($jsonNewValues));
        return($this->send_UDP_command($jsonNewValues, Params::MEDIA_CONTROLLER_UDP_PORT));
    }
    
    public function cmd_download($data)
    {
        $arrValues = array(
                "comment"   => "",
                "targ"      => "",
                "cmd"       => "set_media",
                "login_ack" => $this->username,
                "pwd_msg"   => $this->password,
                "data" => array(array(
                        "ftpServer" => $data['server'],
                        "ftpUser"   => $data["user"],
                        "ftpPwd"    => $data["password"],
                        "ftpPath"   => $data["path"],                      
                        "objects"   => $data["files"],
                        "action"    => "download",
                ))
        );
        
        $jsonNewValues = json_encode($arrValues, JSON_UNESCAPED_SLASHES);
        
        //return($this->send_TCP_command($jsonNewValues));
        return($this->send_UDP_command($jsonNewValues, Params::MEDIA_CONTROLLER_UDP_PORT));
    }
    
    // Peter controller UDP port = 3700
    // Jack Media controller UDP port = 3500
    public function send_UDP_command($textStringPacket, $destinationPort = Params::PETER_CONTROLLER_UDP_PORT, $timeout = Params::UDP_SOCKET_TIMEOUT)
    {
		Logger::getInstance()->addDebug('SENDING: ' .$textStringPacket);

		$hexStringPacket = $this->text_to_hex_command($textStringPacket);

		$destinationAddress = Params::PROD_PROTOCOL_DESTINATION_ADDRESS;
		$myServerIP         = Params::PROD_PROTOCOL_ORIGIN_ADDRESS;
		$originClientPort   = Params::PROD_PROTOCOL_ORIGIN_PORT;
		
		if (Params::ENV == 'DEV')
		{
		    $destinationAddress = Params::DEV_PROTOCOL_DESTINATION_ADDRESS_UDP;
		    $myServerIP         = Params::DEV_PROTOCOL_ORIGIN_ADDRESS;
		}

		$clientSocket     = new UdpSocket($myServerIP, $originClientPort, $destinationAddress, $destinationPort, "UDP", "SOCKET_CLIENT_STREAM");
		$lastErrorMessage = $clientSocket->get_lastErrorMessage();

		if (!empty($lastErrorMessage) )
        {
            Logger::getInstance()->addError('DEBUG: Error client socket: ' .$clientSocket->get_lastErrorMessage());
            return null;
        }

        Logger::getInstance()->addDebug('SENDING: IP:'.$myServerIP.':'.$originClientPort.' TO:'.$destinationAddress.':'.$destinationPort);
        
        $clientSocket->send_unicast_packet($hexStringPacket, $destinationAddress);        

        Logger::getInstance()->addDebug('DEBUG: Opening listening port.');

		$buffer = $clientSocket->receive_unicast_packet($timeout);

		$clientSocket->close();
		
		return ($buffer);
		
    }
    
    public function send_TCP_command($textStringPacket, $timeout = Params::TCP_SOCKET_TIMEOUT)
    {
        Logger::getInstance()->addDebug('SENDING: ' .$textStringPacket);
        
        $hexStringPacket = $this->text_to_hex_command($textStringPacket);
        
        $destinationAddress = Params::PROD_PROTOCOL_DESTINATION_ADDRESS;
        $myServerIP         = Params::PROD_PROTOCOL_ORIGIN_ADDRESS;
        $originClientPort   = Params::PROD_PROTOCOL_ORIGIN_PORT;
        $destinationtPort   = Params::MEDIA_CONTROLLER_TCP_PORT;
        
        if (Params::ENV == 'DEV')
        {
            $destinationAddress = Params::DEV_PROTOCOL_DESTINATION_ADDRESS_TCP;
            $myServerIP         = Params::DEV_PROTOCOL_ORIGIN_ADDRESS;
        }

        $clientSocket     = new IpSocket($myServerIP, $originClientPort, $destinationAddress, $destinationtPort, "TCP", "SOCKET_CLIENT_STREAM");
        $lastErrorMessage = $clientSocket->get_lastErrorMessage();

        if (!empty($lastErrorMessage) )
        {
            Logger::getInstance()->addError('Error client socket: ' .$clientSocket->get_lastErrorMessage());
            return null;
        }
        
        Logger::getInstance()->addDebug('SENDING: IP:'.$myServerIP.':'.$originClientPort.' TO:'.$destinationAddress.':'.$destinationtPort);
        
        $clientSocket->send_unicast_packet($hexStringPacket, $destinationAddress);
        
        Logger::getInstance()->addDebug('DEBUG: Opening listening port.');
        
        $buffer = $clientSocket->receive_unicast_packet_response($timeout);
        
        $clientSocket->close();
        
        return ($buffer);        
    }
    
    public function is_usb_mounted()
    {
        return $this->check_flash("USB");
    }
    
    public function is_sd_mounted()
    {
        return $this->check_flash("SD");
    }
    
    protected function check_flash($option)
    {
        if ($option== "USB")
        {
            $deviceResponse = $this->cmd_check_usb();
        } else {
            $deviceResponse = $this->cmd_check_sd();
        }
        
        if ($deviceResponse)
        {
            $arrParameters = $this->hex_command_to_text($deviceResponse[0]);
            if (!empty($arrParameters) && $arrParameters['login_ack'] == 'OK')
            {
                return true;
            } 
        }

        return false;
    }
}

?>