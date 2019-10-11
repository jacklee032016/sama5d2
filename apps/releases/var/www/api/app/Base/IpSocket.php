<?php

namespace App\Base;

use App\Params;
use App\Helpers\StringHelper;

class IpSocket
{
	const SOCKET_PROTOCOL_TCP = "TCP";
	const SOCKET_PROTOCOL_UDP = "UDP";
	
	const SOCKET_TYPE_CLIENT = "SOCKET_CLIENT_STREAM";
	const SOCKET_TYPE_SERVER = "SOCKET_SERVER_STREAM";
	
	const SOCKET_TYPE_CLIENT_SOCK = "SOCKET_CLIENT_SOCK";
	const SOCKET_TYPE_SERVER_SOCK = "SOCKET_SERVER_SOCK";
	
	const BROADAST_ADDRESS = "255.255.255.255";
	
	const MSG_DONTWAIT = 0x40;
	
	public $socketDescriptor;
	
	private $socketType = '';
	
	private $socketProtocol = '';

	private $sourceAddress = '';
	
	private $sourcePort = '';
	
	private $destinationAddress = '';
	
	private $destinationPort = '';
	
	private $sendingBuffer = '';
	
	private $receivingBuffer = '';
	
	private $lastErrorMessage = '';
	
	private $byteSent = 0;
	private $byteRecieved = 0;
	

	function __construct($sourceAddress, $sourcePort, $destinationAddress, $destinationPort, $socketProtocol, $socketType)
	{
	 	$this->sourceAddress = $sourceAddress;
	 	$this->sourcePort = $sourcePort;
	 	$this->destinationAddress = $destinationAddress;
	 	$this->destinationPort = $destinationPort;
	 	$this->socketProtocol = $socketProtocol;
	 	$this->socketType = $socketType;
	 	
	 	if ($socketProtocol == IpSocket::SOCKET_PROTOCOL_UDP)
	 	{
	 		if ($socketType == IpSocket::SOCKET_TYPE_CLIENT)
	 		{
	 			$this->socketDescriptor = $this->create_socket_client_udp();
	 			
// 	 			if (!$this->socketDescriptor)
// 	 			{
// 	 				return false;
// 	 			}
	 		}
	 		else if ($socketType == IpSocket::SOCKET_TYPE_SERVER)
	 		{
	 			$this->socketDescriptor = $this->create_socket_server_udp();
	 			
// 	 			if (!$this->socketDescriptor)
// 	 			{
// 	 				return false;
// 	 			}
	 		}
	 		else if ($socketType == IpSocket::SOCKET_TYPE_SERVER_SOCK)
	 		{
	 		    $this->socketDescriptor = $this->create_socket_server_udp_sock();
	 		    	
// 	 			if (!$this->socketDescriptor)
// 	 			{
// 	 				return false;
// 	 			}
	 		}	 		
	 		else
	 		{
	 			$this->lastErrorMessage =  'Unknown socketType requested: $socketType';
//	 			return false;
	 		}
	 		
	 	}
	 	else // if ($socketProtocol == IpSocket::SOCKET_PROTOCOL_TCP)
	 	{
	 	    if ($socketType == IpSocket::SOCKET_TYPE_CLIENT)
	 		{
	 			$this->socketDescriptor = $this->create_socket_client_tcp();
	 			
// 	 			if (!$this->socketDescriptor)
// 	 			{
// 	 				return false;
// 	 			}
	 		}
	 		else if ($socketType == IpSocket::SOCKET_TYPE_SERVER_SOCK)
	 		{
	 			$this->socketDescriptor = $this->create_socket_server_tcp();
	 			
// 	 			if (!$this->socketDescriptor)
// 	 			{
// 	 				return false;
// 	 			}
	 		}
	 		else
	 		{
	 			$this->lastErrorMessage =  'Unknown socketType requested: $socketType';
//	 			return false;
	 		}
	 	}
	 	
//	 	return true;
	}
	
	public function get_receivingBuffer()
	{
		return $this->receivingBuffer;
	}
	
