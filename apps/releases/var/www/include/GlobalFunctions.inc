<?php
class GlobalFunctions
{
    
    static function isDeviceTX()
    {
        if (DEVICE_NAME == "TX-500768") return true;
        else return false;        
    }
    
    static function isDeviceRX()
    {
        if (DEVICE_NAME == "RX-500768") return true;
        else return false;
    }

	static function getRealIpAddr()
	{
		if (!empty($_SERVER['HTTP_CLIENT_IP']))   //check ip from share internet
		{
			$ip=$_SERVER['HTTP_CLIENT_IP'];
		}
		elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR']))   //to check ip is pass from proxy
		{
			$ip=$_SERVER['HTTP_X_FORWARDED_FOR'];
		}
		else
		{
			$ip=$_SERVER['REMOTE_ADDR'];
		}
		return $ip;
	}
    
	static function generateNewFormToken()
	{
		$token = md5(uniqid(rand(), true));
		PersistantVariables::getInstance()->MyContext->set_FormToken($token);
		return $token;
	}
	
	static function getCurrentFormToken()
	{
		return PersistantVariables::getInstance()->MyContext->get_FormToken();
	}
	
	static function isFormTokenValide($formToken)
	{
		if(empty($formToken))
			return FALSE;
					
		if( PersistantVariables::getInstance()->MyContext->get_FormToken() == $formToken)
		{
			return TRUE;
		}
	
		return FALSE;
	}	
	
	static function destroySessionCompletly(){
	
		// kill session variables
		session_unset();  // Same as $_SESSION = array(); // reset session array
	
		$_POST = array(); // reset
	
		// delete session cookie
		$params = session_get_cookie_params();
		setcookie(session_name(), '', time() - 36000, $params["path"], $params["domain"], $params["secure"], $params["httponly"] );
	
		// destroy session.
		session_destroy();
	}
		
	static function validateEmailInputIntegrity($lenghtMin, $lenghtMax, $emailInput) {
	
		if (strip_tags($emailInput) != $emailInput)
			return FALSE;
			
		//$email_pattern = '/^[^@\s<&>]+@([-a-z0-9]+\.)+[a-z]{2,}$/i';
		$email_pattern = '/^\w([-_.]?\w)*@\w([-_.]?\w)*\.([a-zA-Z]{2,4})$/';
		
		if(!preg_match($email_pattern, $emailInput)){
			return FALSE;
		}

		if ( (strlen($emailInput)< $lenghtMin) || (strlen($emailInput) > $lenghtMax) ){
			return FALSE;	
		}
		
		return TRUE;
	}
	
	/**
	 * 
	 * Validate the password input format that shall be as followed:
	 * Shall contain at least 1 letter and 1 digit
	 * @param unknown_type $lenghtMin
	 * @param unknown_type $lenghtMax
	 * @param unknown_type $passwordInput
	 */
	static function validatePasswordInputIntegrity($lenghtMin, $lenghtMax, $passwordInput) {

		if (strip_tags($passwordInput) != $passwordInput)
			return FALSE;
			
		// contain at least 1 lower case letter, 1 Upercase letter and 1 digit	
		//$password_pattern = '/^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])[a-zA-Z0-9]{' .$lenghtMin .',' .$lenghtMax .'}$/';

		// Shall contain at least 1 letter and 1 digit	
		$password_pattern = '/^(?=.*[0-9])(?=.*[a-zA-Z])[a-zA-Z0-9]{' .$lenghtMin .',' .$lenghtMax .'}$/';
			
			
		if(!preg_match($password_pattern, $passwordInput)){
			return FALSE;
		}
		
		return TRUE;
	}
	
	/**
	 * 
	 * Validate a regular address input format that shall accept the following characters:
	 * 'a' to 'z', 'A' to 'Z', �� ���'-.
	 * @param unknown_type $lenghtMin
	 * @param unknown_type $lenghtMax
	 * @param unknown_type $addressInput
	 */
	static function validateAddressInputIntegrity($lenghtMin, $lenghtMax, $addressInput) {

		$address_pattern = '/^[0-9](?!.*(--))([-0-9\'a-z A-Z��,��������\.]){' .($lenghtMin-2) .',' .($lenghtMax-2) .'}[0-9a-zA-Z���������\.]$/';
		
		if (strip_tags($addressInput) != $addressInput){
			Logger::getInstance()->lwrite('ERROR: address Input strip_tags failed');
			return FALSE;
		}
		
		// In Go Daddy host the apostrophe is ignore so make this work around
		$addressInputCopy = stripslashes($addressInput); 
	
		if(!preg_match($address_pattern, $addressInputCopy)){
			Logger::getInstance()->lwrite('ERROR: address Input preg_match failed');
			return FALSE;
		}
	
		return TRUE;
	}
	
	static function validateDateInputIntegrity($dateInput) {
		//$arr=split("-",$dateInput); // splitting the array
		$arr=explode("-",$dateInput); // splitting the array
		if (($arr == false) || (count($arr) == 1))
			return false;
		$yyyy=$arr[0]; // first element of the array is year
		$mm=$arr[1]; // second element is month 
		$dd=$arr[2]; // third element is date
		if (count($arr) > 3)
			return false;
		if(!checkdate($mm,$dd,$yyyy))
			return false;
		if ( $yyyy > 2200 )
			return false;
		else
			return true;
	}
	
	static function validateFirsLasttNameInputIntegrity($lenghtMin, $lenghtMax, $firstLastNameInput) {
		
		if (strip_tags($firstLastNameInput) != $firstLastNameInput)
			return FALSE;

		// In Go Daddy host the single quote is backslashed so make this work around
		$firstLastNameInputCopy = stripslashes($firstLastNameInput); 
			
		$firstLastNameInputLenght = strlen($firstLastNameInputCopy);
		
		if ( ($firstLastNameInputLenght == 0) && ($lenghtMin == 0) )
			return TRUE;
		elseif ( ($firstLastNameInputLenght == 1) && ($lenghtMin <= 1) && ($lenghtMax >= 1))
			$firstLastName_pattern = '/^[a-zA-Z����������]$/';
		elseif ( ($firstLastNameInputLenght == 2) && ($lenghtMin <= 2) && ($lenghtMax >= 2))
			$firstLastName_pattern = '/^[a-zA-Z����������][-a-z A-Z���������\.�\']$/';
		else{
			if ($lenghtMin >= 2)
				$lenghtMin = $lenghtMin -2;
			if ($lenghtMax >= 2)
				$lenghtMax = $lenghtMax -2;				
			$firstLastName_pattern = '/^[a-zA-Z����������](?!.*(--))([-a-z A-Z���������\.�\']){' .$lenghtMin .',' .$lenghtMax .'}[a-zA-Z���������\.]$/';
		}

		
		if(!preg_match($firstLastName_pattern, $firstLastNameInputCopy)){
			return FALSE;
		}
		
		return TRUE;
	}
	
