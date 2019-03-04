

#include "muxStrapCmn.h"
#define	WITH_PCA		0

#define	MDIO_BUS		0xFF

// Init FPGA registers
unsigned char  mdio_init(void)
{
	return 1;
}


unsigned char  mdio_write_addr(unsigned char  prtad, unsigned char  devad, unsigned int  data)
{
	unsigned char	buf[6];
	unsigned int bus;

	bus = muxHwGetTwiBus();
#if WITH_PCA
	buf[0] = 0x04 | MDIO_BUS;	// set mux for si5351b 
	twi_write(bus, PCA9544_ADDR, 0, 0, buf, 1);
#endif

	buf[0] = MDIO_OPS_ADDR;		// op code
	buf[1] = prtad;
	buf[2] = devad;
	buf[3] = data;
	buf[4] = (data>>8);
	buf[5] = 0x01;	// start cmd
		
	if(twi_write(bus, I2C_ADDR_MDIO, MDIO_OPS, 1, buf, 6))
	{
		dbg_info("MDIO write failed\n");
		return 1;
	}

	return 0;
}

unsigned char  mdio_write_data(unsigned char  prtad, unsigned char  devad, unsigned int  data)
{
	unsigned char	buf[6];
	unsigned int bus;

	bus = muxHwGetTwiBus();
#if WITH_PCA
	buf[0] = 0x04 | MDIO_BUS;	// set mux for si5351b 
	twi_write(bus, PCA9544_ADDR, 0, 0, buf, 1);
#endif

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
	unsigned char	buf[6];
	unsigned int bus;

	bus = muxHwGetTwiBus();
#if WITH_PCA
	buf[0] = 0x04 | MDIO_BUS;	// set mux for si5351b 
	twi_write(bus, PCA9544_ADDR, 0, 0, buf, 1);
#endif

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
	unsigned char	buf[6];
	unsigned int bus;

	bus = muxHwGetTwiBus();
#if WITH_PCA
	buf[0] = 0x04 | MDIO_BUS;	// set mux for si5351b 
	twi_write(bus, PCA9544_ADDR, 0, 0, buf, 1);
#endif

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
	unsigned char	buf[6];
	unsigned int bus;

	bus = muxHwGetTwiBus();
#if WITH_PCA
	buf[0] = 0x04 | MDIO_BUS;	// set mux for si5351b 
	twi_write(bus, PCA9544_ADDR, 0, 0, buf, 1);
#endif

	twi_read(bus, I2C_ADDR_MDIO, FPGA_DATA, 1, buf, 2);  // read data register
	*data = (buf[1]<<8 | buf[0]);
	
	return 1;
}


