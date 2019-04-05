<?php

    if (isset($_POST['submit']))
    {     
		if ($_POST['submit'] == "Login")
		{			
			require_once('./include/GlobalConfiguration.inc');		
			require_once ('./include/UserLoginFile.inc');
			$login = new UserLoginFile();

			$login->fileLogin((string)$_POST['username'],(string)$_POST['password']);
			if($login->isAuthenticated())
			{
				header("Location: ./index.php?link=info&message=".urlencode("Logged in"));
				//echo "<SCRIPT type='text/javascript'>submit_main('info.php','Logged in');</script>";
				die();			
			}
			else
			{
				header("Location: ./index.php?link=userlogin&message=".urlencode("Bad Username or Password"));				
				die();							
			}

		}
    }    
?>

<DIV id="forms">
	<FORM method="post" id="formEncoder" action="userlogin.php">
		<DIV class="form">
			<DIV class="title">
				<H2>User Login</H2>
			</DIV>
			<DIV class="fields">

				<DIV class="field">
					<LABEL for="username">Username:</LABEL> 
					<INPUT name="username" id="username" type="text" size="40" value="">
				</DIV>

				<DIV class="field">
					<LABEL for="password">Password:</LABEL> 
					<INPUT name="password" id="password" type="password" size="40" value="">
				</DIV>
				
				<DIV class="field">
					<INPUT name="submit" type="submit" class="btnSubmit" value="Login" />
				</DIV>

			</DIV>
		</DIV>
	</FORM>
</DIV>

