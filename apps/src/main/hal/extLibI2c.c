#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>	
#include <unistd.h>
#include <string.h>

#include "FPGA.h"
#include "controler.h"

void fpga_init()
{
	byte deviceId;
	byte offset;
	byte data_write[32];
	word length;
	
	unsigned char buffer[256];
	/* //init in fpga_ip_addr_inf(root_msg);
	//Set video ip local
	memset(data_write, 0, 32);
	memset(buffer, 0, 256);
	deviceId = 0x68;
	offset = 16;
	length = 4;
	getip(buffer, 0);
	parse_ip_to_bytes(buffer, data_write);
	I2C_Write(deviceId, offset, data_write, length);
	*/
	
	
	/* init in fpga_ip_addr_inf(root_msg);
	//set video ip dest to 0
	memset(data_write, 0, 32);
	deviceId = 0x68;
	offset = 20;
	length = 4;
	I2C_Write(deviceId, offset, data_write, length);
	*/
	
	/*  init fpga_ip_addr_inf(root_msg);
	//set video mac address local
	memset(data_write, 0, 32);
	memset(buffer, 0, 256);
	deviceId = 0x68;
	offset = 0x04;
	length = 6;
	getMac(buffer, 18);
	parse_mac_to_bytes(buffer, data_write);
	I2C_Write(deviceId, offset, data_write, length);
	
	//set video mac dest to 0
	memset(data_write, 0, 32);
	memset(buffer, 0, 256);
	deviceId = 0x68;
	offset = 10;
	length = 6;
	I2C_Write(deviceId, offset, data_write, length);
	*/
	
	//set audio channel sel and mask
	memset(data_write, 0, 32);
	deviceId = 0x60;
	offset = 0x1A;
	length = 2;
	data_write[0] = 0x07;
	data_write[1] = 0xff;
	I2C_Write(deviceId, offset, data_write, length);	
	
	/*
	memset(data_write, 0, 32);
	deviceId = 0x60;
	//offset = 01;
	//length = 01;
	//data_write[0] = 0x1C;
	//I2C_Write(deviceId, offset, data_write, length);
	//
	offset = 03;
	length = 01;
	data_write[0] = 0x00;
	I2C_Write(deviceId, offset, data_write, length);
	*/
	
	//disable FPGA stream
	data_write[0] = 0;
	data_write[1] = 0;
	I2C_Write(0x60, 0x03, data_write, 1);	
	
	data_write[0] = 0;
	data_write[1] = 0;
	I2C_Write(0x60, 32, data_write, 1);
}


int read_fpga_data(unsigned char *data)
{
	byte deviceId = 0x60;
	byte offset;
	byte fpga_data[57];
	word length;

	offset = 00;
	length = 57;
	I2C_Read(deviceId, offset, fpga_data, length);
	memcpy(data, fpga_data, length);
	
	//for (int i = 0; i < length; i++)
	//{
		//printf("Read successful at Device id: %02x reg[%d] = ", deviceId, offset + i); 
		//printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(data_read[i]));
		//printf("%02x", data_read[i]);
		//printf("\n");
	//}
	return 1;
}

void fpga_set_ip(void)
{
	byte offset;
	byte avstream_en[2];
	byte data_write[32];
	word length;

	printf("Reload FPGA IP settings\n");
	
	I2C_Read(0x60, 0x03, avstream_en, 1);
	//disable FPGA stream
	avstream_en[0] &= 0xFC;
	I2C_Write(0x60, 0x03, avstream_en, 1);
	
	// Local MAC Address
	data_write[0] = 0x10;
	data_write[1] = 0x00;
	data_write[2] = 0x00;
	data_write[3] = 0xE5;
	data_write[4] = 0xA0;
	data_write[5] = 0x00;
	I2C_Write(0x68, 4, data_write, 6);
	
	// Destination MAC Address
	data_write[0] = 0x11;
	I2C_Write(0x68, 10, data_write, 6);
	
	// Local Video IP Address
	data_write[0] = 210;
	data_write[1] = 4;
	data_write[2] = 0;
	data_write[3] = 10;
	I2C_Write(0x68, 16, data_write, 4);
	
	// Destination Video IP Address
	data_write[0] = 211;
	I2C_Write(0x68, 20, data_write, 4);
	
	// Local Video UDP Port
	data_write[0] = 0x20;
	data_write[1] = 0xC2;
	I2C_Write(0x68, 32, data_write, 2);
	
	// Destination Video UDP Port
	data_write[0] = 0x30;
	data_write[1] = 0xC2;
	I2C_Write(0x68, 34, data_write, 2);
	
	// Local Audio UDP Port
	data_write[0] = 0x40;
	data_write[1] = 0xC2;
	I2C_Write(0x68, 36, data_write, 2);
	
	// Destination Audio UDP Port
	data_write[0] = 0x50;
	data_write[1] = 0xC2;
	I2C_Write(0x68, 38, data_write, 2);
	
	// Local Ancillary UDP Port
	data_write[0] = 0x60;
	data_write[1] = 0xC2;
	I2C_Write(0x68, 40, data_write, 2);
	
	// Destination Ancillary UDP Port
	data_write[0] = 0x70;
	data_write[1] = 0xC2;
	I2C_Write(0x68, 42, data_write, 2);

	//set audio channel sel and mask
	data_write[0] = 0x07;
	data_write[1] = 0xff;
	I2C_Write(0x60, 26, data_write, 2);	
	
	//enable FPGA stream
	avstream_en[0] |= 0x3;
	I2C_Write(0x60, 0x03, avstream_en, 1);	
}

