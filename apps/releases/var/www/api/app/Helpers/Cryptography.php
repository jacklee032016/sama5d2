<?php

namespace App\Helpers;

class Cryptography
{
    static function generateToken()
    {
        //basic token, 500769 php needs more modules to run better functions as openssl_random_pseudo_bytes
        return md5(uniqid(mt_rand() . microtime(true) * 10000, true));
    }
}