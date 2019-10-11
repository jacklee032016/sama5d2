<!doctype html>
<HTML lang="en">
<HEAD>
    <TITLE>Muxlab 500774</TITLE>
    <META charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
 
    <LINK href="css/bootstrap.css" type="text/css" rel="stylesheet">
    <LINK href="css/styles.css" type="text/css" rel="stylesheet">
    <LINK href="css/9-cube-grid.css" type="text/css" rel="stylesheet">
    <LINK href="external_libraries/jquery-UI/jquery-ui.min.css" type="text/css" rel="stylesheet">
    <LINK href="css/jquery.timepicker.min.css" type="text/css" rel="stylesheet">
    <LINK href="css/boostrap-button-toggle.css" type="text/css" rel="stylesheet">
</HEAD>

<BODY>
	<DIV id="body">
		<DIV id="header">
			<a id="logo" href="./index.php?link=info">
			<img alt="Muxlab Control Panel" src="images/logo.jpg"></a>  <br/>
			<div data-text="dt_productName" id="id_mainProductName"> </div>
		</DIV>
			<DIV id="blink">
                <button type="button" class="btn btn-primary btn-sm" onClick="javascript:blink('true');">Blink LED</button>
			</DIV>
		<DIV id="nav">
           	<a data-text="system-info" id="nav_info" class="" href="javascript:submit_main('info.html');">System Info</a>
<!--        		<a data-text="Decoder" id="nav_decoder" class="" href="javascript:submit_main('decoder.html');" >Decoder</a>  -->
        	<a data-text="media" id="nav_media" class="" href="javascript:submit_main('media.html');">Media</a>	
<!--             <a data-text="Script" id="nav_script" class="" href="javascript:submit_main('script.html');">Script</a>    --> 
<!--             <a data-text="Osd" id="nav_osd" class="" href="javascript:submit_main('osd.html');">Overlay</a> -->
<!--             <a data-text="Osd" id="nav_osd" class="" href="javascript:submit_main('layout.html');">Layout</a>  -->
<!--           	<a data-text="Network" id="nav_network" class="" href="javascript:submit_main('network.html');">Network</a> -->
        	<a data-text="rs232" id="nav_rs232" class="" href="javascript:submit_main('rs232.html');">RS-232</a>
        	<a data-text="settings" id="nav_settings" class="" href="javascript:submit_main('settings.html');">Settings</a> 
        	<!-- <a data-text="log-out" id="nav_logout" class="" href="javascript:signOut();" >Log Out</a>  -->
		</DIV>
		<DIV id="message"></DIV>
		<DIV id="loading-image">		
			<div class="sk-cube-grid">
				<div class="sk-cube sk-cube1"></div>
				<div class="sk-cube sk-cube2"></div>
				<div class="sk-cube sk-cube3"></div>
				<div class="sk-cube sk-cube4"></div>
				<div class="sk-cube sk-cube5"></div>
				<div class="sk-cube sk-cube6"></div>
				<div class="sk-cube sk-cube7"></div>
				<div class="sk-cube sk-cube8"></div>
				<div class="sk-cube sk-cube9"></div>
			</div>
		</DIV>	
		<DIV id="content"></DIV>
		<DIV id="footer"></DIV>
	</DIV>
	<SCRIPT src="js/jquery-3.2.1.min.js" type="text/javascript"></SCRIPT>
	<SCRIPT src="js/jquery.cookie.js" type="text/javascript"></SCRIPT>
	<SCRIPT src="js/popper.min.js" type="text/javascript"></SCRIPT>
	<SCRIPT src="js/bootstrap.js" type="text/javascript"></SCRIPT>
	<SCRIPT src="js/jquery.timepicker.min.js" type="text/javascript"></SCRIPT>
    <SCRIPT src="external_libraries/jquery-UI/jquery-ui.min.js" type="text/javascript"></SCRIPT>
	<script src="js/index.js" type="text/javascript"></script>
</BODY>
</HTML>