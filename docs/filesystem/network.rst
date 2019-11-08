Read me for Network in File System
######################################################
Auguest 7th, 2019

Configure default IP address no matter DHCP
============================================
* udhcpc only fetch release from DHCP, it never configure interface address directly;
* udhcpc set environment variables and call script with event when something happened in udhcpc;
* default script of udhcpc is /usr/share/udhcpc/default.script, which call script in /etc/udhcpc.d/50default :

::

	#!/bin/sh
    exec run-parts -a "$1" /etc/udhcpc.d

* event **deconfig** is used when udhcpcd starts or release is lost:
    * eg. to make udhcpc works, interface must be up and given a default address, like 0.0.0.0.
	* when udhcpc quit or is killed( "udhcpc -B "), this event is also used;
	* when **defconfig** is used, IP address can gotten from diffrent position, such as kernel command line, for example:

::

    deconfig)
		grep -q -v ip= /proc/cmdline # ip field in command line
		if [ $? -eq 0 ]; then
			/sbin/ifconfig $interface up
		fi
		grep -q -v nfsroot= /proc/cmdline  ## nfsroot field
		if [ $? -eq 0 ]; then
			/sbin/ifconfig $interface 0.0.0.0
		fi
		if [ -x /usr/sbin/avahi-autoipd ]; then
			/usr/sbin/avahi-autoipd -wD $interface --no-chroot
		fi
		;;

* change /usr/share/udhcpc/default.script like following:

::


	if [ -f /etc/mLab/mux.conf ]; then
	  source /etc/mLab/mux.conf
	else
	  MUX_IP=192.168.168.66
	fi
	echo "default 774 IP $MUX_IP, device $MUX_TYPE"


	case "$1" in
		deconfig)
			/sbin/ifconfig $interface $MUX_IP
			;;

		renew|bound)
			exec run-parts -a "$1" /etc/udhcpc.d
			;;

	esac

	exit 0
	 

Can't set IP address in release when DHCP server works
---------------------------------------------------------------------
in /etc/udhcpc.d/50default , it failed when it calls:

::

	/sbin/ip addr add dev $interface local $ip/$mask $BROADCAST

So disable **ip**, instead **ifconfig** by disable check of /sbin/ip:






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



udhcpc [-fbqaRB] [-t N] [-T SEC] [-A SEC/-n] [-i IFACE] [-s PROG] [-p PIDFILE] [-oC] [-r IP] [-V VENDOR] [-F NAME] [-x OPT:VAL]... [-O OPT]...

        -i,--interface IFACE    Interface to use (default eth0)
        -s,--script PROG        Run PROG at DHCP events (default /etc/udhcpc/default.script)
        -p,--pidfile FILE       Create pidfile
        -B,--broadcast          Request broadcast replies
        -t,--retries N          Send up to N discover packets (default 3)
        -T,--timeout SEC        Pause between packets (default 3)
        -A,--tryagain SEC       Wait if lease is not obtained (default 20)
        -n,--now                Exit if lease is not obtained
        -q,--quit               Exit after obtaining lease
        -R,--release            Release IP on exit
        -f,--foreground         Run in foreground
        -b,--background         Background if lease is not obtained
        -S,--syslog             Log to syslog too
        -a,--arping             Use arping to validate offered address
        -r,--request IP         Request this IP address
        -o,--no-default-options Don't request any options (unless -O is given)
        -O,--request-option OPT Request option OPT from server (cumulative)
        -x OPT:VAL              Include option OPT in sent packets (cumulative)
                                Examples of string, numeric, and hex byte opts:
                                -x hostname:bbox - option 12
                                -x lease:3600 - option 51 (lease time)
                                -x 0x3d:0100BEEFC0FFEE - option 61 (client id)
        -F,--fqdn NAME          Ask server to update DNS mapping for NAME
        -V,--vendorclass VENDOR Vendor identifier (default 'udhcp VERSION')
        -C,--clientid-none      Don't send MAC as client identifier
Signals:

        USR1    Renew lease
        USR2    Release lease
    


