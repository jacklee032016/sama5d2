
#include "libCmn.h"
#include "libMux.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/watchdog.h>

#include "mux7xx.h"


int muxReboot(MuxMain *muxMain)
{
	MUX_DEBUG("Reboot...");					

	//Cleanup system and reboot					
	system("/usr/sbin/avahi-daemon -k");
	sync(); //
	system("/bin/umount -a -r");	//unmount sd card, usb key, ...
	reboot(RB_AUTOBOOT);							
	return EXIT_SUCCESS;
}

int muxReset(MuxMain *muxMain)
{
	MUX_DEBUG("Reset to factory default...");
	
	SET_LED_BLINK(muxMain);
	
	sleep(5);
	eraseParamDrive(CONFIG_INI);
	eraseParamDrive(EDID_CONFIG_INI);

	reboot(RB_AUTOBOOT);	

	muxReboot();
	return EXIT_SUCCESS;
}


unsigned int ASCII_2_HEX(unsigned char * ASCIIBuffer)
{
	unsigned int i;
	// First charecter
	if ((ASCIIBuffer[0] >= 0x30) && (ASCIIBuffer[0] <= 0x39))
	{
		i = (ASCIIBuffer[0] - 0x30) << 12;
	}
	else
	{
		if (((ASCIIBuffer[0] >= 0x41) && (ASCIIBuffer[0] <= 0x46)) || ((ASCIIBuffer[0] >= 0x61) && (ASCIIBuffer[0] <= 0x66)))
		{
			i = ((ASCIIBuffer[0] & 0x0F) + 9) << 12;
		}

	}
	//  Secound charecter
	if ((ASCIIBuffer[1] >= 0x30) && (ASCIIBuffer[1] <= 0x39))
	{
		i = i | (ASCIIBuffer[1] - 0x30) << 8;
	}
	else
	{
		if (((ASCIIBuffer[1] >= 0x41) && (ASCIIBuffer[1] <= 0x46)) || ((ASCIIBuffer[1] >= 0x61) && (ASCIIBuffer[1] <= 0x66)))
		{
			i = i | ((ASCIIBuffer[1] & 0x0F) + 9) << 8;
		}
	}


	// Third charecter
	if ((ASCIIBuffer[2] >= 0x30) && (ASCIIBuffer[2] <= 0x39))
	{
		i = (ASCIIBuffer[2] - 0x30) << 4;
	}
	else
	{
		if (((ASCIIBuffer[2] >= 0x41) && (ASCIIBuffer[2] <= 0x46)) || ((ASCIIBuffer[2] >= 0x61) && (ASCIIBuffer[2] <= 0x66)))
		{
			i = ((ASCIIBuffer[2] & 0x0F) + 9) << 4;
		}

	}

	// Fourth charecter
	if ((ASCIIBuffer[3] >= 0x30) && (ASCIIBuffer[3] <= 0x39))
	{
		i = i | (ASCIIBuffer[3] - 0x30);
	}
	else
	{
		if (((ASCIIBuffer[3] >= 0x41) && (ASCIIBuffer[3] <= 0x46)) || ((ASCIIBuffer[3] >= 0x61) && (ASCIIBuffer[3] <= 0x66)))
		{
			i = i | ((ASCIIBuffer[3] & 0x0F) + 9);
		}
	}
	
	
	return (i);
}

#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


#define EXP_RST_MAGIC 'P'
#define EXP_RST_HIGH  	_IO(EXP_RST_MAGIC,0)  
#define EXP_RST_LOW 	_IO(EXP_RST_MAGIC,1)

#define HDMI_RST_MAGIC 'H'
#define HDMI_RST_HIGH  _IO(HDMI_RST_MAGIC,0)  
#define HDMI_RST_LOW _IO(HDMI_RST_MAGIC,1)

void EXP_reset(void)
{
	int fd;
	int cmd;
	int i;
	char val;
	fd = open(MUX_DEV_EXP_RESET, O_RDWR);
	if (fd < 0)
	{
		printf("can not open "MUX_DEV_EXP_RESET);
		//return -1;
	}  

	ioctl(fd, EXP_RST_LOW, 0); 
	
	usleep(50000);
	ioctl(fd, EXP_RST_HIGH, 0); 
	close(fd);
}


void HDMI_reset(void)
{
	int fd;
	int cmd;
	int i;
	char val;
	fd = open(MUX_DEV_HDMI_RESET, O_RDWR);
	if (fd < 0)
	{
		printf("can not open "MUX_DEV_HDMI_RESET);
		//return -1;
	}  

	ioctl(fd, HDMI_RST_LOW, 0);  
	usleep(500000);
	ioctl(fd, HDMI_RST_HIGH, 0);
	close(fd);
	printf("HDMI reset\n");

}

