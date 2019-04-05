<?PHP

class UdpSocket
{
	const SOCKET_PROTOCOL_UDP = "UDP";

	const SOCKET_TYPE_CLIENT = "SOCKET_CLIENT_STREAM";
	const SOCKET_TYPE_SERVER = "SOCKET_SERVER_STREAM";
	
	const SOCKET_TYPE_CLIENT_SOCK = "SOCKET_CLIENT_SOCK";
	const SOCKET_TYPE_SERVER_SOCK = "SOCKET_SERVER_SOCK";

	const TAG_FOR_CMD = "faa0";
	const TAG_FOR_RESP = "fba0";    

	const MSG_DONTWAIT = 0x40;

	public $socketDescriptor;

	private $sourceAddress = '127.0.0.1';
	private $sourcePort = '3700';
	private $destinationAddress = '127.0.0.1';
	private $destinationPort = '3600';
    private $socketProtocol = '';
	private $socketType = '';    


	private $receivingBuffer = '';

	private $lastErrorMessage = '';	

	public function get_lastErrorMessage()
	{
		return $this->lastErrorMessage;
	}

	function __construct($sourceAddress, $sourcePort, $destinationAddress, $destinationPort, $socketProtocol, $socketType)
	{
	 	$this->sourceAddress = $sourceAddress;
	 	$this->sourcePort = $sourcePort;
	 	$this->destinationAddress = $destinationAddress;
	 	$this->destinationPort = $destinationPort;
	 	$this->socketProtocol = $socketProtocol;
	 	$this->socketType = $socketType;
	 	
	 	if ($socketProtocol == UdpSocket::SOCKET_PROTOCOL_UDP)
	 	{
	 		if ($socketType == UdpSocket::SOCKET_TYPE_CLIENT)
	 		{
	 			$this->socketDescriptor = $this->create_socket_client_udp();
			}
	 		else if ($socketType == UdpSocket::SOCKET_TYPE_SERVER)
	 		{
	 			$this->socketDescriptor = $this->create_socket_server_udp();
	 		}
	 		else if ($socketType == UdpSocket::SOCKET_TYPE_SERVER_SOCK)
	 		{
	 		    $this->socketDescriptor = $this->create_socket_server_udp_sock();
	 		}	 		
	 		else
	 		{
	 			$this->lastErrorMessage =  'Unknown socketType requested: $socketType';
	 		}
	 	}
	}

	private function create_socket_client_udp()
	{
		$this->socketDescriptor = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);		
			
		if ($this->socketDescriptor == false)
		{
			$this->lastErrorMessage =  'The socket could not be created !';
			Logger::getInstance()->lwrite('DEBUG: ' .$this->lastErrorMessage);
			return false;
		}
			
		if (socket_bind($this->socketDescriptor, $this->sourceAddress, $this->sourcePort) === false)
		{
			$this->lastErrorMessage = "socket_bind() failed:" . socket_strerror(socket_last_error($this->socketDescriptor)) . "\n";
			Logger::getInstance()->lwrite('DEBUG: ' .$this->lastErrorMessage);
			return false;
		}
		
		Logger::getInstance()->lwrite('DEBUG: Open Client Socket' );
		
