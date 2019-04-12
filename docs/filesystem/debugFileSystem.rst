Work and Debug of File System
################################
April 10th, 2019


Questions
============================
#. MAC address is same as the one used in u-boot. Where does the kernel get it?
#. `hwclock: can't open '/dev/misc/rtc': No such file or directory`?



Init and script
============================

bootargs of u-boot

::

   bootargs=root=/dev/mtdblock7 rootfstype=jffs2 rw init=/sbin/init.sysvinit console=ttyS0,115200
   
   init script is /sbin/init.sysvinit, replace /linuxrc
   

Network

Add following in `start)` section of /etc/init.d/networking

::   

   ifup -a
   echo -n "After up, make ethernet as 100MB/Duplex"
   ethtool -s eth0 speed 100 duplex full autoneg off

When ethernet is working in u-boot, then this command make it work again;



dropdear and ssh
============================
Using dropdear, instead of openssh

::

    /usr/sbin/dropbear -r /etc/dropbear/dropbear_rsa_host_key -p 22 -B

connect to board

   ssh 192.168.168.103
   
   ssh 192.168.168.103 -l root

   

Relogin failed
----------------------------
Rebuild image of file system, and start with same IP address, then authentication failed:
::

   Host key verification failed.

   


JFFS2 
============================

Bug info
---------------

::

		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e20000: 0x419a instead
		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e20004: 0x0820 instead
		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e20008: 0x0841 instead
		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e2000c: 0x40c8 instead
		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e20010: 0x50a7 instead
		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e20014: 0x1000 instead
		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e20018: 0x0345 instead
		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e2001c: 0x0400 instead
		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e20020: 0x0c94 instead
		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e20024: 0xe895 instead
		jffs2: Further such events for this erase block will not be printed
		jffs2: jffs2_scan_eraseblock(): Magic bitmask 0x1985 not found at 0x00e30000: 0x419a instead


Analysis
---------------
	
Address 0x00e20000 is the first address which is complainted. And it is 14M, and the size of file system is 14M;

Check the binary of jffs2 image, which is only padded to the address of 0xe1ffff when pad size is 16MB; more 128KB than 14MB is created by mkjffs2 command;

The raw file system in tar.gz format is 11.75MB, uncompressed size is about 32.5MB;

Flash partition is 16MB, and created by following command:

::

   sf erase 0x4d0000 0x1400000
   sf write 0x21000000 0x4d0000 0x1400000

eg. 20MB is erased for an 16MB partition, and 20MB is copied from memory for binary image of 14.1MB: other 6MB is random data from memory (at least, it is not 0xFF), this section of data is not padded with 0xFF;


Fixed
--------------
Make flash file system in u-boot by following command:

::

   # erase as the size of partition: 16M
   sf erase 0x4d0000 0x1000000
   # copy the exact size of fs image, the size can be checked from ultra-editor when it opens the image file or after tftp download from host in u-boot
   sf write 0x21000000 0x4d0000 0xe20000


Notes:
--------------
* Directory of 59MB, into jffs2 file system of 23MB;


MTD partitions definition
===============================

#. root FS 48MB
   * defined in dts file:at91-sama5d2_xplained_common.dtsi
   
