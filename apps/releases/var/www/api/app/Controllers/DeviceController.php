<?php 

namespace App\Controllers;

use App\Base\MUXProtocol;
use App\Base\MuxDevice;
use App\Params;
use App\Base\Logger;
use App\Models\Settings;
use Respect\Validation\Validator as v;
use Respect\Validation\Rules\IntVal;

/**
 * @SWG\Tag(
 *   name="Device",
 *   description="Related to device settings",
 * )
 */
class DeviceController extends Controller
{
    protected $muxProtocol;
    
    public function __construct($container) {
        parent::__construct($container);
        $this->muxProtocol = new MUXProtocol(Params::DEVICE_CONTROLLER_USERNAME, Params::DEVICE_CONTROLLER_PASSWORD);
    }
    /**
     * @SWG\Get(
     *     path="/device/param",
     *     summary="Get system information",
     *     tags={"Device"},
     *     description="Get system information.",
     *     operationId="device_param_get",
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
     *                    property="system",
     *                    ref="#/definitions/system"
     *                  ),
     *                  @SWG\Property(
     *                    property="video",
     *                    ref="#/definitions/video"
     *                  ),
     *                  @SWG\Property(
     *                    property="audio",
     *                    ref="#/definitions/audio"
     *                  ),
     *                  @SWG\Property(
     *                    property="anc",
     *                    ref="#/definitions/anc"
     *                  ),
     *                  @SWG\Property(
     *                    property="sdp",
     *                    ref="#/definitions/sdp"
     *                  ),
     *                  @SWG\Property(
     *                    property="rs232",
     *                    ref="#/definitions/rs232"
     *                  ),
     *                  @SWG\Property(
     *                    property="IR",
     *                    ref="#/definitions/IR"
     *                  ),
     *                  @SWG\Property(
     *                    property="security",
     *                    ref="#/definitions/security"
     *                  ),
     *                  @SWG\Property(
     *                    property="rs232List",
     *                    ref="#/definitions/rs232List"
     *                  ),
     *                  @SWG\Property(
     *                    property="videoList",
     *                    ref="#/definitions/videoList"
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
    public function getParam($request, $response)
    {
        $parameters = $this->muxProtocol->cmd_get_param();
        
        if ($parameters)
        {
            $arrParameters = $this->muxProtocol->hex_command_to_text($parameters[0]);
            
            if (!empty($arrParameters) && $arrParameters['login-ack'] == 'OK')
            {
                $dataList = array("rs232List" => MuxDevice::RS232_LIST) + array("videoList" => MuxDevice::VIDEO_LIST) + array("audioList" => MuxDevice::AUDIO_LIST);
                //$arrParameters['data']['system']['ver'] = Params::VERSION;
                unset($arrParameters['data']['others']);
                $data = $arrParameters['data'] + $dataList;
                //update device type if necessary
                $settings = new Settings();
                $settings->setDeviceTypeByGetParam($data);
                //
                $this->arrSuccess['data'] = $data;
                return $response->withJson($this->arrSuccess);
            }
        }
        
        $errorMessage = (!empty($arrParameters))? $arrParameters["pwd-msg"]: "";
        Logger::getInstance()->addError("get param failed " . $errorMessage);
        
        //json failed response
        return $response->withJson($this->arrFailed);
    }
    
    /**
     * @SWG\Get(
     *     path="/device/security-check",
     *     summary="Get security status infomation",
     *     tags={"Device"},
     *     description="Get security status infomation.",
     *     operationId="device_security_get",
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
     *                      property="security_status",
     *                      type="string",
     *                      example="PASS",
     *                      description="values: PASS, FAILED"
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
    public function getSecurityStatus($request, $response)
    {
        $parameters = $this->muxProtocol->cmd_security_status();
        
        //check if good
        if ($parameters)
        {
            $arrParameters = $this->muxProtocol->hex_command_to_text($parameters[0]);
            if (!empty($arrParameters))
            {
                if ($arrParameters['login-ack'] == 'OK')
                {
                    $this->arrSuccess["data"] = ["security_status" => "PASS"];
                    return $response->withJson($this->arrSuccess);
                } else {
                    $this->arrSuccess["data"] = ["security_status" => "FAILED"];
                    return $response->withJson($this->arrSuccess);
                }
            }
        }
        
        $errorMessage = (!empty($arrParameters))? $arrParameters["pwd-msg"]: "";
        $this->arrFailed["data"] = $errorMessage;
        
        //json failed response
        return $response->withJson($this->arrFailed);
    }
    
    /**
     * @SWG\Post(
     *     path="/device/network",
     *     summary="Set network infomation",
     *     tags={"Device"},
     *     description="Set network infomation.",
     *     operationId="device_network_Set",
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
     *                  @SWG\Property(
     *                      property="ip",
     *                      type="string",
     *                      example="192.168.168.205"
     *                  ),
     *                  @SWG\Property(
     *                      property="mask",
     *                      type="string",
     *                      example="255.255.255.0"
     *                  ),
     *                  @SWG\Property(
     *                      property="getway",
     *                      type="string",
     *                      example="192.168.168.1"
     *                  ),
     *                  @SWG\Property(
     *                      property="isDhcp",
     *                      type="string",
     *                      example="0"
     *                  ),
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
     *                  @SWG\Items(
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
    public function setNetwork($request, $response)
    {
        $isDhcp = $request->getParam("isDhcp");
        $rule = [];
        if ($request->getParam("ip"))
            $rule["ip"] = v::ip();
        if ($request->getParam("mask"))
            $rule["mask"] = v::ip();
        if ($request->getParam("gateway"))
            $rule["gateway"] = v::ip();
        if (isset($isDhcp))
            $rule["isDhcp"] = v::in([0,1]);
        
        $validation = $this->Validator->validate($request, $rule);
        
        if ($validation->failed())
        {
            $this->arrFailed['data'] = $validation->getErrors();
            return $response->withJson($this->arrFailed);
        }
        
        if (isset($isDhcp) && $isDhcp)
        {
            $data = array("system" => array("isDhcp" => 1));
        } else {
            $arr = $request->getParsedBody();
            //not allow to change others parameters
            $arr = array_intersect_key($arr, array_flip(["ip","mask","gateway"]));
            
            $arr["isDhcp"] = 0;
            $data = array("system" => $arr);
        }
        
        $deviceResponse= $this->muxProtocol->cmd_set_param($data);
        
        //check if good
        if ($deviceResponse)
        {
            $arrParameters = $this->muxProtocol->hex_command_to_text($deviceResponse[0]);
            
            if (!empty($arrParameters) && $arrParameters['login-ack'] == 'OK')
            {
                $data = array("system" => array("reboot" => 1));
                $deviceResponse = $this->muxProtocol->cmd_set_param($data);
                
                if ($deviceResponse)
                {
                    $arrParameters = $this->muxProtocol->hex_command_to_text($deviceResponse[0]);
                    if (!empty($arrParameters) && $arrParameters['login-ack'] == 'OK')
                        return $response->withJson($this->arrSuccess);
                }
            }
        }
        $errorMessage = (!empty($arrParameters))? $arrParameters['pwd-msg'] : "";
        Logger::getInstance()->addError('set network param api error' . $errorMessage);
        return $response->withJson($this->arrFailed);
    }

    /**
     * @SWG\Put(
     *     path="/device/ptp-settings",
     *     summary="Set device ptp settings",
     *     tags={"Device"},
     *     description="Set device ptp settings.",
     *     operationId="device_ptp-settings",
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
     *           @SWG\Property(
     *                property="enable",
     *                type="integer",
     *                example=1
     *           ),
     *           @SWG\Property(
     *               property="domainNumber",
     *               type="integer",
     *               example=1
     *           ),
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
     *                  @SWG\Items(
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
    public function setPtpSettings($request, $response)
    {
        
        $validation = $this->Validator->validate($request, [
            "domainNumber" => v::between(0,127),//v::intVal(),
            "enable" => v::in(["1", "0"]),
        ]);
        
        if ($validation->failed())
        {
            $this->arrFailed['data'] = $validation->getErrors();
            return $response->withJson($this->arrFailed);
        }
       
        $data["ptp"]["isEnable"] = intval($request->getParam("enable"));
        $data["ptp"]["domainNumber"] = intval($request->getParam("domainNumber"));
        
        $deviceResponse= $this->muxProtocol->cmd_set_param($data);
        
        //check if good
        if ($deviceResponse)
        {
            $arrParameters = $this->muxProtocol->hex_command_to_text($deviceResponse[0]);
            
            if (!empty($arrParameters) && $arrParameters['login-ack'] == 'OK')
            {
                return $response->withJson($this->arrSuccess);
            }
        }
        $errorMessage = (!empty($arrParameters))? $arrParameters['pwd-msg'] : "";
        Logger::getInstance()->addError('set param api error' . $errorMessage);
        return $response->withJson($this->arrFailed);
    }
    
    
    /**
     * @SWG\Post(
     *     path="/device/manual-settings",
     *     summary="Set device settings",
     *     tags={"Device"},
     *     description="Set device settings.",
     *     operationId="device_manual-settings",
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
     *                  @SWG\Property(
     *                      property="videoip",
     *                      type="string",
     *                      example="239.0.0.101"
     *                  ),
     *                  @SWG\Property(
     *                      property="videoport",
     *                      type="integer",
     *                      example=36000
     *                  ),
     *                  @SWG\Property(
     *                      property="audioip",
     *                      type="string",
     *                      example="239.0.0.100"
     *                  ),
     *                  @SWG\Property(
     *                      property="audioport",
     *                      type="integer",
     *                      example=36001
     *                  ),
     *                  @SWG\Property(
     *                      property="fps",
     *                      type="string",
     *                      example="60"
     *                  ),
     *                  @SWG\Property(
     *                      property="colorSpace",
     *                      type="string",
     *                      example="RGB"
     *                  ),
     *                  @SWG\Property(
     *                      property="colorDepth",
     *                      type="string",
     *                      example=16
     *                  ),
     *                  @SWG\Property(
     *                      property="resolution",
     *                      type="string",
     *                      example="1920x1080"
     *                  ),
     *                  @SWG\Property(
     *                      property="intlce",
     *                      type="integer",
     *                      example=1
     *                  ),
     *                  @SWG\Property(
     *                      property="channels",
     *                      type="integer",
     *                      example=16
     *                  ),
     *                  @SWG\Property(
     *                      property="sample",
     *                      type="string",
     *                      example="48000"
     *                  ),
     *                  @SWG\Property(
     *                      property="pktSize",
     *                      type="string",
     *                      example="1ms"
     *                  ),
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
     *                  @SWG\Items(
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
    public function setSettings($request, $response)
    {
        $rule = [];
        $data = [];
        $mediaVideo = [];
        $mediaAudio = [];
        if ($request->getParam("videoip"))
        {
            $rule["videoip"] = v::ip();
            $mediaVideo["ip"] = $request->getParam("videoip");
        }
        if ($request->getParam("videoport"))
        {
            $rule["videoport"] = v::between(1,65535);
            $mediaVideo["port"] = intval($request->getParam("videoport"));
        }
        
        $videoForceStream = $request->getParam("videoForceStream");
        if (isset($videoForceStream))
        {
            $rule["videoForceStream"] = v::in([0, 1]);
            $data["system"]["forceStream"] = intval($videoForceStream);
        }

        if ($request->getParam("sfd"))
        {
            $rule["sfd"] = v::between(1,4);
            $data["system"]["sfpCfg"] = intval($request->getParam("sfd"));
        }
        if ($request->getParam("fps"))
        {
            $rule["fps"] = v::notBlank();
            $mediaVideo["fps"]= $request->getParam("fps");
        }
        if ($request->getParam("audioip"))
        {
            $rule["audioip"] = v::ip();
            $mediaAudio["ip"] = $request->getParam("audioip");
        }
        if ($request->getParam("audioport"))
        {
            $rule["audioport"] = v::between(1,65535);
            $mediaAudio["port"] = intval($request->getParam("audioport"));
        }
        if ($request->getParam("colorSpace"))
        {
            $rule["colorSpace"] = v::in(MuxDevice::VIDEO_LIST["colorSpace"]);
            $mediaVideo["colorSpace"] = $request->getParam("colorSpace");
        }
        if ($request->getParam("colorDepth"))
        {
            $rule["colorDepth"] = v::in(MuxDevice::VIDEO_LIST["colorDepth"]);
            $mediaVideo["depth"] = intval($request->getParam("colorDepth"));
        }
        if ($request->getParam("intlce"))
        {
            $rule["intlce"] = v::in(MuxDevice::VIDEO_LIST["intlce"]);
            $mediaVideo["intlce"] = intval($request->getParam("intlce"));
        }
        if ($request->getParam("resolution"))
        {
            $rule["resolution"] = v::in(MuxDevice::VIDEO_LIST["resolution"]);
            $resolution = explode("x", $request->getParam("resolution"));
            $mediaVideo["height"] = intval($resolution[1]);
            $mediaVideo["width"] = intval($resolution[0]);
        }
        if ($request->getParam("channels"))
        {
            $rule["channels"] = v::in(MuxDevice::AUDIO_LIST["audioChs"]);
            $mediaAudio["channels"] = intval($request->getParam("channels"));
        }
        if ($request->getParam("sample"))
        {
            $rule["sample"] = v::in(MuxDevice::AUDIO_LIST["audioRate"]);
            $mediaAudio["sample"] = $request->getParam("sample");
        }
        if ($request->getParam("pktSize"))
        {
            $rule["pktSize"] = v::in(MuxDevice::AUDIO_LIST["audioPKT"]);
            $mediaAudio["pktSize"] = $request->getParam("pktSize");
        }
        
        $validation = $this->Validator->validate($request, $rule);
        
        if ($validation->failed())
        {
            $this->arrFailed['data'] = $validation->getErrors();
            return $response->withJson($this->arrFailed);
        }
        
        if (!empty($mediaAudio))
        {
            $data["audio"] = $mediaAudio;
        }
        if (!empty($mediaVideo))
        {
            $data["video"] = $mediaVideo;
        }
        
        $deviceResponse= $this->muxProtocol->cmd_set_param($data);
        
        //check if good
        if ($deviceResponse)
        {
            $arrParameters = $this->muxProtocol->hex_command_to_text($deviceResponse[0]);
            
            if (!empty($arrParameters) && $arrParameters['login-ack'] == 'OK')
            {
                return $response->withJson($this->arrSuccess);
            }
        }
        $errorMessage = (!empty($arrParameters))? $arrParameters['pwd-msg'] : "";
        Logger::getInstance()->addError('set param api error' . $errorMessage);
        return $response->withJson($this->arrFailed);
    }
    
    /**
     * @SWG\Post(
     *     path="/device/sdp",
     *     summary="Set sdp parameters",
     *     tags={"Device"},
     *     description="Set sdp parameters.",
     *     operationId="device_sdp",
     *     produces={"application/json"},
     *     @SWG\Parameter(
     *         name="Authorization",
     *         in="header",
     *         required=true,
     *         type="string",
     *         description="Access token retrieved from the login API"
     *     ),
     *     @SWG\Parameter(
     *          name="body",
     *          in="body",
     *          required=true,
     *          @SWG\Schema(
     *                @SWG\Property(
     *                      property="video",
     *                      @SWG\Property(
     *                         property="ip",
     *                         type="string",
     *                         example="192.168.168.101"
     *                      ),
     *                         @SWG\Property(
     *                         property="port",
     *                         type="integer",
     *                         example=36001
     *                      ),
     *                      @SWG\Property(
     *                        property="uri",
     *                        type="string",
     *                        example="video/sdp2"
     *                      ),
     *                ),
     *                @SWG\Property(
     *                      property="audio",
     *                      @SWG\Property(
     *                         property="ip",
     *                         type="string",
     *                         example="192.168.168.101"
     *                      ),
     *                         @SWG\Property(
     *                         property="port",
     *                         type="integer",
     *                         example=36001
     *                      ),
     *                      @SWG\Property(
     *                        property="uri",
     *                        type="string",
     *                        example="video/sdp2"
     *                      ),
     *                ),
     *           )
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
     *                  @SWG\Items(
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
    public function setSdp($request, $response)
    {
        $video = $request->getParam("video");
        $audio = $request->getParam("audio");
        
        $error = [];
        $data = [];
        $mediaVideo = [];
        $mediaAudio = [];
        if ($video)
        {
            if(isset($video["uri"]))
            {
                if (empty($video["uri"]))
                    $error["video"]["uri"] = "Video uri can not be blank";
                else 
                    $mediaVideo["uri"] = $video["uri"];
            }
            if(isset($video["ip"]))
            {
                if (filter_var($video["ip"], FILTER_VALIDATE_IP))
                    $mediaVideo["ip"] = $video["ip"];
                else
                    $error["video"]["ip"] = "Video ip not valid";
            }
            if(isset($video["port"]))
            {
                if ($video["port"] >= 1 && $video["port"] <= 65535)
                    $mediaVideo["port"] = intval($video["port"]);
                else 
                    $error["video"]["port"] = "Video port not valid";
            }
        }
        if ($audio)
        {
            if(isset($audio["uri"]))
            {
                if (empty($audio["uri"]))
                    $error["audio"]["uri"] = "Audio uri can not be blank";
                else
                   $mediaAudio["uri"] = $audio["uri"];
            }
            if(isset($audio["ip"]))
            {
                if (filter_var($audio["ip"], FILTER_VALIDATE_IP))
                    $mediaAudio["ip"] = $audio["ip"];
                else
                    $error["audio"]["ip"] = "Audio ip not valid";
            }
            if(isset($audio["port"]))
            {
                if ($audio["port"] >= 1 && $audio["port"] <= 65535)
                    $mediaAudio["port"] = intval($audio["port"]);
                else
                    $error["audio"]["port"] = "Audio port not valid";
            }
        }
        
        if (!empty($error))
        {
            $this->arrFailed['data'] = $error;
            return $response->withJson($this->arrFailed);
        }
        
        if (!empty($mediaAudio))
        {
           $mediaAudio["media"] = "audio";
           $data[] = $mediaAudio;
        }

        if (!empty($mediaVideo))
        {
            $mediaVideo["media"] = "video";
            $data[] = $mediaVideo;
        }
        
        $data = array("sdp" => $data);
        
        $deviceResponse= $this->muxProtocol->cmd_set_param($data);
        
        //check if good
        if ($deviceResponse)
        {
            $arrParameters = $this->muxProtocol->hex_command_to_text($deviceResponse[0]);
            
            if (!empty($arrParameters) && $arrParameters['login-ack'] == 'OK')
            {
                return $response->withJson($this->arrSuccess);
            }
        }
        $errorMessage = (!empty($arrParameters))? $arrParameters['pwd-msg'] : "";
        Logger::getInstance()->addError('set sdp param api error' . $errorMessage);
        return $response->withJson($this->arrFailed);
    }
    
    /**
     * @SWG\Get(
     *     path="/device/notAuthParam",
     *     summary="Get some device information without authentication",
     *     tags={"Device"},
     *     description="Get some device information without authentication.",
     *     operationId="device_notAuthParam",
     *     produces={"application/json"},
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
     *                        property="isTX",
     *                        type="integer",
     *                        description="1 for TX, 0 for RX",
     *                        example=0
     *                 ),
     *                  @SWG\Property(
     *                        property="version",
     *                        type="string",
     *                        example="1.0.0"
     *                 ),
     *                  @SWG\Property(
     *                        property="built",
     *                        type="string",
     *                        example="Aug 14 2019 16:09:55"
     *                 ),
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
    public function getNotAuthParam($request, $response)
    {
        $parameters = $this->muxProtocol->cmd_get_param();
        
        //check if good
        if ($parameters)
        {
            $arrParameters = $this->muxProtocol->hex_command_to_text($parameters[0]);
            
            if (!empty($arrParameters) && $arrParameters['login-ack'] == 'OK')
            {
                //$this->arrSuccess["data"] = $arrParameters["data"];
                $this->arrSuccess["data"] = ["isTx" => $arrParameters["data"]["system"]["isTx"], "version" =>$arrParameters["data"]["system"]["ver"], "built" => $arrParameters["data"]["system"]["built"]];
                return $response->withJson($this->arrSuccess);
            }
        }
        
        $errorMessage = (!empty($arrParameters))? $arrParameters["pwd-msg"]: "";
        Logger::getInstance()->addError("get notAuthParam failed " . $errorMessage);
        
        //json failed response
        return $response->withJson($this->arrFailed);
    }
    
    /**
     * @SWG\Post(
     *     path="/device/reset",
     *     summary="Reset device",
     *     tags={"Device"},
     *     description="Reset device.",
     *     operationId="device_reset",
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
    public function reset($request, $response)
    {
        $data = array("system" => array("reset" => 1));
        
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
        Logger::getInstance()->addError('reset failed ' . $errorMessage);
        return $response->withJson($this->arrFailed);
    }
    
    /**
     * @SWG\Post(
     *     path="/device/reboot",
     *     summary="Reboot device",
     *     tags={"Device"},
     *     description="Reboot device.",
     *     operationId="device_reboot",
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
    public function reboot($request, $response)
    {
        $data = array("system" => array("reboot" => 1));
        
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
        Logger::getInstance()->addError('reboot failed ' . $errorMessage);
        return $response->withJson($this->arrFailed);
    }
    
    /**
     * @SWG\Put(
     *     path="/device/blink",
     *     summary="Blink on/off",
     *     tags={"Device"},
     *     description="Blink on/off.",
     *     operationId="device_blink",
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
     *              required={"blink"},
     *              @SWG\Property(
     *                  property="blink",
     *                  type="string",
     *                  example="true",
     *                  description="true or false"
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
     *                      property="blink",
     *                      type="integer",
     *                      description="0 - blink off, 1 - blink on",
     *                      example=0
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
    public function blink($request, $response)
    {
        $blink = $request->getParam('blink');
        $blink = ($blink == 'true')? 1 : 0;
        
        $data = array("system" => array("blinkLed" => $blink));
        
        $deviceResponse = $this->muxProtocol->cmd_set_param($data);
        
        //check if good
        $arrParameters = $this->muxProtocol->hex_command_to_text($deviceResponse[0]);
        
        if (!empty($arrParameters) && $arrParameters['login-ack'] == 'OK')
        {
            Logger::getInstance()->addInfo('BLINK: ', array('VALUE' => $arrParameters['data'][0]['system']['blinkLed']));
            $this->arrSuccess['data'] = array("blink" => $arrParameters['data'][0]['system']['blinkLed']);
            return $response->withJson($this->arrSuccess);
        }
        $errorMessage = (!empty($arrParameters))? $arrParameters["pwd-msg"]: "";
        Logger::getInstance()->addError('BLINK device api error ' . $errorMessage);
        return $response->withJson($this->arrFailed);
    }
    
    /**
     * @SWG\Put(
     *     path="/device/upgrade",
     *     summary="Upgrade device",
     *     tags={"Device"},
     *     description="Upgrade device.",
     *     operationId="device_upgrade",
     *     produces={"application/json"},
     *     consumes={"multipart/form-data"},
     *     @SWG\Parameter(
     *         name="Authorization",
     *         in="header",
     *         required=true,
     *         type="string",
     *         description="Access token retrieved from the login API"
     *     ),
     *     @SWG\Parameter(
     *         name="fileToUpload",
     *         in="formData",
     *         required=true,
     *         type="file",
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
     *                  @SWG\Items(
     *                  )
     *              )
     *         ),
     *     ),
     *     @SWG\Response(
     *         response=201,
     *         description="validation failed - response code is 200 not 201, swagger limitation",
     *         @SWG\Schema(
     *              @SWG\Property(
     *                  property="status",
     *                  type="string",
     *                  example="failed"
     *              ),
     *              @SWG\Property(
     *                  property="data",
     *                  @SWG\Property(
     *                        property="fileTOUpload",
     *                        type="string",
     *                        example="Firmware is not compatible!"
     *                 ),
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
    public function upgrade($request, $response)
    {
        $uploadedFiles  = $request->getUploadedFiles();
        
        if (!isset($uploadedFiles['fileToUpload']))
        {
            Logger::getInstance()->addError("UPLOAD error: no file uploaded");
            $this->arrFailed["data"] = ["fileToUpload"=>"no file uploaded"];
            return $response->withJson($this->arrFailed);
        }
        
        $uploadedFile   = $uploadedFiles['fileToUpload'];
        
        $fwFileReceived = $uploadedFile->getStream()->getContents();
        
        $fwHeader = substr($fwFileReceived, 0, 12);
        $fwData   = substr($fwFileReceived, 12);
       
        $productModelReceived = substr($fwHeader,3,9);
        
        //get device type - TX or RX
        $settings   = new Settings();
        $deviceType = $settings->getDeviceType();
                
        if ($deviceType == 'TX')
        {
            $device = Params::MCU_UPGRADE_VALIDATION."-TX";
        } else {
            $device = Params::MCU_UPGRADE_VALIDATION."-RX";
        }

        if ($productModelReceived != $device)
        {
            $this->arrFailed["data"] = ["fileToUpload"=>"Firmware is not compatible!"];
            return $response->withJson($this->arrFailed);
        }
        
        // Save the firmware to a file
        $fp = fopen(Params::FIRMWARE_MCU_FILE, 'w');
        if (!$fp)
        {
            $this->arrFailed["data"] = ["fileToUpload"=>"could not open firmware file!"];
            return $response->withJson($this->arrFailed);
        }
        
        fwrite($fp,$fwData);
        fclose($fp);
        
        //rebooting
        $data = array("system" => array("reboot" => 1));
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
        //just in case muxMgr - jack manager - is not working properly
        $this->arrFailed["data"] = ["reboot" => "manual reboot"];
        return $response->withJson($this->arrFailed);
    }
    
    /**
     * @SWG\Put(
     *     path="/device/upgradeFPGA",
     *     summary="Upgrade device FPGA",
     *     tags={"Device"},
     *     description="Upgrade device FPGA.",
     *     operationId="device_upgrade_FPGA",
     *     produces={"application/json"},
     *     consumes={"multipart/form-data"},
     *     @SWG\Parameter(
     *         name="Authorization",
     *         in="header",
     *         required=true,
     *         type="string",
     *         description="Access token retrieved from the login API"
     *     ),
     *     @SWG\Parameter(
     *         name="fileToUploadFPGA",
     *         in="formData",
     *         required=true,
     *         type="file",
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
     *                  @SWG\Items(
     *                  )
     *              )
     *         ),
     *     ),
     *     @SWG\Response(
     *         response=201,
     *         description="validation failed - response code is 200 not 201, swagger limitation",
     *         @SWG\Schema(
     *              @SWG\Property(
     *                  property="status",
     *                  type="string",
     *                  example="failed"
     *              ),
     *              @SWG\Property(
     *                  property="data",
     *                  @SWG\Property(
     *                        property="fileTOUpload",
     *                        type="string",
     *                        example="Firmware is not compatible!"
     *                 ),
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
    public function upgradeFPGA($request, $response)
    {
        $uploadedFiles  = $request->getUploadedFiles();
        
        if (!isset($uploadedFiles['fileToUploadFPGA']))
        {
            Logger::getInstance()->addError("UPLOAD error: no file uploaded");
            $this->arrFailed["data"] = ["fileToUpload"=>"no file uploaded"];
            return $response->withJson($this->arrFailed);
        }
        
        $uploadedFile   = $uploadedFiles['fileToUploadFPGA'];
        
        $fwFileReceived = $uploadedFile->getStream()->getContents();
        
        $fwHeader = substr($fwFileReceived, 0, 12);
        $fwData   = substr($fwFileReceived, 12);
        
        $productModelReceived = substr($fwHeader,3,9);
        
        //get device type - TX or RX
        $settings   = new Settings();
        $deviceType = $settings->getDeviceType();
                
        if ($deviceType == 'TX')
        {
            $device = Params::FPGA_UPGRADE_VALIDATION."-TX";
        } else {
            $device = Params::FPGA_UPGRADE_VALIDATION."-RX";
        }
        
        if ($productModelReceived != $device)
        {
            $this->arrFailed["data"] = ["fileToUpload"=>"Firmware is not compatible!"];
            return $response->withJson($this->arrFailed);
        }
        
        // Save the firmware to a file
        $fp = fopen(Params::FIRMWARE_FPGA_FILE, 'w');
        if (!$fp)
        {
            $this->arrFailed["data"] = ["fileToUpload"=>"could not open firmware file!"];
            return $response->withJson($this->arrFailed);
        }
        
        fwrite($fp,$fwData);
        fclose($fp);
        
        //rebooting
        $data = array("system" => array("reboot" => 1));
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
        //just in case muxMgr - jack manager - is not working properly
        $this->arrFailed["data"] = ["reboot" => "manual reboot"];
        return $response->withJson($this->arrFailed);
    }
}