	private function create_socket_client_tcp()
	{
	    //$this->socketDescriptor = stream_socket_client("tcp://$this->destinationAddress:$this->destinationPort", $errno, $errorMessage);
	    $this->socketDescriptor = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	    	
	    if ($this->socketDescriptor == false)
	    {
	        $this->lastErrorMessage =  'The TCP socket could not be created !';
	        Logger::getInstance()->addError($this->lastErrorMessage);
	        return false;
	    }
	    
	    Logger::getInstance()->addDebug('DEBUG: Attempting to connect to ' .$this->destinationAddress .' on port ' .$this->destinationPort .'...\n');
	    
	    socket_set_nonblock($this->socketDescriptor);
	    
	    $timeout = 3; // 3 sec
	    $time = time();
	    $result = FALSE;
	    while ($result === FALSE)
	    {
	        $result = socket_connect($this->socketDescriptor, $this->destinationAddress, $this->destinationPort);
	        
	        $err = socket_last_error($this->socketDescriptor);
	        if ($err == 115)
	           Logger::getInstance()->addDebug('DEBUG: socket_connect error = ' .$err);
	        elseif ($err == 111)
	           Logger::getInstance()->addDebug('DEBUG: socket_connect error = ' .$err);
	        elseif ($err != 0)
	           Logger::getInstance()->addError('socket_connect error = ' .$err);
	            
	        if ($err != SOCKET_EISCONN) /// 10056 in windows
	        {
	            if ((time() - $time) >= $timeout)
	            {
	                $this->lastErrorMessage =  "socket_connect() failed. Connection timed out.";
	                socket_set_block($this->socketDescriptor);
	                socket_close($this->socketDescriptor);
	                return false;
	            }
	            usleep(100);  // 1/10000  sec
	            continue;
	        }
	        else 
	        {
	            // SOckect is connected
	            break;
	        }
	            
	    }
	    
	    socket_set_block($this->socketDescriptor);
	    
	    Logger::getInstance()->addDebug('DEBUG: Opened TCP Client Socket' );
	
	    return $this->socketDescriptor;
	}

	private function create_socket_server_tcp()
	{
	    //$this->socketDescriptor = stream_socket_client("tcp://$this->destinationAddress:$this->destinationPort", $errno, $errorMessage);
	    $this->socketDescriptor = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	
	    if ($this->socketDescriptor == false)
	    {
	        $this->lastErrorMessage =  'The TCP socket could not be created !';
	        Logger::getInstance()->addError($this->lastErrorMessage);
	        return false;
	    }
	     
	    if (socket_bind($this->socketDescriptor, $this->sourceAddress, $this->sourcePort) === false)
	    {
	        $this->lastErrorMessage = "socket_bind() failed:" . socket_strerror(socket_last_error($this->socketDescriptor)) . "\n";
	        Logger::getInstance()->addError($this->lastErrorMessage);
	        return false;
	    }
	     
	    Logger::getInstance()->addDebug('DEBUG: Open TCP server Socket' );
	
	    return $this->socketDescriptor;
	}	
	
	private function create_socket_client_udp()
	{
		$this->socketDescriptor = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
			
		if ($this->socketDescriptor == false)
		{
			$this->lastErrorMessage =  'The socket could not be created !';
			Logger::getInstance()->addError($this->lastErrorMessage);
			return false;
		}
			
		if (socket_bind($this->socketDescriptor, $this->sourceAddress, $this->sourcePort) === false)
		{
			$this->lastErrorMessage = "socket_bind() failed:" . socket_strerror(socket_last_error($this->socketDescriptor)) . "\n";
			Logger::getInstance()->addError($this->lastErrorMessage);
			return false;
		}
		
		Logger::getInstance()->addDebug('DEBUG: Open Client Socket' );
		
		return $this->socketDescriptor;
	}
	
	private function create_socket_server_udp() 
	{
		$errno=0;
		$errstr='';
		
		if(empty($this->sourceAddress) OR empty($this->sourcePort))
		{
			$this->lastErrorMessage = 'Server source address empty';
			Logger::getInstance()->addError($this->lastErrorMessage);
			return false;
		}
		
		$this->socketDescriptor = stream_socket_server('udp://'.$this->sourceAddress.':'.$this->sourcePort, $errno , $errstr, STREAM_SERVER_BIND);
		
		if (!$this->socketDescriptor)
		{
			$this->lastErrorMessage = 'Server source address empty';
			Logger::getInstance()->addError($this->lastErrorMessage);
			return false;
		}
		
		Logger::getInstance()->addDebug('DEBUG: Open Server Socket' );
		
		return $this->socketDescriptor;
	}
	
	public function send_broadcat_packet($dataToSendHexString)
	{
		socket_set_option($this->socketDescriptor, SOL_SOCKET, SO_BROADCAST, 1);
		
		$this->sendingBuffer = hex2bin($dataToSendHexString);

		$this->byteSent = socket_sendto($this->socketDescriptor, $this->sendingBuffer, strlen($this->sendingBuffer), 0, IpSocket::BROADAST_ADDRESS, $this->destinationPort);
		
		if (!$this->byteSent) 
		{
			$this->lastErrorMessage = 'Broadcast packect could not be sent !';
			Logger::getInstance()->addError($this->lastErrorMessage);
			return false;
		}
		
		Logger::getInstance()->addDebug('DEBUG: Broadcast packet sent from ' .$this->sourceAddress .' (port:' .$this->sourcePort .') to port:' .$this->destinationPort );
		//if (IS_ALL_LOGS_ENABLED == TRUE)
		  Logger::getInstance()->addDebug('DEBUG: Packet sent =  ' .$dataToSendHexString );
	}
	