	static function validateBusinessNameInputIntegrity($lenghtMin, $lenghtMax, $businessNameInput) {
		
		if (strip_tags($businessNameInput) != $businessNameInput)
			return FALSE;

		// In Go Daddy host the single quote is backslashed so make this work around
		$businessNameInputCopy = stripslashes($businessNameInput); 
			
		$businessNameInputLenght = strlen($businessNameInputCopy);
		
		if ( ($businessNameInputLenght == 0) && ($lenghtMin == 0) )
			return TRUE;
		elseif ( ($businessNameInputLenght == 1) && ($lenghtMin <= 1) && ($lenghtMax >= 1))
			$businessName_pattern = '/^[a-zA-Z0-9����������]$/';
		elseif ( ($businessNameInputLenght == 2) && ($lenghtMin <= 2) && ($lenghtMax >= 2))
			$businessName_pattern = '/^[a-zA-Z0-9����������][-a-z A-Z&���������\,\.�\']$/';
		else{
			if ($lenghtMin >= 2)
				$lenghtMin = $lenghtMin -2;
			if ($lenghtMax >= 2)
				$lenghtMax = $lenghtMax -2;				
			$businessName_pattern = '/^[a-zA-Z0-9����������](?!.*(--))([-a-z A-Z0-9&���������\,\.�\']){' .$lenghtMin .',' .$lenghtMax .'}[a-zA-Z0-9&��������\.�]$/';
		}

		
		if(!preg_match($businessName_pattern, $businessNameInputCopy)){
			return FALSE;
		}
		
		return TRUE;
	}
	
	static function validateLetterAndDigitInputIntegrity($lenghtMin, $lenghtMax, $letterAndDiditString) {
		
		if (strip_tags($letterAndDiditString) != $letterAndDiditString)
			return FALSE;
		
		$letterAndDiditStringLenght = strlen($letterAndDiditString);
		
		if ( ($letterAndDiditStringLenght == 0) && ($lenghtMin == 0) )
			return TRUE;
		elseif ( ($letterAndDiditStringLenght == 1) && ($lenghtMin <= 1) && ($lenghtMax >= 1))			
			$LetterAndDigit_pattern = '/^[a-zA-Z0-9]$/';
		elseif ( ($letterAndDiditStringLenght == 2) && ($lenghtMin <= 2) && ($lenghtMax >= 2))
			$LetterAndDigit_pattern = '/^[a-zA-Z0-9][a-zA-Z0-9]$/';
		else{
			if ($lenghtMin >= 2)
				$lenghtMin = $lenghtMin -2;
			if ($lenghtMax >= 2)
				$lenghtMax = $lenghtMax -2;
			$LetterAndDigit_pattern = '/^[a-zA-Z0-9](?!.*(--))([a-zA-Z0-9]){' .$lenghtMin .',' .$lenghtMax .'}[a-zA-Z0-9]$/';
		}
			
		if(!preg_match($LetterAndDigit_pattern, $letterAndDiditString)){
			return FALSE;
		}
		
		return TRUE;
	}
	
	static function validateLetterAndDigitAndSpaceInputIntegrity($lenghtMin, $lenghtMax, $letterAndDiditAndSpaceString) {
		
		if (strip_tags($letterAndDiditAndSpaceString) != $letterAndDiditAndSpaceString)
			return FALSE;
			
		$letterAndDiditAndSpaceStringLenght = strlen($letterAndDiditAndSpaceString);
		
		if ( ($letterAndDiditAndSpaceStringLenght == 0) && ($lenghtMin == 0) )
			return TRUE;
		elseif ( ($letterAndDiditAndSpaceStringLenght == 1) && ($lenghtMin <= 1) && ($lenghtMax >= 1))
			$LetterAndDigit_pattern = '/^[a-zA-Z0-9]$/';
		elseif ( ($letterAndDiditAndSpaceStringLenght == 2) && ($lenghtMin <= 2) && ($lenghtMax >= 2))
			$LetterAndDigit_pattern = '/^[a-zA-Z0-9][a-zA-Z 0-9]$/';
		else{
			if ($lenghtMin >= 2)
				$lenghtMin = $lenghtMin -2;
			if ($lenghtMax >= 2)
				$lenghtMax = $lenghtMax -2;
			$LetterAndDigit_pattern = '/^[a-zA-Z0-9](?!.*(--))([a-zA-Z 0-9]){' .$lenghtMin .',' .$lenghtMax .'}[a-zA-Z0-9]$/';
		}
			
		if(!preg_match($LetterAndDigit_pattern, $letterAndDiditAndSpaceString)){
			return FALSE;
		}
		
		return TRUE;
	}
	
	static function customValidationInputIntegrity($lenghtMin, $lenghtMax, $stringToValidate, $regexCharactersAllowed) {
		
		if (strip_tags($stringToValidate) != $stringToValidate)
			return FALSE;
			
		$letterAndDiditAndSpaceStringLenght = strlen($stringToValidate);
		
		if ( ($letterAndDiditAndSpaceStringLenght == 0) && ($lenghtMin == 0) )
			return TRUE;
		elseif ( ($letterAndDiditAndSpaceStringLenght == 1) && ($lenghtMin <= 1) && ($lenghtMax >= 1))
			$LetterAndDigit_pattern = '/^[' .$regexCharactersAllowed .']$/';
		elseif ( ($letterAndDiditAndSpaceStringLenght == 2) && ($lenghtMin <= 2) && ($lenghtMax >= 2))
			$LetterAndDigit_pattern = '/^[' .$regexCharactersAllowed .'][' .$regexCharactersAllowed .']$/';
		else{
			if ($lenghtMin >= 2)
				$lenghtMin = $lenghtMin -2;
			if ($lenghtMax >= 2)
				$lenghtMax = $lenghtMax -2;
			$LetterAndDigit_pattern = '/^[' .$regexCharactersAllowed .'](?!.*(--))([' .$regexCharactersAllowed .']){' .$lenghtMin .',' .$lenghtMax .'}[' .$regexCharactersAllowed .']$/';
		}
			
		if(!preg_match($LetterAndDigit_pattern, $stringToValidate)){
			return FALSE;
		}
		
		return TRUE;
	}
	
