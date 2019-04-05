<?PHP

require_once('./include/UdpSocket.inc');	

class MuxDevice
{
	const RESOLUTION_PROTOCOLS = array(									
									0=>array('raw'=>'HI_UNF_ENC_FMT_1080P_60','text'=>'1080p 60 Hz'),
									1=>array('raw'=>'HI_UNF_ENC_FMT_1080P_50','text'=>'1080p 50 Hz'),
									2=>array('raw'=>'HI_UNF_ENC_FMT_1080P_30','text'=>'1080p 30 Hz'),
									3=>array('raw'=>'HI_UNF_ENC_FMT_1080P_25','text'=>'1080p 25 Hz'),
									4=>array('raw'=>'HI_UNF_ENC_FMT_1080P_24','text'=>'1080p 24 Hz'),
									5=>array('raw'=>'HI_UNF_ENC_FMT_1080i_60','text'=>'1080i 60 Hz'),
									6=>array('raw'=>'HI_UNF_ENC_FMT_1080i_50','text'=>'1080i 50 Hz'),
									7=>array('raw'=>'HI_UNF_ENC_FMT_720P_60','text'=>'720p 60 Hz'),
									8=>array('raw'=>'HI_UNF_ENC_FMT_720P_50','text'=>'720p 50 Hz'),
									9=>array('raw'=>'HI_UNF_ENC_FMT_576P_50','text'=>'576p 50 Hz'),
									10=>array('raw'=>'HI_UNF_ENC_FMT_480P_60','text'=>'480p 60 Hz'),
									22=>array('raw'=>'HI_UNF_ENC_FMT_861D_640X480_60','text'=>'640x480 60 Hz'),
									23=>array('raw'=>'HI_UNF_ENC_FMT_VESA_800X600_60','text'=>'800x600 60 Hz'),
									24=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1024X768_60','text'=>'1024x768 60 Hz'),
									25=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1280X720_60','text'=>'1280x720 60 Hz'),
									26=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1280X800_60','text'=>'1280x800 60 Hz'),
									27=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1280X1024_60','text'=>'1280x1024 60 Hz'),
									28=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1360X768_60','text'=>'1360x768 60 Hz'),
									29=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1366X768_60','text'=>'1366x768 60 Hz'),
									30=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1400X1050_60','text'=>'1400x1050 60 Hz'),
									31=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1440X900_60','text'=>'1440x900 60 Hz'),
									34=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1600X1200_60','text'=>'1600x1200 60 Hz'),
									35=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1680X1050_60','text'=>'1680x1050 60 Hz'),
									37=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1920X1080_60','text'=>'1920x1080 60 Hz'),
									38=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1920X1200_60','text'=>'1920x1200 60 Hz'),
									39=>array('raw'=>'HI_UNF_ENC_FMT_VESA_1920X1440_60','text'=>'1920x1440 60 Hz'),
									40=>array('raw'=>'HI_UNF_ENC_FMT_VESA_2048X1152_60','text'=>'2048x1152 60 Hz'),
									41=>array('raw'=>'HI_UNF_ENC_FMT_VESA_2560X1440_60_RB','text'=>'2560x1440 60 Hz'),
									42=>array('raw'=>'HI_UNF_ENC_FMT_VESA_2560X1600_60_RB','text'=>'2560x1600 60 Hz'),
									64=>array('raw'=>'HI_UNF_ENC_FMT_3840X2160_24','text'=>'3840x2160 24 Hz'),
									65=>array('raw'=>'HI_UNF_ENC_FMT_3840X2160_25','text'=>'3840x2160 25 Hz'),
									66=>array('raw'=>'HI_UNF_ENC_FMT_3840X2160_30','text'=>'3840x2160 30 Hz'),
									67=>array('raw'=>'HI_UNF_ENC_FMT_3840X2160_50','text'=>'3840x2160 50 Hz'),
									68=>array('raw'=>'HI_UNF_ENC_FMT_3840X2160_60','text'=>'3840x2160 60 Hz'),
									69=>array('raw'=>'HI_UNF_ENC_FMT_4096X2160_24','text'=>'4096x2160 24Hz'),
									70=>array('raw'=>'HI_UNF_ENC_FMT_4096X2160_25','text'=>'4096x2160 25 Hz'),
									71=>array('raw'=>'HI_UNF_ENC_FMT_4096X2160_30','text'=>'4096x2160 30 Hz'),
									72=>array('raw'=>'HI_UNF_ENC_FMT_4096X2160_50','text'=>'4096x2160 50 Hz'),
									73=>array('raw'=>'HI_UNF_ENC_FMT_4096X2160_60','text'=>'4096x2160 60 Hz'),
									100=>array('raw'=>'HI_UNF_ENC_FMT_4096X2160_AUTO','text'=>'AUTO')
	);