	public function send_multicast_packet($dataToSendHexString, $multicastAddress)
	{
	    socket_set_option($this->socketDescriptor, SOL_SOCKET, SO_BROADCAST, 1);
	
	    $this->sendingBuffer = hex2bin($dataToSendHexString);
	
	    $this->byteSent = socket_sendto($this->socketDescriptor, $this->sendingBuffer, strlen($this->sendingBuffer), 0, $multicastAddress, $this->destinationPort);
	
	    if (!$this->byteSent)
	    {
	        $this->lastErrorMessage = 'Multicast packet could not be sent !';
	        Logger::getInstance()->addError($this->lastErrorMessage);
	        return false;
	    }
	
	    Logger::getInstance()->addDebug('DEBUG: Multicast packet sent from ' .$this->sourceAddress .' (port:' .$this->sourcePort .') to port:' .$this->destinationPort );
	    //if (IS_ALL_LOGS_ENABLED == TRUE)
	    Logger::getInstance()->addDebug('DEBUG: Packet sent =  ' .$dataToSendHexString );
	    
	    return true;
	}
	
	public function send_unicast_packet($dataToSendHexString, $ipAddress)
	{
		//socket_set_option($this->socketDescriptor, SOL_SOCKET, SO_BROADCAST, 1);
	
	    if ($this->socketProtocol == IpSocket::SOCKET_PROTOCOL_UDP)
	    {
		  $this->sendingBuffer = hex2bin($dataToSendHexString);
	
		  $this->byteSent = socket_sendto($this->socketDescriptor, $this->sendingBuffer, strlen($this->sendingBuffer), 0, $ipAddress, $this->destinationPort);
	
		  if (!$this->byteSent)
		  {
			 $this->lastErrorMessage = 'Unicast packet could not be sent to ip address ' .$ipAddress .' !';
			 Logger::getInstance()->addError($this->lastErrorMessage);
			 return false;
		  }
	
		  Logger::getInstance()->addDebug('DEBUG: Unicast packet sent from ' .$this->sourceAddress .' (port:' .$this->sourcePort .') to address: ' .$ipAddress .' port:' .$this->destinationPort );
		  //if (IS_ALL_LOGS_ENABLED == TRUE)
		      //Logger::getInstance()->lwrite('DEBUG: Packet sent =  ' .$dataToSendHexString );
		      
		  return true;
		      
	    }
	    else  if ($this->socketProtocol == IpSocket::SOCKET_PROTOCOL_TCP)
	    {
	        $this->sendingBuffer = hex2bin($dataToSendHexString);
	         
	        //$this->byteSent = fwrite($this->socketDescriptor, $dataToSendAsciiString);
	         
	        $this->byteSent = socket_write($this->socketDescriptor, $this->sendingBuffer, strlen($this->sendingBuffer));
	         
	        if (!$this->byteSent)
	        {
	            $this->lastErrorMessage = 'Unicast TCP packet could not be sent to ip address ' .$ipAddress .' !';
	            Logger::getInstance()->addError($this->lastErrorMessage);
	            return false;
	        }
	         
	        Logger::getInstance()->addDebug('DEBUG: Unicast TCP packet sent from ' .$this->sourceAddress .' (port:' .$this->sourcePort .') to address: ' .$ipAddress .' port:' .$this->destinationPort );
	        //if (IS_ALL_LOGS_ENABLED == TRUE)
	           //Logger::getInstance()->lwrite('DEBUG: Packet sent =  ' .$dataToSendHexString );
	         
	        return true;
	    }	    

	}
	
	public function send_unicast_packet_asciiString($dataToSendAsciiString, $ipAddress)
	{
	    //socket_set_option($this->socketDescriptor, SOL_SOCKET, SO_BROADCAST, 1);
	
	    if ($this->socketProtocol == IpSocket::SOCKET_PROTOCOL_UDP)
	    {
	        $this->sendingBuffer = $dataToSendAsciiString;
	
	        $this->byteSent = socket_sendto($this->socketDescriptor, $this->sendingBuffer, strlen($this->sendingBuffer), 0, $ipAddress, $this->destinationPort);
	
	        if (!$this->byteSent)
	        {
	            $this->lastErrorMessage = 'Unicast UDP packet could not be sent to ip address ' .$ipAddress .' !';
	            Logger::getInstance()->addError($this->lastErrorMessage);
	            return false;
	        }
	
	        Logger::getInstance()->addDebug('DEBUG: Unicast UDP packet sent from ' .$this->sourceAddress .' (port:' .$this->sourcePort .') to address: ' .$ipAddress .' port:' .$this->destinationPort );
	        //if (IS_ALL_LOGS_ENABLED == TRUE)
	           Logger::getInstance()->addDebug('DEBUG: Packet sent =  ' .$dataToSendAsciiString );
	        
	        return true;
	    }
	    else  if ($this->socketProtocol == IpSocket::SOCKET_PROTOCOL_TCP)
	    {
	        $this->sendingBuffer = $dataToSendAsciiString;
	        
	        //$this->byteSent = fwrite($this->socketDescriptor, $dataToSendAsciiString);
	        
	        $this->byteSent = socket_write($this->socketDescriptor, $dataToSendAsciiString, strlen($dataToSendAsciiString));
	        
	        if (!$this->byteSent)
	        {
	            $this->lastErrorMessage = 'Unicast TCP packet could not be sent to ip address ' .$ipAddress .' !';
	            Logger::getInstance()->addError($this->lastErrorMessage);
	            return false;
	        }
	        
	        Logger::getInstance()->addDebug('DEBUG: Unicast TCP packet sent from ' .$this->sourceAddress .' (port:' .$this->sourcePort .') to address: ' .$ipAddress .' port:' .$this->destinationPort );
	        //if (IS_ALL_LOGS_ENABLED == TRUE)
	           Logger::getInstance()->addDebug('DEBUG: Packet sent =  ' .$dataToSendAsciiString );
	        
	        return true;
	    }
	
	    return false;
	}
		
