
#include "libCmnSys.h"

#define CMN_MODULE_TEST_NAME			"testsMux" //module name

#define	TEST_CMD_IGMP					"igmp"
#define	TEST_CMD_DIP_SWITCH			"dip"

#define	TEST_CMD_LED_VIDEO			"ledV"
#define	TEST_CMD_LED_POWER			"ledP"
#define	TEST_CMD_LED_ACT				"ledA"

#define	TEST_CMD_LEDS					"led"
#define	TEST_CMD_SW1					"sw1"
#define	TEST_CMD_SECURE_CHIP			"sc"

#define	TEST_CMD_I2C					"i2c"
#define	TEST_CMD_SPI					"spi"

int testIgmp(void);
int testDipSwitch(void);
int testLedVideo(void);
int testLeds(void);
int testSw1(void);
int testSecureChipId(void);

int testDs2815(void );
int testSC(void );
int testI2c(void);

static void _usage(char* base)
{
	printf("%s: \n\tTest command line utility.\n"\
		"\t%s -c command "
		"\t\t Current command:  " \
		"\n\t\t\t"TEST_CMD_IGMP", "TEST_CMD_DIP_SWITCH", "TEST_CMD_LEDS", "TEST_CMD_SW1", "TEST_CMD_SECURE_CHIP", "TEST_CMD_I2C ", "TEST_CMD_SPI" \n", 
		  base, base);

	exit(-1);
}

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <math.h>

//#define SPI_DEVICE 	"/dev/spidev32766.0"
#define SPI_CMD_WRITE_STATUS_REGISTER	0x01
#define SPI_CMD_WRITE					0x02
#define SPI_CMD_READ					0x03
#define SPI_CMD_WRITE_DISABLE			0x04
#define SPI_CMD_READ_STATUS				0x05
#define SPI_CMD_WRITE_ENABLE			0x06
#define SPI_CMD_CHIP_ERASE				0xC7
#define SPI_CMD_CLEAR_AREA					0xD8
#define SPI_CMD_READ_DEVICE_ID				0x9E
#define SPI_CMD_ENTER_4BYTE_AD_MODE		0xB7
#define SPI_CMD_EXIT_4BYTE_AD_MODE 		0xE9
#define SPI_CMD_READ_4BYTE 				0x13
#define SPI_CMD_SECTOR_ERASE_4BYTE		0xDC	//64KB
#define SPI_CMD_PAGE_PROGRAM_4BYTE  	0x12
#define SPI_PAGE_SIZE					256

#define _SPI_DEVICE 	"/dev/spidev0.0"

int spi_read_id(int fd)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[5], data_buf[5];
	int	ret;
	int flash_id;
	
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	
	cmd_buf[0] = SPI_CMD_READ_DEVICE_ID;	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = 5; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		EXT_ERRORF("read ID SPI_IOC_MESSAGE fail.");
		return -1;
	}
	
	flash_id = (data_buf[1]<<24) + (data_buf[2]<<16) + (data_buf[3]<<8) + data_buf[4];	
	return flash_id;
}

int spi_init(int fd, uint8_t mode, uint8_t bits, uint32_t speed)
{
	int ret;
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
		EXT_ERRORF("can't set spi mode");
		return -1;
	}
	
	// bits per word	
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		EXT_ERRORF("can't set bits per word");
		return -1;
	}
	
	//max speed hz	
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{	
		EXT_ERRORF("can't set max speed hz.");
		return -1;
	}
	return 0;
}


int	testSpiFlash(void)
{
	uint8_t mode; 
	uint8_t bits; 
	uint32_t speed;
	//spi parameter
	mode = 0;
	bits = 8;
	speed = 2000000;	
	
	int fd;
	fd = open(_SPI_DEVICE, O_RDWR);
	if(fd < 0)
	{
		EXT_ERRORF("can't open spi device: %s.", _SPI_DEVICE);
		return -1;
	}
	
	int ret = spi_init(fd, mode, bits, speed);
	if(ret < 0)
	{
		EXT_ERRORF("Spi device init error.");
		return -1;
	}
	
	int flash_id = spi_read_id(fd);
	EXT_INFOF("Read flash ID #1:0x%x", flash_id);
	flash_id = spi_read_id(fd);
	if(flash_id == 0)
	{
		EXT_ERRORF("Spi device read id error.");
	}
	else
		EXT_INFOF("Flash_id = 0x%X.", flash_id);			

	return 0;

}

int main(int argc, char *argv[])
{
	int opt;
	char		cmd[CMN_NAME_LENGTH];

	while ((opt = getopt (argc, argv, "c:")) != -1)
	{
		switch (opt)
		{

			case 'c': /* command */
				snprintf(cmd, sizeof(cmd), "%s", optarg);
				break;

			default:
				_usage(argv[0]);
		}

	}

	printf(CMN_VERSION_INFO(CMN_MODULE_TEST_NAME) EXT_NEW_LINE );

	if( IS_STRING_NULL_OR_ZERO(cmd) )
	{
		printf("Command is not defined\n");
		_usage(argv[0]);
	}
	
	if(IS_STRING_EQUAL(cmd, TEST_CMD_IGMP) )
	{
		testIgmp();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_DIP_SWITCH))
	{
		testDipSwitch();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_LEDS))
	{
		testLeds();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_SW1))
	{
		testSw1();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_SPI))
	{
		testSpiFlash();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_SECURE_CHIP))
	{
		testSC();
//		testDs2815();
//		testSecureChipId();
	}
	else if(IS_STRING_EQUAL(cmd, TEST_CMD_I2C))
	{
		testI2c();
	}
	else
	{
		printf("Unknonw command '%s' is defined\n", cmd);
		_usage(argv[0]);
	}
		
	return 0;
}

