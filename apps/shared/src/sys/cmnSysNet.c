
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

#include <stdint.h>

#include "libCmn.h"
#include "mux7xx.h"

#define __IP_DEBUG		1

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
				ip = ntohl(ip);
				
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
	mask = ntohl(mask);
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
					gwIp = ntohl(gwIp);
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


char *cmnSysNetAddress( uint32_t address)
{
	static char addressStr[20];
	struct sockaddr_in sockAddr;
	
	sockAddr.sin_addr.s_addr = htonl(address);
	snprintf(addressStr, sizeof(addressStr), "%s",  (char *)(inet_ntoa(sockAddr.sin_addr)));
	
#if __IP_DEBUG
	MUX_DEBUG("Address : %s, 0x%x", addressStr, address );
#endif

	return addressStr;
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
	MUX_DEBUG("Mac : %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n" , mac->address[0], mac->address[1], mac->address[2], mac->address[3], mac->address[4], mac->address[5]);
#endif
	close(sockfd);

	return ret;
}