	const RX_SUPPORTED_RESOLUTION_PROTOCOLS = array(100,0,1,2,3,4,5,6,7,8,10,38,41,42,64,65,66,67,68);

	const PROTECTED_PARAMETERS = array('pName','model','fwVer','cName','mac','dns1','dns2');

	public $pName; //HdmiOverIPh264-h265
    public $model; //TX-500768
	public $fwVer; //1.0.0
	public $cName; //myRXname1
	public $mac; //00:13:14:15:69:18

	public $ip; //192.168.168.110
	public $mask; //255.255.255.0
	public $gateway; //192.168.1.1
	public $isDhcp; //0
	public $isDipOn; //1
	
	public $isReset; //0
	public $isReboot; //0
	
	public $isTS_1; //1
	public $TSurl_1; //video0.ts
	public $isHLS_1; //1
	public $HLSurl_1; //video0.m3u8
	public $isFLV_1; //1
	public $FLVurl_1; //0.flv
	public $HTTPport_1; //80
	public $isRTSP_1; //1
	public $RTSPurl_1; //video0
	public $RTSPport_1; //38001
	public $isRTSPph_1; //0
	public $isRTMP_1; //1
	public $isRTMPph_1; //0
	public $RTMPurl_1; //rtmp://192.168.1.50/live/0
	public $RTMPport_1; //1935
	public $isMCAST_1; //1
	public $MCASTip_1; //239.100.0.15
	public $MCASTport_1; //111
	public $MCASTurl_1; //udp://@239.100.0.15:111
	public $SourceIp_1; //1
	public $VidResI_1; //1920x1080
	public $VidFpsI_1; //60
	public $VidResO_1; //1920x1080
	public $VidFpsO_1; //30
	public $VidBitR_1; //1500
	public $VidCodec_1; //H.264-HighProfile
	public $AudFrq_1; //44100
	public $AudBitR_1; //48000
	public $AudCodec_1; //MP3
	public $isAudin; //HDMI, ANALOG
	public $isAudioAuto; //1, 0

	public $Encoding_type; //H.264, H.265
	public $Encoding_level; //HighProfile, MainProfile, BaselineProfile
	public $Deep_Color; // 8, 10, 12, AUTO

	public $isVBR_1; //0/1 (0-CBR, 1-VBR) (Default: VBR)
	public $GOP_1; //5 to 300 (Default: 30)
	public $CPU_Usage; //0 to 100
	public $Memory_Usage; //0 to 374.8M

	public $vid_width_I;
	public $vid_Height_I;
	public $vid_fps_I;

	const PARAM_PLAYER_PARAMETERS = array('display_mode','display_res','deep_color','num_aud_on','player');

	public $display_mode;
	public $display_res;
	public $deep_color;
	public $num_aud_on;
	//public $player;

	const PLAYER_PARAMETERS = array('player_num','player_enable','play_state','low_delay','repeatable','src_onplaying','playlist');

	public $player_num;
	public $player_enable;
	public $play_state;
	public $low_delay;
	public $repeatable;
	public $src_onplaying;
	public $playlist;

	function __construct($arrParameters)
	{
		if ($arrParameters == null)
		{
			return FALSE;
		}

		$this->fill_attributes($arrParameters);

		//if (!in_array($this->VidCodec_1,array('H265','H264','')))
		if (strpos($this->VidCodec_1,'-'))
		{
			$this->Encoding_type = explode('-',$this->VidCodec_1)[0];
			$this->Encoding_level = explode('-',$this->VidCodec_1)[1];
		}
		else
		{
			$this->Encoding_type = $this->VidCodec_1;
			$this->Encoding_level = null;
		}
	}

