#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdio.h>
#include <unistd.h>
// /home/alex/buildroot/output/host/bin/arm-buildroot-linux-gnueabihf-gcc exprst_test.c -static -o exprst_test

#define EXP_RST_MAGIC 'P'
#define EXP_RST_HIGH  	_IO(EXP_RST_MAGIC,0)  
#define EXP_RST_LOW 	_IO(EXP_RST_MAGIC,1)


int main(int argc,char **argv)
{
	int fd;
	int cmd;
	int i;
	char val;
	fd = open("/dev/exprst",O_RDWR);
	if(fd < 0 )
	{
		printf("can not open /dev/exprst!\n");
		return -1;
	}	
	
	for( i = 0; i < 30; i++)
	{
		usleep(300000);
		ioctl(fd, EXP_RST_HIGH,0);  
		usleep(300000);
		ioctl(fd, EXP_RST_LOW,0);  	
	}
	ioctl(fd, EXP_RST_HIGH, 0); 
	close(fd);
	return 0;		
}