	public function receive_unicast_packet_response($timeout)
	{
	    socket_set_option($this->socketDescriptor,SOL_SOCKET,SO_RCVTIMEO,array("sec"=>$timeout,"usec"=>0));
	    
	    if ($this->socketProtocol == IpSocket::SOCKET_PROTOCOL_UDP)
	    {
	        return $this->receivingBuffer;
	    }
	    else if ($this->socketProtocol == IpSocket::SOCKET_PROTOCOL_TCP)
	    {
	        //$response_packet = stream_get_contents($this->socketDescriptor);
	        
	        $response_packet = socket_read($this->socketDescriptor, Params::TCP_SOCKET_BUFFER_SIZE);
	        
	        if ($response_packet === false)
	        {
	            Logger::getInstance()->addError('receive_unicast_packet_response (TCP) error =  ' .socket_strerror(socket_last_error()) );
	            $this->receivingBuffer[] = "";
	            return $this->receivingBuffer;
	        }
	        $response_packet= bin2hex($response_packet);
	        $this->receivingBuffer[] = $response_packet;
	        
	        //Logger::getInstance()->lwrite('DEBUG: Server received packet from ' .$this->sourceAddress .' (port:' .$this->sourcePort .') to port:' .$this->destinationPort );
	        Logger::getInstance()->addDebug('DEBUG: receive_unicast_packet_response =  ' .StringHelper::string_hex2str($response_packet) );
	        
	        return $this->receivingBuffer;
	    }
	
	}	
	public function close() 
	{
	    //if (PersistantVariables::getInstance()->MySystem->get_isCurrentSocketToBeSaved())
	    //    return;
	    
		if ($this->socketType == IpSocket::SOCKET_TYPE_CLIENT)
		{
		    if ($this->socketProtocol == IpSocket::SOCKET_PROTOCOL_UDP)
		    {
		        socket_close($this->socketDescriptor);
		        Logger::getInstance()->addDebug('DEBUG: Close UDP Client Socket' );
		    }
		    else  if ($this->socketProtocol == IpSocket::SOCKET_PROTOCOL_TCP)
		    {
		        socket_close($this->socketDescriptor);
		        Logger::getInstance()->addDebug('DEBUG: Close TCP Client Socket' );
		    }
		}
		else if ($this->socketType == IpSocket::SOCKET_TYPE_SERVER)
		{
			stream_socket_shutdown ($this->socketDescriptor, STREAM_SHUT_RDWR);
			Logger::getInstance()->addDebug('DEBUG: Close Server Socket' );
		}
		else if ($this->socketType == IpSocket::SOCKET_TYPE_SERVER_SOCK)
		{
		    socket_close($this->socketDescriptor);
		    Logger::getInstance()->addDebug('DEBUG: Close Server Socket Sock' );
		}
	}
	

	public function get_lastErrorMessage()
	{
		return $this->lastErrorMessage;
	}
	
