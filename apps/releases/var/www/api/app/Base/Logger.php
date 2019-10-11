<?php

namespace App\Base;

use App\Params;
use App\Models\LogLevel;

/* Logger class:  
 * - It's a wrapper for monolog logger
 */  
class Logger{

    private static $instance;
    
    private static $logLevel = "";
    
    private static $day = "";

    private static $enabled = false;
    
    public static function getLogLevelList()
    {
        return Params::$logLevels;
    }
    
    
    public static function getInstance()
    {
        if (!isset(self::$instance))
        {
            self::$instance = new Logger();
        }
        return self::$instance;
        
    }

    public function addDebug($message, array $context = array())
    {
        return "";
    }
    
    public function addInfo($message, array $context = array())
    {
        return "";
    }
    
    public function addNotice($message, array $context = array())
    {
        return "";
    }
    
    public function addWarning($message, array $context = array())
    {
        return "";
    }
    
    public function addError($message, array $context = array())
    {
        return "";
    }
    
    public function addCritical($message, array $context = array())
    {
        return "";
    }
    
    public function addAlert($message, array $context = array())
    {
        return "";
    }
    
    public function addEmergency($message, array $context = array())
    {
        return "";
    }

/*    public static function getInstance() 
    {
        try {
            $log = new LogLevel();
            $curLogLevel = $log->getLogLevel();
        } catch (\Exception $e) {  //if error, default log level used
            $curLogLevel = Params::DEFAULT_LOG_LEVEL;
            unlink(Params::LOG_LEVEL_FILE);
        }
        $curDay = date('j');
        
        if (!isset(self::$instance) || self::$day != $curDay || self::$logLevel != $curLogLevel)
        {
            self::$logLevel = $curLogLevel;
            self::$day = $curDay;

            $monologLevel = self::monologLevelWrapper($curLogLevel);
            
            // define log file
            $lfile = __DIR__ . '/../../../' . Params::DEFAULT_LOG_NAME;
            
            // define the current date (it will be appended to the log file name)
            $today = date('Y-m-d');
            
            // define log file path and name
            $lfile = $lfile . $today .'.log';
            
            $logger = new \Monolog\Logger('mux_logger');
            if (self::$enabled)
            {
                $fileHandler = new \Monolog\Handler\StreamHandler($lfile, $monologLevel);
                $logger->pushHandler($fileHandler);
                $logger->pushProcessor(new \Monolog\Processor\IntrospectionProcessor());
            }
            self::$instance = $logger;
        }
        return self::$instance;
    }*/
    
    private static function monologLevelWrapper($logLevel)
    {
        $level = \Monolog\Logger::DEBUG;
        switch ($logLevel) {
            case Params::LOG_LEVEL_DEBUG:
                $level = \Monolog\Logger::DEBUG;
            break;
            case Params::LOG_LEVEL_INFO:
                $level = \Monolog\Logger::INFO;
            break;
            case Params::LOG_LEVEL_NOTICE:
                $level = \Monolog\Logger::NOTICE;
            break;
            case Params::LOG_LEVEL_WARNING:
                $level = \Monolog\Logger::WARNING;
            break;
            case Params::LOG_LEVEL_ERROR:
                $level = \Monolog\Logger::ERROR;
            break;
            case Params::LOG_LEVEL_CRITICAL:
                $level = \Monolog\Logger::CRITICAL;
            break;
            case Params::LOG_LEVEL_ALERT:
                $level = \Monolog\Logger::ALERT;
            break;
            case Params::LOG_LEVEL_EMERGENCY:
                $level = \Monolog\Logger::EMERGENCY;
            break;
            default:
                $level = \Monolog\Logger::DEBUG;
            break;
        }
        return $level;
    }
}
?>