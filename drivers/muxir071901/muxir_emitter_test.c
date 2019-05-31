#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/// /home/alex/buildroot/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-gcc muxir_emitter_test.c -o muxir_emitter_test

#define SYMBOL_CNT 200

#define IR_MAGIC 'I'
#define IR_SENSOR_MODE	  _IO(IR_MAGIC, 0)  
#define IR_EMITTER_MODE	  _IO(IR_MAGIC, 1)  

unsigned char I2C_Read(unsigned char deviceID, unsigned char offset, unsigned char *buffer, int length)
{

	int fd;
	struct i2c_rdwr_ioctl_data messages;
	struct i2c_msg msg[2];
	unsigned char data[57];
			
	unsigned short slave_addr = (deviceID >> 1);
	unsigned char word_address = offset & 0xFF;
		
	if ((fd = open("/dev/i2c-1", O_RDWR)) < 0)
	{
		printf("Unable to open i2c control file /dev/i2c-1 \n");
		
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
	}
		
	for (int i = 0; i < length; i++)
	{
		buffer[i] = data[i];
	}
	close(fd);

	return (0);
}


unsigned char I2C_Write(unsigned char deviceID, unsigned char offset, unsigned char *buffer, int length)
{
		
	int fd;
	struct i2c_rdwr_ioctl_data messages;
	struct i2c_msg msg[1];
	unsigned char chars[length + 1];
		
	unsigned short slave_addr = (deviceID >> 1);
	unsigned char word_address = offset;
		
	if ((fd = open("/dev/i2c-1", O_RDWR)) < 0)
	{
		printf("Unable to open i2c control file /dev/i2c-1 \n");
		
	}
	/*Check if slave device is accessible*/
	if (ioctl(fd, I2C_SLAVE, slave_addr) < 0)
	{
		printf("Problem with slave  \n");
	}

	chars[0] = word_address;
	for (int i = 1; i < length + 1; i++)
	{
		chars[i] = buffer[i - 1];
	}
	
	messages.nmsgs = 1;
	messages.msgs = msg;

	messages.msgs[0].addr = slave_addr;
	messages.msgs[0].len = length + 1;
	messages.msgs[0].flags = 0;
	messages.msgs[0].buf = chars;

	if (ioctl(fd, I2C_RDWR, &messages) < 0)
	{
		printf("Write failed at Device id : %02x , reg[%d] \n", deviceID, word_address);
	}
	else
	{
		// printf("Write successful at Devide id: %02x, Reg adddress: %d and Data = \n", deviceID, data_unsigned chars[0]);
		// for (int i = 1; i < length + 1; i++)
		// {
		// 	printf("%d\n", data_unsigned chars[i]);
		// }
	}
	close(fd);
	return (0);
}




