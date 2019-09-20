/*
 * SPI testing utility (using spidev driver)
 *
  
  /home/alex/buildroot/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-gcc fpga_update.c -o fpga_update
*/

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


//Default 3 byte address flash reading
int spi_read_data(int fd, int address)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[100], data_buf[100];
	int	len, ret;
	int flash_id;
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	if (len > sizeof data_buf)
		len = sizeof data_buf;
	cmd_buf[0] = SPI_CMD_READ;	
	cmd_buf[1] = 0x15;
	cmd_buf[2] = 0xEF;
	cmd_buf[3] = 0x30;	
	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = 100; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		printf("read ID SPI_IOC_MESSAGE fail.\n");
		return -1;
	}			
	for(len = 4; len < 100; len++)
	{
		printf(" 0x%02x ", data_buf[len]);
		if( (len - 4 + 1)% 16 == 0)
			printf("\n");
	}	
	return 0;
}


int spi_read_nonvolatile_config_reg(int fd)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[5], data_buf[5];
	int	len, ret, status;
	int flash_id;
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	if (len > sizeof data_buf)
		len = sizeof data_buf;
	cmd_buf[0] = 0x5A;	
	//cmd_buf[1] = 0x00;
	//cmd_buf[2] = 0x01;
	//cmd_buf[3] = 0x02;
	//cmd_buf[4] = 0x03;
	
	cmd_buf[4] = 0x00;
	cmd_buf[3] = 0x01;
	cmd_buf[2] = 0x02;
	cmd_buf[1] = 0x03;
	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = 5; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		printf("read ID SPI_IOC_MESSAGE fail.\n");
		return -1;
	}			
	for(len = 0; len < 5; len++)
		printf(" 0x%x ", data_buf[len]);
	status = (data_buf[1]<<24) + (data_buf[2]<<16) + (data_buf[3]<<8) + data_buf[4];	
	return status;
}

int main(int argc, char **argv)
{	
	uint32_t start_addr = 0x0;
	int ret;	
	if (argc != 2)
	{
		printf("\nUsage: ./fpga_update fpgabinfile\n");
		printf("argc = %d\n", argc);
	 	return -1;
	}	
	printf("Download to address 0\n");
	ret = fpga_flash_prog(argv[1], start_addr);		
	return ret;	
}