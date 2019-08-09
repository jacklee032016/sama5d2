Read me for Network in File System
######################################################
Auguest 7th, 2019


Network configuration file of systemd
============================================
systemd-udevd -d

/etc/network/interfaces

udhcpc -R -b -p /var/run/udhcpc.eth0.pid -i eth0

	-R,--release    Release IP on quit
	-b              Background if lease is not immediately obtained ) 
	-p 				pidfile: /var/run/udhcpc.eth0.pid
	