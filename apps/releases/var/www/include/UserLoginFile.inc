<?php 

class UserLoginFile
{
    private $file_folder = FILE_FOLDER;
    private $login_file = LOGIN_FILE;
    private $settings_file = SETTINGS_FILE;

    //settings            
    public $username = DEFAULT_USERNAME;
    public $password = DEFAULT_PASSWORD;
    public $json_username = JSON_USERNAME;
    public $json_password = JSON_PASSWORD;
    public $dns1 = DEFAULT_DNS1;
    public $dns2 = DEFAULT_DNS2;
        
    private $authenticated = FALSE;
    
    private $file;
    
    function __construct()
    {
        //create login folder
        if (!file_exists(dirname($this->login_file)))
        {
            if (!mkdir(dirname($this->login_file), 0777, TRUE))
            {
                Logger::getInstance()->lwrite('USERLOGIN: Folder cannot be made '.dirname($this->login_file) );                    
            }
        }

        //create settings folder and file
        if (!file_exists($this->settings_file)) 
        {
            if (!file_exists(dirname($this->settings_file)))
            {
                if (!mkdir(dirname($this->settings_file), 0777, TRUE))
                {
                    Logger::getInstance()->lwrite('USERLOGIN: Folder cannot be made '.dirname($this->settings_file) );
                }
                else
                {
                    $this->fileWriteNewFile();
                }
            }
            else
            {
                $this->fileWriteNewFile();
            }
        }
        else 
        {
            $this->fileRead();
        }   
    }

	public function get($parameter)
	{
		switch($parameter)
		{
			default:
				return($this->{$parameter});
		}		
	}    

	private function set($parameter,$value)
	{
		switch($parameter)
		{
			default:
                $this->{$parameter} = $value;
				return(TRUE);
		}		
	}

    public function set_DNS($dns1,$dns2)
    {
        $this->set('dns1',$dns1);
        $this->set('dns2',$dns2);
    }

    
    public function fileRead()
    {
        if (!$this->file = fopen($this->settings_file, "r"))
        {
            Logger::getInstance()->lwrite('USERLOGIN: File cannot be opened/created' );
            return FALSE;
        }
        
        if (filesize($this->settings_file) == 0)        
        {
            Logger::getInstance()->lwrite('USERLOGIN: File is empty' );
            return FALSE;
        }


        $json = fread($this->file, filesize($this->settings_file));
        
        $txt = json_decode($json, true);

        foreach ($txt as $parameter=>$value)
        {
            if (isset($this->{$parameter}))
            {
                $this->{$parameter} = $value;
            }
        }

        fclose($this->file);
               
        return TRUE;
    }
    
    public function fileWriteNewFile()
    {
        //$this->deleteFile();
        $this->fileWrite();
    }

    public function fileWrite()
    {
        Logger::getInstance()->lwrite('USERLOGIN: Writing settings file' );

        if (!$this->file = fopen($this->settings_file, "w"))
        {
            Logger::getInstance()->lwrite('USERLOGIN: File cannot be opened/created' );
            return(FALSE);
        }
        else
        {
            $txt = array (  
                "username"          => $this->username,
                "password"          => $this->password,
                "json_username"     => $this->json_username,
                "json_password"     => $this->json_password,                
                "dns1"              => $this->dns1,
                "dns2"              => $this->dns2);             
            
            $json = json_encode($txt);
            
            fwrite($this->file, $json);         
            fclose($this->file);
            return(TRUE);
        }   
    }
    
    public function userChangePassword($old_password, $new_password)
    {
        if ($this->password == $old_password)
        {
            $this->password = $new_password;
            return($this->fileWrite());
        }
        else
        {
            return (FALSE);
        }        
    }

    public function fileLogin($username=null, $password=null)
    {

        Logger::getInstance()->lwrite("USERLOGIN: $this->username == $username and $this->password = $password" );

        if ($this->username == $username and $this->password == $password)
        {
            $this->Authenticate(TRUE);
        }
        else
        {            
            $this->Authenticate(FALSE);
        }
    }
     
    public function Authenticate($flag = FALSE)
    {
        if ($flag === TRUE) 
        {
            if (!touch($this->login_file))
            {
                Logger::getInstance()->lwrite("USERLOGIN: Error touching ".$this->login_file );
            }
            else
            {
                if (!$this->file = fopen($this->login_file, "w"))
                {
                    Logger::getInstance()->lwrite('USERLOGIN: File cannot be opened/created' );
                    $this->authenticated = FALSE;
                }
                else
                {
                    $txt = array (  
                        "ip_address" => GlobalFunctions::getRealIpAddr()
                    );             
                    
                    $json = json_encode($txt);
                    
                    fwrite($this->file, $json);         
                    fclose($this->file);
                    $this->authenticated = TRUE;
                }                
            }
        }
        else 
        {
            Logger::getInstance()->lwrite("USERLOGIN: Bad login" );
            if (file_exists($this->login_file)) unlink($this->login_file);
            $this->authenticated = FALSE;
        }
    }
    
    public function isAuthenticated()
    {
        $this->authenticated = FALSE;

        if (file_exists($this->login_file)) 
        {
            if (filesize($this->login_file) == 0)
            {
                Logger::getInstance()->lwrite('USERLOGIN: Login file is empty' );
            }
            else
            {
                if (!$this->file = fopen($this->login_file, "r"))
                {
                    Logger::getInstance()->lwrite('USERLOGIN: File cannot be opened/created' );
                }
                else
                {
                    $json = fread($this->file, filesize($this->login_file));            
                    $txt = json_decode($json, true);
                    fclose($this->file);

                    if ($txt['ip_address'] == GlobalFunctions::getRealIpAddr()) 
                    {
                            Logger::getInstance()->lwrite('USERLOGIN: Logged in' );
                            $this->authenticated = TRUE;
                    }
                    else 
                    {
                        Logger::getInstance()->lwrite('USERLOGIN: IP address does not match' );
                    }
                }
            }
        }

        return $this->authenticated;        
    }


    public function LogOut()
    {
        $this->Authenticate(FALSE);
    }
    
    public function close()
    {        
        //unset($this); //PHP5 - Fix Later        
    }
    
    public function deleteFile()
    {
        if (unlink($this->settings_file)) return TRUE;
        else return FALSE;
    }
    
}

?>