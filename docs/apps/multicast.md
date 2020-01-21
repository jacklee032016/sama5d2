# Multicast Debug in Linux
14, Jan., 2020    Jack Lee


## problem
Sometimes, switch lost the group; and after a while, 

##ping multicast

Host 192.168.168.104(RX) has join the group of 226.2.0.1, following command works in RX and other host

```
	 echo "0" > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts   ## this command run in the receiver which joined some group

	 
	 ping 226.2.0.1 # this command run in other hosts, which may or may not join that group


PING 226.2.0.1 (226.2.0.1) 56(84) bytes of data.
64 bytes from 192.168.168.104: icmp_seq=1 ttl=64 time=0.904 ms

```

A host with multicast enabled will listen by default to 224.0.0.1, so all the hosts on the network can be pinged.

```
	echo "0" > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts   # run on local host
	ping 10 224.0.0.1		# ping local group address
```

*** Note***
* 224.x.x.x group can't ping normally;


## proc file for multicast group

** /proc/net/dev_mcast**
Contains Layer2 multicast groups which a device is listening to (interface index, label, number of references, number of bound addresses).

** /proc/net/igmp **
Contains IPv4 multicast groups joined by this host.

```
	route add -net 224.0.0.0 netmask 240.0.0.0 eth0
	route -n
```

** /proc/net/igmp6 **
Contains IPv6 multicast groups joined by this host. 


## IGMP protocol

3 kind of packets:
* ***Report***: join group, sent by receiver
* ***Leave***: leave group, sent by receiver;
* ***Query***: sent by router; Whether IGMP snooper send it???

Test with D-Link DXS-1218: IGMP snooper also send QUERY packet in every 125 seconds;

## unsolicited report

Adds the new procfs knobs:

    /proc/sys/net/ipv4/conf/*/igmpv2_unsolicited_report_interval
    /proc/sys/net/ipv4/conf/*/igmpv3_unsolicited_report_interval

Which will allow userspace configuration of the IGMP unsolicited report
interval (see below) in milliseconds.  The defaults are 10000ms for IGMPv2
and 1000ms for IGMPv3 in accordance with RFC2236 and RFC3376.

Background:

If an IGMP join packet is lost you will not receive data sent to the
multicast group. So if no data arrives from that multicast group in a
period of time after the IGMP join, a second IGMP join will be sent.  The
delay between joins is the "IGMP Unsolicited Report Interval".

Prior to this patch this value was hard coded in the kernel to 10s for
IGMPv2 and 1s for IGMPv3.  10s is unsuitable for some use-cases, such as
IPTV as it can cause channel change to be slow in the presence of packet
loss.

This patch allows the value to be overridden from userspace for both
IGMPv2 and IGMPv3 such that it can be tuned accoding to the network.

Tested with Wireshark and a simple program to join a (non-existent)
multicast group.  The distribution of timings for the second join differ
based upon setting the procfs knobs.

igmpvX_unsolicited_report_interval is intended to follow the pattern
established by force_igmp_version, and while a procfs entry has been added
a corresponding sysctl knob has not as it is my understanding that sysctl
is deprecated[1].



Unsolicited Report Interval for IGMP doesn't work?

According to the RFC or help file, "Unsolicited Report Interval" is the period of time (in seconds) between sending repetitions of the initial report of membership in a group.

With a default value of 10 seconds, I expect the IGMP hosts to send Join messages every 10 seconds after initial Join.

But the messages are not being sent periodically, only once the unsolicited reports are sent.

Is my understanding wrong or is this a bug? 

Its just a bit confusing but the RFC indicates it is of the initial join, not after the initial join. See FAQ15377 which indicates.

        This is the time between repetitions of the Host’s “initial report” .
        The repetition of the initial report is specified by the robustness variable.
        The default of the robustness variable is 2. This value can be changed to a higher value but still this is only used for the initial report.
        With this default setting the Customer should be seeing 2 reports.
        After these 2 reports, the only other reports he will see is if the client receives a query from the DUT. 

1 Hide 2 · Share
avatar image Sreerag ♦ · Feb 17, 2015 at 01:51 PM 0

Thanks Mike..

One more question, since robustness variable is for specifying the "number" of repetitions, what does the "interval" stand for? I am not seeing the repeated joins after the specified interval of 10 Seconds. They occur immediately even for a higher robustness variable.

I knew I got confused somewhere :) And this is the right place to seek help.

Thanks again for your time.
avatar image mnakamura ♦ · Feb 17, 2015 at 02:20 PM 0 Like Share

If you found its not working as you just commented, you can open a CR but your initial understanding of the unsolicited report is not for sending continuous reports every X interval, it is only for the initial join.


## force_igmp_version
file: /proc/sys/net/ipv4/force_igmp_version
variable: net.ipv4.force_igmp_version
Official reference

0 - (default) No enforcement of a IGMP version, IGMPv1/v2 fallback allowed. Will back to IGMPv3 mode again if all IGMPv1/v2 Querier Present timer expires. 
1 - Enforce to use IGMP version 1. Will also reply IGMPv1 report if receive IGMPv2/v3 query. 
2 - Enforce to use IGMP version 2. Will fallback to IGMPv1 if receive IGMPv1 query message. Will reply report if receive IGMPv3 query. 
3 - Enforce to use IGMP version 3. The same react with default 0.

Note: this is not the same with force_mld_version because IGMPv3 RFC3376 Security Considerations does not have clear description that we could ignore other version messages completely as MLDv2 RFC3810. So make this value as default 0 is recommended.

```
	sysctl -w net.ipv4.conf.eth0.force_igmp_version=2
```


### IGMP packets sent by 774

TTL=1, so PC can't capture it through switch

```
	sudo tcpdump -vvv -i eth0 igmp

17:27:53.217513 IP (tos 0xc0, ttl 1, id 0, offset 0, flags [DF], proto IGMP (2), length 32, options (RA))
    192.168.168.104 > 225.192.0.10: igmp v2 report 225.192.0.10
17:27:53.217534 IP (tos 0xc0, ttl 1, id 0, offset 0, flags [DF], proto IGMP (2), length 32, options (RA))
    192.168.168.104 > 226.2.0.1: igmp v2 report 226.2.0.1
17:27:54.086621 IP (tos 0xc0, ttl 1, id 0, offset 0, flags [DF], proto IGMP (2), length 32, options (RA))
    192.168.168.104 > 225.192.0.10: igmp v2 report 225.192.0.10
17:27:54.117175 IP (tos 0xc0, ttl 1, id 0, offset 0, flags [DF], proto IGMP (2), length 32, options (RA))
    192.168.168.104 > 225.192.0.10: igmp v2 report 225.192.0.10
17:27:54.166581 IP (tos 0xc0, ttl 1, id 0, offset 0, flags [DF], proto IGMP (2), length 32, options (RA))
    192.168.168.104 > 226.2.0.1: igmp v2 report 226.2.0.1
17:27:54.785898 IP (tos 0xc0, ttl 1, id 0, offset 0, flags [DF], proto IGMP (2), length 32, options (RA))
    192.168.168.104 > 226.2.0.1: igmp v2 report 226.2.0.1
```