		return $this->socketDescriptor;
	}

	private function create_socket_server_udp() 
	{
		$errno=0;
		$errstr='';
		
		if(empty($this->sourceAddress) OR empty($this->sourcePort))
		{
			$this->lastErrorMessage = 'Server source address empty';
			Logger::getInstance()->lwrite('DEBUG: ' .$this->lastErrorMessage);
			return false;
		}
		
		$this->socketDescriptor = stream_socket_server('udp://'.$this->sourceAddress.':'.$this->sourcePort, $errno , $errstr, STREAM_SERVER_BIND);
		
		if (!$this->socketDescriptor)
		{
			$this->lastErrorMessage = 'Server source address empty';
			Logger::getInstance()->lwrite('DEBUG: ' .$this->lastErrorMessage);
			return false;
		}
		
		Logger::getInstance()->lwrite('DEBUG: Open Server Socket' );
		
		return $this->socketDescriptor;
	}
	    
	private function create_socket_server_udp_sock()
	{
	    $errno=0;
	    $errstr='';
	
	    if(empty($this->sourceAddress) OR empty($this->sourcePort))
	    {
	        $this->lastErrorMessage = 'Server source address empty';
	        Logger::getInstance()->lwrite('DEBUG: ' .$this->lastErrorMessage);
	        return false;
	    }
	
	    $this->socketDescriptor = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP); 
	    //stream_socket_server('udp://'.$this->sourceAddress.':'.$this->sourcePort, $errno , $errstr, STREAM_SERVER_BIND);
	
	    if (!$this->socketDescriptor)
	    {
	        $errorcode = socket_last_error();
	        $errormsg = socket_strerror($errorcode);
	        
	        $this->lastErrorMessage = 'Cannot create socket Server sock : ' .$errormsg;
	        Logger::getInstance()->lwrite('DEBUG: ' .$this->lastErrorMessage);
	        return false;
	    }
	
	    Logger::getInstance()->lwrite('DEBUG: Open Server Socket sock' );
	
	    // Bind the source address
	    if( !socket_bind($this->socketDescriptor, $this->sourceAddress , $this->sourcePort) )
	    {
	        $errorcode = socket_last_error();
	        $errormsg = socket_strerror($errorcode);
	         
	        $this->lastErrorMessage = 'Cannot bind socket Server sock : ' .$errormsg;
	        Logger::getInstance()->lwrite('DEBUG: ' .$this->lastErrorMessage);
	        
	        $this->close();
	        return false;
	    }
	    
	    Logger::getInstance()->lwrite('DEBUG: Server Socket bind OK to: ' .$this->sourceAddress .' => ' .$this->sourcePort);
	    
	    return $this->socketDescriptor;
	}	

	public function send_unicast_packet($dataToSendHexString, $ipAddress)
	{
		//socket_set_option($this->socketDescriptor, SOL_SOCKET, SO_BROADCAST, 1);

		if ($this->socketProtocol == UdpSocket::SOCKET_PROTOCOL_UDP)
		{
			$this->sendingBuffer = hex2bin($dataToSendHexString);

			$this->byteSent = socket_sendto($this->socketDescriptor, $this->sendingBuffer, strlen($this->sendingBuffer), 0, $ipAddress, $this->destinationPort);

			if (!$this->byteSent)
			{
				$this->lastErrorMessage = 'Unicast packet could not be sent to ip address ' .$ipAddress .' !';

				$errorcode = socket_last_error();
        		$errormsg = socket_strerror($errorcode);

				Logger::getInstance()->lwrite('DEBUG: ' .$this->lastErrorMessage);				
				Logger::getInstance()->lwrite('DEBUG: ' .$errorcode . ': ' . $errormsg);
				return false;
			}

			Logger::getInstance()->lwrite('DEBUG: SENT ' .$this->sourceAddress .':' .$this->sourcePort .' to ' .$ipAddress .':' .$this->destinationPort );

			return true;
		}
	}

	public function receive_unicast_packet ()
	{
			
		socket_set_option($this->socketDescriptor,SOL_SOCKET,SO_RCVTIMEO,array("sec"=>2,"usec"=>0));

		$from = "127.0.0.1";
		$port = 3700;

		socket_recvfrom($this->socketDescriptor, $buf, 65536, 0, $from, $port);

		if ($buf)
		{
			Logger::getInstance()->lwrite("RECEIVED: " . "$buf FROM $from:$port");

			$response_HexString = bin2hex($buf);
			$this->receivingBuffer = array();
			$this->receivingBuffer[] = $response_HexString;

			return $this->receivingBuffer;
		}
		else
		{
			Logger::getInstance()->lwrite('DEBUG: ' . "Didn't receive buffer" );				
			return FALSE;
		}

	}


	public function listen($timeout = 200000)
	{
		$this->receivingBuffer = array();

		$client_socks = array();
		$read_socks = $client_socks;
		$read_socks[] = $this->socketDescriptor;

		//var_dump($read_socks); echo "<BR />";

		$write= null;
		$expect = null;
		$changed_num = stream_select($read_socks, $write, $expect, 0, $timeout);

		//var_dump($changed_num); echo "<BR />";
		//var_dump($read_socks); echo "<BR />";

		if (false === $changed_num) 
		{
			Logger::getInstance()->lwrite("DEBUG: Something went wrong while selecting" );
		}
		if (0 === $changed_num) {
			return;
		}

		//message from existing client
		foreach($read_socks as $sock)
		{

			var_dump($sock);

			$response_BinString = fread($sock, $expectedDataLenghtToReceive);
			if(!$response_BinString)
			{
				unset($client_socks[ array_search($sock, $client_socks) ]);
				@fclose($sock);
				Logger::getInstance()->lwrite("A client disconnected. Now there are total ". count($client_socks) . " clients.n");
				continue;
			}

			//send the message back to client
			//fwrite($sock, $data);
			
			$response_HexString = bin2hex($response_BinString);

			$this->receivingBuffer[] = $response_HexString;
			
			//Logger::getInstance()->lwrite('DEBUG: Server received packet from ' .$this->sourceAddress .' (port:' .$this->sourcePort .') to port:' .$this->destinationPort );
			Logger::getInstance()->lwrite('DEBUG: Server received packet =  ' .$response_HexString );
			
			if ($bytesDataLenghtToStopListening != NULL)
			{
				// Stop listening accoriing to the data lenght received (for quick living)
				if (strlen($response_HexString) == ($bytesDataLenghtToStopListening*2) )
				{
					Logger::getInstance()->lwrite('DEBUG: QUICK STOP Listening (Based on dataLenght) on port:' .$this->sourcePort );
					return $this->receivingBuffer;
				}
			}
			
			if (!empty($keyValueToCheckToStopListening)) 
			{
				$isExpected = true; 
				
				// The 'key' is the position and the 'value' is the value to compare with 
				foreach ($keyValueToCheckToStopListening as $key => $value) 
				{
					if ( strcasecmp(substr($response_HexString, $key, strlen($value) ) , $value ) != 0)
					{
						$isExpected = false;
						break;
					}
				}
				
				if ($isExpected) {
					Logger::getInstance()->lwrite('DEBUG: QUICK STOP Listening on port:' .$this->sourcePort );
					return $this->receivingBuffer;
				}
			}
			
		}

		return $this->receivingBuffer;

	}


	public function listening_UDP_Sock($timeToWaitInSec, $expectedDataLenghtToReceive, $keyValueToCheckToStopListening = NULL, $timeToWaitInMicroSec = 0)
	{
	    $this->receivingBuffer = array();
	
	    $startingTime = strtotime("now");
	
	    $ellapseTime = 0;
	
	    Logger::getInstance()->lwrite('DEBUG: Listening UDP SOCK on port:' .$this->sourcePort );

		//var_dump($this->socketDescriptor);

	    while( $ellapseTime <  $timeToWaitInSec )
	    {
	       $byteReceived = socket_recvfrom($this->socketDescriptor, $response_BinString, $expectedDataLenghtToReceive, UdpSocket::MSG_DONTWAIT, $from, $port); // WORK ONLY IN LINUX not in WINDOWS

	       //$byteReceived = socket_recvfrom($this->socketDescriptor, $response_BinString, $expectedDataLenghtToReceive, 0x40, $from, $port);
	
	        //start reading and use the timeout provided
	        if($byteReceived !== FALSE)
	        {
	            $response_HexString = bin2hex($response_BinString);
	            
	            $this->receivingBuffer[] = $response_HexString;
	            
	            Logger::getInstance()->lwrite('DEBUG: from: ' .$from .':' .$port .' =>  ' .$response_HexString );
	            
	            if (!empty($keyValueToCheckToStopListening))
	            {
	                $isExpected = true;
	            
	                // The 'key' is the position and the 'value' is the value to compare with
	                foreach ($keyValueToCheckToStopListening as $key => $value)
	                {
	                    if ( strcasecmp(substr($response_HexString, $key, strlen($value) ) , $value ) != 0)
	                    {
	                        $isExpected = false;
	                        break;
	                    }
	                }
	            
	                if ($isExpected) {
	                    Logger::getInstance()->lwrite('DEBUG: QUICK STOP Listening on port:' .$this->sourcePort );
	                    return $this->receivingBuffer;
	                }
	            }
	        }
	        	
	        $ellapseTime = strtotime("now") - $startingTime;
	    }
	
	    Logger::getInstance()->lwrite('DEBUG: STOP Listening on port:' .$this->sourcePort );
	
	    return $this->receivingBuffer;
	}

	public function listening($timeToWaitInSec, $expectedDataLenghtToReceive, $keyValueToCheckToStopListening = NULL, $timeToWaitInMicroSec = 0, $bytesDataLenghtToStopListening = NULL)
	{
		$this->receivingBuffer = array();
	
		$startingTime = strtotime("now");
	
		$ellapseTime = 0;

		$client_socks = array();
		$read_socks = $client_socks;
		$read_socks[] = $this->socketDescriptor;
		
		Logger::getInstance()->lwrite('DEBUG: Listening on port:' .$this->sourcePort );

		while( $ellapseTime <  $timeToWaitInSec )
		{

			$except = null;
			//if(!stream_select($read_socks, $write, $except, $timeToWaitInSec ))
			if(false === stream_select($read_socks, $write, $except, $timeToWaitInSec ))			
			{
				//$this->lastErrorMessage = "Something went wrong while stream selecting";
				Logger::getInstance()->lwrite("DEBUG: Something went wrong while selecting" );

				$errorcode = socket_last_error();
        		$errormsg = socket_strerror($errorcode);

				Logger::getInstance()->lwrite('DEBUG: ' .$errorcode);
				Logger::getInstance()->lwrite('DEBUG: ' .$errormsg);				
			}

			//message from existing client
			foreach($read_socks as $sock)
			{
				$response_BinString = fread($sock, $expectedDataLenghtToReceive);

				Logger::getInstance()->lwrite("DEBUG: ".(string)$response_BinString );

				if(!$response_BinString)
				{
					unset($client_socks[ array_search($sock, $client_socks) ]);
					@fclose($sock);
					Logger::getInstance()->lwrite("A client disconnected. Now there are total ". count($client_socks) . " clients.n");
					continue;
				}
	
				//send the message back to client
				//fwrite($sock, $data);
				
				$response_HexString = bin2hex($response_BinString);
	
				$this->receivingBuffer[] = $response_HexString;
				
				//Logger::getInstance()->lwrite('DEBUG: Server received packet from ' .$this->sourceAddress .' (port:' .$this->sourcePort .') to port:' .$this->destinationPort );
				Logger::getInstance()->lwrite('DEBUG: Server received packet =  ' .$response_HexString );
				
				if ($bytesDataLenghtToStopListening != NULL)
				{
				    // Stop listening accoriing to the data lenght received (for quick living)
				    if (strlen($response_HexString) == ($bytesDataLenghtToStopListening*2) )
				    {
				        Logger::getInstance()->lwrite('DEBUG: QUICK STOP Listening (Based on dataLenght) on port:' .$this->sourcePort );
				        return $this->receivingBuffer;
				    }
				}
				
				if (!empty($keyValueToCheckToStopListening)) 
				{
					$isExpected = true; 
					
					// The 'key' is the position and the 'value' is the value to compare with 
					foreach ($keyValueToCheckToStopListening as $key => $value) 
					{
						if ( strcasecmp(substr($response_HexString, $key, strlen($value) ) , $value ) != 0)
						{
							$isExpected = false;
							break;
						}
					}
					
					if ($isExpected) {
						Logger::getInstance()->lwrite('DEBUG: QUICK STOP Listening on port:' .$this->sourcePort );
						return $this->receivingBuffer;
					}
				}
				
			}
	
			$ellapseTime = strtotime("now") - $startingTime;

		}
	
		Logger::getInstance()->lwrite('DEBUG: STOP Listening on port:' .$this->sourcePort );
	
		return $this->receivingBuffer;
	}

	public function get_receivingBuffer()
	{
		return $this->receivingBuffer;
	}

	public function close() 
	{
	    //if (PersistantVariables::getInstance()->MySystem->get_isCurrentSocketToBeSaved())
	        //return;
	    
		if ($this->socketType == UdpSocket::SOCKET_TYPE_CLIENT)
		{
		    if ($this->socketProtocol == UdpSocket::SOCKET_PROTOCOL_UDP)
		    {
		        socket_close($this->socketDescriptor);
		        Logger::getInstance()->lwrite('DEBUG: Close UDP Client Socket' );
		    }

		}
		else if ($this->socketType == UdpSocket::SOCKET_TYPE_SERVER)
		{
			stream_socket_shutdown ($this->socketDescriptor, STREAM_SHUT_RDWR);
			Logger::getInstance()->lwrite('DEBUG: Close Server Socket' );
		}
		else if ($this->socketType == UdpSocket::SOCKET_TYPE_SERVER_SOCK)
		{
		    socket_close($this->socketDescriptor);
		    Logger::getInstance()->lwrite('DEBUG: Close Server Socket Sock' );
		}
	}

	/* ************************************************************************* */	

}

?>
