<?php

namespace App\Middleware;

/**
*
*/
class AuthMiddleware extends Middleware
{

	public function __invoke($request,$response,$next)
	{
	    if(!$this->container->UserLogin->validateToken($request->getHeader('Authorization')))
	    {
            //return json failed
	        return $response->withJson(array("status" => "failed"), 401);
	    }

		$response = $next($request,$response);
		return $response;

	}
}