	public function fill_attributes($arrParameters)
	{
		if (!is_null($arrParameters)) 
		{
			foreach ($arrParameters as $name=>$value)
			{
				//var_dump($name);
				if ( property_exists('MuxDevice', $name) )
				{				
					$this->{$name} = $value;
				}
				
				if ( $name == 'player' )
				{
					foreach ( $value[0] as $namePlayer=>$valuePlayer )
					{
						if ( property_exists('MuxDevice', $namePlayer) )
						{				
							$this->{$namePlayer} = $valuePlayer;
						}

						/*
						if ( $namePlayer == 'playlist')
						{
							$this->playlist = [];
							foreach ( $valuePlayer as $valuePlaylist )
							{
								$this->playlist[] = $valuePlaylist;
							}

						}
						*/
					}
				}			
			}
		}
	}
	
	public function return_resolution($resolution_code)
	{
		if ($tmp = MuxDevice::RESOLUTION_PROTOCOLS[(int)$resolution_code]['text'])
		{
			return $tmp;
		}

		return($resolution_code);
	}

	public function get($parameter)
	{
		switch($parameter)
		{
			case "TSurl_1":
				if (GlobalFunctions::isDeviceTX()) return($this->changeURLtoFilename($this->{$parameter})); else return($this->{$parameter});
				break;

			case "HLSurl_1":
				if (GlobalFunctions::isDeviceTX()) return($this->changeURLtoFilename($this->{$parameter})); else return($this->{$parameter});
				break;			

			case "FLVurl_1":
				if (GlobalFunctions::isDeviceTX()) return($this->changeURLtoFilename($this->{$parameter})); else return($this->{$parameter});
				break;

			default:
				return($this->{$parameter});
		}		
	}

