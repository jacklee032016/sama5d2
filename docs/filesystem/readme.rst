Read me for Yocto File System
################################
April 9th, 2019


How to build
============================

::

    bitbake core-image-minimal
	
	bitbake core-image-minimal -c populate_sdk 

	
	bitbake -b php_7.2.7 -c fetch|unpack/patch/configure/compile/stage/install/package/listtasks/devshell -D

Yocto Packages
============================

#. apache2
#. php
#. avahi: bonjour service;
#. ssd: openssh or dropbear
#. ethtool: configure speed and duplex mode of ethernet to make it works;

   ethtool -s eth0 speed 100 duplex full autoneg off

   ifconfig eth0 192.168.168.183 netmask 255.255.255.0
   # ifconfig eth0 10.0.1.240 netmask 255.255.255.0

   route add -net 224.0.0.0 netmask 224.0.0.0 eth0


   
Not supports:

* udhcpc: use udhcpc from busybox
* gdbserver
* python: too big;


opkg
==========

package manager, support run `udhcpc` by `/usr/lib/opkg/alternatives/udhcpc`;

Naming of the alternative commands

When supplying alternative commands the target command itself is not installed directly by any of the available alternatives. 
This is to ensure that no package will replace files that were installed by one of the other available alternative packages. 
The alternatives system will create a symlink for the target command that points to the required alternative.


Check all receipes with this URL:

   https://layers.openembedded.org/layerindex/branch/master/recipes/?q=php
   