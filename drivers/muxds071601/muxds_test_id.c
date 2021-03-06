#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>


/// /home/alex/buildroot/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-gcc muxds_test_id.c -o muxds_test_id


#define DS_MAGIC 'D'
#define SHA256_READ_ROM_ID			_IOWR(DS_MAGIC, 1, unsigned int *)  
#define SHA256_INIT_CHIP_PRODUCTION	_IOWR(DS_MAGIC, 2, unsigned int *)  
#define SHA256_READ_CHIP_MEM		_IOWR(DS_MAGIC, 3, unsigned int *)  
#define SHA256_COMPUTE_READ_MAC		_IOWR(DS_MAGIC, 4, unsigned int *)  
#define SHA256_CHIP_TIMING_INIT		_IOWR(DS_MAGIC, 5, unsigned int *)  

// delay durations
#define EEPROM_WRITE_DELAY       10  	//tprd unit:ms
#define SHA_COMPUTATION_DELAY    3   	//unit:ms
#define SECRET_PROGRAM_DELAY	 100	//tprs unit:ms

#define CMD_READ_ROM			 	0x33	//Read rom ID
#define CMD_READ_WRITE_SCRATCHPAD   0x0F	//Read/write scratchpad 
#define CMD_ROM_SKIP             	0xCC
#define CMD_WRITE_MEMORY         	0x55	//Write to memory
#define CMD_READ_MEMORY         	0xF0	
#define CMD_READ_STATUS	         	0xAA
#define CMD_COMPUTE_READ_PAGE_MAC	0xA5
#define CMD_LOAD_LOCK_SECRET		0x33 

struct ds_msg 
{
	unsigned char rom_id[8];		//7 byte id(start from 0x17) + 1 byte CRC
	unsigned char secret[32];		//secret write to chip
	unsigned char challenge[32];	//random data write to chip
	unsigned char mac[32];			//mac data read from chip
	unsigned char page_0_data[32];	//page 0 data write to/read from chip
	unsigned char page_1_data[32];	//page 1 data write to/read from chip
	unsigned char man_id[2];		//manufacturer ID
	unsigned char page_num;			//page number, which page is used for MAC 
	unsigned char anon;				//Anonymous Indicator(ANON). These bits specify whether the device’s ROM ID is used for the MAC computation
									//1--anonymous mode, 0--NOT anonymous mode.
	unsigned char lock_flag;		//1-lock secret, secret is fixed for ever	
	unsigned char lock_magic;		//when (logic_magic == 0x5A)&&(lock_flag == 1), lock the secret
};

int calc_crc(int data_byte, int crc)
{
	int bit_mask = 0, carry_check = 0, temp_data = 0;
	temp_data = data_byte;
	for ( bit_mask = 0; bit_mask <= 7; bit_mask ++)
	{
		data_byte = data_byte ^ crc;
		crc = crc / 2;
		temp_data = temp_data / 2;
		carry_check = data_byte & 0x01;
		if (carry_check)
		{
			crc = crc ^ 0x8C;
		}
		data_byte = temp_data;
	}
	return ( crc );
}

int main(int argc,char **argv)
{
	int fd;
	int cmd, ret, i, errnum,totalnum;
	unsigned int value;
	unsigned char dev_info[8];
	int testcrc, data_byte;
	struct ds_msg	chip_ds_msg;
	
	//init chip_ds_msg
	memset(&chip_ds_msg.rom_id[0], 0, sizeof(chip_ds_msg));
	/*
	for (i = 0; i < 8; i++)
	{
		chip_ds_msg.rom_id[i] = 0;
		chip_ds_msg.challenge[i] = 0xa9;
	}	
	for (i = 0; i < 32; i++)
		chip_ds_msg.secret[i] = 0x5a;
	chip_ds_msg.flag = 0;
	for (i = 0; i < 20; i++)
		chip_ds_msg.mac[i] = 0x0;
	for (i = 0; i < 32; i++)
		chip_ds_msg.page_data[i] = 0x0;	
	chip_ds_msg.flag = 0;
	*/
	testcrc = 0x00;
	fd =open("/dev/muxds",O_RDWR);
	if(fd < 0)
	{
		printf("can not open /dev/muxds!\n");
		return -1;
	}
	
	totalnum = 0; errnum = 0;
	while(1)
	{		
		totalnum++;
		memset(dev_info, 0, sizeof(dev_info));
		ret = ioctl(fd, SHA256_READ_ROM_ID, &chip_ds_msg);
		if(ret != 0)
		{
			printf("DS IOCTL READ_ROM read ds error!\n");
		}		
		for(i = 0; i < 8; i++)
		{
			dev_info[i] = chip_ds_msg.rom_id[i];
			printf(" 0x%02x ", dev_info[i]);		
		}					
		testcrc = 0x00;		
		for(i = 0; i < 7; i++)
		{
			data_byte = dev_info[i];			
			testcrc = calc_crc(data_byte, testcrc);
		}
		printf("CRC: 0x%x ", testcrc);
	
		if( (testcrc == dev_info[7]) && ( dev_info[0] == 0x17) )	//check device family code 0x17 for DS28E15
		{	
			printf("OK!");
		}
		else
		{
			printf("ERROR!");
			errnum++;
		}
		printf("ratio:%d/%d\n", errnum, totalnum);
		usleep(100000);
	}
	
	close(fd);
	return 0;		
}


