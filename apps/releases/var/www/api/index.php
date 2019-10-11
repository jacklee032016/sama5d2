<?php
error_reporting(0);
require "bootstrap/app.php";

header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS, DELETE, PUT');
header('Access-Control-Allow-Headers: Content-Type, Accept, Origin, Authorization');

$app->run();