	static function validateGeneralTextInputIntegrity($lenghtMin, $lenghtMax, $textInput) {

		//$text_pattern = '/^[0-9a-zA-Z�����](?!.*(--))([-0-9\'a-z A-Z���\\�\/�\.]){' .($lenghtMin-2) .',' .($lenghtMax-2) .'}[0-9a-zA-Z�����]$/';
		
		if (strip_tags($textInput) != $textInput){
			Logger::getInstance()->lwrite('ERROR: textInput Input strip_tags failed');
			return FALSE;
		}
		
		// In Go Daddy host the single quote is backslashed so make this work around
		$textInputCopy = stripslashes($textInput);
		
		$textInputLenght = strlen($textInputCopy);
		
		if ( ($textInputLenght == 0) && ($lenghtMin == 0) )
			return TRUE;
		elseif ( ($textInputLenght == 1) && ($lenghtMin <= 1) && ($lenghtMax >= 1))
			$text_pattern = '/^[0-9a-zA-Z��������$�]$/';
		elseif ( ($textInputLenght == 2) && ($lenghtMin <= 2) && ($lenghtMax >= 2))
			$text_pattern = '/^[-0-9a-zA-Z+(��������$�][0-9a-z A-Z(��������\.$�]$/';
		else{
			if ($lenghtMin >= 2)
				$lenghtMin = $lenghtMin -2;
			if ($lenghtMax >= 2)
				$lenghtMax = $lenghtMax -2;
			$text_pattern = '/^[-0-9a-zA-Z+(��������$�](?!.*(--))([-0-9\'a-z A-Z()+������\.�\\�\/$�\.]){' .$lenghtMin .',' .$lenghtMax .'}[0-9a-zA-Z)��������\.$�]$/';
		}
		
		if(!preg_match($text_pattern, $textInputCopy)){
			Logger::getInstance()->lwrite('ERROR: textInput Input preg_match failed');
			Logger::getInstance()->lwrite('ERROR: $text_pattern = ' .$text_pattern);
			Logger::getInstance()->lwrite('ERROR: $textInputCopy = ' .$textInputCopy);
			return FALSE;
		}
	
		return TRUE;
	}
	
	static function validatePostalCodeInputIntegrity($lenghtMin, $lenghtMax, $postalCodeInput, $country) {

		if (strip_tags($postalCodeInput) != $postalCodeInput)
			return FALSE;

		// In Go Daddy host the single quote is backslashed so make this work around
		$country = stripslashes($country);
		
		switch ($country) {

	 		case 'Canada':
	 			$postalCode_pattern = '/^([a-zA-Z])([0-9])([a-zA-Z])([ ])([0-9])([a-zA-Z])([0-9])$/';
	 			break;
	 		case 'United State':
	 			$postalCode_pattern = '/^[a-zA-Z0-9](?!.*(--))([-a-z A-Z0-9]){' .($lenghtMin-2) .',' .($lenghtMax-2) .'}[a-zA-Z0-9]$/';
	 			break;
	 		default:
	 			$postalCode_pattern = '/^[a-zA-Z0-9](?!.*(--))([-a-z A-Z0-9]){' .($lenghtMin-2) .',' .($lenghtMax-2) .'}[a-zA-Z0-9]$/';
	 			break;
		}
		
				
		if(!preg_match($postalCode_pattern, $postalCodeInput)){
			return FALSE;
		}
		
		return TRUE;
	}
	
	static function formatPostalCode($postalCodeInput, $country) {

		if (strip_tags($postalCodeInput) != $postalCodeInput)
			return FALSE;

		// In Go Daddy host the single quote is backslashed so make this work around
		$country = stripslashes($country);
		
		switch ($country) {

	 		case 'Canada':
				// Remove all the <space> and - inside the number
	 			$toSearch  = array(" ", "-");
	 			$toReplace = array("", "");
	 			$tempPostalCodeInput = str_replace($toSearch, $toReplace, $postalCodeInput);
	 			if (strlen($tempPostalCodeInput) == 6 ){
	 				$temp = '' .substr($tempPostalCodeInput, 0, 3) .' ' .substr($tempPostalCodeInput, 3, 3);
	 				return strtoupper($temp);
	 			}
	 			break;
	 		case 'United State':
	 			return strtoupper($postalCodeInput);
	 			break;
	 		default:
	 			return strtoupper($postalCodeInput);
	 			break;
		}

		return strtoupper($postalCodeInput);
	}	
	
	static function formatPhoneNumber($phoneNumberInput, $country) {

		if (strip_tags($phoneNumberInput) != $phoneNumberInput)
			return FALSE;

		// In Go Daddy host the single quote is backslashed so make this work around
		$country = stripslashes($country);
		
		switch ($country) {

	 		case 'Canada':
	 			// Remove all the <space> and - inside the number
	 			$toSearch  = array(" ", "-", ".");
	 			$toReplace = array("", "");
	 			$tempPhoneNumberInput = str_replace($toSearch, $toReplace, $phoneNumberInput);
	 			if (strlen($tempPhoneNumberInput) == 10 ){
	 				$temp = '' .substr($tempPhoneNumberInput, 0, 3) .'-' .substr($tempPhoneNumberInput, 3, 3) .'-' .substr($tempPhoneNumberInput, 6, 4);
	 				return $temp;
	 			}
	 			else if (strlen($tempPhoneNumberInput) == 11 ){
	 				$temp = '' .substr($tempPhoneNumberInput, 0, 1) .'-' .substr($tempPhoneNumberInput, 1, 3) .'-' .substr($tempPhoneNumberInput, 4, 3) .'-' .substr($tempPhoneNumberInput, 7, 4);
	 				return $temp;
	 			}
	 			break;
	 		case 'United State':
	 			// Remove all the <space> and - inside the number
	 			$toSearch  = array(" ", "-", ".");
	 			$toReplace = array("", "");
	 			$tempPhoneNumberInput = str_replace($toSearch, $toReplace, $phoneNumberInput);
	 			if (strlen($tempPhoneNumberInput) == 10 ){
	 				$temp = '' .substr($tempPhoneNumberInput, 0, 3) .'-' .substr($tempPhoneNumberInput, 3, 3) .'-' .substr($tempPhoneNumberInput, 6, 4);
	 				return $temp;
	 			}
	 			else if (strlen($tempPhoneNumberInput) == 11 ){
	 				$temp = '' .substr($tempPhoneNumberInput, 0, 1) .'-' .substr($tempPhoneNumberInput, 1, 3) .'-' .substr($tempPhoneNumberInput, 4, 3) .'-' .substr($tempPhoneNumberInput, 7, 4);
	 				return $temp;
	 			}
	 			break;
	 		default:
	 			return $phoneNumberInput;
	 			break;
		}

		return $phoneNumberInput;
	}	
	
