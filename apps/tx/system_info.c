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
#include "system_info.h"


int readParamFromDrive(char * partitionName, char * param, int bufLens) 
{
	int fd;
	if (!param) 	
		return -1;	
	fd = open(partitionName, O_RDONLY);
	if (fd < 0) 
	{
		printf("failed to open block\n");
		return -1;
	}
	int len = read(fd, param, bufLens);
	if (len == -1) 
	{
		printf("failed to read data from block\n");
		close(fd);
		return -1;
	}	
	close(fd);
	return 0;
}

int saveParamToDrive(char * partitionName, char * param, int strlength) 
{
	int fd;
	if (!param) 
		return -1;	

	char tmp[BLOCK_LENGTH];
	memset(tmp, 0, sizeof(tmp));
	if(strlength < BLOCK_LENGTH)
		memcpy(tmp, param, strlength);
	else
		memcpy(tmp, param, BLOCK_LENGTH);

	fd = open(partitionName, O_WRONLY | O_TRUNC);
	if (fd < 0) 
	{
		printf("failed to open block\n");
		return -1;
	}

	int len = write(fd, tmp, sizeof(tmp));
	if (len == -1) 
	{
		printf("failed to write data to block\n");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

int eraseParamDrive(char * partitionName) 
{
	int fd;
	char tmp[BLOCK_LENGTH];
	memset(tmp, 0, sizeof(tmp));	
	
	fd = open(partitionName, O_WRONLY | O_TRUNC);
	if (fd < 0) 
	{
		printf("failed to open block\n");
		return -1;
	}

	int len = write(fd, tmp, sizeof(tmp));
	if (len == -1) 
	{
		printf("failed to write data to block\n");
		close(fd);
		return -2;
	}

	close(fd);
	return 0;
}

