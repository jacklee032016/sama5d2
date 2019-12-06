<?php 

namespace App\Controllers;

use App\Base\Logger;
use App\Base\LogManager;
use App\Models\LogLevel;
/**
 * @SWG\Tag(
 *   name="Log",
 *   description="Related to log system management",
 * )
 */
class LogController extends Controller
{
    public function __construct($container) {
        parent::__construct($container);
    }
    
    /**
     * @SWG\Get(
     *     path="/log/level/list",
     *     summary="Get device log level list",
     *     tags={"Log"},
     *     description="Get device log level list.",
     *     operationId="log_get_level_list",
     *     produces={"application/json"},
     *     @SWG\Parameter(
     *         name="Authorization",
     *         in="header",
     *         required=true,
     *         type="string",
     *         description="Access token retrieved from the login API"
     *     ),
     *     @SWG\Response(
     *         response=200,
     *         description="successful operation",
     *         @SWG\Schema(
     *              @SWG\Property(
     *                  property="status",
     *                  type="string",
     *                  example="success"
     *              ),
     *              @SWG\Property(
     *                  property="data",
     *                  type="array",
     *                  @SWG\Items(
     *                     type="string",
     *                     example="DEBUG, INFO, NOTICE, WARNING, ERROR, CRITICAL, ALERT, EMERGENCY",
     *                  )
     *              )
     *         ),
     *     ),
     *     @SWG\Response(
     *         response="401",
     *         description="authentication failed",
     *         @SWG\Schema(ref="#/definitions/401_response"),
     *     )
     * )
     */
    public function getLogLevelList($request, $response)
    {
        $logLevelList = Logger::getLogLevelList();
        
        $this->arrSuccess['data'] = $logLevelList;
        return $response->withJson($this->arrSuccess);
    }
    
    /**
     * @SWG\Get(
     *     path="/log/level",
     *     summary="Get device log level",
     *     tags={"Log"},
     *     description="Get device log level. Default is ERROR",
     *     operationId="log_get_level",
     *     produces={"application/json"},
     *     @SWG\Parameter(
     *         name="Authorization",
     *         in="header",
     *         required=true,
     *         type="string",
     *         description="Access token retrieved from the login API"
     *     ),
     *     @SWG\Response(
     *         response=200,
     *         description="successful/failed operation",
     *         @SWG\Schema(
     *              @SWG\Property(
     *                  property="status",
     *                  type="string",
     *                  example="success",
     *                  description="success or failed"
     *              ),
     *              @SWG\Property(
     *                  property="data",
     *                  @SWG\Property(
     *                      property="level",
     *                      type="string",
     *                      example="DEBUG",
     *                  ),
     *              )
     *         ),
     *     ),
     *     @SWG\Response(
     *         response="401",
     *         description="authentication failed",
     *         @SWG\Schema(ref="#/definitions/401_response"),
     *     )
     * )
     */
    public function getLogLevel($request, $response)
    {
        $level = null;
        try {
            $log   = new LogLevel();
            $level = $log->getLogLevel();
        } catch (\Exception $e) {
            Logger::getInstance()->addError('get level error: ' . $e->getMessage());
            $this->arrFailed['data'] = $e->getMessage();
            return $response->withJson($this->arrFailed);
        }
        
        $this->arrSuccess['data'] = array("level" => $level);
        return $response->withJson($this->arrSuccess);
    }
    
    /**
     * @SWG\Put(
     *     path="/log/level",
     *     summary="Set device log level",
     *     tags={"Log"},
     *     description="Set device log level. Default is ERROR",
     *     operationId="log_set_level",
     *     produces={"application/json"},
     *     @SWG\Parameter(
     *         name="Authorization",
     *         in="header",
     *         required=true,
     *         type="string",
     *         description="Access token retrieved from the login API"
     *     ),
     *     @SWG\Parameter(
     *         name="body",
     *         in="body",
     *         required=true,
     *         @SWG\Schema(
     *              required={"level"},
     *              @SWG\Property(
     *                  property="level",
     *                  type="string",
     *                  example="DEBUG",
     *              ),
     *         )
     *     ),
     *     @SWG\Response(
     *         response=200,
     *         description="successful/failed operation",
     *         @SWG\Schema(
     *              @SWG\Property(
     *                  property="status",
     *                  type="string",
     *                  example="success",
     *                  description="success or failed"
     *              ),
     *              @SWG\Property(
     *                  property="data",
     *                  @SWG\Property(
     *                      property="level",
     *                      type="string",
     *                      example="DEBUG"
     *                  ),
     *              )
     *         ),
     *     ),
     *     @SWG\Response(
     *         response="401",
     *         description="authentication failed",
     *         @SWG\Schema(ref="#/definitions/401_response"),
     *     )
     * )
     */
    public function setLogLevel($request, $response)
    {
        $level = $request->getParam('level');
        
        try {
            $log  = new LogLevel();
            $log->setLogLevel($level);
        } catch (\Exception $e) {
            Logger::getInstance()->addError('set level error: ' . $e->getMessage());
            $this->arrFailed['data'] = $e->getMessage();
            return $response->withJson($this->arrFailed);
        }
        
        $this->arrSuccess['data'] = array("level" => $level);
        return $response->withJson($this->arrSuccess);
    }
    
    /**
     * @SWG\Get(
     *     path="/log",
     *     summary="Make a compressed log files available to download",
     *     tags={"Log"},
     *     description="Make a compressed log files available to download. use address http://deviceIP/ + response.data.file to download the compressed file",
     *     operationId="log_download",
     *     produces={"application/json"},
     *     @SWG\Parameter(
     *         name="Authorization",
     *         in="header",
     *         required=true,
     *         type="string",
     *         description="Access token retrieved from the login API"
     *     ),
     *     @SWG\Response(
     *         response=200,
     *         description="successful operation",
     *         @SWG\Schema(
     *              @SWG\Property(
     *                  property="status",
     *                  type="string",
     *                  example="success"
     *              ),
     *              @SWG\Property(
     *                  property="data",
     *                  @SWG\Property(
     *                      property="file",
     *                      type="string",
     *                      example="LogsArchive_20180222-150243.gz"
     *                  ),
     *              )
     *         ),
     *     ),
     *     @SWG\Response(
     *         response="401",
     *         description="authentication failed",
     *         @SWG\Schema(ref="#/definitions/401_response"),
     *     )
     * )
     */
    public function getLog($request, $response)
    {
        $logManager = new LogManager();
        $file       = $logManager->backup();
        if ($file)
        {
            $this->arrSuccess['data'] = array("file" => $file);
            return $response->withJson($this->arrSuccess);
        }
        
        return $response->withJson($this->arrFailed);
    }
}