	static function formatPersonName($personName) {

		if (strip_tags($personName) != $personName)
			return FALSE;

		// In Go Daddy host the single quote is backslashed so make this work around
		$personName = stripslashes($personName);
		
		$personNameFormatted = '';
		
		// Separate text using space separator
		$wordList = explode(" ", $personName);
		for ($i = 0; $i < count($wordList); $i++)
		{
			// Put all word to LOWER then Put first Letter to Upper
			$wordList[$i] = ucfirst(strtolower($wordList[$i]));
			
			if ($i != 0)
				$personNameFormatted = $personNameFormatted .' ';
				
			$personNameFormatted = $personNameFormatted .$wordList[$i];
		}
	
		return $personNameFormatted;
	}	
	
	static function formatAdress($address) {

		if (strip_tags($address) != $address)
			return FALSE;

		// In Go Daddy host the single quote is backslashed so make this work around
		$address = stripslashes($address);
		
		$addressFormatted = '';
		
		// Separate text using space separator
		$wordList = explode(" ", $address);
		for ($i = 0; $i < count($wordList); $i++)
		{
			// Put all word to LOWER then Put first Letter to Upper
			if ($i == 0)
			{
				// Do nothing must be the number of the address
				$addressFormatted = strtolower($wordList[$i]);
			}
			else if ($i == 1)
			{
				$addressFormatted = $addressFormatted .' ' .ucfirst(strtolower($wordList[$i]));
			}
			else 
			{
				$addressFormatted = $addressFormatted .' ' .strtolower($wordList[$i]);
			}
			
		}
	
		return $addressFormatted;
	}	
	
	static function validatePhoneNumberInputIntegrity($phoneNumberInput, $country) {

		if (strip_tags($phoneNumberInput) != $phoneNumberInput)
			return FALSE;

		// In Go Daddy host the single quote is backslashed so make this work around
		$country = stripslashes($country);
		
		switch ($country) {

	 		case 'Canada':
	 			$phoneNumber_pattern1 = '/^([0-9]){3}([-])([0-9]){3}([-])([0-9]){4}$/';
	 			$phoneNumber_pattern2 = '/^([0-9]){1}([-])([0-9]){3}([-])([0-9]){3}([-])([0-9]){4}$/';
	 		case 'United State':
	 			$phoneNumber_pattern1 = '/^([0-9]){3}([-])([0-9]){3}([-])([0-9]){4}$/';
	 			$phoneNumber_pattern2 = '/^([0-9]){1}([-])([0-9]){3}([-])([0-9]){3}([-])([0-9]){4}$/';
	 			break;
	 		default:
	 			$phoneNumber_pattern1 = '/^([0-9]){3}([-])([0-9]){3}([-])([0-9]){4}$/';
	 			$phoneNumber_pattern2 = '/^([0-9]){1}([-])([0-9]){3}([-])([0-9]){3}([-])([0-9]){4}$/';
	 			break;
		}
		
				
		if(!preg_match($phoneNumber_pattern1, $phoneNumberInput)){
			if(!preg_match($phoneNumber_pattern2, $phoneNumberInput)){
				return FALSE;	
			}
		}
		
		return TRUE;
	}
		
	static function validateDigitInputIntegrity($min, $max, $digitInput) {

		if (strip_tags($digitInput) != $digitInput)
			return FALSE;
		
		if ($max == null )
			if ( strlen($digitInput)< $min ){
				return FALSE;	
			}
		else 
			if ( (strlen($digitInput)< $min) || (strlen($digitInput) > $max) ){
				return FALSE;	
			}

		if(!empty($digitInput))
			if (!ctype_digit($digitInput)) {
				return FALSE;
			}
			
		return TRUE;
	
	}

	static function validateIsFloat($strintToEvaluate) {
		
		if (strip_tags($strintToEvaluate) != $strintToEvaluate)
			return FALSE;
				
		$float_pattern = '/[0-9]*\.?[0-9]+/';

		if(!preg_match($float_pattern, $strintToEvaluate)){
			return FALSE;
		}
		
		return TRUE;
	}

	static function validateIsInteger($strintToEvaluate) {
	
	    if (strip_tags($strintToEvaluate) != $strintToEvaluate)
	        return FALSE;

	   if (!ctype_digit($strintToEvaluate)) {
			return FALSE;
		}
	
	    return TRUE;
	}
		
	static function validateAlphaNumericInputIntegrity($lenghtMin, $lenghtMax, $alphaNumericInput) {

		if (strip_tags($alphaNumericInput) != $alphaNumericInput)
			return FALSE;
			
		if (!ctype_alnum($alphaNumericInput)) {
			return FALSE;
		}

		if ( (strlen($alphaNumericInput)< $lenghtMin) || (strlen($alphaNumericInput) > $lenghtMax) ){
			return FALSE;	
		}
		
		return TRUE;
	}
	
	static function validateAlphabeticInputIntegrity($lenghtMin, $lenghtMax, $alphabeticInput) {
		
		if (strip_tags($alphabeticInput) != $alphabeticInput)
			return FALSE;
		
		if (!ctype_alpha($alphabeticInput)) {
			return FALSE;
		}

		if ( (strlen($alphabeticInput)< $lenghtMin) || (strlen($alphabeticInput) > $lenghtMax) ){
			return FALSE;	
		}
		
		return TRUE;
	}

