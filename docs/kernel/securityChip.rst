Security Chip in Kernel
################################
05.30, 2019


* Read Challenge 
--------------------
* Write challenge: same as Write scratch:
* Read MAC:
	* Write command: command: 0xA5; parameter bytes: 0x00(Use ROM IR); 0xE0(anonymous);
	* Read 16 bits CRC;
	* Delay;
	* Read status byte: 0xAA: OK;
	* Read 32 bytes MAC data;
	* Read 16 bits CRC;
	


* Key write
--------------------
* Read/Write scratch:
	* Write command: command: 0x0F; parameter byte:0x0F/0x00;
	* Read 16 bits CRC;
	* Read/Write 32 bytes;
	* Read 16 bits CRC;
* Lock secret:
	* Write command: command: 0x33; parameter bytes: 0x00(unlock); 0xE0(lock);
	* Read 16 bits CRC;
	* Write RELEASE(0xAA) command;
	* Delay;
	* Read status byte: 0xAA: OK; 0x55:has been locked before;
		



* Page read/write
--------------------
	* Continue for all 8 segments of one page;

* Write page:
	* Write command: command: 0x55; parameter byte: 0x00: from seg0 of pag0 to end of page;
	* Read 16 bits CRC;
	* Write one segment:
	    * write 4 bytes;
		* read 16 bits CRC;
		* Write RELEASE command;
		* Delay;
		* Read status;
	

	
* Basic read/write operation
---------------------------------
* Reset bus;
* Send ROM ID of target device or send SKIP_ROM_ID command;
* Send command;
* Send parameter byte;
* Read CRC(16 bits);

	
	