udhcpc [-Cfbnqtvo] [-c CID] [-V VCLS] [-H HOSTNAME] [-i INTERFACE]
	[-p pidfile] [-r IP] [-s script] [-O dhcp-option]...

        -V,--vendorclass=CLASSID        Vendor class identifier
        -i,--interface=INTERFACE        Interface to use (default eth0)
        -H,-h,--hostname=HOSTNAME       Client hostname
        -c,--clientid=CLIENTID  Client identifier
        -C,--clientid-none      Suppress default client identifier
        -p,--pidfile=file       Create pidfile
        -r,--request=IP         IP address to request
        -s,--script=file        Run file at DHCP events (default /usr/share/udhcpc/default.script)
        -t,--retries=N          Send up to N request packets
        -T,--timeout=N          Try to get a lease for N seconds (default 3)
        -A,--tryagain=N         Wait N seconds (default 20) after failure
        -O,--request-option=OPT Request DHCP option OPT (cumulative)
        -o,--no-default-options Do not request any options (unless -O is also given)
        -f,--foreground Run in foreground
        -b,--background Background if lease is not immediately obtained
        -S,--syslog     Log to syslog too
        -n,--now        Exit with failure if lease is not immediately obtained
        -q,--quit       Quit after obtaining lease
        -R,--release    Release IP on quit
        -a,--arping     Use arping to validate offered address

If the requested IP address cannot be obtained, the client accepts the
address that the server offers.


udhcp client scripts
-------------------

When an event occurs, udhcpc calls the action script. The script by
default is /usr/share/udhcpc/default.script but this can be changed via 
the command line arguments. The three possible arguments to the script 
are:

	deconfig: This argument is used when udhcpc starts, and
	when a leases is lost. The script should put the interface in an
	up, but deconfigured state, ie: ifconfig $interface 0.0.0.0.
	
	bound: This argument is used when udhcpc moves from an
	unbound, to a bound state. All of the paramaters are set in
	enviromental variables, The script should configure the interface,
	and set any other relavent parameters (default gateway, dns server, 
	etc).
	
	renew: This argument is used when a DHCP lease is renewed. All of
	the paramaters are set in enviromental variables. This argument is
	used when the interface is already configured, so the IP address,
	will not change, however, the other DHCP paramaters, such as the
	default gateway, subnet mask, and dns server may change.

	nak: This argument is used with udhcpc receives a NAK message.
	The script with the deconfig argument will be called directly
	afterwards, so no changes to the network interface are neccessary.
	This hook is provided for purely informational purposes (the
	message option may contain a reason for the NAK).

The paramaters for enviromental variables are as follows:

	$HOME		- The set $HOME env or "/"
	$PATH		- the set $PATH env or "/bin:/usr/bin:/sbin:/usr/sbin"
	$1		- What action the script should perform
	interface	- The interface this was obtained on
	ip		- The obtained IP
	siaddr		- The bootp next server option
	sname		- The bootp server name option
	boot_file	- The bootp boot file option
	subnet		- The assigend subnet mask
	timezone	- Offset in seconds from UTC
	router		- A list of routers
	timesvr		- A list of time servers
	namesvr		- A list of IEN 116 name servers
	dns		- A list of DNS server
	logsvr		- A list of MIT-LCS UDP log servers
	cookiesvr	- A list of RFC 865 cookie servers
	lprsvr		- A list of LPR servers
	hostname	- The assigned hostname
	bootsize	- The length in 512 octect blocks of the bootfile
	domain		- The domain name of the network
	swapsvr		- The IP address of the client's swap server
	rootpath	- The path name of the client's root disk
	ipttl		- The TTL to use for this network
	mtu		- The MTU to use for this network
	broadcast	- The broadcast address for this network
	ntpsrv		- A list of NTP servers
	wins		- A list of WINS servers
	lease		- The lease time, in seconds
	dhcptype	- DHCP message type (safely ignored)
	serverid	- The IP of the server
	message		- Reason for a DHCPNAK
	tftp		- The TFTP server name
	bootfile	- The bootfile name

additional options are easily added in options.c.


note on udhcpc's random seed
---------------------------

udhcpc will seed its random number generator (used for generating xid's)
by reading /dev/urandom. If you have a lot of embedded systems on the same
network, with no entropy, you can either seed /dev/urandom by a method of
your own, or doing the following on startup:

ifconfig eth0 > /dev/urandom

in order to seed /dev/urandom with some data (mac address) unique to your
system. If reading /dev/urandom fails, udhcpc will fall back to its old
behavior of seeding with time(0).


signals accepted by udhcpc
-------------------------

udhcpc also responds to SIGUSR1 and SIGUSR2. SIGUSR1 will force a renew state,
and SIGUSR2 will force a release of the current lease, and cause udhcpc to
go into an inactive state (until it is killed, or receives a SIGUSR1). You do
not need to sleep between sending signals, as signals received are processed
sequencially in the order they are received.


compile time options	