	static function validateCountryInputIntegrity($countryInput) {
		
		if (strip_tags($countryInput) != $countryInput)
			return FALSE;

		// In Go Daddy host the single quote is backslashed so make this work around
		$countryInputCopy = stripslashes($countryInput);
		
		switch ($countryInputCopy) {

	 		case 'Canada':
	 		case 'United State':
	 			return TRUE;
	 			break;
	 		default:
	 			return FALSE;
		}
	}
	

	static function validateThisIdFromThisTable($Id, $Table) {

		if (strip_tags($Id) != $Id)
			return FALSE;

		$counter = 0;
		foreach ($Table as $key => $value) {
			if ($Id == $key)
			return TRUE;
		}
		return FALSE;

	}
	
	static function validateStateInputIntegrity($stateInput, $country) {

		if (strip_tags($stateInput) != $stateInput)
		return FALSE;

		// In Go Daddy host the single quote is backslashed so make this work around
		$countryCopy = stripslashes($country);
		$stateInputCopy = stripslashes($stateInput);

		switch ($countryCopy) {
			case 'Canada':
				switch ($stateInputCopy) {
					case 'Alberta':
					case 'AB':	
					case 'British Columbia':
					case 'BC':
					case 'Manitoba':
					case 'MB':
					case 'New Brunswick':
					case 'NB':
					case 'Newfoundland':
					case 'NL':
					case 'Northwest Territory':
					case 'NT':
					case 'Nova Scotia':
					case 'NS':
					case 'Nunavut':
					case 'NU':
					case 'Ontario':
					case 'ON':
					case 'Prince Edward Island':
					case 'PE':	
					case 'Quebec':
					case 'QC':	
					case 'Saskatchewan':
					case 'SK':	
					case 'Yukon':
					case 'YT':	
						return TRUE;
						break;
					default:
						return FALSE;
				}
			case 'United State':
				switch ($stateInputCopy) {
					case 'Alabama':
					case 'Alaska':
					case 'American Samoa':
					case 'Arizona':
					case 'Arkansas':
					case 'California':
					case 'Colorado':
					case 'Connecticut':
					case 'Delaware':
					case 'District of Columbia':
					case 'Florida':
					case 'Georgia':
					case 'Guam':
					case 'Hawaii':
					case 'Idaho':
					case 'Illinois':
					case 'Indiana':
					case 'Iowa':
					case 'Kansas':
					case 'Kentucky':
					case 'Louisiana':
					case 'Maine':
					case 'Maryland':
					case 'Massachusetts':
					case 'Michigan':
					case 'Minnesota':
					case 'Mississippi':
					case 'Missouri':
					case 'Montana':
					case 'Nebraska':
					case 'Nevada':
					case 'New Hampshire':
					case 'New Jersey':
					case 'New Mexico':
					case 'New York':
					case 'North Carolina':
					case 'North Dakota':
					case 'Northern Marianas Islands ':
					case 'Ohio':
					case 'Oklahoma':
					case 'Oregon':
					case 'Pennsylvania':
					case 'Puerto Rico':
					case 'Rhode Island':
					case 'South Carolina':
					case 'South Dakota':
					case 'Tennessee':
					case 'Texas':
					case 'Utah':
					case 'Vermont':
					case 'Virginia ':
					case 'Virgin Islands ':
					case 'Washington':
					case 'West Virginia':
					case 'Wisconsin':
					case 'Wyoming ':
						return TRUE;
						break;
					default:
						return FALSE;
				}
			default:
				return FALSE;
		}
	}
	
	static function generateRandomSalt($lenght, $isOnlyNumbers = false) {
		
		if( $lenght == "" ) $lenght = 8;

		$chaine = ""; 

		if ($isOnlyNumbers == true)
		    $list = "0123456789";
		else
		    $list = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		
		mt_srand((double)microtime()*1000000);
		$newstring="";

		while( strlen( $newstring )< $lenght ) {
			$newstring .= $list[mt_rand(0, strlen($list)-1)];
		}
		
		return $newstring;
	}
	
	static function generateRandomHexString($lenght) 
	{
		if( $lenght == "" ) $lenght = 8;
	
		$chaine = "";
	
		$list = "ABCDEF0123456789";
		mt_srand((double)microtime()*1000000);
		$newstring="";
	
		while( strlen( $newstring )< $lenght ) {
			$newstring .= $list[mt_rand(0, strlen($list)-1)];
		}
	
		return $newstring;
	}
	
	static function displayFieldTable($table, $fieldName) {
		if (isset ($table[$fieldName]))
			return $table[$fieldName];
		else 
		 return "";
	}

	static function sendMail_Previous($fromName, $fromEmail, $to, $reply, $subject, $textMessage, $attachmentFilePath='', $attachmentFileName='',$attachmentFileExtention='', $attachmentFileinRaw='') {

		if (IS_LOCAL_SERVER == TRUE){
			return true; // for test Purpose
		}
		//-----------------------------------------------
		//GENERE LA FRONTIERE DU MAIL ENTRE TEXTE ET HTML
		//-----------------------------------------------
		$frontiere = '-----=' . md5(uniqid(mt_rand()));

		//-----------------------------------------------
		//HEADERS DU MAIL
		//-----------------------------------------------

		//$headers = 'From: "no_reply@invoicemailbox.com" <no_reply@invoicemailbox.com>'."\n";
	 	$headers = 'From: "'.$fromName .'" <'.$fromEmail.'>'."\n";
	 	//$headers = 'From: "Nom" <'.$email_expediteur.'>'."\n";
	 	//$headers = 'From: "no_reply@invoicemailbox.com" <no_reply@invoicemailbox.com>'."\n";
	 	$headers .= 'Reply-To: <'.$fromEmail.'>'."\n";
	 	$headers .= 'MIME-Version: 1.0'."\n";
	 	$headers .= 'Content-Type: multipart/mixed; boundary="'.$frontiere.'"';

	 	//-----------------------------------------------
	 	//MESSAGE TEXTE
	 	//-----------------------------------------------
	 	$message = 'This is a multi-part message in MIME format.'."\n\n";
	 
	 	$message .= '--'.$frontiere."\n";
	 	$message .= 'Content-Type: text/plain; charset="iso-8859-1"'."\n";
	 	$message .= 'Content-Transfer-Encoding: 8bit'."\n\n";
	 	$message .= $textMessage."\n\n";
	  
	 	$message .= '--'.$frontiere."\n";

	 	//-----------------------------------------------
	 	//PIECE JOINTE
	 	//-----------------------------------------------

	 	// If there is an attachment then ...
	 	if (!empty($attachmentFileName)) {
	 		
	 		$message .= 'Content-Type: image/' .$attachmentFileExtention .'; name="' .$attachmentFileName .'"'."\n";
	 		$message .= 'Content-Transfer-Encoding: base64'."\n";
	 		$message .= 'Content-Disposition:attachement; filename="' .$attachmentFileName .'"'."\n\n";

	 		if (empty($attachmentFileinRaw)) // The file is not in Raw format
	 			$message .= chunk_split(base64_encode(file_get_contents($attachmentFilePath .$attachmentFileName)))."\n";
	 		else 
		 		$message .= chunk_split(base64_encode($attachmentFileinRaw))."\n";
	 	}

	 	if(mail($to,$subject,$message,$headers))
		 	return true;
	 	else
		 	return false;

	}
	
