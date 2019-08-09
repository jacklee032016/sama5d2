
#include <linux/sockios.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/route.h>		/*rtentry */
#include <stdint.h>

#include "libCmnSys.h"
#include "mux7xx.h"

#define __IP_DEBUG		0

#define	DEFAULT_MCAST_PORT		65520


uint32_t cmnSysNetGetIp(char * hwName)
{
#if __IP_DEBUG
	char ipAddr[20];
#endif
	struct ifaddrs *ifaddr, *ifa;
	int family;
	uint32_t ip;

	if(hwName == NULL)
	{
		return MUX_INVALIDATE_INTEGER;
	}
	if (getifaddrs(&ifaddr) == -1)
	{
		MUX_ERROR("getifaddrs: %s", strerror(errno) );
		return MUX_INVALIDATE_INTEGER;
	}

	/* Walk through linked list, maintaining head pointer so we
	 can free list later */

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
		{
			continue;
		}

		family = ifa->ifa_addr->sa_family;

		/* Display interface name and family (including symbolic
		 form of the latter for the common families) */

		/* For an AF_INET* interface address, display the address */
		if (strcmp(ifa->ifa_name, hwName) == 0)
		{
			if (family == AF_INET || family == AF_INET6)
			{
				ip = (uint32_t) ((struct sockaddr_in *) ifa->ifa_addr)->sin_addr.s_addr;
				
#if __IP_DEBUG
				void *tmpAddrPtr = &ip;
				memset(ipAddr, 0, sizeof(ipAddr) );
				inet_ntop(AF_INET, tmpAddrPtr, ipAddr, INET_ADDRSTRLEN);
				MUX_DEBUG("%s, 0x%x", ipAddr, ip);
#endif

#if IP_ADDRESS_IN_NET_ORDER
#else
				ip = ntohl(ip);
#endif				
				freeifaddrs(ifaddr);
				return ip;
			}
		}
	}
	
	freeifaddrs(ifaddr);
	
	return MUX_INVALIDATE_INTEGER;
}


uint32_t cmnSysNetGetMask(char *hwName)
{
#if __IP_DEBUG
	char maskStr[20];
	struct sockaddr_in *pAddr;
#endif
	
	struct ifreq ifr;
	int sockfd;
	uint32_t mask;

	if (hwName == NULL)
	{
		return MUX_INVALIDATE_INTEGER;
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		return MUX_INVALIDATE_INTEGER;
	}
	
	ifr.ifr_addr.sa_family = AF_INET;
//	strcpy(ifr.ifr_name, hwName);
	strncpy(ifr.ifr_name , hwName , IFNAMSIZ-1);

	if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0)
	{
		MUX_ERROR("SIOCGIFADDR socket failed: %s", strerror(errno) );
		close(sockfd);
		return MUX_INVALIDATE_INTEGER;
	}

	
	mask = (uint32_t) ((struct sockaddr_in *)  &(ifr.ifr_addr))->sin_addr.s_addr;
	
#if IP_ADDRESS_IN_NET_ORDER
#else
	mask = ntohl(mask);
#endif

#if __IP_DEBUG
	pAddr = (struct sockaddr_in *) &(ifr.ifr_addr);
	snprintf(maskStr, sizeof(maskStr), "%s",  (char *)(inet_ntoa(pAddr->sin_addr)));
	MUX_DEBUG("mask : %s, 0x%x", maskStr, mask )
#endif
	close(sockfd);

	return mask;
}

uint32_t cmnSysNetGetDefaultGw(char *hwName)
{
	FILE *f;
	char line[100] , *ifName , *dest, *gw, *saveptr;
	uint32_t gwIp = MUX_INVALIDATE_INTEGER;
//	int nRet=1;

	f = fopen(LINUX_SYS_ROUTE_PROC, "r");
	if(f == NULL)
	{
		MUX_ERROR("Open proc route failed: %s", strerror(errno) );
		return gwIp;
	}

	while(fgets(line , 100 , f))
	{
		ifName = strtok_r(line , " \t", &saveptr);
		dest = strtok_r(NULL , " \t", &saveptr);
		gw = strtok_r(NULL , " \t", &saveptr);

		if(ifName !=NULL && dest!=NULL )
		{
			if(strcmp(dest , "00000000") == 0)
			{
				//printf("Default interface is : %s \n" , p);
				if (gw)
				{
					char *pEnd;
					gwIp = strtol(gw, &pEnd, 16);
#if IP_ADDRESS_IN_NET_ORDER
#else
					gwIp = ntohl(gwIp);
#endif

#if 0
					//ng=ntohl(ng);
					struct in_addr addr;
					addr.s_addr=ng;
					gw=std::string( inet_ntoa(addr) );
					nRet=0;
#endif				
				}
				break;
			}
		}
	}

	fclose(f);
	
	return gwIp;
}


