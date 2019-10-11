<?php

namespace App\Controllers;

/**
* 
*/
class Controller
{
	protected $container;
	protected $arrSuccess;
	protected $arrFailed;

	public function __construct($container)
	{
	    $this->arrSuccess = array("status" => "success", "data" => array());
	    $this->arrFailed = array("status" => "failed", "data" => array());
	    $this->container = $container;
	}

	public function __get($property)
	{
		if ($this->container->{$property}) {
			return $this->container->{$property};
		}
	}
}