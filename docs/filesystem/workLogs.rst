Work Logs for File System
################################
April 17th, 2019

11.12, 2019
-------------------------------------------------
* add libstc++ into image/usr/lib and run first hello world of c++ program;


11.06, 2019
-------------------------------------------------
* Modification of PTP configuration
    * add 2 fields of PTP data:
	   * "errCode": 200, OK; others check "msg" fields
	   * "msg": "OK or error msg";
    * IP Command/REST API support modify the writable fields:
	   * data can save;
	   * can't used to control ptp daemon now;
	* PTP is disable:
	   * SDP clock type refer to local clock;
	   * return alert info and zero data for most fields;
	   * never stop at the polling PTP connection;
	* check status of PTP server when PTP is enabled;
	* debug the problem of clear PTP configuration data when connection can't be created;

* Debug info and log in log file
    * show important start information;

* Repackage install package and update package:	 
	* install package: 
	    * install all application after new file system is flashed;
		* mux7xxApp_7.3.0_arm_debug_2019_11_05.zip;
	* update package:
	    * update binaries and some required configuration data;
	* packaging version for install and update packages: etc/mLab/muxMgr.ver	   



11.04, 2019
-------------------------------------------------
* Integrated with new web pages;
* Support default IP when DHCP server is not available: TX: 66, RX: 67;
* New FPGA configuration: read Audio params from FPGA, instead of HDMI chip;
* Debug the problem 'apiClient' can't send command out;


* udhcpc of busybos configuration: 10.30, 2019
-------------------------------------------------
remove command options from ifup: "-R -b"


* ntpd and busybos configuration: 10.07, 2019
-------------------------------------------------

::

	enter `$POKY/build-microchip`:
	export OEROOT=../
	source ../scripts/oe-buildenv-internal

    bitbake -c menuconfig busybox: execure command `menuconfig` on package 'busybox';
    bitbake busybox : build package `busybox`;
    bitbake core-image-minimal: rebuild image from packages;  
		: core-image-minimal-sama5d2-xplained-20191007184529.rootfs.tar.gz in tmp/deploy/images/sama5d2-xplained


	file system:
		copy tar.gz into fs/;
		remove other tar.gz files and mimimal in fs;
		run ``loadTar.sh`` to uncompress tar file into minimal;
		Modify or update files in ``minimal`` as wish:
			Remove usr/bin/phpdbg
			
			for example: in minimal, ``unzip ../mux7xxApp_7.3.0_arm_debug_2019_10_07.zip``;
		``mkJffs.sh`` to make new JFFS2 image which is updated onboard by tftp in u-boot;
		
		Test file system:
			sudo ./load.sh root.jffs2.2019_10_07 : mount file system into minimal



06.16, 2019
==============================
Add scripts of runlevel-5 in /etc/rc5.d/S95Led
   * scripts can be symbol link to /etc/init.d/;
   * script must be in executing previledge;

05.16, 2019
==============================
Add i2c-tools
------------------------------
Recipe position: poky/meta/recipes-devtools/i2c-tools/i2c-tools_4.0.bb

Add into image: IMAGE_INSTALL_append += " i2c-tools "  in local.conf


Add python3-six
------------------------------
Module six is needed by argparser in flask;

Recipe position: poky/meta/recipes-devtools/python/python3-six_1.11.0.bb

Add into image: CORE_IMAGE_EXTRA_INSTALL = "python3-core python3-pip python3-flask python3-flask-restful python3-six"  in local.conf

Add fw-printenv
------------------------------
fw-printenv is from u-boot, used to operate environment of u-boot in Linux;

It failed when build with yocto SDK;

Recipe position: poky/meta/recipes-bsp/u-boot/u-boot-fw-utils_2018.01.bb

Add into image: CORE_IMAGE_EXTRA_INSTALL = "python3-core python3-pip python3-flask python3-flask-restful python3-six"  in local.conf


bitbake core-image-minima
bitbake core-image-minimal -c populate_sdk



04.18, 2019
------------------------------
* remove `ethtool` from image in local.conf;
* remove `phpdbg` 

* hostname, J.L. 04.18, 2019, add in local.conf
   * `hostname_pn-base-files = "mux744"`
   * recipe file is "base-files";
   * "hostname" is a variable of this recipe file, default it is `hostname = "${MACHINE}"`;
* Change password of root to Angelica_M774;
* Add new user jackl with password of 123;
   
Apache + Flask REST service
++++++++++++++++++++++++++++++++++++
* Enable mod_proxy+mod_http_proxy and configure http proxy between apache service and flash service:
   * /etc/apache2/httpd.conf for board
   * /etc/apache2/apache2.conf for ubuntun; `sudo service apache2 restart` re-start apache

::

    LoadModule proxy_module lib/apache2/modules/mod_proxy.so
    LoadModule proxy_http_module lib/apache2/modules/mod_proxy_http.so

    ProxyPass /api/ http://127.0.0.1:5000/
    ProxyPassReverse /api/ http://127.0.0.1:5000/


04.17, 2019
------------------------------
Configuration
++++++++++++++++++
* support bonjour service: found by Bonjour browser
  * add multicast route for eth0: 
::

   route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0
   
  * start `avahi-daemon`;
  * Create file /etc/avahi/services/apache.service with following content:
::

   <?xml version="1.0" standalone='no'?><!--*-nxml-*-->
   <!DOCTYPE service-group SYSTEM "avahi-service.dtd">
   <service-group>
       <name replace-wildcards="yes">%h HTTP</name>
       <service>
          <type>_http._tcp</type>
          <port>80</port>
       </service>
   </service-group>

   * Be careful the router or switch configuration
   * Bonjour browser will find all services in /etc/avahi/services/;

Testing
++++++++++++++++++
With following commands in PC Linux:

::

   avahi-resolve -a 192.168.168.102 
       # IP is the address of avahi-daemon host;
   
   avahi-browse --resolve -a 
       # list all mDNS records received from network;

   sudo tcpdump -i eth0 igmp
       dump info like `IP 192.168.168.102 > 224.0.0.251: igmp v2 report 224.0.0.251`