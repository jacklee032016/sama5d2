<?php

namespace App\Base;

//use App\Helpers\StringHelper;
use App\Params;
use App\Helpers\Compress;

class LogManager
{
    const COMPRESSED_FILE_NAME = "LogsArchive_";
    const MAX_NUMBER_LOG_FILES = 3;
    const LOG_MAX_SIZE         = 10240000; //(10MB) in bytes 
    
    public function backup()
    {
        $compressedFileName    = self::COMPRESSED_FILE_NAME .date('Ymd-His') .'.tar.gz';
        $dir                   = str_replace("/api/app/Base", "", __DIR__);
        $compressedFilePath    = $dir . '/' . $compressedFileName;
        $logFilesName          = $dir.'/'.Params::DEFAULT_LOG_NAME."*";
        
        $this->removeOldCompress($dir);
        
        $filesPathToCompress = array($logFilesName, Params::DEFAULT_LOG_FW);
      
        if ( Compress::compressUnixLogFiles($filesPathToCompress, $compressedFilePath))
        {
            return $compressedFileName;
        } else {
            return false;
        }
    }
    
    public function backup_old()
    {
        $filesName_to_compress = array();
        $filesPath_to_compress = array();
        $dir                   = str_replace("/api/app/Base", "", __DIR__);
        $achiveFileName        = self::COMPRESSED_FILE_NAME .date('Ymd-His') .'.gz';
        $achivePathFileName    = $dir . '/' . $achiveFileName;
        
        $this->reduceFiles($dir, Params::DEFAULT_LOG_NAME);
        foreach(scandir($dir) as $file)
        {
            if ('.' === $file || '..' === $file)
                continue;
                
                if (StringHelper::startsWith($file, Params::DEFAULT_LOG_NAME))
                {
                    $filesName_to_compress[] = $file;
                    $filesPath_to_compress[] = "$dir/$file";
                }
                else
                {
                    continue;
                }
        }
        $this->removeOldCompress($dir);
        
        if ( Compress::compressLogFiles($filesPath_to_compress, $filesName_to_compress, $achivePathFileName))
        {
            return $achiveFileName;
        } else {
            return false;
        }
    }
    
    /*
     * Delete all logs files.
     */
    public function deleteLogs($dir)
    {
        //remove all compressed files
        $filePattern = $dir."/".self::COMPRESSED_FILE_NAME."*.*";
        array_map('unlink', glob($filePattern));
        //remove all log files
        $filePattern = $dir."/".Params::DEFAULT_LOG_NAME."*.*";
        array_map('unlink', glob($filePattern));
    }
    
    /*
     * Delete current log if size bigger than 10Mb.
     */
    public function limitCurrentLog($dir)
    {
        // define the current date (it will be appended to the log file name)
        $today = date('Y-m-d');
        $logFileName = $dir.'/'.Params::DEFAULT_LOG_NAME.$today .'.log';
        if (filesize($logFileName) > self::LOG_MAX_SIZE)
            unlink($logFileName);
    }
    /*
     * Every day a log file can be created.
     * This function reduces the number of log files to the most recent 3 files.
     */
    public function reduceFiles($dir, $pattern)
    {
        $filePattern = $dir."/".$pattern."*.*";
        $files       = glob($filePattern);
        
        if (count($files) > self::MAX_NUMBER_LOG_FILES)
        {
            // sort files by last modified date
            usort($files, function($x, $y) {
                return filemtime($x) < filemtime($y);
            });
            
            $toRemove = array_slice($files, self::MAX_NUMBER_LOG_FILES);
            
            array_map('unlink', $toRemove);
        }
    }
    
    protected function removeOldCompress($dir)
    {
        $filePattern = $dir."/".self::COMPRESSED_FILE_NAME."*.*";
        array_map('unlink', glob($filePattern));
    }
}
