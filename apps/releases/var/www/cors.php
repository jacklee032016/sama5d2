<?php
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS, DELETE, PUT');
header('Access-Control-Allow-Headers: Content-Type, Accept, Origin, Authorization');
header('Content-Type: application/octet-stream');
header('Content-Type: application/download');

if (preg_match("/^\/usb|\/sd/",$_SERVER["REQUEST_URI"]))
    $filePath = "/media{$_SERVER["REQUEST_URI"]}";
else if (strpos($_SERVER["REQUEST_URI"],"LogsArchive_") !== false)
    $filePath = "/Web{$_SERVER["REQUEST_URI"]}";
else {
    http_response_code(404);
    die();
}
            
$chunkSize = 1024 * 1024;
$fd = fopen(urldecode($filePath), 'rb');
            
if ($fd) {
    while (!feof($fd)) {
        $buffer = fread($fd, $chunkSize);
        echo $buffer;
        ob_flush();
        flush();
    }
               
    fclose($fd);
} else {
    http_response_code(404);
    die();
}
            
return true;

