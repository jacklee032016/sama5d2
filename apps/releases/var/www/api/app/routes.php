<?php 

//use App\Middleware\AuthMiddleware;

$app->get('/', 'HomeController:index');
$app->post('/auth/signin', 'AuthController:signIn');
$app->get('/device/notAuthParam', 'DeviceController:getNotAuthParam');

//$app->group('',function () {
    //auth api end points
    $app->post('/auth/change-password', 'AuthController:changePassword');
    $app->post('/auth/signout', 'AuthController:signOut');
    //device api end points
    $app->get('/device/param', 'DeviceController:getParam');
    $app->post('/device/reset', 'DeviceController:reset');
    $app->post('/device/reboot', 'DeviceController:reboot');
    $app->post('/device/network', 'DeviceController:setNetwork');
    $app->put('/device/sdp', 'DeviceController:setSdp');
    $app->put('/device/manual-settings', 'DeviceController:setSettings');
    $app->get('/device/security-check', 'DeviceController:getSecurityStatus');
    $app->put('/device/blink', 'DeviceController:blink');
    $app->post('/device/upgrade', 'DeviceController:upgrade');
    $app->post('/device/upgradeFPGA', 'DeviceController:upgradeFPGA');
    //rs232 end points
    $app->put('/rs232/sendData', 'Rs232Controller:sendData');
    $app->put('/rs232/configuration', "Rs232Controller:setConfiguration");
//})->add(new AuthMiddleware($container));
    