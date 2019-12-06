<?php 

namespace App\Models;

use App\Base\Logger;
use App\Params;

class Settings
{
    private $settings_file = Params::SETTINGS_FILE;
    private $deviceType    = Params::DEFAULT_DEVICE_TYPE;
    
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
        $data = json_encode(array ("deviceType" => $this->deviceType));
        
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
    
    public function getDeviceType()
    {
        return $this->deviceType;
    }
    
    public function setDeviceType($deviceType)
    {
        if ($deviceType == 'TX' || $deviceType == 'RX')
        {
            $this->deviceType = $deviceType;
            $this->fileWrite();
        } else {
            Logger::getInstance()->addError('SETTINGS: device type must be RX or TX');
        }
    }
    
    public function setDeviceTypeByGetParam($data)
    {
        if (isset($data['system']['isTx']))
        {
            if ($data['system']['isTx'])
            {
                if ($this->deviceType != "TX")
                    $this->setDeviceType("TX");
            } else {
                if ($this->deviceType != "RX")
                    $this->setDeviceType("RX");
            }
        } else {
            Logger::getInstance()->addError('SETTINGS: isTx system param not set');
        }
    }
}
