<?php 

namespace App\Models;

use App\Base\Logger;
use App\Params;
use App\Helpers\TZList;

class Settings
{
    private $settings_file   = Params::SETTINGS_FILE;
    private $dns1            = Params::DEFAULT_DNS1;
    private $dns2            = Params::DEFAULT_DNS2;
    private $tz              = Params::DEFAULT_TZ;
    private $ntp             = Params::DEFAULT_NTP;
//    private $resolution      = Params::DEFAULT_RESOLUTION;
//    private $colorDepth      = Params::DEFAULT_COLOR_DEPTH;
//    private $colorSpace      = Params::DEFAULT_COLOR_SPACE;
    private $resolutionMode  = Params::DEFAULT_RESOLUTION_MODE;
    
    public function __construct()
    {
        //create user folder and file
        if (!file_exists($this->settings_file))
        {
            if (!file_exists(dirname($this->settings_file)))
            {
                if (!mkdir(dirname($this->settings_file), 0777, TRUE))
                {
                    Logger::getInstance()->addError('SETTINGS: Folder cannot be made '.dirname($this->settings_file));
                }
                else
                {
                    $this->fileWrite();
                }
            }
            else
            {
                $this->fileWrite();
            }
        }
        else
        {
            $this->fileRead();
        }
    }
    
    protected function fileWrite()
    {
        $data = json_encode(array (
            "dns1"            => $this->dns1,
            "dns2"            => $this->dns2,
            "tz"              => $this->tz,
            "ntp"             => $this->ntp,
//            "resolution"      => $this->resolution,
//            "colorDepth"      => $this->colorDepth,
//            "colorSpace"      => $this->colorSpace,
            "resolutionMode"  => $this->resolutionMode,
        ));
        
        $nBytes = file_put_contents($this->settings_file, $data);
        if (!$nBytes)
            Logger::getInstance()->addError('Could not write on file '.$this->settings_file.'. Default values used');
    }
    
    protected function fileRead()
    {
        $json = file_get_contents($this->settings_file);
        if ($json)
        {
            $txt = json_decode($json, true);
            foreach ($txt as $parameter=>$value)
            {
                if (isset($this->{$parameter}))
                {
                    $this->{$parameter} = $value;
                }
            }
        } else {
            Logger::getInstance()->addError('Could not read file '.$this->settings_file.'. Default values used');
        }
    }
    
    public function getDns1()
    {
        return $this->dns1;
    }
    
    public function getDns2()
    {
        return $this->dns2;
    }
    
    public function getTZ()
    {
        $tzList   = TZList::getTZList();
        
        $location = $this->tz;
        $data = array("location" => $location, "tz_string" => $tzList[$location]);
        return $data;
    }
    
    public function getNtp()
    {
        return $this->ntp;
    }
    
    public function getResolutionMode()
    {
        return $this->resolutionMode;
    }
    
/*    public function getResolutionColors()
    {
        return array("resolution" => $this->resolution, "colorDepth" => $this->colorDepth, "colorSpace" => $this->colorSpace);
    }*/
    
    public function setResolutionMode($mode)
    {
        $this->resolutionMode = $mode;
        $this->fileWrite();
    }
    
    
    public function setNtp($ntp)
    {
        $this->ntp = $ntp;
        $this->fileWrite();
    }
    
    public function setTz($tz)
    {
        if (empty($tz))
        {
            Logger::getInstance()->addError('SETTINGS: time zone can not be empty');
            throw new \Exception('SETTINGS: time zone can not be empty');
        }
        
        $tzList = TZList::getTZList();
        if (!array_key_exists($tz, $tzList))
        {
            Logger::getInstance()->addError('SETTINGS: time zone does not exist');
            throw new \Exception('SETTINGS: time zone does not exist');
        }
        
        $this->tz = $tz;
        $this->fileWrite();
    }
    
    public function checkAndUpdateDns($dns1, $dns2)
    {
        if (empty($dns1) || empty($dns2))
        {
            Logger::getInstance()->addError('SETTINGS: dns can not be empty');
            throw new \Exception('SETTINGS: dns can not be empty');
        }

        if ($this->dns1 != $dns1 || $this->dns2 != $dns2)
        {
            $this->dns1 = $dns1;
            $this->dns2 = $dns2;
            $this->fileWrite();
            return true;
        }
        return false;
    }
}
