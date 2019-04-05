<?php 
	require_once('./include/GlobalConfiguration.inc');
?> 

<a id="logo" href="./index.php?link=info">
<img alt="Muxlab Control Panel" src="images/logo.jpg"></a>  <br />

	<?php if (GlobalFunctions::isDeviceTX()) : ?>
		<div data-text="dt_productName" id="id_mainProductName">HDMI OVER IP H.264/H.265 POE EXTENDER (TX-500774)</div>
	<?php elseif (GlobalFunctions::isDeviceRX()) : ?>
		<div data-text="dt_productName" id="id_mainProductName">HDMI OVER IP H.264/H.265 POE EXTENDER (RX-500774)</div>	
	<?php else : ?>
	<?php endif; ?>