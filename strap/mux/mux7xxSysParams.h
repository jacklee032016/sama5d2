

#define	EXT_MAGIC_VALUE_A				0xA5
#define	EXT_MAGIC_VALUE_B				0x5A

#define	EXT_MAC_ADDRESS_LENGTH			6

#define	EXT_IP_CFG_DHCP_ENABLE			(1<<0)
#define	EXT_IP_CFG_MC_ENABLE				(1<<1)	/* multicast */
#define	EXT_IP_CFG_ARP_ENABLE			(1<<2)		

/* configuration options for 767 board */
/** MAC address definition.  The MAC address must be unique on the network. */
#define ETHERNET_CONF_ETHADDR0                        0x00
#define ETHERNET_CONF_ETHADDR1                        0x04
#define ETHERNET_CONF_ETHADDR2                        0x25
#define ETHERNET_CONF_ETHADDR3                        0x1C
#if EXTLAB_BOARD
#define ETHERNET_CONF_ETHADDR5                        0x01
#else
#define ETHERNET_CONF_ETHADDR5                        0x02
#endif

#define ETHERNET_CONF_ETHADDR4_TX			0x10
#define ETHERNET_CONF_ETHADDR4_RX			0xA0


#define	MCAST_DEFAULT_IPADDR0				239
#define	MCAST_DEFAULT_IPADDR1				100
#define	MCAST_DEFAULT_IPADDR2				1
#define	MCAST_DEFAULT_IPADDR3				1


/** The IP address being used. */
#define ETHERNET_CONF_IPADDR0					192
#define ETHERNET_CONF_IPADDR1					168
#if ARCH_ARM
	#if EXTLAB_BOARD
	#define ETHERNET_CONF_IPADDR2_TX			168
	#define ETHERNET_CONF_IPADDR2_RX			168
	#define ETHERNET_CONF_IPADDR3_TX			64
	#else
	#define ETHERNET_CONF_IPADDR2_TX			169
	#define ETHERNET_CONF_IPADDR2_RX			169
	#define ETHERNET_CONF_IPADDR3_TX			121
	#endif
#else
	/* X86 test environment */
	#define ETHERNET_CONF_IPADDR2_TX			166
	#define ETHERNET_CONF_IPADDR2_RX			167
	#define ETHERNET_CONF_IPADDR3_TX			2
#endif

#if ARCH_ARM
#define ETHERNET_CONF_IPADDR3_RX				65
#else
#define ETHERNET_CONF_IPADDR3_RX				3
#endif

#ifdef	ARM
#define	CONF_SVR811_IPADDR2						168 		/* X86-->board */
//#define	CONF_SVR811_IPADDR3						102 		/* test with my computer */
#define	CONF_SVR811_IPADDR3						50
#else
#define	CONF_SVR811_IPADDR2						166 		/* simhost */
#define	CONF_SVR811_IPADDR3						1 		/* simhost */
#endif
#define	CONF_SVR811_PORT							3840


/** The gateway address being used. */
#define ETHERNET_CONF_GATEWAY_ADDR0                   192
#define ETHERNET_CONF_GATEWAY_ADDR1                   168
#if ARCH_ARM
	#if EXTLAB_BOARD
	#define ETHERNET_CONF_GATEWAY_ADDR2_TX			168
	#define ETHERNET_CONF_GATEWAY_ADDR2_RX			168
	#else
	#define ETHERNET_CONF_GATEWAY_ADDR2_TX			169
	#define ETHERNET_CONF_GATEWAY_ADDR2_RX			169
	#endif
#else
	#define ETHERNET_CONF_GATEWAY_ADDR2_TX			166
	#define ETHERNET_CONF_GATEWAY_ADDR2_RX			167
#endif

#define ETHERNET_CONF_GATEWAY_ADDR3                   1

/** The network mask being used. */
#define ETHERNET_CONF_NET_MASK0                       255
#define ETHERNET_CONF_NET_MASK1                       255
#define ETHERNET_CONF_NET_MASK2                       255
#define ETHERNET_CONF_NET_MASK3                       0

#define	EXT_HTTP_SVR_PORT						80

#define	EXT_SDP_SVR_PORT						80

#define	EXT_MEDIA_PORT_RANGE					10		/* range of RTP port */

#define	EXT_MEDIA_PORT_TX_VIDEO				40000
#define	EXT_MEDIA_PORT_TX_AUDIO				40010
#define	EXT_MEDIA_PORT_TX_DATA				40020
#define	EXT_MEDIA_PORT_TX_STREA				40030

#define	EXT_MEDIA_PORT_RX_VIDEO				48000
#define	EXT_MEDIA_PORT_RX_AUDIO				48010
#define	EXT_MEDIA_PORT_RX_DATA				48020
#define	EXT_MEDIA_PORT_RX_STREA				48030


