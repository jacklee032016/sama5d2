<?php
date_default_timezone_set('America/New_York');
set_include_path('./include/');

define('DEVICE_NAME', 'TX-500768'); // 'TX-500768' or 'RX-500768'
define('IS_TEST_SERVER', FALSE);
define('DEBUG_LOG', TRUE);
define('WEB_VERSION', 'last update: 2018-05-24');

define('YES', 1);
define('NO', 0);
define('Yes', 1);
define('No', 0);
define('NOT_SET', -1);
define('NOT_SET_NULL', null);

define('IS_LOCAL_SERVER', TRUE);

define('SERVER_HOST', $_SERVER['HTTP_HOST']);
//define('DOC_ROOT', preg_replace("!${_SERVER['SCRIPT_NAME']}$!", '', $_SERVER['SCRIPT_FILENAME']));
//define('FILE_FOLDER', pathinfo(DOC_ROOT)['dirname'] . '/tmp/files/');
define('FILE_FOLDER', '/tmp/muxlab/');

define('DEFAULT_USERNAME', 'admin');
define('DEFAULT_PASSWORD', 'admin');
define('DEFAULT_DNS1', '8.8.8.8');
define('DEFAULT_DNS2', '4.2.2.1');

// ============ INITIALIZATION and Global variable ====================

if (DEBUG_LOG) Logger::getInstance()->enableLogger();
else Logger::getInstance()->disableLogger();

define('IS_SECURITY_CHECK', NO);

if (IS_TEST_SERVER)
{
    define('SETTINGS_FILE', './xml/settings.file');
    define('LOGIN_FILE', './xml/login.file');
    define('PARAMETERS_FILE', './xml/parameters.file');
    define('FIRMWARE_FILE','./xml/upgrade.file');
    define('FPGA_LOG','/xml/fpga.log');    
}
elseif (DEVICE_NAME == "TX-500768")
{
    define('MUXPROTOCOL_FILE', './include/MUXProtocol_TX.inc');
    define('SETTINGS_FILE', FILE_FOLDER . 'settings.file');
    define('LOGIN_FILE', FILE_FOLDER . 'login.file');
    define('PARAMETERS_FILE', FILE_FOLDER . 'parameters.file');
    //define('CONFIG_FILE', '/tmp/muxlab/config.ini');
    define('FIRMWARE_FILE','/tmp/upgrade');
    define('FPGA_LOG','/tmp/log/fpga.log');    
}
elseif (DEVICE_NAME == "RX-500768")
{
    define('MUXPROTOCOL_FILE', './include/MUXProtocol_RX.inc');
    define('SETTINGS_FILE', FILE_FOLDER . 'settings.file');
    define('LOGIN_FILE', FILE_FOLDER . 'login.file');
    define('PARAMETERS_FILE', FILE_FOLDER . 'parameters.file');
    //define('CONFIG_FILE', '/tmp/muxlab/config.ini');
    define('FIRMWARE_FILE','/ftp/upgrade');    
    define('FPGA_LOG','/tmp/log/fpga.log'); 
}
else
{
    define('SETTINGS_FILE', FILE_FOLDER . 'settings.file');
    define('LOGIN_FILE', FILE_FOLDER . 'login.file');
    define('PARAMETERS_FILE', FILE_FOLDER . 'parameters.file');
    define('FIRMWARE_FILE','./xml/upgrade.file');
    define('FPGA_LOG','/xml/fpga.log');
}

//check config file for json username and password
$json_username = '';
$json_password = '';
$hardware_version = '2.0.0';
if (defined('CONFIG_FILE'))
{
    if (file_exists(CONFIG_FILE)) 
    {
        if ($file = fopen(CONFIG_FILE, "r"))
        {
            $json = fread($file, filesize(CONFIG_FILE));
            $txt = json_decode($json, true);

            $json_username = $txt['data'][0]['login'];
            $json_password = $txt['data'][0]['password'];
            $hardware_version = $txt['data'][0]['fwVer'];
        }
    }
}
define('JSON_USERNAME', $json_username);
define('JSON_PASSWORD', $json_password);
define('HARDWARE_VERSION', $hardware_version);

?>