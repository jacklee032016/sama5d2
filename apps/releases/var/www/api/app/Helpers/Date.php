<?php

namespace App\Helpers;

class Date
{
    static function setSystemDate($year, $month, $day, $hour, $minute, $second)
    {
        if (!isset($second))
            $second = '00';
        
        $date = "$year-$month-$day $hour:$minute:$second";
        
        exec("date -s '".$date."'");
        
        return self::getSystemDate();
    }
    
    static function getSystemDate()
    {
        $date = array(
            "year"   => exec('date +"%Y"'),
            "month"  => exec('date +"%m"'),
            "day"    => exec('date +"%d"'),
            "hour"   => exec('date +"%H"'),
            "minute" => exec('date +"%M"'),
            "second" => exec('date +"%S"')
        );

        return $date;
    }
    
    //adjusts the time format HH:mm to HH:mm:ss if necessary (old version)
    static function timeFormatAdapt($time)
    {
        $colonNumber = substr_count($time, ':');
        if ($colonNumber == 1)
        {
            $time .= ':00';
        }
        return $time;
    }
    
    /*
     * change str time format H:i:s to seconds
     */
    static function strTimeToSeconds($time)
    {
        sscanf($time, "%d:%d:%d", $hours, $minutes, $seconds);
        return $hours * 3600 + $minutes * 60 + $seconds;
    }
    /*
     * change seconds to str time format H:i:s
     */
    static function secondsToStrTime($seconds)
    {
        return sprintf('%02d:%02d:%02d', floor($seconds/3600),($seconds/60)%60, $seconds%60);
    }
}   
