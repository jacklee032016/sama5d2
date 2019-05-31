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

#define SPI_DEVICE 	"/dev/spidev32766.0"
#define SPI_CMD_WRITE_STATUS_REGISTER	0x01
#define SPI_CMD_WRITE					0x02
#define SPI_CMD_READ					0x03
#define SPI_CMD_WRITE_DISABLE			0x04
#define SPI_CMD_READ_STATUS				0x05
#define SPI_CMD_WRITE_ENABLE			0x06
#define SPI_CMD_CHIP_ERASE				0xC7
#define SPI_CMD_CLEAR_AREA				0xD8
#define SPI_CMD_READ_DEVICE_ID			0x9E
#define SPI_CMD_ENTER_4BYTE_AD_MODE		0xB7
#define SPI_CMD_EXIT_4BYTE_AD_MODE 		0xE9
#define SPI_CMD_READ_4BYTE 				0x13
#define SPI_CMD_SECTOR_ERASE_4BYTE		0xDC	//64KB
#define SPI_CMD_PAGE_PROGRAM_4BYTE  	0x12
#define SPI_PAGE_SIZE					256

//uint32_t crc32_table[256];
//int make_crc32_table()
int make_crc32_table(uint32_t *crc32_table)
{
	uint32_t c;
	int i = 0;
	int bit = 0;	
	for(i = 0; i < 256; i++)
	{
		c  = (uint32_t)i;		
		for(bit = 0; bit < 8; bit++)
		{
			if(c&1)
			{
				c = (c >> 1)^(0xEDB88320);
			}
			else
			{
				c =  c >> 1;
			}			
		}
		crc32_table[i] = c;			
	}
	return 1;
}

uint32_t make_crc(uint32_t crc, uint8_t *string, uint32_t size, uint32_t *crc32_table) 
{
	while(size--)
		crc = (crc >> 8)^(crc32_table[(crc ^ *string++)&0xff]);
	return crc;
}

int spi_open(void)
{
	int fd;
	fd = open(SPI_DEVICE, O_RDWR);
	if(fd < 0)
	{
		printf("can't open spi device: %s.\n", SPI_DEVICE);
		return -1;
	}
	return fd;
}

int spi_init(int fd, uint8_t mode, uint8_t bits, uint32_t speed)
{
	int ret;
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
		printf("can't set spi mode.\n");
		return -1;
	}
	// bits per word	
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		printf("can't set bits per word\n");
		return -1;
	}
	//max speed hz	
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{	
		printf("can't set max speed hz.\n");
		return -1;
	}
	return 0;
}

int spi_close(int fd)
{
	close(fd);
	return 0;
}

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
		printf("read ID SPI_IOC_MESSAGE fail.\n");
		return -1;
	}				
	flash_id = (data_buf[1]<<24) + (data_buf[2]<<16) + (data_buf[3]<<8) + data_buf[4];	
	return flash_id;
}

int spi_read_status(int fd)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[2], data_buf[2];
	int	len, ret, status;
	int flash_id;
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	if (len > sizeof data_buf)
		len = sizeof data_buf;
	cmd_buf[0] = SPI_CMD_READ_STATUS;	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = 2; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		printf("Read status SPI_IOC_MESSAGE fail.\n");
		return -1;
	}				
	status = data_buf[1];
	return status;
}

int spi_write_enable(int fd)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[5], data_buf[5];
	int	len, ret;
	int flash_id;
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	if (len > sizeof data_buf)
		len = sizeof data_buf;
	cmd_buf[0] = SPI_CMD_WRITE_ENABLE;
	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = 1; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		printf("Write enable SPI_IOC_MESSAGE fail.\n");
		return -1;
	}				
	return 0;
}

int spi_write_disable(int fd)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[5], data_buf[5];
	int	len, ret;
	int flash_id;
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	if (len > sizeof data_buf)
		len = sizeof data_buf;
	cmd_buf[0] = SPI_CMD_WRITE_DISABLE;
	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = 1; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		printf("Write disable SPI_IOC_MESSAGE fail.\n");
		return -1;
	}				
	return 0;
}

int spi_enter_4byte_ad_mode(int fd)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[5], data_buf[5];
	int	len, ret;
	int flash_id;
	
	//write enable before enter 4 byte address mode.
	spi_write_enable(fd);
	
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	if (len > sizeof data_buf)
		len = sizeof data_buf;
	cmd_buf[0] = SPI_CMD_ENTER_4BYTE_AD_MODE;	
	cmd_buf[1] = 0x0;
	cmd_buf[2] = 0x0;
	cmd_buf[3] = 0x0;
	cmd_buf[4] = 0x0;
	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = 5; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		printf("read ID SPI_IOC_MESSAGE fail.\n");
		return -1;
	}				
	return 0;
}

