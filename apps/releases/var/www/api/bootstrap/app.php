<?php

use Respect\Validation\Validator as v;

require __DIR__ . '/../vendor/autoload.php';

$config = [
    'settings' => [
        'displayErrorDetails' => false,
    ],
];

$app = new \Slim\App($config);

/*$app->options('/{routes:.+}', function ($request, $response, $args) {
    return $response;
});
    
$app->add(function ($req, $res, $next) {
    $response = $next($req, $res);
    return $response
    ->withHeader('Access-Control-Allow-Origin', '*')
    ->withHeader('Access-Control-Allow-Headers', 'Content-Type, Accept, Origin, Authorization')
    ->withHeader('Access-Control-Allow-Methods', 'POST, GET, OPTIONS, DELETE, PUT');
});*/

$container = $app->getContainer();

$container['HomeController'] = function ($container) {
    return new \App\Controllers\HomeController($container);
};

$container['Validator'] = function ($container) {
    return new App\Validation\Validator();
};

$container['AuthController'] = function($container) {
    return new \App\Controllers\AuthController($container);
};

$container['DeviceController'] = function($container) {
    return new \App\Controllers\DeviceController($container);
};

$container['Rs232Controller'] = function($container) {
    return new \App\Controllers\Rs232Controller($container);
};

$container['UserLogin'] = function($container) {
    return new \App\Models\UserLogin;
};

/*$container['PlayMediaController'] = function($container) {
    return new \App\Controllers\PlayMediaController($container);
};

$container['GetMediaController'] = function($container) {
    return new \App\Controllers\GetMediaController($container);
};

$container['PlayListController'] = function($container) {
    return new \App\Controllers\PlayListController($container);
};

$container['ScriptController'] = function($container) {
    return new \App\Controllers\ScriptController($container);
};

$container['OsdController'] = function($container) {
    return new \App\Controllers\OsdController($container);
};

$container['TextCaseController'] = function($container) {
    return new \App\Controllers\TextCaseController($container);
};

$container['IrController'] = function($container) {
    return new \App\Controllers\IrController($container);
};

$container['ImageCaseController'] = function($container) {
    return new \App\Controllers\ImageCaseController($container);
};

$container['FileController'] = function($container) {
    return new \App\Controllers\FileController($container);
};

$container['ScheduleController'] = function($container) {
    return new \App\Controllers\ScheduleController($container);
};
*/
$container['LogController'] = function($container) {
    return new \App\Controllers\LogController($container);
};

v::with('App\\Validation\\Rules\\');

require __DIR__ . '/../app/routes.php';
