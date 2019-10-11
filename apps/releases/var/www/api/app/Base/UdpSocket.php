<?PHP

namespace App\Base;

use App\Params;
use App\Helpers\StringHelper;

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


	private $receivingBuffer = [];

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
	
	    $this->socketDescriptor = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP); 
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

				Logger::getInstance()->addError($this->lastErrorMessage);				
				Logger::getInstance()->addError($errorcode . ': ' . $errormsg);
				return false;
			}

			Logger::getInstance()->addDebug('DEBUG: SENT ' .$this->sourceAddress .':' .$this->sourcePort .' to ' .$ipAddress .':' .$this->destinationPort );
			//Logger::getInstance()->lwrite('DEBUG: Packet sent =  ' .$dataToSendHexString );

			return true;
		}
	}

	public function receive_unicast_packet ($timeout)
	{
        //wait for 2 secs max			
		socket_set_option($this->socketDescriptor,SOL_SOCKET,SO_RCVTIMEO,array("sec"=>$timeout,"usec"=>0));

		socket_recvfrom($this->socketDescriptor, $buf, Params::UDP_SOCKET_BUFFER_SIZE, 0, $this->destinationAddress, $this->destinationPort);

		if ($buf)
		{
			$response_HexString = bin2hex($buf);
			$this->receivingBuffer[] = $response_HexString;
			
			Logger::getInstance()->addDebug("DEBUG: Packet received: " . StringHelper::string_hex2str($response_HexString));
			
			return $this->receivingBuffer;
		}

		$errorcode = socket_last_error();
		$errormsg = socket_strerror($errorcode);

		Logger::getInstance()->addError("No data received or socket error");
		Logger::getInstance()->addError($errorcode . ': ' . $errormsg);
		return false;
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
		        Logger::getInstance()->addDebug('DEBUG: Close UDP Client Socket' );
		    }

		}
		else if ($this->socketType == UdpSocket::SOCKET_TYPE_SERVER)
		{
			stream_socket_shutdown ($this->socketDescriptor, STREAM_SHUT_RDWR);
			Logger::getInstance()->addDebug('DEBUG: Close Server Socket' );
		}
		else if ($this->socketType == UdpSocket::SOCKET_TYPE_SERVER_SOCK)
		{
		    socket_close($this->socketDescriptor);
		    Logger::getInstance()->addDebug('DEBUG: Close Server Socket Sock' );
		}
	}

	/* ************************************************************************* */	

}

?>
