#include "link_10G.h"
#include "FPGA.h"

#define ACT_LED_MAGIC 'A'
#define ACT_LED_ON  	_IO(ACT_LED_MAGIC, 0)  
#define ACT_LED_OFF 	_IO(ACT_LED_MAGIC, 1)

extern int data_valid_10G;

#define LEN_STATUS_PCS_PMA 12
unsigned char g_flag_LOGs;
char g_flag_PCS_PMA;
char g_status_PCS_PMA[LEN_STATUS_PCS_PMA];
byte g_control_Color[2];

void * link_10G_routine()
{
	byte buf_control_mode[2];
	unsigned char  prtad = 0x00;
	unsigned char  devad = 0x01;
	unsigned int data;
	unsigned char link_status;
	unsigned char dhcp_status;
	unsigned char * gateway;
	static unsigned int link_status_prev = 1;	
	int i;
	int status;	
	int act_led_fd;
	act_led_fd = open("/dev/actled",O_RDWR);
	if(act_led_fd < 0)
		printf("can not open /dev/actled!\n");
	//init active led as OFF
	ioctl(act_led_fd, ACT_LED_OFF, 0);  
	
	g_flag_PCS_PMA = 0;
	g_control_Color[0] = 0xFF;
	g_control_Color[1] = 0xFF;
	g_flag_LOGs = 0x00;//0x0F;
	
	data = 0x0001;
#ifdef MDIO_DRV_ENABLE
	mdio_write_addr(prtad, devad, data);
#endif
	while (1)
	{	
	
		checkColorSpace();
		checkStatusPCS_PMA();

#ifdef FLAGS_BY_DIP_SWITCH
		// define logs by dipSwitch
		unsigned int dipSwitchValue;
		dipSwitchValue = readDipSwitch();
		if (dipSwitchValue != 0x100) {
			if ((g_flag_LOGs & 0xFF) != (dipSwitchValue & 0xFF)) {
				printf("DIP Switch %02X. \n", dipSwitchValue & 0xFF);
				buf_control_mode[1] = 0;
				if (((g_flag_LOGs & 0x04) == 0)&&((dipSwitchValue & 0x04) != 0)) {
					printf("SPI to FLASH. \n");
					buf_control_mode[0] = 1;
					pthread_mutex_lock(&i2c_bus_lock);	
					I2C_Write(0x60, (byte)32, buf_control_mode, (word)1);
					pthread_mutex_unlock(&i2c_bus_lock);
				}
				else if (((g_flag_LOGs & 0x04) != 0)&&((dipSwitchValue & 0x04) == 0)) {
					printf("SPI to HDMI. \n");
					buf_control_mode[0] = 0;
					pthread_mutex_lock(&i2c_bus_lock);	
					I2C_Write(0x60, (byte)32, buf_control_mode, (word)1);
					pthread_mutex_unlock(&i2c_bus_lock);
				}
				g_flag_LOGs = dipSwitchValue & 0xFF;
			}
		}
#endif
		pthread_mutex_lock(&i2c_bus_lock);
#ifdef MDIO_DRV_ENABLE
		mdio_read_data(prtad, devad, &data);
#endif
		pthread_mutex_unlock(&i2c_bus_lock);
		link_status = ((data & 0x04) >> 2); 
				
		if((link_status_prev == 0) && (link_status == 1))
		{
			printf("10G link is up. \n");
			//need to reboot or call udhcpc again???
		}
		if(link_status == 0)
		{
			ioctl(act_led_fd, ACT_LED_OFF, 0);
			sleep(1);
		}
		else if((link_status == 1) && (data_valid_10G == 0))
		{
			ioctl(act_led_fd, ACT_LED_ON, 0);
			sleep(1);
		}	
		else if((link_status == 1) && (data_valid_10G == 1))
		{
			for(i = 0; i < 5; i++)
			{
				ioctl(act_led_fd, ACT_LED_ON, 0);
				usleep(200000);
				ioctl(act_led_fd, ACT_LED_OFF, 0);
				usleep(200000);
			}
		}
		link_status_prev = link_status;		
	}	
	close(act_led_fd);
}

#ifdef MDIO_DRV_ENABLE
unsigned char  mdio_write_addr(unsigned char  prtad, unsigned char  devad, unsigned int  data)
{
	int i;
	unsigned char   buf[6];
	unsigned int bus;
	buf[0] = 0;  // remove PCS and PMA resets bits
	I2C_Write(I2C_ADDR_PCS_PMA, PCS_PMA_RESET, buf, 1);
	return 1;
}

