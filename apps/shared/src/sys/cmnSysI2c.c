
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>	
#include <unistd.h>
#include <string.h>

#include "libCmn.h"
#include "mux7xx.h"

static int _cmnSysI2CFile(unsigned short slaveAddr)
{
	int fd = 0;

	if ((fd = open(MUX_DEV_I2C_1, O_RDWR)) < 0)
	{
		MUX_ERROR("Unable to open i2c control file "MUX_DEV_I2C_1);
		return -1;
	}
	
	/*Check if slave device is accessible*/
	if (ioctl(fd, I2C_SLAVE, slaveAddr) < 0)
	{
		MUX_WARN("Problem with slave device 0x%x", slaveAddr);
	}

	return fd;
}


int cmnSysI2cWrite(int deviceID, unsigned char offset, unsigned char *buffer, int length)
{
	int ret = EXIT_SUCCESS;
	int fd, i;
	struct i2c_rdwr_ioctl_data messages;
	struct i2c_msg msg[1];
	unsigned char data_bytes[length + 1];
		
	unsigned char word_address = offset;
	unsigned short slaveAddr = (deviceID >> 1);
	
	fd = _cmnSysI2CFile(slaveAddr);
	if(fd <= 0)
	{
		return EXIT_FAILURE;
	}

	data_bytes[0] = word_address;
	for(i = 1; i < length + 1; i++)
	{
		data_bytes[i] = buffer[i - 1];
	}
	
	messages.nmsgs = 1;
	messages.msgs = msg;

	messages.msgs[0].addr = slaveAddr;
	messages.msgs[0].len = length + 1;
	messages.msgs[0].flags = 0;
	messages.msgs[0].buf = data_bytes;

	if (ioctl(fd, I2C_RDWR, &messages) < 0)
	{
		MUX_ERROR("Write failed at Device id : %02x , reg[%d]", deviceID, word_address);
		ret = EXIT_FAILURE;
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
	
	return ret;
}

int cmnSysI2cRead(int deviceID, unsigned char offset, unsigned char *buffer, int length)
{
	int ret = EXIT_SUCCESS;
	int fd, i;
	struct i2c_rdwr_ioctl_data messages;
	struct i2c_msg msg[2];
	unsigned char data[57];
			
	unsigned char word_address = offset & 0xFF;
		
	unsigned short slaveAddr = (deviceID >> 1);
	
	fd = _cmnSysI2CFile(slaveAddr);
	if(fd <= 0)
	{
		return EXIT_FAILURE;
	}
	
	messages.nmsgs = 2;
	messages.msgs = msg;

	messages.msgs[0].addr = slaveAddr;
	messages.msgs[0].len = 1;
	messages.msgs[0].flags = 0;
	messages.msgs[0].buf = &word_address;
	
	messages.msgs[1].addr = slaveAddr;
	messages.msgs[1].len = length;
	messages.msgs[1].flags = I2C_M_RD;
	messages.msgs[1].buf = data;
	
	if (ioctl(fd, I2C_RDWR, &messages) < 0)
	{
		MUX_ERROR("Read failed at Device id : %02x , reg[%d]", deviceID, word_address);
		ret = EXIT_FAILURE;
	}
	else
	{
		for (i = 0; i < length; i++)
		{
			buffer[i] = data[i];
		}
	}
	
	close(fd);

	return ret;
}

