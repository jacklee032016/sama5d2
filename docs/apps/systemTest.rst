System Testing for Linux system
##############################################
Jack Lee

fw_printenv
================================
Access environment partition of u-boot in Linux

Build with yocto: Add into minimal image: IMAGE_INSTALL_append += " u-boot-fw-utils "  in local.conf

/etc/fw_env.conf
::

	# Char device            offset within part
	# MTD device name       Device offset   Env. size       Flash sector size       Number of sectors
	/dev/mtd1               0x00000         0x10000         0x1000

Debug:
--------------
When no partition and its CRC is found, it prints default environment variables, defined in default configuration;


Case 1: Define partition as:
::

	/dev/mtd1               0x00000         0x10000         0x1000

	fw_printenv read out:
    Read error on /dev/mtd1: Success
	
	It looks like part of CRC, maybe the end of CRC is right; correct parameters


Case 2: 
::

	/dev/mtd1               0x00000         0x8000          0x4000

	fw_setenv ethaddr fc:c4:3e:0e:18:15
	MTD erase error on /dev/mtd1: Invalid argument
	Error: can't write fw_env to flash	
	
	Because sector size of n25Q 512Mb flash is 64K (0x10000); Change configuration as following:
	/dev/mtd1               0x00000         0x8000          0x10000
	

Case 3: 
::

	/dev/mtd1               0x00000         0x8000          0x10000

	After ``fw_setenv ethaddr XX:XX..`` or ``fw_setenv  test testValue``; fw_printenv complains "CRC error, using default.."
	
	Tested with old version kernel, this problem fixed. So flash driver drivers/mtd/spi-nor/spi-nor.c has some problem:
	
	Before this, I have fixed problem in :spi_nor_select_erase() of this file;

	
	
Modify ethernet address:
::
	
	fw_printenv ethaddr
	fw_setenv ethaddr XX:XX:XX:XX:XX:XX or fw_setenv test testValue (add new variable in environment);

	
Tools for Hardware
================================

I2C tools

https://i2c.wiki.kernel.org/index.php/I2C_Tools
i2cdetect 	detect I2C chips
i2cdump 	examine I2C registers
i2cget 	read from I2C/SMBus chip registers
i2cset 	set I2C registers
i2ctransfer 	send user-defined I2C messages in one transfer

git clone git://git.kernel.org/pub/scm/utils/i2c-tools/i2c-tools.git


Show threads of one process
================================
::

   ¡°-H¡± option of top to view threads
   top -Hbn1 

   top -Hbn1 -p 23075


   ps -e -T 
     -e shows all processes
     -T lists all threads

