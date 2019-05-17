readme for Application in Mux7XX
#######################################
Jack Lee, April 3rd, 2019


Installation
=====================
::

	make install
	scp mux7xxApp_7.3.0_arm_debug_2019_05_15.zip root@192.168.168.102:/var/volatile/

In python source directory:
::
	
	./install
	scp scp REST_API_2019_05_14.zip root@192.168.168.102:/var/volatile



How to build
=====================

Build for X86
-----------------
::
   make

Build for ARM
-----------------
initialize build environment from Yocoto, for example:

::
	
	/home/lzj/yocto/opt/sdk/
    source  environment-setup-cortexa5hf-neon-poky-linux-gnueabi

Then ARCH environment variable is set, then ``make`` directly to build for ARM;

Check whether SDK is initialized, use ``printenv | grep ARCH`` or ``printenv | grep CC``;


 make CROSS_COMPILE=arm-poky-linux-gnueabi- envtools 

	