int spi_exit_4byte_ad_mode(int fd)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[5], data_buf[5];
	int	len, ret;
	int flash_id;	
	
	//write enable before exit 4 byte address mode.
	spi_write_enable(fd);
	
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	if (len > sizeof data_buf)
		len = sizeof data_buf;
	cmd_buf[0] = SPI_CMD_EXIT_4BYTE_AD_MODE;	
	cmd_buf[1] = 0x0;
	cmd_buf[2] = 0x0;
	cmd_buf[3] = 0x0;
	cmd_buf[4] = 0x0;
	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = 5; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		printf("read ID SPI_IOC_MESSAGE fail.\n");
		return -1;
	}				
	return 0;
}


//read data(count bytes) from start_addr, count <=512
int spi_read_data_4byte(int fd, int start_addr, unsigned char *data, int count)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[520], data_buf[520];
	int	i, ret;
	int flash_id;
	if(count > 512)
	{
		printf("Cannot read more than 512 bytes one time.\n");
		return -1;
	}
	
	spi_enter_4byte_ad_mode(fd);		
	
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	cmd_buf[0] = SPI_CMD_READ_4BYTE;	
	cmd_buf[1] = (start_addr >> 24) & 0xff;
	cmd_buf[2] = (start_addr >> 16) & 0xff;
	cmd_buf[3] = (start_addr >> 8) & 0xff;
	cmd_buf[4] = start_addr & 0xff;
	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = count + 5; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		printf("read ID SPI_IOC_MESSAGE fail.\n");
		return -1;
	}
	for(i = 5; i < count + 5; i++)
		data[i - 5] = data_buf[i];	
	return 0;
}


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

int spi_sector_erase_4byte(int fd, int start_addr)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[520], data_buf[520];
	int	i, ret;
	int flash_id;
	int timeout;
	int status;
	timeout = 3000;
	
	//write enable before sector erase command.
	spi_write_enable(fd);
	
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	cmd_buf[0] = SPI_CMD_SECTOR_ERASE_4BYTE;	
	cmd_buf[1] = (start_addr >> 24) & 0xff;
	cmd_buf[2] = (start_addr >> 16) & 0xff;
	cmd_buf[3] = (start_addr >> 8) & 0xff;
	cmd_buf[4] = start_addr & 0xff;
	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = 5; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		printf("Sector erase SPI_IOC_MESSAGE fail.\n");
		return -1;
	}
	//check status register bit 0--write in progress
	status = spi_read_status(fd) & 0x1;
	while(status)
	{
		timeout--;
		usleep(1000);
		status = spi_read_status(fd) & 0x1;
		if(timeout == 0)
			break;
	}	
	return 0;
}

int spi_page_program_4byte(int fd, int start_addr, unsigned char *prog_data, int count)
{
	struct spi_ioc_transfer xfer[1];
	unsigned char  cmd_buf[262], data_buf[262];
	int	i, ret;
	int flash_id;
	int timeout;
	int status;
	
	if(count > SPI_PAGE_SIZE)
	{
		printf("Cannot program more than 256 bytes in one page.\n");
		return -1;
	}
	//write enable before page program.
	spi_write_enable(fd);
	
	timeout = 3000;
	memset(xfer, 0, sizeof xfer);
	memset(cmd_buf, 0, sizeof cmd_buf);
	memset(data_buf, 0, sizeof data_buf);
	
	cmd_buf[0] = SPI_CMD_PAGE_PROGRAM_4BYTE;	
	cmd_buf[1] = (start_addr >> 24) & 0xff;
	cmd_buf[2] = (start_addr >> 16) & 0xff;
	cmd_buf[3] = (start_addr >> 8) & 0xff;
	cmd_buf[4] = start_addr & 0xff;
	
	for(i = 0; i < count; i++)
		cmd_buf[i + 5] = *(prog_data + i);
	
	xfer[0].tx_buf = (unsigned long)cmd_buf;
	xfer[0].len = count + 5; 
	xfer[0].rx_buf = (unsigned long)data_buf;
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if (ret < 0) 
	{
		printf("Page program SPI_IOC_MESSAGE fail.\n");
		return -1;
	}
	//check status register bit 0--write in progress
	status = spi_read_status(fd) & 0x1;
	while(status)
	{
		timeout--;
		usleep(1000);
		status = spi_read_status(fd) & 0x1;
		if(timeout == 0)		
			break;	
	}
	if(timeout == 0)
		return -1;
	else
		return 0;
}

