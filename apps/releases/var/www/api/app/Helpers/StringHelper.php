<?php

namespace App\Helpers;

class StringHelper
{
    static function string_hex2str($hexString, $isIgnore00 = false)
    {
        $str = "";
        
        for($i=0;$i<strlen($hexString);$i+=2)
        {
            $currentByte = hexdec(substr($hexString,$i,2));
            
            if ( $currentByte != 0 )
                $str .= chr($currentByte);
                else if ( $isIgnore00 == false )
                    $str .= chr($currentByte);
        }
        
        return $str;
    }
    
    static function string_hex2str_withKeyValueSeparator($hexString, $decimalSeparator)
    {
        $outputDataArr = array();
        
        $str = "";
        
        for($i=0;$i<strlen($hexString);$i+=2)
        {
            $currentByte = hexdec(substr($hexString,$i,2));
            
            if ( $currentByte != $decimalSeparator )
                $str .= chr($currentByte);
                else
                {
                    if (!empty($str))
                        $outputDataArr[] = $str;
                        else
                            return $outputDataArr;
                            
                            $str = "";
                            
                }
        }
        
        return $outputDataArr;
    }
    
    static function string_str2Hex($string)
    {
        $hex='';
        
        for ($i=0; $i < strlen($string); $i++)
        {
            //$hex .= dechex(ord($string[$i]));
            $hex .= self::string_dec2HexStr(ord($string[$i]));
        }
        
        return $hex;
    }
    
    static function string_dec2HexStr($decimal, $minStringLength = 2)
    {
        if ($decimal<0)
            $decimal=$decimal+256;
            
            $hexString='';
            
            $hexString = dechex($decimal);
            
            if ( strlen($hexString) % 2)
            {
                $hexString = '0' .$hexString;
            }
            
            if (strlen($hexString) < $minStringLength)
            {
                $nbOfCharToAdd = $minStringLength - strlen($hexString);
                
                for ($i = 0; $i < $nbOfCharToAdd; $i++)
                {
                    $hexString = '0' .$hexString;
                }
            }
            
            return $hexString;
    }
    
    static function string_HexStrToBin($hexString, $minStringLength = 8)
    {
        $binString='';
        
        $binString = base_convert($hexString, 16, 2);
        
        if (strlen($binString) < $minStringLength)
        {
            $nbOfCharToAdd = $minStringLength - strlen($binString);
            
            for ($i = 0; $i < $nbOfCharToAdd; $i++)
            {
                $binString = '0' .$binString;
            }
        }
        
        return $binString;
    }
    
    static function string_addPadding($stringToPadd, $isPaddFront, $paddingCharacter, $minStringLength = 2)
    {
        $paddedString = $stringToPadd;
        
        if (strlen($stringToPadd) < $minStringLength)
        {
            $nbOfCharToAdd = $minStringLength - strlen($stringToPadd);
            
            for ($i = 0; $i < $nbOfCharToAdd; $i++)
            {
                if ($isPaddFront == true)
                    $paddedString = '0' .$paddedString;
                    else
                        $paddedString = $paddedString .'0';
            }
        }
        
        return $paddedString;
    }
    
    static function startsWith($haystack,$needle,$case=false)
    {
        if($case){
            return (strcmp(substr($haystack, 0, strlen($needle)),$needle)===0);
        }
        return (strcasecmp(substr($haystack, 0, strlen($needle)),$needle)===0);
    }
    
    static function isJson($text)
    {
        json_decode($text);
        return (json_last_error() == JSON_ERROR_NONE);
    }
}