int cmnSysNetSetGateway(char *gateway, char *devName)
{
	int sockfd;
	struct rtentry rt;	/* route entry */

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1)
	{
		EXT_ERRORF("socket creation failed: %m");
		return EXIT_FAILURE;
	}

	struct sockaddr_in *sockinfo = (struct sockaddr_in *)&rt.rt_gateway;
	sockinfo->sin_family = AF_INET;
	sockinfo->sin_addr.s_addr = inet_addr(gateway);

	sockinfo = (struct sockaddr_in *)&rt.rt_dst;
	sockinfo->sin_family = AF_INET;
	sockinfo->sin_addr.s_addr = INADDR_ANY;

	sockinfo = (struct sockaddr_in *)&rt.rt_genmask;
	sockinfo->sin_family = AF_INET;
	sockinfo->sin_addr.s_addr = INADDR_ANY;

	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	rt.rt_dev = devName;

	if (ioctl(sockfd, SIOCADDRT, &rt) < 0)
	{
		EXT_ERRORF("set gateway of %s as %s failed: %m", devName, gateway);
		close(sockfd);
		return EXIT_FAILURE;
	}

	close(sockfd);
	
	return EXIT_FAILURE;
}



char *cmnSysNetAddress( uint32_t address)
{
	static char addressStr[20];
	struct sockaddr_in sockAddr;
#if IP_ADDRESS_IN_NET_ORDER	
	sockAddr.sin_addr.s_addr = address;
#else
	sockAddr.sin_addr.s_addr = htonl(address);
#endif
	snprintf(addressStr, sizeof(addressStr), "%s",  (char *)(inet_ntoa(sockAddr.sin_addr)));
	
#if __IP_DEBUG
	MUX_DEBUG("Address : %s, 0x%x", addressStr, address );
#endif

	return addressStr;
}


/* only used to parse data from REST or IP command */
uint32_t cmnSystemNetIp(char *ip)
{/* INADDR_NONE (usually -1) */
	uint32_t _ipAddr = inet_addr(ip);	/* return network byte-order */
	if(_ipAddr == INADDR_NONE)
	{/* it also is validate address */
		return INVALIDATE_VALUE_U32;
	}

#if IP_ADDRESS_IN_NET_ORDER	
	return _ipAddr;
#else
	return ntohl(_ipAddr);
#endif
}

int cmnSysNetGetMacAddress(char *hwName, EXT_MAC_ADDRESS *mac)
{
	struct ifreq ifr;
	int sockfd;
	int ret = EXIT_FAILURE;

	if (hwName == NULL)
	{
		return ret;
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		return ret;
	}
	
	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , hwName , IFNAMSIZ-1);

	if (0 == ioctl(sockfd, SIOCGIFHWADDR, &ifr))
	{
		memcpy(mac->address, (unsigned char *)ifr.ifr_hwaddr.sa_data, EXT_MAC_ADDRESS_LENGTH);

		ret = EXIT_SUCCESS;
    	}
	
#if __IP_DEBUG
	MUX_DEBUG("Mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X" , mac->address[0], mac->address[1], mac->address[2], mac->address[3], mac->address[4], mac->address[5]);
#endif
	close(sockfd);

	return ret;
}

int cmnSysNetInterfaceIndex(int fd, const char *devName)
{
	struct ifreq ifreq;
	int err;

	memset(&ifreq, 0, sizeof(ifreq));
	strncpy(ifreq.ifr_name, devName, sizeof(ifreq.ifr_name) - 1);
	
	err = ioctl(fd, SIOCGIFINDEX, &ifreq);
	if (err < 0)
	{
		EXT_ERRORF("ioctl SIOCGIFINDEX failed: %m");
		return err;
	}
	
	return ifreq.ifr_ifindex;
}


