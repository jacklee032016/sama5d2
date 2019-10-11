<?php 

namespace App\Controllers;

use App\Base\MUXProtocol;
use App\Base\Logger;
use App\Models\Rs232Manager;
use App\Params;
use Respect\Validation\Validator as v;

/**
 * @SWG\Tag(
 *   name="Rs232",
 *   description="Related to rs232 operations",
 * )
 */
class Rs232Controller extends Controller
{
    protected $muxProtocol;
    private $rs232Manager;
    
    public function __construct($container) {
        parent::__construct($container);
        $this->muxProtocol = new MUXProtocol(Params::DEVICE_CONTROLLER_USERNAME, Params::DEVICE_CONTROLLER_PASSWORD);
    }
    
    /**
     * @SWG\Put(
     *     path="/rs232/sendData",
     *     summary="rs232 send hexadecimal data",
     *     tags={"Rs232"},
     *     description="send hexadecimal data.",
     *     operationId="rs232_sendData",
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
     *              required={"data", "isFeedback"},
     *              @SWG\Property(
     *                  property="data",
     *                  type="string",
     *                  example="A055B1FF"
     *              ),
     *              @SWG\Property(
     *                  property="isFeedback",
     *                  type="string",
     *                  example="0",
     *                  description="1 or 0"
     *              ),
     *              @SWG\Property(
     *                  property="waitTime",
     *                  type="string",
     *                  example="1000",
     *                  description="in seconds"
     *              ),
     *         )
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
     *                      property="feedbackdata",
     *                      type="string",
     *                      example=""
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
    public function sendData($request, $response)
    {
        $rule["data"] = v::notEmpty()->regex("/^[0-9a-fA-F]+$/");
        $rule["isFeedback"] = v::in([0,1]);
        $waitTime = $request->getParam("waitTime");
        
        if ($waitTime)
        {
            $rule["waitTime"] = v::positive();
        }
        
        $validation = $this->Validator->validate($request, $rule);
        
        if ($validation->failed())
        {
            //return json failed
            $this->arrFailed['data'] = $validation->getErrors();
            return $response->withJson($this->arrFailed);
        }

        if ($request->getParam("isFeedback"))
        {
            if ($waitTime)
                $data = array("data" => $request->getParam("data"), "isFeedback" => 1, "waitTime" => intval($waitTime));
            else 
                $data = array("data" => $request->getParam("data"), "isFeedback" => 1, "waitTime" => 1000);
                
        } else {
            $data = array("data" => $request->getParam("data"), "isFeedback" => 0);
        }
        
        $deviceResponse = $this->muxProtocol->cmd_rs232($data);
        
        if ($deviceResponse)
        {
            $arrParameters = $this->muxProtocol->hex_command_to_text($deviceResponse[0]);
            
            if (!empty($arrParameters) && $arrParameters['login-ack'] == 'OK')
            {
                $this->arrSuccess['data']['feedbackdata'] = $arrParameters['data']['0']['feedbackdata'];
                return $response->withJson($this->arrSuccess);
            }
        }
        $errorMessage = (!empty($arrParameters))? $arrParameters["pwd-msg"]: "";
        Logger::getInstance()->addError('rs232 send data failed ' . $errorMessage);
        $this->arrFailed['data'] = $errorMessage;
        return $response->withJson($this->arrFailed);
    }

    /**
     * @SWG\Put(
     *     path="/rs232/configuration",
     *     summary="Set rs232 configuration",
     *     tags={"Rs232"},
     *     description="Set rs232 configuration",
     *     operationId="rs232_setConfiguration",
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
     *              @SWG\Property(
     *                  property="RS232Baudrate",
     *                  type="integer",
     *                  example="9600",
     *                  description="9600, 19200, 38400, 57600, 115200"
     *              ),
     *              @SWG\Property(
     *                  property="RS232Databits",
     *                  type="integer",
     *                  example="8",
     *                  description="7,8"
     *              ),
     *              @SWG\Property(
     *                  property="RS232Parity",
     *                  type="string",
     *                  example="none",
     *                  description="none, odd, even"
     *              ),
     *              @SWG\Property(
     *                  property="RS232Stopbits",
     *                  type="integer",
     *                  example="1",
     *                  description="1, 2"
     *              )
     *         )
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
     *                  )
     *             )
     *         ),
     *     ),
     *     @SWG\Response(
     *         response="401",
     *         description="authentication failed",
     *         @SWG\Schema(ref="#/definitions/401_response"),
     *     )
     * )
     */
    public function setConfiguration($request, $response)
    {
        $rule = [];
        $arr = $request->getParsedBody();
        //$arr["data"] = "";
        if ($request->getParam("baudrate"))
        {
            $rule["baudrate"] = v::in([9600, 19200, 38400, 57600, 115200]);
            $arr["baudrate"]  = intval($request->getParam("baudrate"));
        }
        if ($request->getParam("databit"))
        {
            $rule["databit"] = v::in([7, 8]);
            $arr["databit"]  = intval($request->getParam("databit"));
        }
        if ($request->getParam("parity"))
            $rule["parity"] = v::in(["none", "odd", "even"]);
        if ($request->getParam("stopbit"))
        {
            $rule["stopbit"] = v::in([1,2]);
            $arr["stopbit"]  = intval($request->getParam("stopbit"));
        }   
        $validation = $this->Validator->validate($request, $rule);
        
        if ($validation->failed())
        {
            $this->arrFailed['data'] = $validation->getErrors();
            return $response->withJson($this->arrFailed);
        }
        $data = array("rs232" => $arr);
        
        $deviceResponse = $this->muxProtocol->cmd_set_param($data);

        //check if good
        if ($deviceResponse)
        {
            $arrParameters = $this->muxProtocol->hex_command_to_text($deviceResponse[0]);
            
            if (!empty($arrParameters) && $arrParameters['login-ack'] == 'OK')
            {
                return $response->withJson($this->arrSuccess);
            }
        }
        
        $errorMessage = (!empty($arrParameters))? $arrParameters["pwd-msg"]: "";
        Logger::getInstance()->addError('rs232 set configuration failed ' . $errorMessage);
        $this->arrFailed['data'] = 'rs232 set configuration failed ' . $errorMessage;
        return $response->withJson($this->arrFailed);
    }
   
}