	static function sendMail($fromName, $fromEmail, $to, $reply, $subject, $textMessage, $attachmentFilePath='', $attachmentFileName='',$attachmentFileExtention='', $attachmentFileinRaw='') {

		if (IS_LOCAL_SERVER == TRUE){
			return true; // for test Purpose
		}
		
		include 'Mail.php';
		include 'Mail/mime.php' ;

		$crlf = "\n";
		$from = $fromName ." <" .$fromEmail .">";
		
		$hdrs = array(
              'From'    => $from,
              'Subject' => $subject
              );
		
	 	//-----------------------------------------------
	 	//PIECE JOINTE
	 	//-----------------------------------------------

	 	// If there is an attachment then ...
	 	if (!empty($attachmentFileName)) {
	 		
	 		$mime = new Mail_mime(array('eol' => $crlf));
	 		
	 		$mime->setTXTBody($textMessage);
	 		
	 		if (empty($attachmentFileinRaw)) // The file is not in Raw format
	 		{
	 			$mime->addAttachment($attachmentFilePath .$attachmentFileName);
	 			
	 			$body = $mime->get();
				$hdrs = $mime->headers($hdrs);

				$mail =& Mail::factory('mail');
				$result = $mail->send($to, $hdrs, $body);
				
	 			if (PEAR::isError($result)) 
	 			{
	 				Logger::getInstance()->lwrite('INFO E-MAIL FAILED: Send email WITH attachement NOT in RAW from ' .$fromEmail .' To ' .$to .'ERROR=' .$result->getMessage());
	 				return false;
 				}
 				else 
 				{
  					Logger::getInstance()->lwrite('INFO E-MAIL SUCCESS: Send email WITH attachement NOT in RAW from ' .$fromEmail .' To ' .$to );
  					return true;
 				}
	 			
	 		}
	 		else
	 		{ 
	 			$mime->addAttachment($attachmentFileinRaw, 'application/octet-stream',  $attachmentFileName, false);
	 			
	 			$body = $mime->get();
				$hdrs = $mime->headers($hdrs);

				$mail =& Mail::factory('mail');
				$result = $mail->send($to, $hdrs, $body);
				
	 			if (PEAR::isError($result)) 
	 			{
	 				Logger::getInstance()->lwrite('INFO E-MAIL FAILED: Send email WITH attachement in RAW from ' .$fromEmail .' To ' .$to .'ERROR=' .$result->getMessage());
	 				return false;
 				}
 				else 
 				{
  					Logger::getInstance()->lwrite('INFO E-MAIL SUCCESS: Send email WITH attachement in RAW from ' .$fromEmail .' To ' .$to );
  					return true;
 				}
			

	 		}
	 		
	 	}
	 	else // NO attachment 
	 	{
	 		$body = $textMessage;

	 		$from = "<" .$fromEmail .">";
		
	 		$header = array(
	 						'From'    => $from,
					 		'Subject' => $subject
                            );

            $mail =& Mail::factory('mail');
	 		
	 		$result = $mail->send($to, $header, $body);
	 		
	 		if (PEAR::isError($result)) 
	 		{
	 			Logger::getInstance()->lwrite('INFO E-MAIL FAILED: Send SIMPLE email from ' .$fromEmail .' To ' .$to .'ERROR=' .$result->getMessage());
	 			return false;
 			}
 			else 
 			{
  				Logger::getInstance()->lwrite('INFO E-MAIL SUCCESS: Send SIMPLE email from ' .$fromEmail .' To ' .$to );
  				return true;
 			}
	 	}

	}	

	static function startsWith($haystack,$needle,$case=false) {
		if($case){
			return (strcmp(substr($haystack, 0, strlen($needle)),$needle)===0);
		}     
		return (strcasecmp(substr($haystack, 0, strlen($needle)),$needle)===0); 
	}	

	static function https_Rediretion(){
	
		if (IS_LOCAL_SERVER == FALSE){

			if (IS_USE_SECURE_CONNECTION == TRUE){ // (it is the case of REAL server)
				
				if ( startsWith($_SERVER[HTTP_HOST], MY_TEST_DOMAIN_NAME)){
					header("Location: http://" .MY_TEST_DOMAIN_NAME .$_SERVER['REQUEST_URI']);
					exit();
				}
				 
				if($_SERVER['SERVER_PORT'] != 443) { // if it is NOT https
					header("Location: https://".MY_DOMAIN_NAME .$_SERVER['REQUEST_URI']);
					exit();
				}
			}
			else { // IS_USE_SECURE_CONNECTION == FALSE){ // (it is the case of TEST server)

				if($_SERVER['SERVER_PORT'] == 443) { // if it is https
					header("Location: http://".MY_DOMAIN_NAME .$_SERVER['REQUEST_URI']);
					exit();
				}
			}
		}	
	}
	

	static function checkDuplicatesInArray($array)
	{
	    $duplicates=FALSE;
    	foreach($array as $k=>$i)
    	{
        	if(!isset($value_{$i}))
        	{
            	$value_{$i}=TRUE;
        	}
        	else
        	{
            	$duplicates|=TRUE;          
        	}
    	}
    	
    	return ($duplicates);
	}
	
