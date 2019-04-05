<?php

	ini_set('display_errors', 1);
	ini_set('display_statup_errors', 1);
	error_reporting(E_ALL | E_STRICT);

	include_once './include/Authentication.php';
	$login->LogOut();
	
?>

<SCRIPT type='text/javascript'>$('#blink').css('visibility', 'hidden').css( 'display','none');</script>

<DIV class="title">
	<H2 style="text-align:center;">Logged out</H2>
</DIV>
