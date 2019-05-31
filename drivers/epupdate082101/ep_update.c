#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>	
#include <unistd.h>
#include <string.h>

typedef unsigned char		bool;
typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned long		dword;

//    /home/alex/buildroot/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-gcc ep_update.c -o ep_update

unsigned char EP91A6SX_I2C_Write(unsigned char deviceID, unsigned char offset, unsigned char *buffer, unsigned short length)
{
		
	int fd;
	struct i2c_rdwr_ioctl_data messages;
	struct i2c_msg msg[1];
	unsigned char data_bytes[length + 1];
		
	unsigned short slave_addr = (deviceID >> 1);
	unsigned char word_address = offset;
		
	if ((fd = open("/dev/i2c-3", O_RDWR)) < 0)
	{
		printf("Unable to open i2c control file /dev/i2c-3 \n");
		
	}
	/*Check if slave device is accessible*/
	if (ioctl(fd, I2C_SLAVE, slave_addr) < 0)
	{
		printf("Problem with slave  \n");
	}

	data_bytes[0] = word_address;
	for (int i = 1; i < length + 1; i++)
	{
		data_bytes[i] = buffer[i - 1];
	}
	
	messages.nmsgs = 1;
	messages.msgs = msg;

	messages.msgs[0].addr = slave_addr;
	messages.msgs[0].len = length + 1;
	messages.msgs[0].flags = 0;
	messages.msgs[0].buf = data_bytes;

	if (ioctl(fd, I2C_RDWR, &messages) < 0)
	{
		printf("Write failed at Device id : %02x , reg[%d] \n", deviceID, word_address);
		return -1;
	}
	else
	{
		// printf("Write successful at Devide id: %02x, Reg adddress: %d and Data = \n", deviceID, data_bytes[0]);
		// for (int i = 1; i < length + 1; i++)
		// {
		// 	printf("%d\n", data_bytes[i]);
		// }
	}
	close(fd);
	return (0);
}

unsigned char EP91A6SX_I2C_Read(unsigned char deviceID, unsigned char offset, unsigned char *buffer, unsigned short length)
{

	int fd;
	struct i2c_rdwr_ioctl_data messages;
	struct i2c_msg msg[2];
	unsigned char data[57];
			
	unsigned short slave_addr = (deviceID >> 1);
	unsigned char word_address = offset & 0xFF;
		
	if ((fd = open("/dev/i2c-3", O_RDWR)) < 0)
	{
		printf("Unable to open i2c control file /dev/i2c-3 \n");
		
	}
	/*Check if slave device is accessible*/
	if (ioctl(fd, I2C_SLAVE, slave_addr) < 0)
	{
		printf("Problem with slave \n");
	}
	
	messages.nmsgs = 2;
	messages.msgs = msg;

	messages.msgs[0].addr = slave_addr;
	messages.msgs[0].len = 1;
	messages.msgs[0].flags = 0;
	messages.msgs[0].buf = &word_address;
	
	messages.msgs[1].addr = slave_addr;
	messages.msgs[1].len = length;
	messages.msgs[1].flags = I2C_M_RD;
	messages.msgs[1].buf = data;
	
	if (ioctl(fd, I2C_RDWR, &messages) < 0)
	{
		printf("Read failed at Device id : %02x , reg[%d] \n", deviceID, word_address);
		return -1;
	}
		
	for (int i = 0; i < length; i++)
	{
		buffer[i] = data[i];
	}
	close(fd);

	return (0);
}

/*
byte EP91A6SX_Init(void)
{
	int i;	
	byte audiopath;	
	
	EP91A6SX_I2C_Read(0xC8, 0x10, &audiopath, 1);	
	audiopath &= 0xDF;
	EP91A6SX_I2C_Write(0xC8, 0x10, &audiopath, 1);
	EP91A6SX_I2C_Read(0xC8, 0x10, &audiopath, 1);
	printf("Set EP91A6SX audiopath : 0x%x\n", audiopath);
	//init HDCP register on TX side
	audiopath  = 0;
	EP91A6SX_I2C_Write(0xC8, 0x42, &audiopath, 1);
	return 0;
}
*/

