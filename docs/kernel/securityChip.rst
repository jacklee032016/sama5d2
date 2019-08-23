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

tests
==========================================

Command:
-----------------------------------------
cat /sys/devices/w1_bus_master1/17-0000008846b3/MAC | hexdump

cat /sys/devices/w1_bus_master1/17-0000008846b3/page0 | hexdump
0000000 b317 8846 0000 a600 3035 3730 3437 6453
0000010 4f69 6576 4972 2d50 3737 a534 a55a a55a
0000020

cat /sys/devices/w1_bus_master1/17-0000008846b3/key | hexdump
0000000 0200 0000 3f5e ffff ffff ffff ffff ffff
0000010 ffff ffff ffff ffff ffff ffff ffff ffff
0000020

cat /sys/devices/w1_bus_master1/17-0000008846b3/id | hexdump
0000000 b317 8846 0000 a600                    
0000008


log
-----------------------------------------
root@mux744:~# testMux -c sc
testsMux 01.0.01-01, Aug 21 2019 20:03:35
Send list master command...
Pre-parse header: 16 bytes len=52 type=3 seq=1 pid=0 (our pid=438)
NLMSGHDR: len=52 type=3 flags=0 seq=1 pid=0
CN_MSG: idx/val=3/1 seq=1 ack=2 len=16 flags=25193
W1 MSG: type:ListMaster(6); len=4; id=1869819491
DATA: 01 00 00 00
DATA: 01 00 00 00
Add #0 w1_bus_master#1
Master ID:1
Send list slave command to master...
Pre-parse header: 16 bytes len=60 type=3 seq=2 pid=0 (our pid=438)
NLMSGHDR: len=60 type=3 flags=0 seq=2 pid=0
CN_MSG: idx/val=3/1 seq=2 ack=3 len=24 flags=0
W1 MSG: type:MasterCmd(4); len=12; id=1
W1 CMD: cmd:ListSlave(8); len=8
DATA: 17 B3 46 88 00 00 00 A6
DATA: 17 B3 46 88 00 00 00 A6
Slave ID: 17 B3 46 88 00 00 00 A6
INFO:2019-05-16 18:53:23 [INFO,testMux] : cmnSysSecureChip.c:214 | ROM ID
00000000  17 b3 46 88 00 00 00 a6                         ..F.....
2019-05-16 18:53:23 [INFO,testMux]: [cmnSysSecureChip.c-231]:
	Read Personality...
INFO:2019-05-16 18:53:23 [INFO,testMux] : cmnSysSecureChip.c:238 | Personality
00000000  00 02 00 00 5e 3f ff ff ff ff ff ff ff ff ff ff ....^?..........
00000010  ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ................
INFO:2019-05-16 18:53:23 [INFO,testMux] : cmnSysSecureChip.c:248 | ManuID: 0x00:0x00
2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-38]:
PAGE
	Write ...
INFO:2019-05-16 18:53:23 [INFO,testMux] : testSecure.c:39 | Write page:
00000000  17 b3 46 88 00 00 00 a6 35 30 30 37 37 34 53 64 ..F.....500774Sd
00000010  69 4f 76 65 72 49 50 2d 37 37 34 a5 5a a5 5a a5 iOverIP-774.Z.Z.
2019-05-16 18:53:23 [DBUG,testMux] : cmnSysSecureChip.c:270 | 
2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-43]:
	Read ...
2019-05-16 18:53:23 [DBUG,testMux] : cmnSysSecureChip.c:265 | 
INFO:2019-05-16 18:53:23 [INFO,testMux] : testSecure.c:45 | Read Page
00000000  17 b3 46 88 00 00 00 a6 35 30 30 37 37 34 53 64 ..F.....500774Sd
00000010  69 4f 76 65 72 49 50 2d 37 37 34 a5 5a a5 5a a5 iOverIP-774.Z.Z.
2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-53]:
	Page is OK

2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-57]:
KEY
	Write ...
INFO:2019-05-16 18:53:23 [INFO,testMux] : testSecure.c:58 | Write key:
00000000  43 02 89 dc 44 51 4a f2 6a fa 8b e1 46 54 49 fe C...DQJ.j...FTI.
00000010  67 02 8d e0 55 4a 4c ed 6d 0e 8f de 58 49 5c d8 g...UJL.m...XI\.
2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-62]:
	Read Personality...
INFO:2019-05-16 18:53:23 [INFO,testMux] : testSecure.c:64 | Personality
00000000  00 02 00 00 5e 3f ff ff ff ff ff ff ff ff ff ff ....^?..........
00000010  ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ................
2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-83]:
Challenge
	Write ...
INFO:2019-05-16 18:53:23 [INFO,testMux] : testSecure.c:84 | Write Challenge:
00000000  d2 04 00 00 2e 16 00 00 34 23 00 00 0a 05 00 00 ........4#......
00000010  80 0d 00 00 80 0d 00 00 80 0d 00 00 80 0d 00 00 ................
2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-89]:
	Read ...
INFO:2019-05-16 18:53:23 [INFO,testMux] : testSecure.c:92 | Read Challenge:
00000000  8e 83 bf 16 c3 b9 1d 1a da 99 f9 7e 1b 4d 9d 3a ...........~.M.:
00000010  59 d0 f4 25 00 6f dc 7a 1d a6 d3 2b a8 a3 0b 46 Y..%.o.z...+...F
INFO:2019-05-16 18:53:23 [INFO,testMux] : cmnSysSecureChip.c:143 | raw MT data:
00000000  17 b3 46 88 00 00 00 a6 35 30 30 37 37 34 53 64 ..F.....500774Sd
00000010  69 4f 76 65 72 49 50 2d 37 37 34 a5 5a a5 5a a5 iOverIP-774.Z.Z.
00000020  d2 04 00 00 2e 16 00 00 34 23 00 00 0a 05 00 00 ........4#......
00000030  80 0d 00 00 80 0d 00 00 80 0d 00 00 80 0d 00 00 ................
00000040  43 02 89 dc 44 51 4a f2 6a fa 8b e1 46 54 49 fe C...DQJ.j...FTI.
00000050  67 02 8d e0 55 4a 4c ed 6d 0e 8f de 58 49 5c d8 g...UJL.m...XI\.
00000060  17 b3 46 88 00 00 00 a6 00 00 00 00 00 00 00 00 ..F.............
00000070  00 00 00 00 00 00 00                            .......
INFO:2019-05-16 18:53:23 [INFO,testMux] : cmnSysSecureChip.c:160 | Calculated MAC:
00000000  8e 83 bf 16 c3 b9 1d 1a da 99 f9 7e 1b 4d 9d 3a ...........~.M.:
00000010  59 d0 f4 25 00 6f dc 7a 1d a6 d3 2b a8 a3 0b 46 Y..%.o.z...+...F
2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-101]:
	MAC Validating result: Success

2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-104]:
	Enable CRC ...
2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-107]:
	Disable CRC ...
2019-05-16 18:53:23 [INFO,testMux]: [testSecure.c-110]:
Test OK!!!

	