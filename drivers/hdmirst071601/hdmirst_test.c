#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdio.h>
#include <unistd.h>
// /home/alex/buildroot/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-gcc hdmirst_test.c -static -o hdmirst_test

#define HDMI_RST_MAGIC 'H'
#define HDMI_RST_HIGH  	_IO(HDMI_RST_MAGIC,0)  
#define HDMI_RST_LOW 	_IO(HDMI_RST_MAGIC,1)


int main(int argc,char **argv)
{
	int fd;
	int cmd;
	int i;
	char val;
	fd = open("/dev/hdmirst",O_RDWR);
	if(fd < 0 )
	{
		printf("can not open /dev/hdmirst!\n");
		return -1;
	}	
	
	for( i = 0; i <  50; i++)
	{
		usleep(300000);
		ioctl(fd, HDMI_RST_HIGH,0);  
		usleep(300000);
		ioctl(fd, HDMI_RST_LOW,0);  	
	}
	ioctl(fd, HDMI_RST_HIGH,0);
	close(fd);
	return 0;		
}


