<?php 
	require_once('./include/GlobalConfiguration.inc');

	if (isset($_GET['reboot'])) 
	{
		$url_reboot = $_GET['reboot'];
		if ($url_reboot == 1)
	 	{
			//require_once('./include/GlobalConfiguration.inc');
			require_once('./include/UserLoginFile.inc');
			$login = new UserLoginFile();
			if ($login->isAuthenticated())	
			{
				require_once(MUXPROTOCOL_FILE);
				Logger::getInstance()->disableLogger(); // avoid logging write error message
				$currentMessage = new MUXProtocol((string)$login->json_username, (string)$login->json_password);
				$test = $currentMessage->cmd_reboot();
			}
		} 
	}
?> 

<HTML>
<HEAD>
<TITLE>Muxlab <?php echo DEVICE_NAME;?></TITLE>
<META charset="utf-8">

<LINK href="css/styles.css?v=2018.02.20" type="text/css" rel="stylesheet">
<LINK href="css/9-cube-grid.css" type="text/css" rel="stylesheet">

<SCRIPT src="js/jquery-3.2.1.min.js" type="text/javascript"></SCRIPT>
</HEAD>

<BODY>
	<DIV id="body">
		<DIV id="header"></DIV>
			<DIV id="blink"><INPUT name="blink_button" type="button" value="Blink LED" onClick="javascript:submit_blink('true');"></DIV>
		<DIV id="nav"></DIV>
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

	<SCRIPT type="text/javascript">
		$("#header").load("header.php");
		$("#nav").load("nav.php");
		$("#footer").load("footer.php");
	</SCRIPT>

	<SCRIPT type="text/javascript">

		//debugger;

		var $loading = $('#loading-image').hide();
		$(document)
			.ajaxStart(function () {
				$loading.show();
			})
		.ajaxStop(function () {
			$loading.hide();
		});

		function submit_blink(value) 
		{
			//value = (value == 'true');

			$.ajax({
				method: "POST",
				url: "blink.php",
				data: { "value" : value },
				success: function(data) {

					if (data == 'TRUE') 
					{
						$("#blink input[type=button]").attr("onClick","javascript:submit_blink('false');");
						$("#blink input[type=button]").addClass("blink-button");
						$("#blink input[type=button]").attr("value","Turn OFF");
					}
					else 
					{
						$("#blink input[type=button]").attr("onClick","javascript:submit_blink('true');");
						$("#blink input[type=button]").removeClass("blink-button");
						$("#blink input[type=button]").attr("value","Blink LED");
					}
				},
				error: function() {
					//alert('Error occured');
				}
			});
		}

		function submit_main(pageOrSimpleTask,message=null) 
		{
			$("#content").load(pageOrSimpleTask);

			//bold menu items			
			$('.bolder').each(function(i, obj) {
				$(obj).attr("class","");
			});
			var nav_link = pageOrSimpleTask.replace(".php", "");
			nav_link = "#nav_" + nav_link;
			$(nav_link).attr("class","bolder");

			//set message
			if (message == null) message = getUrlParameter('message');
			if (message)
			{
				document.getElementById('message').style.display = "block";
				document.getElementById('message').innerHTML = urldecode(message);
			}		
			else
			{
				document.getElementById('message').style.display = "none";
				document.getElementById('message').innerHTML = null;	
			}

    		// remove url parameters
    		var uri = window.location.toString();
    		if (uri.indexOf("?") > 0) 
    		{
    			var clean_uri = uri.substring(0, uri.indexOf("?"));
    			window.history.replaceState({}, document.title, clean_uri);
    		}   		
		}

		function urldecode(url) {
			return decodeURIComponent(url.replace(/\+/g, ' '));
		}		

		var getUrlParameter = function getUrlParameter(sParam) {
			var sPageURL = decodeURIComponent(window.location.search.substring(1)),
				sURLVariables = sPageURL.split('&'),
				sParameterName,
				i;

			for (i = 0; i < sURLVariables.length; i++) {
				sParameterName = sURLVariables[i].split('=');

				if (sParameterName[0] === sParam) {
					return sParameterName[1] === undefined ? true : sParameterName[1];
				}
			}
		};

		var link = getUrlParameter('link');
		if (!link)
		{
			submit_main('info.php');
		}
		else
		{
			switch(link)
			{
    			case 'userlogin':
    				submit_main('userlogin.php');
    				break;		
				case 'info':
					submit_main('info.php');
					break;
				case 'status':
					submit_main('info.php');
					break;
				case 'encoder':
					submit_main('encoder.php');
					break;
				case 'decoder':
					submit_main('decoder.php');
					break;					
				case 'network':
					submit_main('network.php');
					break;				
				case 'settings':
					submit_main('settings.php');
					break;
				case 'logout':
					submit_main('logout.php');
					break;		
				default:
					submit_main('userlogin.php');
					break;
			}
		}

	</SCRIPT>

</BODY>

</HTML>