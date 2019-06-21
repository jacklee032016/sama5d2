

#include "common.h"
#include "hardware.h"
#include "board.h"
#include "twi.h"
#include "timer.h"	/* delay */
#include "debug.h"

#include "mux7xx.h"

#if	(MUX_BOARD == MUX_BOARD_768)
#include "si5351bRegs.h"
#elif (MUX_BOARD == MUX_BOARD_774)
#include "si5351bRegisters.h"
#endif

int  muxSi5351bHwInit(void)
{
	int i;
	unsigned char	buffer[2];
	unsigned char reg_addr;
	unsigned int bus;

	bus = muxHwTwiSetMuxChannel(MUX_BUS_SI5351B);
#if	(MUX_BOARD == MUX_BOARD_768)
	buffer[0] = 0xff;
	twi_write(bus, EXT_I2C_DEV_SI5351B, 3, 1, buffer, 1);

	buffer[0] = 0x80;
	twi_write(bus, EXT_I2C_DEV_SI5351B, 16, 1, buffer, 1);
	twi_write(bus, EXT_I2C_DEV_SI5351B, 17, 1, buffer, 1);
	twi_write(bus, EXT_I2C_DEV_SI5351B, 18, 1, buffer, 1);
	twi_write(bus, EXT_I2C_DEV_SI5351B, 19, 1, buffer, 1);
	twi_write(bus, EXT_I2C_DEV_SI5351B, 20, 1, buffer, 1);
	twi_write(bus, EXT_I2C_DEV_SI5351B, 21, 1, buffer, 1);
	twi_write(bus, EXT_I2C_DEV_SI5351B, 22, 1, buffer, 1);
	twi_write(bus, EXT_I2C_DEV_SI5351B, 23, 1, buffer, 1);


	for (i=0; i<NUM_REGS_MAX; i++)
	{
		buffer[0] = Reg_Store[i].Reg_Val;
		twi_write(bus, EXT_I2C_DEV_SI5351B, Reg_Store[i].Reg_Addr, 1, buffer, 1);
	}

	// adjust crystal capacitante register
	buffer[0] = 0x12;
	twi_write(bus, EXT_I2C_DEV_SI5351B, 183, 1, buffer, 1);
	// reset PLL
	buffer[0] = 0xAC;
	twi_write(bus, EXT_I2C_DEV_SI5351B, 177, 1, buffer, 1);
	buffer[0] = 0x00;
	twi_write(bus, EXT_I2C_DEV_SI5351B, 3, 1, buffer, 1);
	
#if 0
	if(twi_read(bus, EXT_I2C_DEV_SI5351B, 0, 1, buffer, 1))
	{
		dbg_info("SI5351B read failed\n");
		return -1;
	}
	dbg_info("SI5351B status: %x, %s\n", buffer[0], (buffer[0]==0x11)?"OK":"Failed");
#endif

#elif (MUX_BOARD == MUX_BOARD_774)
	// Turn off the clocks
	buffer[0] = 0xff;
	if(twi_write(bus, EXT_I2C_DEV_SI5351B, 3, 1, buffer, 1) )
	{
		EXT_ERRORF("SI5351B, 3.1 failed");
	}

	for (i=0; i<SI5351B_REVB_REG_CONFIG_NUM_REGS; i++)
	{
		buffer[0] = si5351b_revb_registers[i].value;
		reg_addr = (unsigned char )si5351b_revb_registers[i].address;
		if(twi_write(bus, EXT_I2C_DEV_SI5351B, reg_addr, 1, buffer, 1) )
		{
			EXT_ERRORF("SI5351B, %x. failed", reg_addr);
		}
		
	}

	EXT_INFOF("SI5351B total %d regsiters have been written", i);

//	buffer[0] = 0xAC;
//	twi_write(bus, EXT_I2C_DEV_SI5351B, 177, 1, buffer, 1);

  // Turn on the clocks 
	buffer[0] = 0x00;
	if(twi_write(bus, EXT_I2C_DEV_SI5351B, 3, 1, buffer, 1) )
	{
		EXT_ERRORF("SI5351B, 3.1 again failed");
	}
#endif

	return 0;
}


int  si5351bTxCfg(void)
{
	unsigned char	buffer[2];
	unsigned int bus;

	bus = muxHwTwiSetMuxChannel(MUX_BUS_SI5351B);

  // Disable the clock 6
	buffer[0] = 0x8C;
	twi_write(bus, EXT_I2C_DEV_SI5351B, 0x16, 1, buffer, 1);
	buffer[0] = 0x00;
	twi_write(bus, EXT_I2C_DEV_SI5351B, 0x5A, 1, buffer, 1);
	
	return 0;
}

#define	WITH_PCA		0


static unsigned char  _mdioWriteAddr(unsigned char  prtad, unsigned char  devad, unsigned int  data)
{
	unsigned char	buf[6];
	unsigned int bus;

#if WITH_PCA
	bus = muxHwTwiSetMuxChannel(MUX_BUS_MDIO);
#else
	bus = muxHwTwiBus();
#endif

	buf[0] = MDIO_OPS_ADDR;		// op code
	buf[1] = prtad;
	buf[2] = devad;
	buf[3] = data;
	buf[4] = (data>>8);
	buf[5] = 0x01;	// start cmd
		
	twi_write(bus, EXT_I2C_DEV_MDIO, MDIO_OPS, 1, buf, 6);

	return 1;
}