unsigned char  mdio_read_data(unsigned char  prtad, unsigned char  devad, unsigned int  *data)
{
	int i;
	unsigned char   buf[12];
	
	I2C_Read(I2C_ADDR_PCS_PMA, PMA_STATUS_0, buf, 12);
	*data = (buf[1] << 8 | buf[0]);
	
	return 1;
}
#endif

void checkColorSpace(void)
{
	byte buf_control_Color[2];
	byte deviceId = 0x60;
	byte ColorReg;
	
	pthread_mutex_lock(&i2c_bus_lock);	
	I2C_Read(deviceId, (byte)44, buf_control_Color, (word)2);
	pthread_mutex_unlock(&i2c_bus_lock);
	
	if ((g_control_Color[0] != buf_control_Color[0]) || (g_control_Color[1] != buf_control_Color[1])) {
		g_control_Color[0] = buf_control_Color[0];
		g_control_Color[1] = buf_control_Color[1];
		if ((g_flag_LOGs & 0x02) != 0) {
			printf("\nMicroblaze video info changed: (reg 44-45): %02x %02x\n", buf_control_Color[0], buf_control_Color[1]);
		}
		ColorReg = 0;
		if (buf_control_Color[0] == 10)
			ColorReg = 1;
		else if (buf_control_Color[0] == 12)
			ColorReg = 2;
		else if (buf_control_Color[0] == 16)
			ColorReg = 3;
//		if (buf_control_Color[1] == 1) // YCbCr 4:4:4
//			ColorReg |= (0<<2);
		if (buf_control_Color[1] == 2) // YCbCr 4:2:2
			ColorReg |= (1<<2);
		else if (buf_control_Color[1] == 3) // YCbCr 4:2:0
			ColorReg |= (2<<2);
		else if (buf_control_Color[1] == 0) // RGB444
			ColorReg |= (4<<2);
		buf_control_Color[0] = ColorReg;
		pthread_mutex_lock(&i2c_bus_lock);	
		I2C_Write(deviceId, (byte)1, buf_control_Color, (word)1);
		pthread_mutex_unlock(&i2c_bus_lock);
		if ((g_flag_LOGs & 0x02) != 0) {
			printf("Set ColorSpace (reg 01): %02x\n", ColorReg);
		}
	}
}

// For PCM PMA Debug. Check and print PCM PMA status in changing any bit.
void checkStatusPCS_PMA(void)
{
	byte buf_status_PCS_PMA[LEN_STATUS_PCS_PMA];
	byte buf_control_PCS_PMA[3];
	byte deviceId = 0x6E;
	byte i;
	byte isChange = 0;
	
	pthread_mutex_lock(&i2c_bus_lock);	
	if (g_flag_PCS_PMA == 0) {
		I2C_Read(deviceId, (byte)0, &buf_control_PCS_PMA[0], (word)1);
		I2C_Read(deviceId, (byte)17, &buf_control_PCS_PMA[1], (word)1);
		I2C_Read(deviceId, (byte)20, &buf_control_PCS_PMA[2], (word)1);
	}
	I2C_Read(deviceId, (byte)32, buf_status_PCS_PMA, (word)LEN_STATUS_PCS_PMA);
	pthread_mutex_unlock(&i2c_bus_lock);
	
	if (g_flag_PCS_PMA == 0) {
		g_flag_PCS_PMA |= 0x08;
		isChange = 1;
		if ((g_flag_LOGs & 0x04) != 0) {
			printf("\nPCS PMA original status (reg 32-43):\n");
		}
	}
	else {
		for (i = 0; i < LEN_STATUS_PCS_PMA; i++) {
			if (g_status_PCS_PMA[i] != buf_status_PCS_PMA[i])
				isChange = 1;
		}
		if ((isChange)&&((g_flag_LOGs & 0x04) != 0))
			printf("\nPCS PMA new status (reg 32-43):\n");
	}
	for (i = 0; i < LEN_STATUS_PCS_PMA; i++) {
		g_status_PCS_PMA[i] = buf_status_PCS_PMA[i];
		if ((isChange)&&((g_flag_LOGs & 0x04) != 0))
			printf("%02x ", buf_status_PCS_PMA[i]);				
	}
	if ((isChange)&&((g_flag_LOGs & 0x04) != 0))
		printf("\nPCS PMA control (reg 0,17,20): %02x %02x %02x\n", buf_control_PCS_PMA[0], buf_control_PCS_PMA[1], buf_control_PCS_PMA[2]);
}

