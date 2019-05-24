Work logs for u-boot
################################


05.17, 2019
--------------------
* Modify environment partition: 0x10000(offset), 0x10000(size), and test with fw_printenv
    *file include/config/at91-sama4_common.h
::

	#define CONFIG_ENV_OFFSET		0x10000
	#define CONFIG_ENV_SIZE			0x10000		/* 64K, one sector */
	    