static unsigned char  _mdioWriteData(unsigned char  prtad, unsigned char  devad, unsigned int  data)
{
	unsigned char	buf[6];
	unsigned int bus;

#if WITH_PCA
	bus = muxHwTwiSetMuxChannel(MUX_BUS_MDIO);
#else
	bus = muxHwTwiBus();
#endif

	buf[0] = MDIO_OPS_WRITE;		// op code
	buf[1] = prtad;
	buf[2] = devad;
	buf[3] = data;
	buf[4] = (data>>8);
	buf[5] = 0x01;	// start cmd

	twi_write(bus, EXT_I2C_DEV_MDIO, MDIO_OPS, 1, buf, 6);
	return 1;
}

static unsigned char  _mdioReadData(unsigned char  prtad, unsigned char  devad, unsigned int  *data)
{
	unsigned char	buf[6];
	unsigned int bus;

#if WITH_PCA
	bus = muxHwTwiSetMuxChannel(MUX_BUS_MDIO);
#else
	bus = muxHwTwiBus();
#endif

	buf[0] = MDIO_OPS_READ;		// op code
	buf[1] = prtad;
	buf[2] = devad;
	buf[3] = 0x55;
	buf[4] = 0x55;
	buf[5] = 0x01;	// start cmd

	twi_write(bus, EXT_I2C_DEV_MDIO, MDIO_OPS, 1, buf, 6);

	twi_read(bus, EXT_I2C_DEV_MDIO, FPGA_DATA, 1, buf, 2);  // read data register
	*data = (buf[1]<<8 | buf[0]);
	
	return 1;
}

#if 0
static unsigned char  _mdioReadDataIncr(unsigned char  prtad, unsigned char  devad, unsigned int  *data)
{
	unsigned char	buf[6];
	unsigned int bus;

#if WITH_PCA
	bus = muxHwTwiSetMuxChannel(MUX_BUS_MDIO);
#else
	bus = muxHwTwiBus();
#endif

	buf[0] = MDIO_OPS_READ_INC;		// op code
	buf[1] = prtad;
	buf[2] = devad;
	buf[3] = 0xDE;
	buf[4] = 0xAD;
	buf[5] = 0x01;	// start cmd

	twi_write(bus, EXT_I2C_DEV_MDIO, MDIO_OPS, 1, buf, 6);

	twi_read(bus, EXT_I2C_DEV_MDIO, FPGA_DATA, 1, buf, 2);  // read data register
	*data = (buf[1]<<8 | buf[0]);
	return 1;
}

static unsigned char  _mdioReadDataBuf(unsigned int  *data)
{
	unsigned char	buf[6];
	unsigned int bus;

#if WITH_PCA
	bus = muxHwTwiSetMuxChannel(MUX_BUS_MDIO);
#else
	bus = muxHwTwiBus();
#endif

	twi_read(bus, EXT_I2C_DEV_MDIO, FPGA_DATA, 1, buf, 2);  // read data register
	*data = (buf[1]<<8 | buf[0]);
	
	return 1;
}


static unsigned char  _mdioInit(void)
{
	return 1;
}
#endif

int mdioInit(void)
{
	int timeout;
	unsigned int mdio_page;
	unsigned int mdio_addr;
	unsigned int mdio_data;

	dbg_info("MDIO init....");
	timeout = 1000 / 50;	// 5 sec delay max
	mdio_page=0x1;  mdio_addr=0xcc02;
	_mdioWriteAddr(AQR105_PRTAD, mdio_page, mdio_addr);	 // set reg addr

	while (--timeout!=0)
	{
		_mdioReadData(AQR105_PRTAD, mdio_page, &mdio_data);	// read data
		dbg_printf("%x", mdio_data);
		if ((mdio_data & 0x03) == 0x01)
		{
			dbg_info("MDIO OK!");
			break;		
		}
		udelay(50000);
		dbg_printf(".");
	}

	if(timeout == 0)
	{
		dbg_info("Timeout when MDIO access\n");
		return -1;
	}

	mdio_page=0x1e;  mdio_addr=0x0020;
	_mdioWriteAddr(AQR105_PRTAD, mdio_page, mdio_addr);	 	// set reg addr
	_mdioReadData(AQR105_PRTAD, mdio_page, &mdio_data);		// read data
	dbg_info("\nAQR105 Firmware version %x.%x\n", (mdio_data>>8), (mdio_data&0xff) );


	mdio_page=0x1e;  mdio_addr=0xC820;
	_mdioWriteAddr(AQR105_PRTAD, mdio_page, mdio_addr);      	// set reg addr
	_mdioReadData(AQR105_PRTAD, mdio_page, &mdio_data);     	// read data
	dbg_info("AQR105 Temperature %d.%d\n", (mdio_data>>8), (mdio_data&0xff) );

	// change LED behaviour
	mdio_page=0x1e;  mdio_addr=0xC430; mdio_data= 0x000F;        	// blink on TX & RX, strech activity by 100 ms
	_mdioWriteAddr(AQR105_PRTAD, mdio_page, mdio_addr);      	// set reg addr
	_mdioWriteData(AQR105_PRTAD, mdio_page, mdio_data);      	// read data

	mdio_page=0x1e;  mdio_addr=0xC431; mdio_data= 0x0083;        	// Link 10G, strech activity by 100 ms
	_mdioWriteAddr(AQR105_PRTAD, mdio_page, mdio_addr);      	// set reg addr
	_mdioWriteData(AQR105_PRTAD, mdio_page, mdio_data);      	// read data

//	mdelay(5000);
	return 0;
}