	public function save_changes($arrPost)
	{
		$arrNewValues = array();
		$player_num_one = array();
		$player_num_one["player_num"] = "1";

		$player_num_two = array('player_num' => '2','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "1", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array());
		$player_num_three = array('player_num' => '3','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "1", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array());
		$player_num_four = array('player_num' => '4','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "1", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array());
		$player_num_five = array('player_num' => '5','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "1", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array());
		$player_num_six = array('player_num' => '6','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "1", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array());		

		foreach ($_POST as $parameter=>$newValue)
		{
		    if ($parameter == 'Encoding_type')
		    {
		        $this->Encoding_type = $newValue;
				if ($this->Encoding_type == 'H.264') $arrNewValues['VidCodec_1'] = 'H.264-HighProfile';
				else $arrNewValues['VidCodec_1'] = 'H.265-HighProfile';
		    }
		    if ($parameter == 'Encoding_level')
		    {
		        $this->$Encoding_level= $newValue;
				if ($this->Encoding_type == 'H.264') $arrNewValues['VidCodec_1'] = 'H.264-HighProfile';
				else $arrNewValues['VidCodec_1'] = 'H.265-HighProfile';
		    }
								
			if($this->isChanged($parameter,$newValue))
			{
				if (isset($_POST['HTTPport_1'])) $this->HTTPport_1 = $_POST['HTTPport_1']; //set the http port beforehand

				switch ($parameter)
				{
					case 'VidResO_1':
						//if ($newValue == "Same as input" ) $newValue = $this->VidResI_1;
						if ($newValue == "Same as input" ) $newValue = "1";
						if ($newValue == "100" and GlobalFunctions::isDeviceRX() ) $newValue = "100";
						break;
					case 'TSurl_1':
						if (GlobalFunctions::isDeviceTX()) $newValue = $this->changeFilenameToURL($newValue);
						if (GlobalFunctions::isDeviceRX()) $arrNewValues['isTS_1'] = '1';
						else $arrNewValues['isTS_1'] = $this->isTS_1;
						break;
					case 'HLSurl_1':
						if (GlobalFunctions::isDeviceTX()) $newValue = $this->changeFilenameToURL($newValue);
						if (GlobalFunctions::isDeviceRX()) $arrNewValues['isHLS_1'] = '1';
						else $arrNewValues['isHLS_1'] = $this->isHLS_1;
						break;
					case 'FLVurl_1':
						if (GlobalFunctions::isDeviceTX()) $newValue = $this->changeFilenameToURL($newValue);
						if (GlobalFunctions::isDeviceRX()) $arrNewValues['isFLV_1'] = '1';
						else $arrNewValues['isFLV_1'] = $this->isFLV_1;
						break;
					case 'HTTPport_1':						
						//$arrNewValues['isTS_1'] = $this->isTS_1;
						$arrNewValues['TSurl_1'] = $this->changeFilenameToURL($_POST['TSurl_1']);
						$arrNewValues['isTS_1'] = $this->changeFilenameToURL($_POST['isTS_1']);
						$arrNewValues['HLSurl_1'] = $this->changeFilenameToURL($_POST['HLSurl_1']);
						$arrNewValues['isHLS_1'] = $this->changeFilenameToURL($_POST['isHLS_1']);
						$arrNewValues['FLVurl_1'] = $this->changeFilenameToURL($_POST['FLVurl_1']);
						$arrNewValues['isFLV_1'] = $this->changeFilenameToURL($_POST['isFLV_1']);
						break;						
					case 'RTSPurl_1':
						if (GlobalFunctions::isDeviceRX()) $arrNewValues['isRTSP_1'] = '1';
						else $arrNewValues['isRTSP_1'] = $this->isRTSP_1;						
						break;
					case 'RTMPurl_1':
						if (GlobalFunctions::isDeviceRX()) $arrNewValues['isRTMP_1'] = '1';
						else $arrNewValues['isRTMP_1'] = $this->isRTMP_1;
						break;
					case 'MCASTurl_1':
						if (GlobalFunctions::isDeviceRX()) $arrNewValues['isMCAST_1'] = '1';
						else $arrNewValues['isMCAST_1'] = $this->isMCAST_1;
						break;
					case 'AudBitR_1':
						$arrNewValues['AudCodec_1'] = (isset($_POST['AudCodec_1']))? $_POST['AudCodec_1'] : $this->AudCodec_1;
						//$arrNewValues['isAudin'] = (isset($_POST['isAudin']))? $_POST['isAudin'] : $this->isAudin;
						break;
					case 'AudCodec_1':
						$arrNewValues['AudBitR_1'] = (isset($_POST['AudBitR_1']))? $_POST['AudBitR_1'] : $this->AudBitR_1;
						//$arrNewValues['isAudin'] = (isset($_POST['isAudin']))? $_POST['isAudin'] : $this->isAudin;
						break;
					case 'isAudin':
						if ($newValue == 'AUTO')
						{
							//$newValue = $this->isAudin;							
							//$arrNewValues['isAudioAuto'] = "1";
							$parameter = 'isAudioAuto';
							$newValue = "1";
						}
						else
						{
							$arrNewValues['isAudioAuto'] = "0";
						}
						//$arrNewValues['AudBitR_1'] = (isset($_POST['AudBitR_1']))? $_POST['AudBitR_1'] : $this->AudBitR_1;
						//$arrNewValues['AudCodec_1'] = (isset($_POST['AudCodec_1']))? $_POST['AudCodec_1'] : $this->AudCodec_1;
						break;
					case 'isDhcp':
						$arrNewValues['ip'] = (isset($_POST['ip']))? $_POST['ip'] : $this->ip;
						$arrNewValues['mask'] = (isset($_POST['mask']))? $_POST['mask'] : $this->mask;
						$arrNewValues['gateway'] = (isset($_POST['gateway']))? $_POST['gateway'] : $this->gateway;
						break;
					default:
						break;
				}

				if ( in_array($parameter,MuxDevice::PLAYER_PARAMETERS) )
				{
					/*
					$parametersToSet = array(	"display_mode" => "0",
						"num_aud_on" => "1",
						"player" => array(
										array(	"player_num" => "1",
												"low_delay" => "0",
												"repeatable" => "1",
												"playlist" => array(),
												"src_onplaying" => $src_onplaying,
												"window_enable" => "1",
												"window_zorder" => "0",
												"window_aspectCvrs" => "0",
												"window_rotate" => "0",
												"window_xcoord_ratio" => "0.00000",
												"window_ycoord_ratio" => "0.00000",
												"window_width_ratio" => "1.00000",
												"window_height_ratio" => "1.00000",
												"window_xcoord_dst" => "0",
												"window_ycoord_dst" => "0",
												"window_width_dst" => "1280",
												"window_height_dst" => "720"),
										array('player_num' => '2','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "1", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array()),
										array('player_num' => '3','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "1", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array()),
										array('player_num' => '4','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "1", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array()),
										array('player_num' => '5','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "1", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array()),
										array('player_num' => '6','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "1", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array()),
					)
					);
					*/
					switch ($parameter)					
					{
						case 'play_state':
							if ($newValue == "0")
							{
								$player_num_one = array('player_num' => '1','play_state' => '0','window_enable' => '0', "windows_zorder" => "0", "src_onplaying" => "", "low_delay" => "0", "window_aspectCvrs" => "0", "src_onplaying" => "", "playlist" => array());
							}
							else
							{
								$arrNewValues['display_mode'] = "0";
								$player_num_one['src_onplaying'] = trim($this->src_onplaying);								
								$player_num_one['playlist'] = array();
								$player_num_one['window_enable'] = "1";
								$player_num_one['low_delay'] = "0";
								$player_num_one['window_zorder'] = "0";
								$player_num_one['window_aspectCvrs'] = "0";
								$player_num_one['window_rotate'] = "0";
								$player_num_one['window_xcoord_ratio'] = "0.00000";
								$player_num_one['window_ycoord_ratio'] = "0.00000";
								$player_num_one['window_width_ratio'] = "1.00000";
								$player_num_one['window_height_ratio'] = "1.00000";
								$player_num_one['window_xcoord_dst'] = "0";
								$player_num_one['window_ycoord_dst'] = "0";
								$player_num_one['window_width_dst'] = "1280";
								$player_num_one['window_height_dst'] = "720";					
							}
							break;

						case 'src_onplaying':
							$arrNewValues['display_mode'] = "0";
							$player_num_one['src_onplaying'] = trim($newValue);
							$player_num_one['playlist'] = array();							
							$player_num_one['window_enable'] = "1";
							$player_num_one['low_delay'] = "0";
							$player_num_one['window_zorder'] = "0";
							$player_num_one['window_aspectCvrs'] = "0";
							$player_num_one['window_rotate'] = "0";
							$player_num_one['window_xcoord_ratio'] = "0.00000";
							$player_num_one['window_ycoord_ratio'] = "0.00000";
							$player_num_one['window_width_ratio'] = "1.00000";
							$player_num_one['window_height_ratio'] = "1.00000";
							$player_num_one['window_xcoord_dst'] = "0";
							$player_num_one['window_ycoord_dst'] = "0";
							$player_num_one['window_width_dst'] = "1280";
							$player_num_one['window_height_dst'] = "720";													
							break;

						default:
							$player_num_one[$parameter] = $newValue;
							break;
					}
					$arrNewValues = array( "player" => array($player_num_one, $player_num_two, $player_num_three, $player_num_four, $player_num_five, $player_num_six) );
							
				}
				else
				{
					$arrNewValues[$parameter] = $newValue;
				}
				
				$this->{$parameter} = $newValue;
			}
		}

		if (!empty($arrNewValues))
		{
			//var_dump($arrNewValues);
			return($arrNewValues);
		}
		else
		{
			return(false);
		}
	}

	private function changeFilenameToURL($filename)
	{
		return("http://".$this->ip.":".$this->HTTPport_1."/".$filename);
	}

	private function changeURLtoFilename($url)
	{
		return(basename($url));
	}

	private function isChanged($parameterName, $newValue)
	{
		if (in_array($parameterName, MuxDevice::PROTECTED_PARAMETERS))
		{
			return(null);
		}

		if ($parameterName == "VidResO_1" and $newValue == "Same as input" )
		{
			$newValue = $this->VidResI_1;
		}

		switch ($parameterName)
		{
			case 'TSurl_1':
				if (GlobalFunctions::isDeviceTX()) $newValue = $this->changeFilenameToURL($newValue);
				break;
			case 'HLSurl_1':
				if (GlobalFunctions::isDeviceTX()) $newValue = $this->changeFilenameToURL($newValue);
				break;
			case 'FLVurl_1':
				if (GlobalFunctions::isDeviceTX()) $newValue = $this->changeFilenameToURL($newValue);
				break;		
			default:
				break;
		}

		if (isset($this->{$parameterName}))
		{
			if ($this->{$parameterName} <> $newValue)
			{
				return(true);
			}
			else
			{				
				return(false);
			}
		}

		return(null);
	}


public function getResolutionFromProtocol($protocol)
{
	return(Muxdevice::RESOLUTION_PROTOCOLS[$protocol]['text']);
}

public function htmlVideoSizeSelectRX()
	{
		$htmlOutput  = '<LABEL for="VideosizeSle"> Output Resolution:</LABEL>';
		$htmlOutput .= '<SELECT id="VideosizeSle" NAME="display_res">';

		$isSelected = false;
		
		foreach (Muxdevice::RX_SUPPORTED_RESOLUTION_PROTOCOLS as $protocol)
		{
			$resolution = Muxdevice::RESOLUTION_PROTOCOLS[$protocol]['text'];

			$htmlOutput .= '<OPTION value="' . $protocol . '"';

			if ($isSelected == false)
			{
				if ($this->display_res == $protocol) 
				{
					$htmlOutput .= ' selected';
					$isSelected = true;				
				}
			}
			$htmlOutput .= '>';
			$htmlOutput .= $resolution;
			$htmlOutput .= '</OPTION>';
		}


		$htmlOutput .= '</SELECT>';
		return ($htmlOutput);


	}

	public function htmlVideoSizeSelect()
	{
		$arrWidth = array('Same as input', '1920x1080', '1680x1050', '1280x720',
				'1024x576', '850x480', '720x576', '720x540', '720x480', '720x404',
				'704x576', '640x480', '640x360', '608x448', '544x480', '480x480',
				'480x384', '480x360', '480x320', '480x272', '480x270', '400x320',
				'400x224', '352x480', '352x228', '320x256', '320x240', '320x180',
				'240x180', '176x144');

		$htmlOutput  = '<LABEL for="VideosizeSle"> Resolution:</LABEL>';
		$htmlOutput .= '<SELECT id="VideosizeSle" NAME="VidResO_1">';

		$isSelected = false;
		foreach ($arrWidth  as $resolution)
		{
			$htmlOutput .= '<OPTION value="' . $resolution . '"';

			if ($isSelected == false)
			{
				if ($this->VidResO_1 == $this->VidResI_1 and $resolution == 'Same as input')
				{
					$htmlOutput .= ' selected';
					$isSelected = true;
				}

				if ($this->VidResO_1 == $resolution) 
				{
					$htmlOutput .= ' selected';
					$isSelected = true;				
				}
			}

			$htmlOutput .= '>';

			$htmlOutput .= $resolution;

			$htmlOutput .= '</OPTION>';
		}


		$htmlOutput .= '</SELECT>';
		return ($htmlOutput);


	}

	public function GetVideoSize($venc_width_height_same_as_input, $width, $height, $des_width, $des_height) 
    {
		$arrWidthTxt = "";
		$IsSlected = 0;
		$result = '<LABEL for="VideosizeSle"> Resolution:</LABEL><select id="VideosizeSle" name="select_big">';
		$width = (integer)$width;
		$arrWidth = array('0x0', '1920x1080', '1680x1050', '1280x720',
				'1024x576', '850x480', '720x576', '720x540', '720x480', '720x404',
				'704x576', '640x480', '640x360', '608x448', '544x480', '480x480',
				'480x384', '480x360', '480x320', '480x272', '480x270', '400x320',
				'400x224', '352x480', '352x228', '320x256', '320x240', '320x180',
				'240x180', '176x144');

		$j = 0;
		if ($venc_width_height_same_as_input == '1') 
        {
			$arrWidthTxt = "Same as input";
			$result .= '<option value="' . $arrWidth[0] . '" selected ="selected">'
					. $arrWidthTxt . '</option>';
			$IsSlected = 1;
			$j = 1;
		}

		for ($i = $j; $i < count($arrWidth); $i++) {
			if ($arrWidth[$i] == "0x0") {
				$arrWidthTxt = "Same as input";
			} else {
				$arrWidthTxt = $arrWidth[$i];
			}

			if ($IsSlected == 1) 
            {
				$result .= '<option value="' . $arrWidth[$i] . '">' . $arrWidthTxt . '</option>';
			} 
            else 
            {
				$str_before = explode('x', $arrWidth[$i])[0]; //$str_before = $arrWidth[$i].split('x')[0];
				$str_before = (integer)$str_before;
				$str_after = explode('x', $arrWidth[$i])[1]; //var str_after = arrWidth[i].split('x')[1];
				$str_after = (integer)$str_after;

				if ($width >= $str_before) {
					if (($des_width == $str_before) and ($des_height == $str_after)) {
						$result .= '<option value="' . $arrWidth[$i]
								. '" selected ="selected">' . $arrWidthTxt
								. '</option>';
						$IsSlected = 1;
					} else {
						$result .= '<option value="' + $arrWidth[$i] + '">'
								. $arrWidthTxt + '</option>';
					}
				}
			}
		}
	
		$result .= '</select>';
		return $result;
	}


	public function print_stuff()
	{
		echo "pName" . " -> " . $this->pName . "<br />";
		echo "isTS_1" . " -> " . $this->isTS_1 . "<br />";
		echo "mac" . " -> " . $this->get('mac') . "<br />";
		echo "Encoding_type" . " -> " . $this->get('Encoding_type') . "<br />";
		echo "Encoding_level" . " -> " . $this->get('Encoding_level') . "<br />";
	}
}

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
    	$jsonCmdMessageCRC = GlobalFunctions::string_addPadding(sprintf("%x", crc32($jsonCmdMessage)), true, '0', 8); 
    
    	$tag = MUXProtocol::TAG_FOR_CMD;
    	$length = GlobalFunctions::string_dec2HexStr( strlen($jsonCmdMessage) + 4, 4);
    	$value =  GlobalFunctions::string_str2Hex($jsonCmdMessage) .$jsonCmdMessageCRC;
    
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

		//if (strcasecmp($cmdTag, MUXProtocol::TAG_FOR_RESP) !== 0)
			//continue; //PHP5 - Fix Later
		if (strcasecmp($cmdTag, MUXProtocol::TAG_FOR_RESP) !== 0) return false;
					
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

		//if ($cmdLength != $actualDataLength)
			//continue; //PHP5 - Fix Later
		//if ($cmdLength != $actualDataLength) return false;
 
		//==============================================
		// STEP #3 : Check CRC od the data	  
		$jsonCmdCrcInHex = substr($cmdValue, -8 ); // To get the last 4 bytes corresponding to the CRC
		
		$jsonData = GlobalFunctions::string_hex2str(substr($cmdValue, 0, $actualDataLength*2 - 8));
		
		$actualJsonCmdCrcInHex = GlobalFunctions::string_addPadding(sprintf("%x", crc32($jsonData)), true, '0', 8);
		
		//if (strcasecmp($jsonCmdCrcInHex, $actualJsonCmdCrcInHex) !== 0)
			//continue; //PHP5 - Fix Later
		if (strcasecmp($jsonCmdCrcInHex, $actualJsonCmdCrcInHex) != 0) return false;		
		
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
		$arrValues = array("targ"=>'',"cmd"=>"set_cmd","login_ack"=>$this->username,"pwd_msg"=>$this->username,"data"=>array($arrNewValues));

		$jsonNewValues = json_encode($arrValues);

        return($this->send_UDP_command($jsonNewValues));
    }

	public function cmd_security_get_status()
	{		
		//{"targ":"","cmd":"security_check","login_ack":"","pwd_msg":"","data":[{"get_status":""}]}

		$arrNewValues = array();
		$arrNewValues['get_status'] = '1';        
		$arrValues = array("targ"=>'',"cmd"=>"security_check","login_ack"=>$this->username,"pwd_msg"=>$this->username,"data"=>array($arrNewValues));

		$jsonNewValues = json_encode($arrValues);

        return($this->send_UDP_command($jsonNewValues));		
	}

    public function cmd_reset()
    {
		//{"targ":"","cmd":"set_param","login_ack":"","pwd_msg":"","data":[{"isRTMP_1":"0","isRTMPph_1":"0","isDipOn":'1',"RTMPport_1":"36004","MCASTurl_1":"udp://@239.111.101.254:37110"}]}

		$arrNewValues = array();
		$arrNewValues['reset'] = '1';        
		$arrValues = array("targ"=>'',"cmd"=>"set_cmd","login_ack"=>$this->username,"pwd_msg"=>$this->username,"data"=>array($arrNewValues));

		$jsonNewValues = json_encode($arrValues);

        return($this->send_UDP_command($jsonNewValues));
    }

    public function cmd_reboot()
    {
		//$arrValues = array("targ"=>'',"cmd"=>"set_param","login_ack"=>(string)$this->username,"pwd_msg"=>(string)$this->username,"data"=>array('isReboot'=>1));

		$arrNewValues = array();
		$arrNewValues['reboot'] = '1';        
		$arrValues = array("targ"=>'',"cmd"=>"set_cmd","login_ack"=>$this->username,"pwd_msg"=>$this->username,"data"=>array($arrNewValues));

		$jsonNewValues = json_encode($arrValues);

        return($this->send_UDP_command($jsonNewValues));
    }

    public function cmd_set_param($arrNewValues)
    {
		//{"targ":"","cmd":"set_param","login_ack":"","pwd_msg":"","data":[{"isRTMP_1":"0","isRTMPph_1":"0","isDipOn":'1',"RTMPport_1":"36004","MCASTurl_1":"udp://@239.111.101.254:37110"}]}

		$arrNewParamValues = array();

		foreach ($arrNewValues as $name=>$value)
		{
			if ( in_array($name, MuxDevice::PARAM_PLAYER_PARAMETERS) )
			{
				;
			}
			else
			{
				$arrNewParamValues[$name] = $value;
			}
		}

		if (!empty($arrNewParamValues))
		{
			$arrValues = array("targ"=>'',"cmd"=>"set_param","login_ack"=>$this->username,"pwd_msg"=>$this->username,"data"=>array($arrNewParamValues));

			$jsonNewValues = json_encode($arrValues);

			return($this->send_UDP_command($jsonNewValues));
		}
		else
		{
			return true;
		}
    }

    public function cmd_get_param()
    {
        //$hexStringPacket = $this->text_to_hex_command('{"targ":"","cmd":"get_param","login_ack":"","pwd_msg":""}');

        $arrValues = array("targ"=>'',"cmd"=>"get_param","login_ack"=>(string)$this->username,"pwd_msg"=>(string)$this->password);
        $jsonNewValues = json_encode($arrValues);
		
        return($this->send_UDP_command($jsonNewValues));
	}
	
	public function cmd_get_param_player()
    {
        //$hexStringPacket = $this->text_to_hex_command('{"targ":"","cmd":"get_param_player","login_ack":"","pwd_msg":""}');

        $arrValues = array("targ"=>'',"cmd"=>"get_param_player","login_ack"=>(string)$this->username,"pwd_msg"=>(string)$this->password);
        $jsonNewValues = json_encode($arrValues);
		
        return($this->send_UDP_command($jsonNewValues,3601));
    }

	public function cmd_set_param_player($arrNewValues)
    {
		//{"targ":"","cmd":"set_param","login_ack":"","pwd_msg":"","data":[{"isRTMP_1":"0","isRTMPph_1":"0","isDipOn":'1',"RTMPport_1":"36004","MCASTurl_1":"udp://@239.111.101.254:37110"}]}

		$arrNewPlayerValues = array();

		foreach ($arrNewValues as $name=>$value)
		{
			if ( in_array($name, MuxDevice::PARAM_PLAYER_PARAMETERS) )
			{
				$arrNewPlayerValues[$name] = $value;
			}
			else
			{
				;
			}
		}

		if (!empty($arrNewPlayerValues))
		{
			$arrValues = array("targ"=>'',"cmd"=>"set_param_player","login_ack"=>$this->username,"pwd_msg"=>$this->username,"data"=>array($arrNewPlayerValues));

			$jsonNewValues = json_encode($arrValues);

			//sleep(1); 

			return($this->send_UDP_command($jsonNewValues,3601));
		}
		else
		{
			return true;
		}
    }


    public function send_UDP_command($textStringPacket, $destinationtPort = 3600)
    {
		Logger::getInstance()->lwrite('SENDING: ' .$textStringPacket);

		$hexStringPacket = $this->text_to_hex_command($textStringPacket);

        $originClientPort = 3700;        
		$destinationAddress = '127.0.0.1';		
        //$destinationtPort = 3600;
		$myServerIP = '127.0.0.1';

        $clientSocket = new UdpSocket($myServerIP, $originClientPort, $destinationAddress, $destinationtPort, "UDP", "SOCKET_CLIENT_STREAM");

        Logger::getInstance()->lwrite('SENDING: IP:'.$myServerIP.':'.$originClientPort.' TO:'.$destinationAddress.':'.$destinationtPort);

        $temp_var = $clientSocket->get_lastErrorMessage();
        if (!empty($temp_var) )
        {
            Logger::getInstance()->lwrite('DEBUG: Error client socket: ' .$clientSocket->get_lastErrorMessage());
            return null;
        }

        $data_HexString_toSend = $hexStringPacket;
        $clientSocket->send_unicast_packet($data_HexString_toSend, $destinationAddress);        
        //$clientSocket->close();

        Logger::getInstance()->lwrite('DEBUG: Opening listening port.');

		$buffer = $clientSocket->receive_unicast_packet();

		$clientSocket->close();
		
		return ($buffer);
		
    }

}

?>