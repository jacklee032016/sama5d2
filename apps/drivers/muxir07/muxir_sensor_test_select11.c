#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h> 

#define IR_MAGIC 'I'
#define IR_SENSOR_MODE	  _IO(IR_MAGIC, 0)  
#define IR_EMITTER_MODE	  _IO(IR_MAGIC, 1)  

#define SYMBOL_CNT 200

// /home/Hi3798SPC040/HiSTBLinuxV100R005C00SPC040/tools/linux/toolchains/arm-histbv310-linux/bin/arm-gcc49-linux-gnueabi-gcc



int main(int argc,char **argv)
{
	int fd;
	int cmd;
	int i, j, ret;	
	unsigned char inputcmd[32];
	unsigned short int buf[SYMBOL_CNT];
	fd_set	fds;
	struct timeval timeout;
	
	timeout.tv_usec = 0;  
	timeout.tv_sec = 10;
	
	fd =open("/dev/muxir",O_RDWR|O_NONBLOCK);
	if(fd < 0)
	{
		printf("can not open /dev/muxir!\n");
		return -1;
	}
	ioctl(fd, IR_SENSOR_MODE, 0);
	
	//printf("\n Non block read\n");
	//ret = read(fd, &buf, sizeof(buf));
	//printf("\n First Non block read return %d\n", ret);
	FD_ZERO(&fds);
	FD_SET(fd, &fds);	

	while (select(fd + 1, &fds, NULL, NULL, &timeout) > 0)
	{
		ret = read(fd, &buf, sizeof(buf));		
		printf("\n return ret = %d", ret);
		printf("\n IR carrier freq = %d", buf[0]);
		printf("\n symbol cnt = %d\n", buf[1]);
		j = 0;
		for( i = 2; i <= (buf[1]+5); i++)
		{	
			//printf(" %7d ", buf[i]);
			printf(" %6u ", buf[i]);
			//if( (((i-2)%10) == 0) && (i != 12))
			//	printf("\n");
		    j++;
			if((j%10) == 0)
				printf("\n");
		}	
	}	
	printf("\n IR read time out. Exit!");	
	close(fd);
    return 0;
}




/*			//block mode 
int main(int argc,char **argv)
{
	int fd;
	int cmd;
	int i, j, ret;	
	unsigned char inputcmd[32];
	unsigned int buf[SYMBOL_CNT];

	fd =open("/dev/muxir",O_RDWR);
	if(fd < 0)
	{
		printf("can not open /dev/muxir!\n");
		return -1;
	}

	ioctl(fd, IR_SENSOR_MODE, 0);

	while(1)
	{
		ret = read(fd, &buf, sizeof(buf));
		printf("\n");
		printf("\n IR carrier freq = %d", buf[0]);
		printf("\n symbol cnt = %d\n", buf[1]);
		j = 0;
		for( i = 2; i <= (buf[1]+5); i++)
		{	
			//printf(" %7d ", buf[i]);
			printf(" %6u ", buf[i]);
			//if( (((i-2)%10) == 0) && (i != 12))
			//	printf("\n");
		    j++;
			if((j%10) == 0)
				printf("\n");
		}
		
	}
	
	close(fd);
    return 0;
}
	
*/
	/*
	unsigned int buf[SYMBOL_CNT];
	struct sockaddr_in myaddr, remaddr;
	int net_fd, slen;
	char val;
	int ret;
	char *server = "192.168.168.220";
	slen = sizeof(remaddr);
	fd =open("/dev/muxir",O_RDWR);
	if(fd < 0 )
	{
		printf("can not open /dev/muxir!\n");
		return -1;
	}
	
	if ( (net_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )
		printf("socket created error!\n");
	// bind it to all local addresses and pick any port number
	memset(&myaddr, 0, sizeof(struct sockaddr_in));	
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);	
	if (bind(net_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) 
	{
		printf("\nbind failed");
		return 0;
	}   
	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(6666);
	if (inet_aton(server, &remaddr.sin_addr)==0) 
	{
		printf("inet_aton() failed\n");		
	}
	
	while (1) 
	{
        ret = read(fd, &buf, sizeof(buf));
		printf("\n");
		printf("\n IR carrier freq = %d", buf[0]);
		printf("\n symbol cnt = %d\n", buf[1]);
		j = 0;
		for( i = 2; i <= (buf[1]+5); i++)
		{	
			printf(" %7d ", buf[i]);
			//if( (((i-2)%10) == 0) && (i != 12))
			//	printf("\n");
		    j++;
			if((j%10) == 0)
				printf("\n");
		}
		if(sendto(net_fd, buf, sizeof(buf), 0, (struct sockaddr *)&remaddr, slen)==-1)
			printf("\n Send IR data to network error!");
		printf("\nData finish.");
		printf("\n");
		
    }
    close(fd);
	close(net_fd);
	*/
