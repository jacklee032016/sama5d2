<?php 
	require_once('./include/GlobalConfiguration.inc');
?> 

<div id="nav">
	<a data-text="Info" id="nav_info" class="" href="javascript:submit_main('info.php');" >Info</a>

	<?php if (GlobalFunctions::isDeviceTX()) : ?>
		<a data-text="Encoder" id="nav_encoder" class="" href="javascript:submit_main('encoder.php');" >Encoder</a>
	<?php elseif (GlobalFunctions::isDeviceRX()) : ?>
		<a data-text="Decoder" id="nav_decoder" class="" href="javascript:submit_main('decoder.php');" >Decoder</a>	
	<?php else : ?>
	<?php endif; ?>	

	<a data-text="Diagnostics" id="nav_diagnostics" class="" href="javascript:submit_main('diagnostics.php');" >Diagnostics</a>	
	<a data-text="Network" id="nav_network" class="" href="javascript:submit_main('network.php');" >Network</a>
	<a data-text="Settings" id="nav_settings" class="" href="javascript:submit_main('settings.php');" >Settings</a>
	<a data-text="Log Out" id="nav_logout" class="" href="javascript:submit_main('logout.php');" >Log Out</a>	
</div>
