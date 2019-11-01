Read me for Network in File System
######################################################
Auguest 7th, 2019


When DHCP server don't exist
============================================
ifup parse this file and call the corresponding command:

/etc/network/interfaces

#default TX static address
auto eth0:1
iface eth0:1 inet static
        address 192.168.168.66
        netmask 255.255.255.0
        network 192.168.168.0
        gateway 192.168.168.1
        hwaddress ether fc:c2:3d:0d:16:13

* these content should be created with different IP address of TX/RX
* Update firmware: keep the configuration data or recreate this file


Network configuration file of systemd
============================================
systemd-udevd -d

/etc/network/interfaces

udhcpc -R -b -p /var/run/udhcpc.eth0.pid -i eth0

	-R,--release    Release IP on quit
	-b              Background if lease is not immediately obtained ) 
	-p 				pidfile: /var/run/udhcpc.eth0.pid
	