	static function nextMonthOrlastDayOfNextMonth($currentDate) // // YYYY-mm-dd
	{
		$DD = substr($currentDate, 8, 2 );  // YYYY-mm-dd
					
   		if ( ($DD == "28") || ($DD == "29") || ($DD == "30") || ($DD == "31"))
		{
			$nextDate = date('Y-m-d', strtotime(date('Y-m-d', strtotime($currentDate)) . " +10 days"));

			$MM = substr($nextDate, 5, 2 );  // YYYY-mm-dd
			//$DD = substr($nextDate, 8, 2 );  // YYYY-mm-dd
			$YYYY = substr($nextDate, 0, 4 );  // YYYY-mm-dd
		
			$result = strtotime("{$YYYY}-{$MM}-01");
   			$result = strtotime('-1 second', strtotime('+1 month', $result));
   			return date('Y-m-d', $result);
		}
		else 
		{
			return date('Y-m-d',strtotime(date('Y-m-d', strtotime($currentDate)) . " +1 month"));
		}
   		
   		return date('Y-m-d', $result);
	}

	static function nextMonth($currentDate, $BaseDay) // YYYY-mm-dd
	{
		$DD = substr($currentDate, 8, 2 );  // YYYY-mm-dd

   		if ( ($BaseDay == "28") || ($BaseDay == "29") || ($BaseDay == "30") )
   		{
   			$nextDate = date('Y-m-d', strtotime(date('Y-m-d', strtotime($currentDate)) . " +10 days"));

   			$MM = substr($nextDate, 5, 2 );  // YYYY-mm-dd
			//$DD = substr($nextDate, 8, 2 );  // YYYY-mm-dd
			$YYYY = substr($nextDate, 0, 4 );  // YYYY-mm-dd
		
			$result = strtotime("{$YYYY}-{$MM}-01");
   			$result = strtotime('-1 second', strtotime('+1 month', $result));
   			$resultDate = date('Y-m-d', $result);
                        
   			$TheNewDay = substr($resultDate, 8, 2 );  // YYYY-mm-dd
   			
   			if( ($TheNewDay == '29') &&  ($BaseDay == "28") )
   			{
   				$resultDate = substr($resultDate, 0, 8 ) .'28';
   			}
   			else if( ($TheNewDay == '30') &&  ($BaseDay == "29") )
   			{
   				$resultDate = substr($resultDate, 0, 8 ) .'29';
   			}
   			else if( ($TheNewDay == '30') &&  ($BaseDay == "28") )
   			{
   				$resultDate = substr($resultDate, 0, 8 ) .'28';
   			}
   			else if( ($TheNewDay == '31') &&  ($BaseDay == "30") )
   			{
   				$resultDate = substr($resultDate, 0, 8 ) .'30';
   			}
   			else if( ($TheNewDay == '31') &&  ($BaseDay == "29") )
   			{
   				$resultDate = substr($resultDate, 0, 8 ) .'29';
   			}
   			else if( ($TheNewDay == '31') &&  ($BaseDay == "28") )
   			{
   				$resultDate = substr($resultDate, 0, 8 ) .'28';
   			}

   			return $resultDate;
   		}
   		else if ( $BaseDay == "31" )  // Chhoose the last day of the month
		{
			$nextDate = date('Y-m-d', strtotime(date('Y-m-d', strtotime($currentDate)) . " +10 days"));

			$MM = substr($nextDate, 5, 2 );  // YYYY-mm-dd
			//$DD = substr($nextDate, 8, 2 );  // YYYY-mm-dd
			$YYYY = substr($nextDate, 0, 4 );  // YYYY-mm-dd
		
			$result = strtotime("{$YYYY}-{$MM}-01");
   			$result = strtotime('-1 second', strtotime('+1 month', $result));
   			return date('Y-m-d', $result);
		}
		else 
		{
			return date('Y-m-d',strtotime(date('Y-m-d', strtotime($currentDate)) . " +1 month"));
		}
   		
		return date('Y-m-d', $result);
	}
	
	static function encryptDataToDB_AES128CFB($encryptedKey, $dataToEncrypt, $databaseName, $databaseField, $WhereStatement )
	{
		$theEncryptedData_base64 = encryptData_AES128CFB($encryptedKey, $dataToEncrypt);
		
		DatabaseControler::getCreatedInstance()->update_Generic_Field("", $databaseName, $databaseField, $theEncryptedData_base64, true, $WhereStatement);
	
	}
	
	static function encryptData_AES128CFB($encryptedKey, $dataToEncrypt)
	{
		$key_size = mcrypt_get_key_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_CFB);
		$encryption_key = $encryptedKey;
		$encryption_key_Len = strlen($encryption_key);
		// Padd the encryption key
		$j = 1; 
		for ($i = $encryption_key_Len; $i < $key_size; $i++) 
		{
			if($j == 10)
				$j = 1;
			$encryption_key .= $j;
			$j++; 
		}