int main(int argc, char **argv)
{	
	int ret;
	unsigned char mode;
	unsigned char status, ispstatus;
	FILE *fp;
	unsigned long i, filesize, blocknum, blockindex, blockoffset;
	unsigned char data[20];
	i = 0;
		
	if (argc != 2)
	{
		printf("\nUsage: ./epupdate epbinfile\n");
		printf("argc = %d\n", argc);
	 	return -1;
	}
	while(1)
	{
		i++;
		EP91A6SX_I2C_Read(0xC8, 0x00, data, 8);
		printf("EP91 Version ID: 0x%02x 0x%02x\n", data[0], data[1]);
		printf("EP91 Device  ID: 0x%02x 0x%02x\n", data[2], data[3]);
		printf("EP91 Version   : 0x%02x \n", data[4]);
		printf("EP91 Year	   : 0x%02x \n", data[5]);
		printf("EP91 Month	   : 0x%02x \n", data[6]);
		printf("EP91 Date	   : 0x%02x \n", data[7]);
		if((data[0] == 0x17) && (data[1] == 0x7A) && (data[2] == 0x94) && (data[3] == 0xA3))
			break;			
		usleep(200000);
		if(i == 20)
		{
			printf("Cannot get right device ID. EXIT.\n");
			return -1;
		}
	}
		
	fp = fopen(argv[1], "rb");	
	if(fp == NULL)
	{
		printf("Cannot open data bin file. Error!\n");
		return -1;
	}
	fseek(fp, 0, SEEK_END); 
	filesize = ftell(fp); 
	fseek(fp, 0, SEEK_SET); 
	
	blocknum = filesize/512;
	
	if( (filesize % 512) != 0)
		blocknum = blocknum + 1;
		
	printf("EP bin filesize = %d, blocknum = %d\n", filesize, blocknum);	
	
	status = EP91A6SX_I2C_Read(0xC8, 0x0F, &mode, 1);
	printf("Read 0x0F value  = 0x%d\n", mode);
		
	//printf("Write to 0x0F  to enter ISP mode.\n");
	mode = 0x40;
	status = EP91A6SX_I2C_Write(0xC8, 0x0F, &mode, 1);			
			
	ret = 0;
	
	if(status == 0)
	{
		printf("Enter into ISP mode.\n");
		usleep(100000);	//delay 10ms
		for(blockindex = 0; blockindex < blocknum; blockindex++)	//each block 512
		{			
			printf(".");
			if( (blockindex + 1) % 32 == 0)
				printf("\n");
			// Set Block Index
			data[0] = blockindex;
			ispstatus = EP91A6SX_I2C_Write(0x50, 0x01, data, 1);
			if(ispstatus != 0)	//write error
			{
				printf("write block index error!\n");
				ret = -1;
				fclose(fp);
				
				//exit ISP mode
				printf("exit isp mode\n");
				mode = 0x00;
				ispstatus = EP91A6SX_I2C_Write(0x50, 0x02, &mode, 1);								
				break;
			}
			
			for(blockoffset = 0; blockoffset < 32; blockoffset++)	//for each block, write 16 bytes one time, total 32
			{
				memset(data + 1, 0xFF, 16);
				fread(data + 1, 16, 1, fp);
				data[0] = 16;
				ispstatus = EP91A6SX_I2C_Write(0x50, 0x41, data, 17);
				if(ispstatus != 0)
				{
					printf("write block data error!\n");
					ret = -1;
					fclose(fp);
					//exit ISP mode
					mode = 0x00;
					ispstatus = EP91A6SX_I2C_Write(0x50, 0x02, &mode, 1);
					break;				
				}
			}			
			usleep(80000);
		}
	}
	else
		printf("Not ISP mode. Close file and exit.\n");
	fclose(fp);
	//exit ISP mode
	mode = 0x00;
	ispstatus = EP91A6SX_I2C_Write(0x50, 0x02, &mode, 1);
	printf("\nEP91 update finish!\n");
	return ret;	
}