//start_addr must be based on 64KB size
int fpga_flash_prog(char *filename, uint32_t start_addr)
{
	int ret;
	int spi_fd;
	int flash_id;
	int status;
	uint8_t mode; 
	uint8_t bits; 
	uint32_t speed;
	uint8_t read_data[SPI_PAGE_SIZE];	
	int i;	
	uint32_t crc32_table[256];
	uint32_t file_crc, read_crc;
	FILE *fp;
	int file_size;
	int erase_sector_cnt, write_page_cnt;
	
	flash_id = 0;
	file_crc = 0xffffffff;
	read_crc = 0xffffffff;	
	
	//spi parameter
	mode = 0;
	bits = 8;
	speed = 2000000;	
	
	//check start_addr
	if((start_addr%65536) != 0)
	{
		printf("start_addr is NOT integer times of 64KB.\n");
		return -1;
	}	
	//init crc table
	make_crc32_table(crc32_table);	
	memset(read_data, 0, sizeof(read_data));		
	fp = fopen(filename, "rb");	
	if(fp == NULL)
	{
		printf("Cannot open data bin file.Error!\n");
		return -1;
	}
	fseek(fp, 0, SEEK_END); 
	file_size = ftell(fp); 
	fseek(fp, 0, SEEK_SET); 
	
	printf("FPGA file size is %d\n", file_size);
	//sector erase
	if((file_size%65536) == 0)
	{		
		erase_sector_cnt = file_size/65536;
		printf("Erase sector number : %d.\n", erase_sector_cnt);		
	}
	else
	{		
		erase_sector_cnt = (file_size/65536) + 1;
		printf("Erase sector number : %d.\n", erase_sector_cnt);
	}
	//write page number
	if((file_size%SPI_PAGE_SIZE) == 0)
	{		
		write_page_cnt = file_size/SPI_PAGE_SIZE;
		printf("write_page_cnt : %d.\n", write_page_cnt);		
	}
	else
	{		
		write_page_cnt = (file_size/SPI_PAGE_SIZE) + 1;
		printf("write_page_cnt : %d.\n", write_page_cnt);
	}
		
	spi_fd = spi_open();	
	if(spi_fd < 0)
	{
		printf("Spi device open error.\n");
		if(fp)
			fclose(fp);
		return -1;
	}		
	ret = spi_init(spi_fd, mode, bits, speed);
	if(ret < 0)
	{
		printf("Spi device init error.\n");		
	}	
	flash_id = spi_read_id(spi_fd);	
	printf("Read flash ID.\n");
	flash_id = spi_read_id(spi_fd);
	if(flash_id == 0)
	{
		printf("Spi device read id error.\n");		
	}
	else
		printf("Flash_id = 0x%X.\n", flash_id);			
	printf("Erase flash...\n");
	for(i = 0; i < erase_sector_cnt; i++)	
		spi_sector_erase_4byte(spi_fd, start_addr + i*0x10000 );		
		
	//read SPI_PAGE_SIZE 256 bytes from file, write to flash, calculate file CRC, until file end.
	printf("Program flash...\n");
	for(i = 0; i < (write_page_cnt - 1); i++)
	{		
		fread(read_data, 1, SPI_PAGE_SIZE, fp);
		//write to flash
		spi_page_program_4byte(spi_fd, start_addr + i*SPI_PAGE_SIZE, read_data, SPI_PAGE_SIZE);	
		//make file crc
		file_crc = make_crc(file_crc, read_data, SPI_PAGE_SIZE, crc32_table);		
	}
	
	//last page	
	if((file_size - i*SPI_PAGE_SIZE) > 0)
	{
		memset(read_data, 0xff, sizeof read_data);
		fread(read_data, 1, (file_size - i*SPI_PAGE_SIZE), fp);
		spi_page_program_4byte(spi_fd, start_addr + i*SPI_PAGE_SIZE, read_data, (file_size - i*SPI_PAGE_SIZE));	
		//make file crc
		file_crc = make_crc(file_crc, read_data, (file_size - i*SPI_PAGE_SIZE), crc32_table);
	}
	file_crc = ~file_crc;	
	//read 256 bytes from flash, calculate read data CRC, until read all the written data(file_size).
	//printf("Read flash to check CRC.\n");		
	for(i = 0; i < (write_page_cnt - 1); i++)
	{			
		spi_read_data_4byte(spi_fd, start_addr + i*SPI_PAGE_SIZE, read_data, SPI_PAGE_SIZE);
		read_crc = make_crc(read_crc, read_data, SPI_PAGE_SIZE, crc32_table);		
	}		
	if((file_size - i*SPI_PAGE_SIZE) > 0)
	{
		memset(read_data, 0xff, sizeof read_data);
		spi_read_data_4byte(spi_fd, start_addr + i*SPI_PAGE_SIZE, read_data, (file_size - i*SPI_PAGE_SIZE));
		read_crc = make_crc(read_crc, read_data, (file_size - i*SPI_PAGE_SIZE), crc32_table);			
	}	
	read_crc = ~read_crc;
	//printf("file_crc = 0x%08x\n", file_crc);		
	//printf("read_crc = 0x%08x\n", read_crc);		
	close(spi_fd);
	fclose(fp);
	
	if(file_crc != read_crc)
	{
		printf("flash program ERROR!\n");
		return -1;
	}
	else
	{	
		printf("flash program OK!\n");
		return 0;
	}	
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