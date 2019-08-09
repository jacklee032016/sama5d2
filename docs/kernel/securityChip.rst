Security Chip in Kernel
################################
05.30, 2019

Application
====================================
Init
----------------------------------------
*. read ROM ID;
*. create page data locally with product name, etc.
*. create secret locally, changing ROM ID with some algorithm; this is shared with client; 
*. read personality from chip;


Test Process
----------------------------------------
*. Write page/read: 32 bytes;
*. Write key(secret)/read key, personality: 32 bytes;
*. Create challenge from random and write, then start caculation of MAC;
*. Read result: 32 bytes:
*. Caculate MAC and validate: 
    *. Caculate MAC with page(32), secret/key(32), challenge(32) and others(manId, etc), 128 bytes; result 32 bytes
	*. Compare 32 bytes


IP Command Client
----------------------------------------
*. get_id:
*. set_key: 
    *. write page data;
    *. write secrect data which is caculated by client, and lock_secret;
*. get_status: 
    *. write challenge; 
	*. start computing;
	*. read 32 bytes MAC;
	*. caculate and validate MAC;

Check when startup
----------------------------------------
*. init library interface;
*. call get_status;


Driver
====================================

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
	* After scratchpad has been loaded into secret, the result of read scratchpad is zero;
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
		* Delay, mandidate;
		* Read status;
	
* Read page:
	* Write command: command: 0xF0; parameter byte: 0x00: from seg0 of pag0 to end of page;
	* Read 16 bits CRC;
	* Read 32 bytes;

	
* Basic read/write operation
---------------------------------
* Reset bus;
* Send ROM ID of target device or send SKIP_ROM_ID command;
* Send command;
* Send parameter byte;
* Read CRC(16 bits);

* Bus operations
---------------------------------
* Reset: reset all devices in bus;
* Send ROM ID to access particular device on bus;
* Or send SKIP_ROM_ID when only one device is on bus;
* Refers to p.32 1-Wire ROM function commands;
	