	public function listening_TCP_unicast($timeToWaitInSec, $expectedDataLenghtToReceive, $keyValueToCheckToStopListening = NULL, $timeToWaitInMicroSec = 0)
	{
	    set_error_handler("myErrorHandler", E_WARNING);
	    
	    $this->receivingBuffer = array();
	
	    $startingTime = strtotime("now");
	
	    $ellapseTime = 0;
	    
	    $isExpectedDataLengthReceived = false;
	
	    Logger::getInstance()->addDebug('DEBUG: TCP Listening on port:' .$this->sourcePort );
	
	    socket_set_option($this->socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, array("sec"=>0, "usec"=>20000)); // 200000
	    
	    $Total_response_BinString = "";
	    if (!empty($keyValueToCheckToStopListening))
	    	$isExpected = false;
	    else	    
	    	$isExpected = true;
	    
	    while( $ellapseTime <  $timeToWaitInSec )
	    {
	        //Logger::getInstance()->lwrite('DEBUG: -------------> socket_read called <----------------');
	        $response_BinString = socket_read($this->socketDescriptor, $expectedDataLenghtToReceive);

	        /*if ($response_BinString === false)
	        {
	            $this->lastErrorMessage =  "socket_read() failed.\nReason: " . socket_strerror(socket_last_error($this->socketDescriptor));
	            Logger::getInstance()->lwrite('DEBUG: ' .$this->lastErrorMessage);
	            //return $this->receivingBuffer;
	        }*/
	            
	        if (strlen($response_BinString) >= $expectedDataLenghtToReceive)
	            $isExpectedDataLengthReceived = true;
	
	        if ($response_BinString != "")
	        {
	           $response_HexString = bin2hex($response_BinString);
	
	           $this->receivingBuffer[] = $response_HexString;
	
	           Logger::getInstance()->addDebug('DEBUG: Server received packet =  ' .$response_HexString );
	        }
	        else if (!empty($keyValueToCheckToStopListening) && ($isExpected == true) )
	        {
	        	Logger::getInstance()->addDebug('DEBUG: QUICK STOP Listening on port:' .$this->sourcePort );
	        	return $this->receivingBuffer;
	        }
	        	
	
	        if (!empty($keyValueToCheckToStopListening) && ($isExpected == false) )
	        {
                // The 'key' is the position and the 'value' is the value to compare with
               foreach ($keyValueToCheckToStopListening as $key => $value)
                {
                	if (is_string($key))
                	{
                		$isExpected = false;
                		
                		$indexValueLengthCouple = explode("-", $key);
                		$count = 0;
                		foreach ($indexValueLengthCouple as $keyIndex => $valueIndex) 
                		{
                			$indexValueLength = explode("_", $valueIndex);
                			$valueArr = explode("-",$value);

                			//$test1 = substr($response_HexString, intval($indexValueLength[0]), strlen($indexValueLength[1]));
                			//$test2 = $valueArr[$count];
                			if ( strcasecmp(substr($response_HexString, intval($indexValueLength[0]), intval($indexValueLength[1]) ) , $valueArr[$count] ) == 0)
                			{
                				$isExpected = true;
                				break;
                			}
                			$count++;
                		}
                			
                	}
                    else if ( strcasecmp(substr($response_HexString, $key, strlen($value) ) , $value ) != 0)
                    {
                        $isExpected = false;
                        break;
                    }
                }
	                	
                if ($isExpected) 
                {
                    //Logger::getInstance()->lwrite('DEBUG: QUICK STOP Listening on port:' .$this->sourcePort );
                    //return $this->receivingBuffer;
                    // Dont break yet make sure there are no more data to receice
                }
            }
            
            if ($isExpectedDataLengthReceived == true)
            {
                Logger::getInstance()->addDebug('DEBUG: QUICK STOP (DataLenghtExpected reached) Listening on port:' .$this->sourcePort );
                return $this->receivingBuffer;
            }
	
	        $ellapseTime = strtotime("now") - $startingTime;
	    }
	
	    if ($response_BinString === false)
	    {
	        $this->lastErrorMessage =  "socket_read() failed.\nReason: " . socket_strerror(socket_last_error($this->socketDescriptor));
	        Logger::getInstance()->addError($this->lastErrorMessage);
	        //return $this->receivingBuffer;
	    }
	    
	    Logger::getInstance()->addDebug('DEBUG: STOP Listening on port:' .$this->sourcePort );
	
	    return $this->receivingBuffer;
	}	
	
