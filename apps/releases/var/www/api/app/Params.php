<?php

namespace App;

class Params
{
    const ENV            = 'PROD'; //PROD DEV
    const USER_FILE      = '/tmp/user.file';
    const SETTINGS_FILE  = '/tmp/settings.file';
    const LOG_LEVEL_FILE = '/tmp/log.file';
    const VERSION        = '1.0.0';
    
    const TOKEN_EXPIRATION_TO_ONE_HOUR = '+1 hour';//"+1 minute";
    const TOKEN_EXPIRATION_TO_NOW      = 'now';
    
    const DEVICE_CONTROLLER_USERNAME = "";
    const DEVICE_CONTROLLER_PASSWORD = "";
    const DEVICE_PLAYER_USERNAME     = "root";
    const DEVICE_PLAYER_PASSWORD     = "123";
    
    const MCU_UPGRADE_VALIDATION  = "MCU774";
    const FPGA_UPGRADE_VALIDATION = "FGA774";
    
    const DEFAULT_USERNAME = "admin";
    const DEFAULT_PASSWORD = "admin";
    const DEFAULT_WINDOW   = 0; //default playlist/file manager window
    const NUMBER_WINDOWS   = 6;
    
//    const DEFAULT_RESOLUTION       = "1080P_60";
//    const DEFAULT_COLOR_DEPTH      = "8";
//    const DEFAULT_COLOR_SPACE      = "RGB444";
    const DEFAULT_RESOLUTION_MODE  = "AUTO";
    
    const DEFAULT_DNS1      = "8.8.8.8";
    const DEFAULT_DNS2      = "4.2.2.1";
    const DEFAULT_TZ        = "America/New York";
    const DEFAULT_NTP       = "pool.ntp.org";
    const DEFAULT_LOG_NAME  = "Muxlab500769_logfile_";
    const DEFAULT_LOG_FW    = "/tmp/muxMedia.log";
    const DEFAULT_LOG_LEVEL = "ERROR";
    
    const LOG_LEVEL_DEBUG     = "DEBUG";
    const LOG_LEVEL_INFO      = "INFO";
    const LOG_LEVEL_NOTICE    = "NOTICE";
    const LOG_LEVEL_WARNING   = "WARNING";
    const LOG_LEVEL_ERROR     = "ERROR";
    const LOG_LEVEL_CRITICAL  = "CRITICAL";
    const LOG_LEVEL_ALERT     = "ALERT";
    const LOG_LEVEL_EMERGENCY = "EMERGENCY";
    
    const OPTION_ON  = "ON";
    const OPTION_OFF = "OFF";
    
    static $logLevels = ["DEBUG","INFO","NOTICE","WARNING", "ERROR", "CRITICAL", "ALERT", "EMERGENCY"];
    
    const FIRMWARE_MCU_FILE  = '/var/www/upgrade';
    const FIRMWARE_FPGA_FILE = '/tmp/upgradeFPGA';
    const DEVICE_NAME        = '500774'; //'RX-500762'
    const DEVICE_LAST_UPDATE = "June 17 2019";
    
    const BEANSTALKD_ADDRESS           = '127.0.0.1'; //DEFAULT PORT 11300
    const BEANSTALKD_SCHEDULE_TUBE     = 'queue';
    
    const PETER_CONTROLLER_UDP_PORT    = '3600';//'3700'; //PETER
    const MEDIA_CONTROLLER_UDP_PORT    = '3600'; //JACK - UDP
    const MEDIA_CONTROLLER_TCP_PORT    = '3600'; //JACK - TCP
    
    const UDP_SOCKET_TIMEOUT     = 2; // in seconds
    const TCP_SOCKET_TIMEOUT     = 2; // in seconds
    const TCP_SOCKET_BIG_TIMEOUT = 10; // in seconds - For listing all media files on USB disk or/and SD card
    
    const UDP_SOCKET_BUFFER_SIZE = 5000; // in bytes
    const TCP_SOCKET_BUFFER_SIZE = 15000; // in bytes
    
    const PROD_PROTOCOL_DESTINATION_ADDRESS = '127.0.0.1';
    const PROD_PROTOCOL_ORIGIN_ADDRESS      = '127.0.0.1';
    const PROD_PROTOCOL_ORIGIN_PORT         = 3601;
    
    const DEV_PROTOCOL_DESTINATION_ADDRESS_UDP = '192.168.168.100';//'192.168.168.100';//'192.168.168.103';// peter controller
    const DEV_PROTOCOL_DESTINATION_ADDRESS_TCP = '127.0.0.1';//'192.168.168.100';//'192.168.168.103';// jack controller
    const DEV_PROTOCOL_ORIGIN_ADDRESS          = '192.168.168.101';// my pc ip
}