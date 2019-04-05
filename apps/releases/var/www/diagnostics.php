<?PHP
    include_once './include/Authentication.php';
	//echo '<pre>'; print_r($arrParameters); echo '</pre>';

	if (file_exists(FPGA_LOG))
	{
		if (!$file = fopen(FPGA_LOG, "r"))
		{
			Logger::getInstance()->lwrite('FPGA LOG: File cannot be opened/created');				
		}
		else
		{
			$netmon_text = fread($file, filesize(FPGA_LOG));

			$netmon_rows = preg_split( "/(\\n|\\e|\[2K|\[)/", $netmon_text );
			
			$fpga_log = array();				

			foreach($netmon_rows as $netmon_row)
			{
				$netmon_line = preg_split( "/:/", $netmon_row );
			
				//echo '<pre>'; print_r($netmon_line); echo '</pre>';
				
				if (trim($netmon_line[0]) != "")
				{
					if (!isset($netmon_line[1]))
					{
						$fpga_log[trim($netmon_line[0])] = true;
					}
					else
					{
						$fpga_log[trim($netmon_line[0])] = trim($netmon_line[1]);
					}
				}
			}				
		}		
		fclose($file);
	}	
	//echo '<pre>'; print_r($fpga_log); echo '</pre>';
?>

<DIV class="title">
	<H2>Diagnostics</H2>
</DIV>

<DIV class="fields-info">

	<?php foreach($fpga_log as $fpga_variable => $fpga_result):?>

		<DIV class="field">
			<LABEL for="<?php echo $fpga_variable;?>"><?php echo $fpga_variable;?>:</LABEL>
			<DIV class="label" id="<?php echo $fpga_variable;?>"><?php echo $fpga_result;?></DIV>
		</DIV>
	
	<?php endforeach;?>

</DIV>
