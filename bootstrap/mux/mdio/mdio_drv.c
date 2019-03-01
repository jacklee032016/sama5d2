

#include "mdio_drv.h"


static unsigned int mdio_get_twi_bus(void)
{
	unsigned int bus = 0;

#if defined(CONFIG_TWI0)
	bus = 0;
#elif defined(CONFIG_TWI1)
	bus = 1;
#elif defined(CONFIG_TWI2)
	bus = 2;
#elif defined(CONFIG_TWI3)
	bus = 3;
#endif
	return bus;
}

// --------------------------------------------
// Init FPGA registers
//
unsigned char  mdio_init(void)
{
	
	return 1;
}


unsigned char  mdio_write_addr(unsigned char  prtad, unsigned char  devad, unsigned int  data)
{
int i;
unsigned char	buf[6];
unsigned int bus;

	bus = mdio_get_twi_bus();

	buf[0] = MDIO_OPS_ADDR;		// op code
	buf[1] = prtad;
	buf[2] = devad;
	buf[3] = data;
	buf[4] = (data>>8);
	buf[5] = 0x01;	// start cmd
		
	twi_write(bus, I2C_ADDR_MDIO, MDIO_OPS, 1, buf, 6);

	return 1;
}

unsigned char  mdio_write_data(unsigned char  prtad, unsigned char  devad, unsigned int  data)
{
int i;
unsigned char	buf[6];
unsigned int bus;

	bus = mdio_get_twi_bus();

	buf[0] = MDIO_OPS_WRITE;		// op code
	buf[1] = prtad;
	buf[2] = devad;
	buf[3] = data;
	buf[4] = (data>>8);
	buf[5] = 0x01;	// start cmd

	twi_write(bus, I2C_ADDR_MDIO, MDIO_OPS, 1, buf, 6);
	return 1;
}

unsigned char  mdio_read_data(unsigned char  prtad, unsigned char  devad, unsigned int  *data)
{
int i;
unsigned char	buf[6];
unsigned int bus;

	bus = mdio_get_twi_bus();

	buf[0] = MDIO_OPS_READ;		// op code
	buf[1] = prtad;
	buf[2] = devad;
	buf[3] = 0x55;
	buf[4] = 0x55;
	buf[5] = 0x01;	// start cmd

	twi_write(bus, I2C_ADDR_MDIO, MDIO_OPS, 1, buf, 6);

	twi_read(bus, I2C_ADDR_MDIO, FPGA_DATA, 1, buf, 2);  // read data register
	*data = (buf[1]<<8 | buf[0]);
	
	return 1;
}

unsigned char  mdio_read_data_incr(unsigned char  prtad, unsigned char  devad, unsigned int  *data)
{
int i;
unsigned char	buf[6];
unsigned int bus;

	bus = mdio_get_twi_bus();

	buf[0] = MDIO_OPS_READ_INC;		// op code
	buf[1] = prtad;
	buf[2] = devad;
	buf[3] = 0xDE;
	buf[4] = 0xAD;
	buf[5] = 0x01;	// start cmd

	twi_write(bus, I2C_ADDR_MDIO, MDIO_OPS, 1, buf, 6);

	twi_read(bus, I2C_ADDR_MDIO, FPGA_DATA, 1, buf, 2);  // read data register
	*data = (buf[1]<<8 | buf[0]);
	return 1;
}

unsigned char  mdio_read_data_buf(unsigned int  *data)
{
int i;
unsigned char	buf[6];
unsigned int bus;

	bus = mdio_get_twi_bus();

	twi_read(bus, I2C_ADDR_MDIO, FPGA_DATA, 1, buf, 2);  // read data register
	*data = (buf[1]<<8 | buf[0]);
	
	return 1;
}


