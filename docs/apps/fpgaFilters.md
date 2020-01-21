
# 

After some tests, I think FPGA should add some functions filtering most of multicast traffic, otherwise 
MCU can't receive other traffic, such as ping, http.

But some multicast group must be sent to MCU:
* 224.0.0.1/224.0.0.2: IGMP group;
* Normally, 224.x.x.x are group address used for well-known services, such as bonjour, routing protocols;

At same time, in RX, FPGA should guarantee IGMP Query( in group 224.0.0.x), from switch's IGMP Snooper, 
will be sent to MCU; otherwise RX can't receive multicast traffic for about 125 seconds util next IGMP
Query be received. (refers to RFC2236)
