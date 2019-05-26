
#include "libCmn.h"
#include "libMux.h"
#include "libCmnSys.h"

#include "muxDrv.h"

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


int testSecureChipId(void)
{
	int fd;
	int cmd, ret, i, errnum,totalnum;
	unsigned int value;
	unsigned char dev_info[8];
	int testcrc, data_byte;
	struct SHA256_ds_msg	chip_ds_msg;
	
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
	fd =open(MUX_DRV_SECURE_CHIP_DEV_NAME, O_RDWR);
	if(fd < 0)
	{
		printf("can not open %s: %m\n", MUX_DRV_SECURE_CHIP_DEV_NAME );
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
		usleep(100000*1000);
	}
	
	close(fd);

	return 0;
}


