#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "libCmnSys.h"
#include "mux7xx.h"


static struct spi_ioc_transfer _xfer[2];
#define SPI_TST_AH 0x00
#define SPI_TST_AL 0x01
#define SPI_TST_DH 0x5A
#define SPI_TST_DL 0xC3

int cmnSysSpiInit(char *filename)
{
	int fd;
	uint8_t    mode = MUX_INVALIDATE_CHAR, lsb = MUX_INVALIDATE_CHAR, bits = MUX_INVALIDATE_CHAR;
	uint32_t	speed=2500000;
 
	if ((fd = open(filename, O_RDWR)) < 0)
	{
		MUX_ERROR("Failed to open the SPI %s: %s", filename, strerror(errno) );
		/* ERROR HANDLING; you can check errno to see what went wrong */
		return -1;
	}
 
	///////////////
	// Verifications
	///////////////
	//possible modes: mode |= SPI_LOOP; mode |= SPI_CPHA; mode |= SPI_CPOL; mode |= SPI_LSB_FIRST; mode |= SPI_CS_HIGH; mode |= SPI_3WIRE; mode |= SPI_NO_CS; mode |= SPI_READY;
	//multiple possibilities using |
	/*
	if (ioctl(file, SPI_IOC_WR_MODE, &mode)<0)   {
	perror("can't set spi mode");
	return;
	}
	*/
 
	if (ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0)
	{
		MUX_ERROR("SPI SPI_IOC_RD_MODE failed: %s", strerror(errno) );
	}

	if (ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb) < 0)
	{
		MUX_ERROR("SPI SPI_IOC_RD_LSB_FIRST failed: %s", strerror(errno) );
	}

	//sunxi supports only 8 bits
	/*
	if (ioctl(file, SPI_IOC_WR_BITS_PER_WORD, (__u8[1]){8})<0) 
	{
		MUX_ERROR("SPI SPI_IOC_WR_BITS_PER_WORD failed: %s", strerror(errno) );
	}
*/

	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
	{
		MUX_ERROR("SPI SPI_IOC_RD_BITS_PER_WORD failed: %s", strerror(errno) );
	}

	/*
	if (ioctl(file, SPI_IOC_WR_MAX_SPEED_HZ, &speed)<0)
	{
		MUX_ERROR("SPI SPI_IOC_RD_BITS_PER_WORD failed: %s", strerror(errno) );
	}
	*/

	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0)
	{
		MUX_ERROR("SPI SPI_IOC_RD_MAX_SPEED_HZ failed: %s", strerror(errno) );
	}


	MUX_DEBUG("SPI device %s : spi mode %d, %d bits %sper word, %d Hz max\n", mode, bits, lsb ? "(lsb first) " : "", speed);

	//_xfer[0].tx_buf = (unsigned long)buf;
	_xfer[0].len = 3; /* Length of  command to write*/
	_xfer[0].cs_change = 0; /* Keep CS activated */
	_xfer[0].delay_usecs = 0, //delay in us
	_xfer[0].speed_hz = 2500000, //speed
	_xfer[0].bits_per_word = 8, // bites per word 8

	//_xfer[1].rx_buf = (unsigned long) buf2;
	_xfer[1].len = 4; /* Length of Data to read */
	_xfer[1].cs_change = 0; /* Keep CS activated */
	_xfer[1].delay_usecs = 0;
	_xfer[1].speed_hz = 2500000;
	_xfer[1].bits_per_word = 8;

	return fd;
}
 
 
 
// Read n bytes from the 2 bytes address
int cmnSysSpiRead(int fd, int address, unsigned char * buffer, int nbytes)
{
	unsigned char   buf[3];
	int status;
	if (buffer == NULL) 
	{
		return EXIT_FAILURE;
	}
	
	memset(buffer, 0, nbytes);
	buf[0] = 0x01;
	buf[1] = (address >> 8) & 0xFF;
	buf[2] = address & 0xFF;
	
	_xfer[0].tx_buf = (unsigned long)buf;
	_xfer[0].len = 3; /* Length of  command to write*/
	_xfer[1].rx_buf = (unsigned long)buffer;
	_xfer[1].len = nbytes; /* Length of Data to read */
	
	status = ioctl(fd, SPI_IOC_MESSAGE(2), _xfer);
	if (status < 0)
	{
		MUX_ERROR("SPI_IOC_MESSAGE: %s", strerror(errno) );
		return EXIT_FAILURE;
	}
	
	//printf("env: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
	//printf("ret: %02x %02x %02x %02x\n", buf2[0], buf2[1], buf2[2], buf2[3]);

	return EXIT_SUCCESS;
}
 
// Write n bytes into the 2 bytes address
int cmnSysSpiWrite(int fd, int address, unsigned char * buffer, int nbytes)
{
	unsigned char   buf[MAX_SPI_WRITE + 3];
	int status;

	if (buffer == NULL) 
	{
		return EXIT_FAILURE;
	}
	
	buf[0] = 0x00;
	buf[1] = (address >> 8) & 0xFF;
	buf[2] = address & 0xFF;
	memcpy(&buf[3], buffer, nbytes);
	
	_xfer[0].tx_buf = (unsigned long)buf;
	_xfer[0].len = nbytes+3; /* Length of  command to write*/

	status = ioctl(fd, SPI_IOC_MESSAGE(1), _xfer);
	if (status < 0)
	{
		MUX_ERROR("SPI_IOC_MESSAGE: %s", strerror(errno));
		return EXIT_FAILURE;
	}
	//printf("env: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
	//printf("ret: %02x %02x %02x %02x\n", buf2[0], buf2[1], buf2[2], buf2[3]);

	return EXIT_SUCCESS;
}


int	cmnSysSpiTest(void)
{
	unsigned char spi_buf[16];
	int ret = EXIT_FAILURE;
	
	int fd = cmnSysSpiInit(SPI_0_FILENAME); //dev
	if(fd > 0 )
	{
		spi_buf[0] = SPI_TST_D0;
		spi_buf[1] = SPI_TST_D1;
		
		ret = cmnSysSpiWrite(fd, SPI_TST_ADDR, spi_buf, 2); //this will write value 0x41FF to the address 0xE60E
		if(ret == EXIT_SUCCESS)
		{
			ret = cmnSysSpiRead(fd, SPI_TST_ADDR, spi_buf, 2); //reading the address 0xE60E
			if(ret == EXIT_SUCCESS)
			{
				MUX_DEBUG("SPI test: 0x%02X%02X vs 0x%02X%02X", spi_buf[0], spi_buf[1], SPI_TST_D0, SPI_TST_D1);
				ret = EXIT_SUCCESS;
			}
			else
			{
				MUX_ERROR("SPI test error: can't read SPI.");
			}
		}
		else
		{
			MUX_ERROR("SPI test error: can't write SPI.");
		}

		close(fd);
	}
	else
	{
		MUX_ERROR("SPI test error: can't init SPI.");
	}

	return ret;
}

