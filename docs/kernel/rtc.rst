RTC and hw clock
#####################################################
06.18, 2019    Jack Lee

RTC Chip and System time
===============================================
10.04, 2019

RTC chip tests
-----------------------------------------------
* Reboot without poweroff:
	* read timestamp of last time;
	* timestamp is changed and increased when different times of read RTC in one boot;

* Poweroff and reboot:
    * read default timestamp in kernel source code;


system timestamp
-----------------------------------------------
systemd keep and restore system clock from: 

    /etc/timestamp 
	 
    The content is "20191004200517"

When boot up, systemd compare the timestamp in this file and output from RTC:
    * If timestamp in RTC is bigger than timestamp in this file, nothing happens;
	* If timestamp in RTC is smaller than timestamp in this file, update system timestamp and write this timestamp to RTC chip; 



Test
===============================================

::

    date -s 201906181627

    hwclock -w