	public function listening_TCP_server($timeToWaitInSec, $expectedDataLenghtToReceive, $keyValueToCheckToStopListening = NULL, $timeToWaitInMicroSec = 0)
	{
	    set_error_handler("myErrorHandler", E_WARNING);
	     
	    $this->receivingBuffer = array();
	
	    $startingTime = strtotime("now");
	
	    $ellapseTime = 0;
	     
	    $isExpectedDataLengthReceived = false;
	    
	    $response_BinString = "";
	
	    Logger::getInstance()->addDebug('DEBUG: TCP Listening on port:' .$this->sourcePort );
	
	    //socket_set_option($this->socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, array("sec"=>0, "usec"=>20000)); // 200000
	    socket_listen($this->socketDescriptor);
	     
	    socket_set_nonblock($this->socketDescriptor);
	    
	    while( $ellapseTime <  $timeToWaitInSec )
	    {
	        //Logger::getInstance()->lwrite('DEBUG: NEWWWWWWWWWWWWWWWWWW ITERATION ->>>>>>>>>>>>>>>>>>>>>');
	        
	        $client =  socket_accept($this->socketDescriptor);

	        //Logger::getInstance()->lwrite('DEBUG: -------------> socket_read called <----------------');
	        if ($client !== FALSE)
	        {
	           socket_set_block($this->socketDescriptor);
	           usleep(250000); // 1/4 sec
	           Logger::getInstance()->addDebug('DEBUG: A Client Sockect is connected >>>>>>>>>>> to my TCP Server');
	           $response_BinString = socket_read($client, $expectedDataLenghtToReceive + 500); // better to wait for more data than expected
	           
	           if ($response_BinString === FALSE)
	           {
	               $errorcode = socket_last_error($client);
	               $errormsg = socket_strerror($errorcode);
	               
	               Logger::getInstance()->addError('SOCKET READ  FAILED : ' .$errormsg );
	               
	               socket_set_nonblock($this->socketDescriptor);
	           }
	           
	           socket_close($client);
	           Logger::getInstance()->addDebug('DEBUG: Close Client Sockect <<<<<<<<<<<<<< that was connected to my TCP Server');
	           
	           //socket_set_nonblock($this->socketDescriptor);
	        }
	        
	        /*if ($response_BinString === false)
	         {
	         $this->lastErrorMessage =  "socket_read() failed.\nReason: " . socket_strerror(socket_last_error($this->socketDescriptor));
	         Logger::getInstance()->lwrite('DEBUG: ' .$this->lastErrorMessage);
	         //return $this->receivingBuffer;
	        }*/
	         
	        if (strlen($response_BinString) >= $expectedDataLenghtToReceive)
	            $isExpectedDataLengthReceived = true;
	
	        if ($response_BinString != "")
	        {
	            $response_HexString = bin2hex($response_BinString);
	
	            $this->receivingBuffer[] = $response_HexString;
	
	            $response_BinString = "";
	            
	            Logger::getInstance()->addDebug('DEBUG: Server received packet =  ' .$response_HexString );
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
	
	            if ($isExpected)
	            {
	                Logger::getInstance()->addDebug('DEBUG: QUICK STOP Listening on port:' .$this->sourcePort );
	                return $this->receivingBuffer;
	            }
	        }
	
	        if ($isExpectedDataLengthReceived == true)
	        {
	            Logger::getInstance()->addDebug('DEBUG: QUICK STOP (DataLenghtExpected reached) Listening on port:' .$this->sourcePort );
	            return $this->receivingBuffer;
	        }
	
	       // usleep(250000); // 1/4 sec
	        
	        $ellapseTime = strtotime("now") - $startingTime;
	    }

	    if ($response_BinString === false)
	    {
	        $this->lastErrorMessage =  "socket_read() failed.\nReason: " . socket_strerror(socket_last_error($this->socketDescriptor));
	        Logger::getInstance()->addError($this->lastErrorMessage);
	        //return $this->receivingBuffer;
	    }
	     
	    Logger::getInstance()->addDebug('DEBUG: STOP Listening on port:' .$this->sourcePort );
	
	    return $this->receivingBuffer;
	}
	
	private function create_socket_server_udp_sock()
	{
	    $errno=0;
	    $errstr='';
	
	    if(empty($this->sourceAddress) OR empty($this->sourcePort))
	    {
	        $this->lastErrorMessage = 'Server source address empty';
	        Logger::getInstance()->addError($this->lastErrorMessage);
	        return false;
	    }
	
	    $this->socketDescriptor = socket_create(AF_INET, SOCK_DGRAM, 0); 
	    //stream_socket_server('udp://'.$this->sourceAddress.':'.$this->sourcePort, $errno , $errstr, STREAM_SERVER_BIND);
	
	    if (!$this->socketDescriptor)
	    {
	        $errorcode = socket_last_error();
	        $errormsg = socket_strerror($errorcode);
	        
	        $this->lastErrorMessage = 'Cannot create socket Server sock : ' .$errormsg;
	        Logger::getInstance()->addError($this->lastErrorMessage);
	        return false;
	    }
	
	    Logger::getInstance()->addDebug('DEBUG: Open Server Socket sock' );
	
	    // Bind the source address
	    if( !socket_bind($this->socketDescriptor, $this->sourceAddress , $this->sourcePort) )
	    {
	        $errorcode = socket_last_error();
	        $errormsg = socket_strerror($errorcode);
	         
	        $this->lastErrorMessage = 'Cannot bind socket Server sock : ' .$errormsg;
	        Logger::getInstance()->addError($this->lastErrorMessage);
	        
	        $this->close();
	        return false;
	    }
	    
	    Logger::getInstance()->addDebug('DEBUG: Server Socket bind OK to: ' .$this->sourceAddress .' => ' .$this->sourcePort);
	    
	    return $this->socketDescriptor;
	}	
	
	// the following is mainly used to get the IP & PORT of the device that communicate with the server
	
}

function telnet_keepSocketOpened()
{
    PersistantVariables::getInstance()->MySystem->set_isCurrentSocketToBeSaved(YES);
}

function telnet_closeClientSocket()
{
    // CLOSE SOCKET
    $clientSocket = PersistantVariables::getInstance()->MySystem->get_current_Socket();
    if ($clientSocket != null)
    {
        PersistantVariables::getInstance()->MySystem->set_isCurrentSocketToBeSaved(NO);
        $clientSocket->close();
        PersistantVariables::getInstance()->MySystem->set_current_Socket(null);
    }
    PersistantVariables::getInstance()->MySystem->set_isCurrentSocketToBeSaved(NO);
}


