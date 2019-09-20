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

/// /home/alex/buildroot/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-gcc muxir_sensor_select_fpga.c -o muxir_sensor_select_fpga

#define SYMBOL_CNT 200

#define IR_MAGIC 'I'
#define IR_SENSOR_MODE		_IO(IR_MAGIC, 0)  
#define IR_EMITTER_MODE		_IO(IR_MAGIC, 1) 
#define IR_IDLE_MODE		_IO(IR_MAGIC, 2) 
#define IR_READ_MODE	  	_IOR(IR_MAGIC, 3, unsigned char *)

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
	int data_cnt;
	unsigned char read_data[10];
	unsigned char write_data[10];
	unsigned char ir_ctrl_data;
	unsigned char ir_freq;
		
	printf("ir_freq = 0x%x\n", ir_freq);		
	ir_ctrl_data = 0;
	ir_ctrl_data |= 0x20;		//IR sensor mode, enable demodulation	
	write_data[0] = ir_ctrl_data;
	I2C_Write(0x60, 0x02, write_data, 1);
	printf("Write config ir_ctrl_data = 0x%x\n", write_data[0]);
	I2C_Read(0x60, 0x02, read_data, 1);
	ir_ctrl_data = read_data[0];	
	printf("Read config ir_ctrl_data = 0x%x\n", read_data[0]);
	
	
	fd_set	fds;
	struct timeval timeout;
	
	timeout.tv_usec = 0;  
	timeout.tv_sec = 5;
	
	fd =open("/dev/muxir",O_RDWR|O_NONBLOCK);
	if(fd < 0)
	{
		printf("can not open /dev/muxir!\n");
		return -1;
	}
	ioctl(fd, IR_SENSOR_MODE, 0);
	while(1)
	{
		timeout.tv_usec = 0;  
		timeout.tv_sec = 5;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);	
		while (select(fd + 1, &fds, NULL, NULL, &timeout) > 0)
		{        
			printf("Wait for IR data...\n");
			ret = read(fd, &buf, sizeof(buf));
			printf("read ret = %d\n", buf[1]);
			printf("symbol cnt = %d\n", buf[2]);
			j = 0;		
		
			data_cnt = buf[2];
			if(data_cnt > 200)
				data_cnt = 200;
			for( i = 0; i < data_cnt; i++)
			{	
				printf(" %7u, ", buf[i]);		
				j++;
				if((j%10) == 0)
					printf("\n");			
			}			
			printf("\nData finish.");
			I2C_Read(0x60, 0x05, read_data, 1);
			ir_freq = read_data[0] & 0x0F;		
			printf("ir_freq = 0x%x\n", ir_freq);
			switch(ir_freq)
			{
				case 0x0:
					printf("carrier :  30K\n");
					break;
				
				case 0x1:
					printf("carrier :  33K\n");
					break;
				
				case 0x2:
					printf("carrier :  36K\n");
					break;

				case 0x3:
					printf("carrier :  38K\n");
					break;
				
				case 0x4:
					printf("carrier :  40K\n");				
					break;
				
				case 0x5:
					printf("carrier :  56K\n");				
					break;
			
				default:
					printf("carrier unknown\n");				
				
			}
			printf("\n");		
		}
		printf("SELECT IR read time out. Set FPGA IR mode. sleep 1s.\n");
		sleep(1);
		ir_ctrl_data = 0;
		ir_ctrl_data |= 0x20;		//IR sensor mode, enable demodulation	
		write_data[0] = ir_ctrl_data;
		I2C_Write(0x60, 0x02, write_data, 1);
	}
    close(fd);
    return 0;
}