int main(int argc,char **argv)
{
	int fd;
	int cmd;
	int i,j;	
	struct sockaddr_in myaddr, remaddr;
	int net_fd, slen;
	slen = sizeof(remaddr);
	unsigned short int buf[SYMBOL_CNT];
	char val;
	int ret;
	unsigned char read_data[10];
	unsigned char write_data[10];
	unsigned char ir_ctrl_data;
	unsigned char ir_freq;
	
	unsigned short int buf_left_38[SYMBOL_CNT] = {0x0768, 38000,72,
    8869,     4463,      553,      562,      553,     1677,      551,      564,      551,     1677,
     551,      564,      551,      564,      551,     1677,      551,     1677,      553,      562,
     551,      564,      551,     1677,      551,      564,      551,     1677,      553,      562,
     551,      564,      551,     1677,      552,      562,      553,      562,      551,      564,
     551,     1677,      552,     1678,      554,      561,      553,      562,      551,      566,
     551,     1678,      551,     1678,      551,     1680,      552,      561,      553,      561,
     552,     1678,      551,     1680,      551,     1675,      552,    19418,     8870,     2240};
	 
	unsigned short int buf_left_40[SYMBOL_CNT] = {0x0768, 40000,   127,
    2341,      611,     1149,      640,     1149,      647,      545,      641,
    1148,      641,     1148,      641,     1148,      641,      580,      613,      551,      641,
    1153,      637,      579,      613,     1149,      641,     1149,      641,      552,      641,
    1149,      641,      552,      641,      556,      637,      550,      643,     1146,      643,
    1175,      613,     1148,    11565,     2342,      638,     1146,      643,     1146,      644,
     549,      643,     1146,      644,     1146,      641,     1152,      638,      552,      640,
     551,      641,     1148,      641,      552,      641,     1148,      641,     1148,      645,
     548,      641,     1148,      643,      549,      643,      549,      644,      549,      641,
    1149,      641,     1153,      638,     1176,    11410,     2338,      641,     1148,      641,
    1148,      641,      584,      609,     1148,      641,     1176,      613,     1148,      641,
     552,      641,      552,      641,     1148,      647,      545,      643,     1146,      643,
    1146,      643,      552,      641,     1148,      641,      552,      641,      556,      638,
	552,      641,     1149,      641,     1148,      641,     1148};
	 
	
	unsigned short int buf_right_40[SYMBOL_CNT] = {0x0768, 40000,   169,
    2312,      642,      549,      643,      550,      648,     1142,      643,
    1146,      643,     1146,      643,     1149,      641,      552,      641,      552,      641,
    1152,      638,      551,      641,     1176,      613,     1148,      641,      580,      613,
    1149,      641,      552,      645,      548,      641,      580,      613,     1149,      641,
    1148,      641,     1148,    12152,     2340,      640,      552,      641,      551,      641,
    1176,      613,     1148,      643,     1146,      647,     1143,      643,      549,      643,
     552,      641,     1148,      641,      553,      640,     1177,      613,     1152,      640,
     549,      643,     1146,      643,      549,      643,      549,      643,      550,      643,
    1146,      643,     1150,      640,     1146,    12043,     2336,      643,      550,      643,
     549,      643,     1150,      640,     1146,      644,     1146,      643,     1146,      643,
     550,      643,      549,      643,     1152,      638,      551,      641,     1148,      641,
    1177,      613,      551,      641,     1148,      641,      551,      645,      548,      641,
     551,      641,     1177,      613,     1148,      643,     1146,    12122,     2341,      639,
     549,      643,      552,      641,     1148,      641,     1148,      641,     1149,      645,
    1145,      641,      580,      613,      552,      641,     1148,      641,      552,      641,
    1177,      613,     1152,      638,      579,      613,     1148,      643,      552,      641,
	549,      643,      550,      643,     1146,      643,     1152,      638,     1148};
	
	
	//ir_freq = 0x3; //38KHz
	ir_freq = 0x4; //40KHz
	I2C_Read(0x60, 0x02, read_data, 1);
	ir_ctrl_data = read_data[0];	
	I2C_Read(0x60, 0x05, read_data, 1);
	ir_freq = read_data[0] & 0x0F;	
	//ir_freq = 0x3; //38KHz
	ir_freq = 0x4; //40KHz
	printf("ir_ctrl_data = 0x%x\n", ir_ctrl_data);
	printf("ir_freq = 0x%x\n", ir_freq);			
	ir_ctrl_data |= 0x10;		//IR emitter mode
	ir_ctrl_data &= 0xF0;
	ir_ctrl_data |= ir_freq;
	write_data[0] = ir_ctrl_data;
	I2C_Write(0x60, 0x02, write_data, 1);
	printf("Write config ir_ctrl_data = 0x%x\n", write_data[0]);
	I2C_Read(0x60, 0x02, read_data, 1);
	ir_ctrl_data = read_data[0];	
	printf("Read config ir_ctrl_data = 0x%x\n", read_data[0]);
			
	fd = open("/dev/muxir",O_RDWR);
	if(fd < 0 )
	{
		printf("can not open /dev/muxir!\n");
		return -1;
	}
	
	ioctl(fd, IR_EMITTER_MODE, 0); 
	
	for(i = 0; i< 5; i++)
	{
		printf("\nWait %d ...\n", i);
		sleep(1);
	}	
	
	while (1) 
	{
		/*
		printf("\nSend 38KHz Left code!\n");
		ret = write(fd, &buf_left_38, sizeof(buf_left_38));           
		sleep(2);	
		printf("\nSend 38KHz Right code!\n");
		ret = write(fd, &buf_right_38, sizeof(buf_right_38));           
		sleep(2);			
		*/
		
		printf("\nSend 40KHz Left code!\n");
		ret = write(fd, &buf_left_40, sizeof(buf_left_40));           		
		sleep(2);		
		printf("\nSend 40KHz right code!\n");
		ret = write(fd, &buf_right_40, sizeof(buf_right_40));           
		sleep(2);	
		
		
		
		//to test carrier 56KHZ
		/*
		printf("\nSend 56KHz standby code!\n");
		ret = write(fd, &buf_pwr_56, sizeof(buf_pwr_56));           
		sleep(5);	
		*/
		
	}	
    close(fd);
    return 0;
}