function telnet_sendReceiveDataPacket($destinationAddress, $destinationPort, $data_String_toSend, $someExpectedDataToReceive, $listeningTimeout, $expectedDataLenghtToReceive, $originClientPort, $isDataToSendAscii, $isSameSocket = true, $isUseServerSock = true)
{
    set_error_handler("myErrorHandler", E_WARNING);

    $myServerIP = $_SERVER["SERVER_ADDR"];

    //=================================================================
    // STETP #1 :  SEND the PACKET
    //=================================================================

    if (PersistantVariables::getInstance()->MySystem->get_isCurrentSocketToBeSaved() && (PersistantVariables::getInstance()->MySystem->get_current_Socket() == null) )
    {
        $clientSocket = new IpSocket($myServerIP, $originClientPort, $destinationAddress, $destinationPort, IpSocket::SOCKET_PROTOCOL_TCP, IpSocket::SOCKET_TYPE_CLIENT);

        $temp_var = $clientSocket->get_lastErrorMessage();
        if (!empty($temp_var) )
        {
            Logger::getInstance()->addError('Error client socket: ' .$clientSocket->get_lastErrorMessage());
            return null;
        }

        PersistantVariables::getInstance()->MySystem->set_current_Socket($clientSocket);
    }
    else if (PersistantVariables::getInstance()->MySystem->get_current_Socket() == null)
    {
        $clientSocket = new IpSocket($myServerIP, $originClientPort, $destinationAddress, $destinationPort, IpSocket::SOCKET_PROTOCOL_TCP, IpSocket::SOCKET_TYPE_CLIENT);

        $temp_var = $clientSocket->get_lastErrorMessage();

        if (!empty($temp_var) )
        {
            Logger::getInstance()->addError('Error client socket: ' .$clientSocket->get_lastErrorMessage());
            return null;
        }
    }
    else
    {
        $clientSocket = PersistantVariables::getInstance()->MySystem->get_current_Socket();
    }

    if ($isDataToSendAscii)
        $isSuccess = $clientSocket->send_unicast_packet_asciiString($data_String_toSend, $destinationAddress);
    else
        $isSuccess = $clientSocket->send_unicast_packet($data_String_toSend, $destinationAddress);

    if ($isSuccess == false)
    {
        $clientSocket->close();
        return null;
    }
    
    //=================================================================
    // STETP #2 :  LISTEN for ACK PACKET + DATA if any
    //=================================================================

    if ($listeningTimeout == 0) // Meaning do not listen
    {
        $clientSocket->close();
        return '';
    }
    
    if ($isSameSocket)
    {
        $receivedBuffer = '';
        $receivedBufferList = $clientSocket->listening_TCP_unicast($listeningTimeout, $expectedDataLenghtToReceive,$someExpectedDataToReceive);
        foreach ($receivedBufferList as $value)
        {
            $receivedBuffer .= $value;
        }
        //Logger::getInstance()->lwrite('DEBUG: DATA RECEIVED: ' .$receivedBuffer);
        Logger::getInstance()->addDebug('DEBUG: DATA RECEIVED ASCII: ' .GlobalFunctions::string_hex2str($receivedBuffer));
    
        $clientSocket->close();
    }
    else 
    {
        $clientSocket->close();
        
        if ($isUseServerSock)
        {
            $serverSocket = new IpSocket($myServerIP,$originClientPort, "", "", IpSocket::SOCKET_PROTOCOL_TCP, IpSocket::SOCKET_TYPE_SERVER_SOCK);
        
            $temp_var = $serverSocket->get_lastErrorMessage();
            if (!empty($temp_var) )
            {
                Logger::getInstance()->addError('Error server socket: ' .$serverSocket->get_lastErrorMessage());
                return null;
            }
        	
            $receivedBuffer = '';
            $receivedBufferList = $serverSocket->listening_TCP_server($listeningTimeout, $expectedDataLenghtToReceive, $someExpectedDataToReceive);
            foreach ($receivedBufferList as $value)
            {
                $receivedBuffer .= $value;
            }
            Logger::getInstance()->addDebug('DEBUG: SERVER SOCK DATA RECEIVED: ' .$receivedBuffer);
            //Logger::getInstance()->lwrite('DEBUG: SERVER SOCK DATA RECEIVED ASCII: ' .GlobalFunctions::string_hex2str($receivedBuffer));
        
            $serverSocket->close();
        }
        else 
        {
            // Another alternative of implementation using STREAM Socket for SERVER
            
            $server = stream_socket_server("tcp://" .$myServerIP .":" .$originClientPort ."", $errno, $errorMessage);
            
            if ($server === false)
            {
                Logger::getInstance()->addError('Error server socket: ' .$errorMessage);
                return null;
            } 

            Logger::getInstance()->addDebug('DEBUG: Open TCP server Socket STREAM and listening on port: ' .$originClientPort);

            // Listening ...
            $startingTime = strtotime("now");
            $ellapseTime = 0;
            $receivedBuffer = '';
            while( $ellapseTime <  $listeningTimeout )
            {
                //for (;;) {
            
                $client = @stream_socket_accept($server, 1);
            
                if ($client)
                {
                    //$dest = fopen('ABCD.txt', 'w');
            
                    //while (!feof($in)) $size += fwrite($out,fread($in,8192));
                    //stream_copy_to_stream($client, $dest);
                    $data = stream_get_contents($client);
                    $receivedBuffer .= bin2hex($data);
                    Logger::getInstance()->addDebug('DEBUG: SERVER STREAM DATA RECEIVED:' .$receivedBuffer);
            
                    fclose($client);
                    
                    if (strlen($receivedBuffer) >= $expectedDataLenghtToReceive)
                    {
                        Logger::getInstance()->addDebug('DEBUG: QUICK STOP (DataLenghtExpected reached) Listening on port:' .$originClientPort );
                        stream_socket_shutdown ($server, STREAM_SHUT_RDWR);
                        Logger::getInstance()->addDebug('DEBUG x1: Close STREAM server Socket' );
                        return $receivedBuffer;
                    }
                }
                //}
                $ellapseTime = strtotime("now") - $startingTime;
            }
            
            stream_socket_shutdown ($server, STREAM_SHUT_RDWR);
            Logger::getInstance()->addDebug('DEBUG x2: Close STREAM server Socket' );
        }
        
    }
    
    return $receivedBuffer;
}