static int _cmnSysNetMcastJoin(CmnMultiGroup *_group, char *groupIp)
{
#define	__WITH_INDEX		1	/* define source interface with index or IP address */
	int err;
//	int off = 0;
#if __WITH_INDEX
	struct ip_mreqn req;
#else	
	struct ip_mreq req;
#endif

	uint32_t	_gIp = cmnSystemNetIp(groupIp);/* */
	if(_gIp == INVALIDATE_VALUE_U32)
	{
		EXT_ERRORF("'%s' is not validate IP address", groupIp);
		return EXIT_FAILURE;
	}

	memset(&req, 0, sizeof(req));

	/* define source interface */
#if __WITH_INDEX
	req.imr_ifindex = _group->ifIndex;
#else
	req.imr_interface.s_addr = htonl(INADDR_ANY);
#endif

	if(_group->address != INVALIDATE_VALUE_U32)
	{
		req.imr_multiaddr.s_addr = _group->address;
		err = setsockopt(_group->socket, IPPROTO_IP, IP_DROP_MEMBERSHIP , &req, sizeof(req));
		if (err)
		{
			EXT_ERRORF("Leave multicast group '%s' failed: %m", _group->groupAddress);
//			return EXIT_FAILURE;
		}
		else
		{
			EXT_DEBUGF(DEBUG_SYS_NET, "Leave multicast group '%s' successfully", _group->groupAddress );
		}
	}

	req.imr_multiaddr.s_addr = _gIp;
	err = setsockopt(_group->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req));
	if (err)
	{
		EXT_ERRORF("Join multicast group '%s' failed: %m", groupIp);
		_group->address = INVALIDATE_VALUE_U32;
		return EXIT_FAILURE;
	}

	_group->address = _gIp;
	snprintf(_group->groupAddress, sizeof(_group->groupAddress), "%s", groupIp);
	EXT_DEBUGF(DEBUG_SYS_NET, "Join multicast group '%s' successfully", _group->groupAddress );

#if 0	
	err = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &off, sizeof(off));
	if (err)
	{
		pr_err("setsockopt IP_MULTICAST_LOOP failed: %m");
		return -1;
	}
#endif

	return EXIT_SUCCESS;
}

static CmnMultiGroup _mGroup;

CmnMultiGroup *cmnSysNetMGroupInit(const char *devName, char *groupIp)
{
	CmnMultiGroup *_group = &_mGroup;	
	struct sockaddr_in addr;
//	int ttl = 10;
	int on = 1;
	
	memset(_group, 0, sizeof(CmnMultiGroup));
	_group->address = INVALIDATE_VALUE_U32;
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(DEFAULT_MCAST_PORT);

	_group->socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(_group->socket < 0)
	{
		EXT_ERRORF("socket failed: %m");
		return NULL;
	}
	
	_group->ifIndex = cmnSysNetInterfaceIndex(_group->socket, devName);
	if (_group->ifIndex < 0)
	{
		goto no_option;
	}
	snprintf(_group->devName, sizeof(_group->devName), "%s", devName);
	_group->changeGroup = _cmnSysNetMcastJoin;

	EXT_DEBUGF(EXT_DBG_OFF, "Index of %s is %d",_group->devName, _group->ifIndex );
#if 1
	if (setsockopt(_group->socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
	{
		EXT_ERRORF("setsockopt SO_REUSEADDR failed: %m");
		goto no_option;
	}
#endif

	if (bind(_group->socket, (struct sockaddr *) &addr, sizeof(addr)))
	{
		EXT_ERRORF("bind failed: %m");
		goto no_option;
	}

	if (setsockopt(_group->socket, SOL_SOCKET, SO_BINDTODEVICE, devName, strlen(devName)))
	{/* only root can call it */
		EXT_ERRORF("setsockopt SO_BINDTODEVICE failed: %m");
		goto no_option;
	}
	
#if 0	
	if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)))
	{
		pr_err("setsockopt IP_MULTICAST_TTL failed: %m");
		goto no_option;
	}
#endif

	if(groupIp)
	{
		if (_cmnSysNetMcastJoin(_group, groupIp) )
		{
			EXT_ERRORF("mcast join failed");
			goto no_option;
		}
	}

	return _group;
	
no_option:
	close(_group->socket);

	return NULL;
}


void cmnSysNetMGroupDestory(CmnMultiGroup *_group)
{
	close(_group->socket);
	memset(_group, 0, sizeof(CmnMultiGroup));
	_group->address = INVALIDATE_VALUE_U32;
}

int cmnSysNetMulticastIP4Mac(uint32_t	ipAddress, EXT_MAC_ADDRESS *macAddress)
{
	if(IP4_ADDR_IS_MULTICAST(ipAddress) )
	{/* Hash IP multicast address to MAC address.*/
		macAddress->address[0] = LL_IP4_MULTICAST_ADDR_0;
		macAddress->address[1] = LL_IP4_MULTICAST_ADDR_1;
		macAddress->address[2] = LL_IP4_MULTICAST_ADDR_2;
		macAddress->address[3] = ip4_addr2(ipAddress) & 0x7f;
		macAddress->address[4] = ip4_addr3(ipAddress);
		macAddress->address[5] = ip4_addr4(ipAddress);

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

void cmnSysNetChangeByteOrderOfMac(EXT_MAC_ADDRESS *mac, unsigned char *address)
{
	int i;
	for(i=0; i<EXT_MAC_ADDRESS_LENGTH; i++ )
	{
		address[i] = mac->address[EXT_MAC_ADDRESS_LENGTH-i-1];
	}
}


