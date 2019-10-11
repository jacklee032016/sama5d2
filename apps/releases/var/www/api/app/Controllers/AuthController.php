<?php 

namespace App\Controllers;

use App\Models\UserLogin;
use Respect\Validation\Validator as v;
use App\Params;

/**
 * @SWG\Tag(
 *   name="Login",
 *   description="Related to login, logout (authentication)",
 * )
 */ 

class AuthController extends Controller
{
    
    /**
     * @SWG\Post(
     *     path="/auth/signin",
     *     summary="User login",
     *     tags={"Login"},
     *     description="basic authentication by token, 1 hour idle token expiration.",
     *     operationId="signin",
     *     produces={"application/json"},
     *     @SWG\Parameter(
     *         name="body",
     *         in="body",
     *         required=true,
     *         @SWG\Schema(ref="#/definitions/UserLogin")
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
     *                         @SWG\Property(
     *                             property="token",
     *                             type="string",
     *                             example="963c6f33e8d7643eb5301c6c630effce"
     *                         )
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
    public function signIn($request, $response)
    { 
        $validation = $this->Validator->validate($request, [
            "username" => v::notEmpty(),
            "password" => v::notEmpty()
        ]);
        
        if ($validation->failed())
        {
            $this->arrFailed['data'] = $validation->getErrors();
            return $response->withJson($this->arrFailed);
        }
        
        $isLogged = $this->UserLogin->login(
            $request->getParam('username'),
            $request->getParam('password')
        );
        
        if ($isLogged)
        {
            //set token expiration: one hour
            $this->UserLogin->setTokenExpiration(Params::TOKEN_EXPIRATION_TO_ONE_HOUR);
            
            //return json response with token
            $this->arrSuccess['data'] = array("token" => $this->UserLogin->getToken());
            return $response->withJson($this->arrSuccess);
        }
        else
        {
            //return json failed response
            return $response->withJson($this->arrFailed, 401);
        }
    }
    
    /**
     * @SWG\Post(
     *     path="/auth/change-password",
     *     summary="Change user password",
     *     tags={"Login"},
     *     description="Change user password",
     *     operationId="change-password",
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
     *              required={"password_old", "password"},
     *              @SWG\Property(
     *                  property="password_old",
     *                  type="string",
     *                  example="admin2"
     *              ),
     *              @SWG\Property(
     *                  property="password",
     *                  type="string",
     *                  example="admin"
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
     *                         @SWG\Property(
     *                             property="token",
     *                             type="string",
     *                             example="963c6f33e8d7643eb5301c6c630effce"
     *                         )
     *                  )
     *              )
     *         ),
     *     ),
     *     @SWG\Response(
     *         response=201,
     *         description="validation failed - OBS: the code response is 200 but swagger does not allow multiple response schemas for the same code",
     *         @SWG\Schema(
     *              @SWG\Property(
     *                  property="status",
     *                  type="string",
     *                  example="failed"
     *              ),
     *              @SWG\Property(
     *                  property="data",
     *                  type="array",
     *                  @SWG\Items(
     *                         @SWG\Property(
     *                             property="password_old",
     *                             type="string",
     *                             example="Old password does not match"
     *                         )
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
    public function changePassword($request, $response)
    {
        $validation = $this->Validator->validate($request, [
            "password_old" => v::noWhitespace()->notEmpty()->matchesPassword($this->UserLogin->getPassword()),
            "password"     => v::noWhitespace()->notEmpty()
        ]);

        if ($validation->failed())
        {
            //return json failed
            $this->arrFailed['data'] = $validation->getErrors();
            return $response->withJson($this->arrFailed);
        }
        
        $this->UserLogin->setPassword($request->getParam("password"));
        
        $this->arrSuccess['data'] = array("token" => $this->UserLogin->getToken());
        return $response->withJson($this->arrSuccess);
    }
    
    /**
     * @SWG\Post(
     *     path="/auth/signout",
     *     summary="User logout",
     *     tags={"Login"},
     *     description="User logout.",
     *     operationId="sighout",
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
     *                  @SWG\Items()
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
    public function signOut($request, $response)
    {
        //token will be expired, force new login
        $this->UserLogin->setTokenExpiration(Params::TOKEN_EXPIRATION_TO_NOW);
         
        return $response->withJson($this->arrSuccess);
    }
}

