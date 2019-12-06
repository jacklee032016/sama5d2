<?php

namespace App\Helpers;

use App\Base\Logger;

class Compress
{
    static function compressLogFiles($filesPath = array(), $filesName = array() , $destination = '',$overwrite = true)
    {
        //if the compress file already exists and overwrite is false, return false
        if(file_exists($destination) && !$overwrite) { return false; }
        
        //vars
        $valid_filesPath = array();
        $valid_filesName = array();
        
        //if files were passed in...
        if(is_array($filesPath))
        {
            //cycle through each file
            for ($i = 0; $i < count($filesPath); $i++)
            {
                //make sure the file exists
                if(file_exists($filesPath[$i]))
                {
                    $valid_filesPath[] = $filesPath[$i];
                    $valid_filesName[] = $filesName[$i];
                }
            }
            
        }
        
            
        $zp = gzopen( $destination, "w9" );
        
        //add the files
        for ($i = 0; $i < count($valid_filesPath); $i++)
        {
            $fp = fopen( $valid_filesPath[$i], "r" );
            $data = fread ( $fp, filesize( $valid_filesPath[$i]) );
            fclose( $fp );
            gzwrite( $zp, "\r\n\r\n*****************".$filesName[$i]."*******************\r\n\r\n");
            gzwrite( $zp, $data );
        }
        gzclose( $zp );

        return $destination;
    }

    static function compressUnixLogFiles($filesPath = array(), $destination = '', $overwrite = true)
    {
        //if the compress file already exists and overwrite is false, return false
        if(file_exists($destination) && !$overwrite) { return false; }

        //check if files exists
        $newFilesPath = array();
        foreach ($filesPath as $index => $path)
        {
            if (glob($path))
                $newFilesPath[] = $path;
        }
        
        $stringPathFiles = implode(" ", $newFilesPath);
        $command         = "tar -zcvf $destination $stringPathFiles";

        exec($command, $output, $returnVar);
        
        if ($returnVar)
        {
            Logger::getInstance()->addError('compress error: '. json_encode($output));
            return false;
        }
        return true;
    }
    
    /*
     * Compress files using php gz functions
     * 
     * @param  Array  $files            array of file path+name
     * @param  string $destination      destination path+name
     * @return string                   destination path
     */
    static function compressFiles($files, $destination = '', $overwrite = true)
    {
        //if the compress file already exists and overwrite is false, return false
        if(file_exists($destination) && !$overwrite) { return false; }
        
        $zp = gzopen( $destination, "w9" );
        
        //add the files
        foreach ($files as $file)
        {
            $fp = fopen( $file, "r" );
            if ($fp)
            {
                $data = fread ( $fp, filesize( $file) );
                fclose( $fp );
                gzwrite( $zp, $data );
            }
        }
        gzclose( $zp );
        
        return $destination;
    }
}