		$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_CFB);
		$iv = mcrypt_create_iv($iv_size, MCRYPT_DEV_URANDOM); // 16 bytes output

		$encrypted_data = mcrypt_encrypt(MCRYPT_RIJNDAEL_128, $encryption_key, $dataToEncrypt, MCRYPT_MODE_CFB, $iv);

		$encrypted_data = $iv . $encrypted_data;

		$encrypted_data_base64 = base64_encode($encrypted_data);
		
		return $encrypted_data_base64;
		
	}
	
	static function getDecrypedData_AES128CFB($encryptedKey, $encryptedData_base64 )
	{
		$key_size = mcrypt_get_key_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_CFB);
		$encryption_key = $encryptedKey;
		$encryption_key_Len = strlen($encryption_key);
		// Padd the encryption key
		$j = 1; 
		for ($i = $encryption_key_Len; $i < $key_size; $i++) 
		{
			if($j == 10)
				$j = 1;
			$encryption_key .= $j;
			$j++; 
		}

		$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_CFB);
		
		$ciphertext_dec = base64_decode($encryptedData_base64);
		$iv_dec = substr($ciphertext_dec, 0, $iv_size);
		$ciphertext_dec = substr($ciphertext_dec, $iv_size);

		$Decrypted_data = mcrypt_decrypt(MCRYPT_RIJNDAEL_128, $encryption_key, $ciphertext_dec, MCRYPT_MODE_CFB, $iv_dec);
	
		return $Decrypted_data;
	}
	
	static function string_hex2str($hexString, $isIgnore00 = false) 
	{
		$str = "";
		
		for($i=0;$i<strlen($hexString);$i+=2) 
		{
		    $currentByte = hexdec(substr($hexString,$i,2));
		    
		    if ( $currentByte != 0 )
		        $str .= chr($currentByte);
		    else if ( $isIgnore00 == false )
		        $str .= chr($currentByte);
		}
		
		return $str;
	}
	
	static function string_hex2str_withKeyValueSeparator($hexString, $decimalSeparator)
	{
	    $outputDataArr = array();
	    
	    $str = "";
	
	    for($i=0;$i<strlen($hexString);$i+=2)
	    {
	        $currentByte = hexdec(substr($hexString,$i,2));
	
	        if ( $currentByte != $decimalSeparator )
	            $str .= chr($currentByte);
	        else 
	        {
	            if (!empty($str))
	               $outputDataArr[] = $str;
	            else
	                return $outputDataArr;
	            
	            $str = "";
	            
	        }
	    }
	
	    return $outputDataArr;
	}
	
	static function string_str2Hex($string)
	{
		$hex='';
		
		for ($i=0; $i < strlen($string); $i++)
		{
			//$hex .= dechex(ord($string[$i]));
		    $hex .= GlobalFunctions::string_dec2HexStr(ord($string[$i]));
		}

		return $hex;
	}
	
	static function string_dec2HexStr($decimal, $minStringLength = 2)
	{
		if ($decimal<0)
			$decimal=$decimal+256;
	
		$hexString='';
	
		$hexString = dechex($decimal);
		
		if ( strlen($hexString) % 2) 
		{
			$hexString = '0' .$hexString;
		}
		
		if (strlen($hexString) < $minStringLength) 
		{
			$nbOfCharToAdd = $minStringLength - strlen($hexString);
			
			for ($i = 0; $i < $nbOfCharToAdd; $i++) 
			{
				$hexString = '0' .$hexString;
			}
		}
		
		return $hexString;
	}

	static function string_HexStrToBin($hexString, $minStringLength = 8)
	{
	    $binString='';
	
	    $binString = base_convert($hexString, 16, 2);
	
	    if (strlen($binString) < $minStringLength)
	    {
	        $nbOfCharToAdd = $minStringLength - strlen($binString);
	        	
	        for ($i = 0; $i < $nbOfCharToAdd; $i++)
	        {
	            $binString = '0' .$binString;
	        }
	    }
	
	    return $binString;
	}
	
	static function string_addPadding($stringToPadd, $isPaddFront, $paddingCharacter, $minStringLength = 2)
	{
	    $paddedString = $stringToPadd;
	    
	    if (strlen($stringToPadd) < $minStringLength)
	    {
	        $nbOfCharToAdd = $minStringLength - strlen($stringToPadd);
	    
	        for ($i = 0; $i < $nbOfCharToAdd; $i++)
	        {
	            if ($isPaddFront == true)
	                $paddedString = '0' .$paddedString;
	            else
	                $paddedString = $paddedString .'0';
	        }
	    }
	    
	    return $paddedString;
	}
			
	static function json_merge2jsonObject($jsonObj1, $jsonObj2) 
	{
		$array1 = json_decode($jsonObj1, TRUE);
		$array2 = json_decode($jsonObj2, TRUE);
		
		$data = array_merge($array1, $array2);
		
		return json_encode($data);
	}
	
	static function validate_ip_address($ipAddress)
	{
		if(preg_match('/^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$/', $ipAddress)){
			return true;
		}else{
			return  false;
		}
	}
	
	static function validate_mac_address($macAddress)
	{
		if(preg_match('/^[0-9a-fA-F]{2}\-[0-9a-fA-F]{2}\-[0-9a-fA-F]{2}\-[0-9a-fA-F]{2}\-[0-9a-fA-F]{2}\-[0-9a-fA-F]{2}$/', $macAddress)){
			return true;
		}else{
			return  false;
		}
	}	
	
	static function curl_send_post_file($url, $fileFieldName,  $absoluteFilePathAndName)
	{
	    $file_to_upload = array($fileFieldName=>'@' .$absoluteFilePathAndName);
	     
	    $theURL = $url;
	    
	    //open curl session
	    $ch = curl_init();
	    //begin seting curl options
	    //set URL
	    curl_setopt($ch, CURLOPT_URL, $theURL );
	    //set method
	    curl_setopt($ch, CURLOPT_POST, 1);
	    //set post data string
	    curl_setopt($ch, CURLOPT_POSTFIELDS, $file_to_upload);
	    //these two options are frequently necessary to avoid SSL errors with PHP
	    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
	    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);
	    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	     
	    //perform the curl post and store the result
	    $result = curl_exec($ch);
	    //close the curl session
	    curl_close($ch);

	    return $result;
	}
		
	static function curl_send_post_json($url, $dataToSendArray)
	{
		//initialize the post string variable
		$fields_string = json_encode($dataToSendArray);

		//open curl session
		$ch = curl_init();
		//begin seting curl options
		//set URL
		curl_setopt($ch, CURLOPT_URL, $url);
		//set method
		curl_setopt($ch, CURLOPT_POST, 1);
		//set post data string
		curl_setopt($ch, CURLOPT_POSTFIELDS, $fields_string);
		//these two options are frequently necessary to avoid SSL errors with PHP
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
		curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
			
		curl_setopt($ch, CURLOPT_HTTPHEADER, array(
					'Content-Type: application/json',
					'Content-Length: ' . strlen($fields_string))
		);
				
		//perform the curl post and store the result
		$result = curl_exec($ch);
		
		//close the curl session
		curl_close($ch);
		
		return $result;
	}
	
	static function curl_send_get($url)
	{
	    //open curl session
	    $ch = curl_init();
	    
	    //begin seting curl options
	    //set URL
	    curl_setopt($ch, CURLOPT_URL, $url);

	    //these two options are frequently necessary to avoid SSL errors with PHP
	    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
	    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);
	    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
	    	
	    //perform the curl GET and store the result
	    $result = curl_exec($ch);
	
	    //close the curl session
	    curl_close($ch);
	
	    return $result;
	}	

    static function get_data($url) 
    {
        $ch = curl_init();
        $timeout = 5;
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, $timeout);
        $data = curl_exec($ch);
        curl_close($ch);
        return $data;
    }	
}
?>