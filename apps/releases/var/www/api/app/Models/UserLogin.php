<?php 

namespace App\Models;

use App\Helpers\Cryptography;
use App\Base\Logger;
use App\Params;

/**
 * @SWG\Definition(required={"username", "password"}, type="object", @SWG\Xml(name="UserLogin"))
 */
class UserLogin
{
    /**
     * @SWG\Property(example="admin")
     * @var string
     */
    private $username        = Params::DEFAULT_USERNAME;
    
    /**
     * @SWG\Property(example="admin")
     * @var string
     */
    private $password        = Params::DEFAULT_PASSWORD;
    private $user_file       = Params::USER_FILE;
    private $token           = "";
    private $tokenExpiration = "";
    
    public function __construct()
    {
        //create user folder and file
        if (!file_exists($this->user_file))
        {
            $this->token           = Cryptography::generateToken();
            $this->tokenExpiration = strtotime(Params::TOKEN_EXPIRATION_TO_ONE_HOUR);

            if (!file_exists(dirname($this->user_file)))
            {
                if (!mkdir(dirname($this->user_file), 0777, TRUE))
                {
                    Logger::getInstance()->addError('USERLOGIN: Folder cannot be made '.dirname($this->user_file) );
                    throw new \Exception('USERLOGIN: Folder cannot be made '.dirname($this->user_file));
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
//        Logger::getInstance()->lwrite('USERLOGIN: Writing settings file');
        Logger::getInstance()->addInfo('USERLOGIN: Writing settings file');
        
        if (!$this->file = fopen($this->user_file, "w"))
        {
            Logger::getInstance()->addError('USERLOGIN: File cannot be opened/created' );
            return(FALSE);
        }
        else
        {
            $txt = array (
                    "username"        => $this->username,
                    "password"        => $this->password,
                    "token"           => $this->token,
                    "tokenExpiration" => $this->tokenExpiration
            );
            
            $json = json_encode($txt);
            
            fwrite($this->file, $json);
            fclose($this->file);
            return(TRUE);
        }
    }
    
    protected function fileRead()
    {
        $content = file_get_contents($this->user_file);
        if (!$content)
        {
            Logger::getInstance()->addError('USERLOGIN: File cannot be opened' );
            return false;
        }
        $data = json_decode($content, true);

        foreach ($data as $parameter=>$value)
        {
            if (isset($this->{$parameter}))
            {
                $this->{$parameter} = $value;
            }
        }
        
        return TRUE;
    }
    
    public function login($username, $password)
    {
        //Logger::getInstance()->lwrite("USERLOGIN: $this->username == $username and $this->password = $password" );
        Logger::getInstance()->addInfo("USERLOGIN: $this->username == $username and $this->password = $password");
        
        if ($this->username == $username and $this->password == $password)
        {
            return true;
        }
        
        return false;
    }

    public function getPassword()
    {
        return $this->password;
    }
    
    public function getToken()
    {
        if (!$this->token)
        {
            $this->token = Cryptography::generateToken();
            $this->fileWrite();
        }
            
        return $this->token;
    }
    
    public function setPassword($password)
    {
        $this->password        = $password;
        $this->token           = Cryptography::generateToken(); //generate new token
        $this->tokenExpiration = strtotime(Params::TOKEN_EXPIRATION_TO_ONE_HOUR); //reset expiration time

        return($this->fileWrite());
    }
    
    public function validateToken($token)
    {
        if (!empty($token) && $this->token == $token[0])
        {
            //check if expired
            if (time() < $this->tokenExpiration)
            {
                //reset expiration time
                $this->tokenExpiration = strtotime(Params::TOKEN_EXPIRATION_TO_ONE_HOUR);
                $this->fileWrite();
                return true;
            }
        }
        return false;
    }
    
    //used in logout - token will be expired need new login
    public function setTokenExpiration($tringTime)
    {
        $this->tokenExpiration = strtotime($tringTime);
        $this->fileWrite();
    }
}