function creation_stream_socket_server($addr, $port)
{
	/* Fonction : cr�ation d�une socket en mode non connect� (UDP */
 
	/* Initialisation des variables */
	$errno=0;
	$errstr='';
 
	if(empty($addr) OR empty($port))
	{
		echo 'Adresse ou port vide.<br />';
		exit(1);
	}
	$socket = stream_socket_server('udp://'.$addr.':'.$port, $errno , $errstr, STREAM_SERVER_BIND);
 
	if (!$socket)
	{
		echo 'La socket n\�a pas �t� cr�e.';
		exit(2);
	}
	return $socket;
}

function reception_donnees($socket, $dataLenghtToReceive)
{
	/* R�ception d'un message, la limite du message est ici en octets, soit 1.5ko */
	//echo 'Message re�u : ' . stream_socket_recvfrom($socket, 1500) . '<br />';
	return stream_socket_recvfrom($socket, $dataLenghtToReceive);
}

function close_stream_socket($socket)
{
	if($socket)
	{
		stream_socket_shutdown ($socket, STREAM_SHUT_RDWR);
	}
}

function creation_stream_socket_client($addr, $port)
{
	/* function : cr�ation d�une socket en mode non connect� (UDP */
	/* Initialisation des variables */
	$errno=0;
	$errstr='';
 
	if(empty($addr) OR empty($port))
	{
		echo 'Adresse ou port vide.<br />';
		exit(1);
	}
 
	$socket = stream_socket_client('udp://'.$addr.':'.$port, $errno, $errstr);
 
	if (!$socket)
	{
		echo 'La socket n\�a pas �t� cr�e.';
		exit(2);
	}

	return $socket;
}

function envoi_donnees($socket, $donnees)
{
	fwrite($socket, $donnees);
}

function myErrorHandler($errno, $errstr, $errfile, $errline)
{
    // do something
    Logger::getInstance()->addError('DEBUG: myErrorHandler called ' .$errno .' = ' .$errstr);
}

function TEST_Listen_UDP()
{
    //Create a UDP socket
    if(!($sock = socket_create(AF_INET, SOCK_DGRAM, 0)))
    {
        $errorcode = socket_last_error();
        $errormsg = socket_strerror($errorcode);
         
        die("Couldn't create socket: [$errorcode] $errormsg \n");
    }
    
    //echo "Socket created \n";
    
    // Bind the source address
    if( !socket_bind($sock, "192.168.168.59" , 3334) )
    {
        $errorcode = socket_last_error();
        $errormsg = socket_strerror($errorcode);
         
        die("Could not bind socket : [$errorcode] $errormsg \n");
    }
    
    //echo "Socket bind OK \n";
    
    //Do some communication, this loop can handle multiple clients
    while(1)
    {
        //echo "Waiting for data ... \n";
         
        //Receive some data
        $r = socket_recvfrom($sock, $buf, 1500, 0, $remote_ip, $remote_port);
        //echo "$remote_ip : $remote_port -- " . $buf;
         
        //Send back the data to the client
        //socket_sendto($sock, "OK " . $buf , 100 , 0 , $remote_ip , $remote_port);
        
        $test = 1;
        if ($test == 2)
        {
            socket_close($sock);
        }
    }
    
    socket_close($sock);
}

?>