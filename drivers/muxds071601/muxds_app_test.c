#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "sha256_software.h"

/// /home/alex/buildroot/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-gcc muxds_app_test.c sha256_software.c -o muxds_app_test

#define DS_MAGIC 'D'
#define SHA256_READ_ROM_ID			_IOWR(DS_MAGIC, 1, unsigned int *)  
#define SHA256_INIT_CHIP_PRODUCTION	_IOWR(DS_MAGIC, 2, unsigned int *)  
#define SHA256_READ_CHIP_MEM		_IOWR(DS_MAGIC, 3, unsigned int *)  
#define SHA256_COMPUTE_READ_MAC		_IOWR(DS_MAGIC, 4, unsigned int *)  


//compute rom_id crc8
int calc_rom_id_crc(int data_byte, int crc)
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



//int main(int argc,char **argv)
int sha256_app_check(void)
{
	int fd;
	int ret, i;
	int testcrc, data_byte;
	unsigned char challenge[32];
	unsigned char secret[32];
	unsigned char chip_mac[32];
	unsigned char page_data[32];
	unsigned char rom_id[8];
	unsigned char man_id[2];
	unsigned char personality_data[4];
	unsigned char page_num;
	unsigned char anon;
	unsigned char lock_magic;
	struct SHA256_ds_msg	chip_ds_msg;
	
	//init chip_ds_msg
	for (i = 0; i < 8; i++)
	{
		chip_ds_msg.rom_id[i] = 0;
		chip_ds_msg.secret[i] = 0x5a;
		chip_ds_msg.challenge[i] = 0xa9;
	}		
	for (i = 0; i < 20; i++)
		chip_ds_msg.mac[i] = 0x0;
	for (i = 0; i < 32; i++)
		chip_ds_msg.page_0_data[i] = 0x0;	
	for (i = 0; i < 32; i++)
		chip_ds_msg.page_1_data[i] = 0x0;		
	chip_ds_msg.lock_flag = 0;
	chip_ds_msg.lock_magic = 0;
		
	anon = 0;
	testcrc = 0x00;	
	fd =open("/dev/muxds",O_RDWR);
	if(fd < 0 )
	{
		printf("can not open /dev/muxds!\n");
		return -1;
	}	
	printf("Step 1: Read DS28E15 ROM ID. Check CRC.\n");
	ret = ioctl(fd, SHA256_READ_ROM_ID, &chip_ds_msg);
	if(ret < 0)
	{
		printf("DS IOCTL READ_ROM read ds error!\n");
		return -2;
	}
	
	memcpy(rom_id, &chip_ds_msg.rom_id[0], 8);	
	
	testcrc = 0x00;		
	for(i = 0; i < 7; i++)
	{
		data_byte = rom_id[i];			
		testcrc = calc_rom_id_crc(data_byte, testcrc);
	}	
	if((testcrc == rom_id[7]) && (rom_id[0] == 0x17))		
		printf("read ROM ID OK!\n");
	else
	{
		printf("read ROM ID FAIL! Exit!\n");	
		return -3;
	}	
	
	printf("Step 2: Read page data, init challenge, read mac from chip and verfify.\n");
	ret = ioctl(fd, SHA256_READ_CHIP_MEM, &chip_ds_msg);
	if(ret < 0)
	{
		printf("DS IOCTL READ_CHIP_MEM FAIL!\n");
		return -4;
	}		
	srand((unsigned)time(NULL));	//init random seed	
	for (i = 0; i < 32; i++)
	{	
		chip_ds_msg.challenge[i] = rand()%256;		
	}
	
	//set page_num	
	chip_ds_msg.page_num = 0;
	//set no anonymous mode
	chip_ds_msg.anon = 0;		
	
	//Read MAC from chip, this step does not need to send secret to chip. ONLY 32 byte challenge data is used.
	ret = ioctl(fd, SHA256_COMPUTE_READ_MAC, &chip_ds_msg);
	if(ret < 0)
	{
		printf("DS IOCTL COMPUTE_READ_MAC FAIL!\n");
		return -5;
	}
	
	anon = chip_ds_msg.anon;
	
	page_num = chip_ds_msg.page_num;
	if(page_num == 0)
		memcpy(page_data, chip_ds_msg.page_0_data, 32);
	else
		memcpy(page_data, chip_ds_msg.page_1_data, 32);	
	
	memcpy(challenge, chip_ds_msg.challenge, 32);	
	memcpy(man_id, chip_ds_msg.man_id, 2);
	memcpy(chip_mac, chip_ds_msg.mac, 32);
	
	//setcret key
	for (i = 0; i < 32; i++)		
		secret[i] = i;		
	
	secret[0] = rom_id[0];
	secret[1] = rom_id[1];
	secret[2] = rom_id[2];
	secret[3] = rom_id[3];
	secret[4] = rom_id[4];
	secret[5] = rom_id[5];
	secret[6] = rom_id[6];
	secret[7] = rom_id[7];	
	
	printf("page_num : %d, anon: %d\n", page_num, anon);
	printf("Rom ID: ");
	for (i = 0; i < 8; i++)		
		printf(" 0x%02X ", rom_id[i]);
	printf("\n");
	printf("Challenge : \n");
	for (i = 0; i < 32; i++)		
		printf(" 0x%02X ", challenge[i]);
	printf("\n");
	printf("Chip MAC: \n");
	for (i = 0; i < 32; i++)		
		printf(" 0x%02X ", chip_mac[i]);
	printf("\n");
	if(!Compute_Software_Mac(page_num, page_data, challenge, rom_id, secret, man_id, anon, chip_mac))
	{
		printf("MAC compare ERROR!\n");
		return -6;
	}
	else
		printf("MAC compare OK!\n");	
		
	close(fd);
	return 0;		
}

int main(int argc,char **argv)
{
	int i, j, ret;
	j = 0;
	for(i = 0; i < 2000; i++)
	{
		printf("\nRatio: %d/%d\n", j, i);
		sleep(1);
		ret = sha256_app_check();
		if(ret != 0)
			j++;		
	}
	
	return 1;
}