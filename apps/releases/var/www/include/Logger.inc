<?php
/* Logger class:  
 * - contains lopen and lwrite methods  
 * - lwrite will write message to the log file  
 * - first call of the lwrite will open log file implicitly  
 * - message is written with the following format: hh:mm:ss (script name) message  
 */  
class Logger{   

	// define log file
	private $log_file = '/tmp/Muxlab500768TX_logfile';
	// define file pointer
	
	private $fp = null;
	// write message to the log file
	
	private $enabled = null;
	
	private static $instance;
	
	public function lwrite($message){
		if ($this->enabled == TRUE){
			// if file pointer doesn't exist, then open log file
			if (!$this->fp) $this->lopen();
			// define script name
			$script_name = pathinfo($_SERVER['PHP_SELF'], PATHINFO_FILENAME);
			// define current time
			$time = date('H:i:s');
			// write current time, script name and message to the log file
			fwrite($this->fp, "$time ($script_name) $message\n");
		}
	}

  private function __construct(){
  	$this->enabled = TRUE;
  }

  public function enableLogger() {
  	$this->enabled = TRUE;
  }
  
  public function disableLogger() {
  	$this->enabled = FALSE;
  }
  
  public static function getInstance() {
    	if (!isset(self::$instance)) {
    		//$c = __CLASS__;
    		//self::$instance = new $c;
    		self::$instance = new Logger();
    	}
    	return self::$instance;
    }
    
  // open log file   
  private function lopen(){   
    // define log file path and name   
    $lfile = $this->log_file;   
    // define the current date (it will be appended to the log file name)   
    $today = date('Y-m-d');   
    // open log file for writing only; place the file pointer at the end of the file   
    // if the file does not exist, attempt to create it   
    //$this->fp = fopen($lfile . '_' . $today, 'a') or exit("Can't open $lfile!");   
    $this->fp = fopen($lfile . '_' . $today .'.log', 'a') or exit("Can't open $lfile!");
  }   
}
?>