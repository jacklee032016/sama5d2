<?php

namespace App\Base;

class MuxDevice
{
    const PROTECTED_PARAMETERS = array('pName','model','fwVer','cName','mac','dns1','dns2');
    const RS232_LIST = array(
        "baudrate"  => array("9600","19200","38400","57600","115200"),
        "databits"  => array("7","8"),
        "parity"    => array("none","odd","even"),
        "stopbits"  => array("1","2"),
    );
    const VIDEO_LIST = array(
        "resolution"  => array("1920x1080", "1280x720"),
        "colorDepth" => array("8","10","12","16"),
        "colorSpace" => array("YCbCr-4:2:2","YCbCr-4:4:4","RGB"),
        "intlce"    => array("1","3"),
    );
    const AUDIO_LIST = array(
        "audioChs"  => array("4","8","12","16"),
        "audioRate" => array("48000","44100","96000"),
        "audioPKT"  => array("1ms","125us")
    );
    
    public $system; //array(model,product,name,ver,build,isTx,ip,mask,gateway,mac,isDhcp,isdipon,reset,reboot,blinkled)
    public $video;  //array(ip,port,sdp,height,width,fps,colorspace,depth,payloadtype,intlce)
    public $audio;  //array(ip,port,sdp,sample,channels,depth,payloadtype,pktsize)
    public $anc;    //array(ip,port,sdp,payloadtype,vpid)
    public $sdp;    //array(media,ip,port,uri)[3]
    public $rs232;  //array(baudrate,databit,parity,stopbit,data,isfeedback,waittime)
    public $security; //array(get_id,get_status,get_key)
    public $others;  //array(authen,debugRest,debugCmd,debugsdp,sdpc array(name,PID)[6],threads array(uri,reqs,pkts,fails,headErrs,dataErrs,msg)[3]) 
    
    function __construct($arrParameters)
    {
        if ($arrParameters == null)
        {
            return FALSE;
        }
        
        foreach ($arrParameters as $name=>$value)
        {
            $this->{$name} = $value;
        }
    }
    
    public function get($parameter)
    {
        $this->{$parameter};
    }
    
/*    public function save_changes($arrPost)
    {
        $arrNewValues = array();
        
        foreach ($arrPost as $parameter=>$newValue)
        {
            if ($parameter == 'Encoding_type')
            {
                $this->Encoding_type = $newValue;
                if ($this->Encoding_type == 'H.264') $arrNewValues['VidCodec_1'] = 'H.264-HighProfile';
                else $arrNewValues['VidCodec_1'] = 'H.265-HighProfile';
            }
            if ($parameter == '$Encoding_level')
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
                        if ($newValue == "100") $newValue = "100";
                        break;
                    case 'TSurl_1':
                        $arrNewValues['isTS_1'] = '1';
                        break;
                    case 'HLSurl_1':
                        $arrNewValues['isHLS_1'] = '1';
                        break;
                    case 'FLVurl_1':
                        $arrNewValues['isFLV_1'] = '1';
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
                        $arrNewValues['isRTSP_1'] = '1';
                        break;
                    case 'RTMPurl_1':
                        $arrNewValues['isRTMP_1'] = '1';
                        break;
                    case 'MCASTurl_1':
                        $arrNewValues['isMCAST_1'] = '1';
                        break;
                    case 'AudBitR_1':
                        $arrNewValues['AudCodec_1'] = (isset($_POST['AudCodec_1']))? $_POST['AudCodec_1'] : $this->AudCodec_1;
                        //$arrNewValues['isAudin'] = (isset($_POST['isAudin']))? $_POST['isAudin'] : $this->isAudin;
                        break;
                    case 'AudCodec_1':
                        $arrNewValues['AudBitR_1'] = (isset($_POST['AudBitR_1']))? $_POST['AudBitR_1'] : $this->AudBitR_1;
                        //$arrNewValues['isAudin'] = (isset($_POST['isAudin']))? $_POST['isAudin'] : $this->isAudin;
                        break;
                        //case 'isAudin':
                        //$arrNewValues['AudBitR_1'] = (isset($_POST['AudBitR_1']))? $_POST['AudBitR_1'] : $this->AudBitR_1;
                        //$arrNewValues['AudCodec_1'] = (isset($_POST['AudCodec_1']))? $_POST['AudCodec_1'] : $this->AudCodec_1;
                        //break;
                    case 'isDhcp':
                        $arrNewValues['ip'] = (isset($_POST['ip']))? $_POST['ip'] : $this->ip;
                        $arrNewValues['mask'] = (isset($_POST['mask']))? $_POST['mask'] : $this->mask;
                        $arrNewValues['gateway'] = (isset($_POST['gateway']))? $_POST['gateway'] : $this->gateway;
                        break;
                    default:
                        break;
                }
                
                $arrNewValues[$parameter] = $newValue;
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
            return(array());
        }
    }
*/    
    
    
    
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

}