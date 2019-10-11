<?php
namespace App\Helpers;

class ArrayHelper
{
    // remove initial and final spaces from the array keys  
    static function trimArrayKey($arr)
    {
        $result = array_combine(
            array_map(function ($str) {
                return trim($str);
            }, array_keys($arr)),
            array_values($arr)
            );
        return $result;
    }
}