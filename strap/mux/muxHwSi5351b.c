
#include "common.h"
#include "hardware.h"
#include "board.h"
#include "twi.h"
#include "timer.h"	/* delay */
#include "debug.h"

#include "mux7xx.h"

#define	SI5351_READ(addr, value) \
	{ if(twi_read(bus, EXT_I2C_DEV_SI5351B, (addr), 1, (value), 1)){ \
		EXT_ERRORF("SI5351B read reg 0x%02x failed", (addr));  return -1;} }


#define	SI5351_WRITE(addr, value)	\
	{ if(twi_write(bus, EXT_I2C_DEV_SI5351B, (addr), 1, (value), 1) ){ \
		EXT_ERRORF("SI5351B write reg 0x%02x failed", (addr)); return 1;} }


#if	(MUX_BOARD == MUX_BOARD_768 )
#include "si5351bRegs.h"
#elif ( MUX_BOARD == MUX_BOARD_774)
//#include "si5351bRegister500774.h"
//#include "si5351bRegisters.09.27.h"
#include "Si5351B-RevB-RegistersTest.h"
#elif (MUX_BOARD == MUX_BOARD_767)
#include "si5351bRegisters.h"
#endif

int  muxSi5351bHwInit(void)
{
	int i;
	unsigned char	buffer[2];
	unsigned char reg_addr;
	unsigned int bus;

	bus = muxHwTwiSetMuxChannel(MUX_BUS_SI5351B);
#if	(MUX_BOARD == MUX_BOARD_768 )// || MUX_BOARD == MUX_BOARD_774)
	buffer[0] = 0xff;
	SI5351_WRITE(3, buffer);

	buffer[0] = 0x80;
	SI5351_WRITE(16, buffer);
	SI5351_WRITE(17, buffer);
	SI5351_WRITE(18, buffer);
	SI5351_WRITE(19, buffer);
	SI5351_WRITE(20, buffer);
	SI5351_WRITE(21, buffer);
	SI5351_WRITE(22, buffer);
	SI5351_WRITE(23, buffer);


	for (i=0; i<NUM_REGS_MAX; i++)
	{
		buffer[0] = Reg_Store[i].Reg_Val;
		SI5351_WRITE(Reg_Store[i].Reg_Addr, buffer);
	}

	// adjust crystal capacitante register
	buffer[0] = 0x12;
	SI5351_WRITE(183, buffer);
	// reset PLL
	buffer[0] = 0xAC;
	SI5351_WRITE(177, buffer);
	buffer[0] = 0x00;
	SI5351_WRITE(3, buffer);
	
#if 0
	SI5351_WRITE(0, buffer);
	EXT_INFOF("SI5351B status: %x, %s\n", buffer[0], (buffer[0]==0x11)?"OK":"Failed");
#endif

#elif (MUX_BOARD == MUX_BOARD_774 )
	// Turn off the clocks
	buffer[0] = 0xff;
	SI5351_WRITE(3, buffer);

	for (i=0; i<SI5351B_REVB_REG_CONFIG_NUM_REGS; i++)
	{
		buffer[0] = si5351b_revb_registers_m774[i].value;
		reg_addr = (unsigned char )si5351b_revb_registers_m774[i].address;
		SI5351_WRITE(reg_addr, buffer);
	}

	EXT_INFOF("SI5351B("BOARD_NAME") total %d regsiters have been written", i);

	// adjust crystal capacitante register
	buffer[0] = 0x12;
	SI5351_WRITE(183, buffer);
	// reset PLL
	buffer[0] = 0xAC;
//	buffer[0] = 0xA0;
	SI5351_WRITE(177, buffer);
	buffer[0] = 0x00;
	SI5351_WRITE(3, buffer);

  // Turn on the clocks 
	buffer[0] = 0x00;
	SI5351_WRITE(3, buffer);
	
	SI5351_READ(0, buffer);
#if 0	
	if(buffer[0]!= 0x11)
	{
#if 0
		/* for output format, this format will make no output */
		EXT_ERRORF("SI5351B status failed, status: 0x%02x"EXT_NEW_LINE, buffer[0] );
#else
		EXT_ERRORF("SI5351B status failed, status: 0x%x"EXT_NEW_LINE, buffer[0] );
#endif
	}
	else
#endif		
	{
		EXT_INFOF("SI5351B has been turn on successfully, status:0x%x"EXT_NEW_LINE, buffer[0] );
	}

#elif (MUX_BOARD == MUX_BOARD_767 )
	// Turn off the clocks
	buffer[0] = 0xff;
	SI5351_WRITE(3, buffer);

	for (i=0; i<SI5351B_REVB_REG_CONFIG_NUM_REGS; i++)
	{
		buffer[0] = si5351b_revb_registers[i].value;
		reg_addr = (unsigned char )si5351b_revb_registers[i].address;
		SI5351_WRITE(reg_addr, buffer);
	}

	EXT_INFOF("SI5351B total %d regsiters have been written", i);

//	buffer[0] = 0xAC;
//	twi_write(bus, EXT_I2C_DEV_SI5351B, 177, 1, buffer, 1);

  // Turn on the clocks 
	buffer[0] = 0x00;
	SI5351_WRITE(3, buffer );
#endif

	return 0;
}


int  si5351bTxCfg(void)
{
	unsigned char	buffer[2];
	unsigned int bus;

	bus = muxHwTwiSetMuxChannel(MUX_BUS_SI5351B);

#if 1
  // Disable the clock 6
	buffer[0] = 0x8C;
	SI5351_WRITE(0x16, buffer);	/* 22, power down */
	buffer[0] = 0x00;
	SI5351_WRITE(0x5A, buffer);	/* 90, MS6  */
#endif	
	EXT_INFOF("SI5351B("BOARD_NAME") Clock6 is disabled on TX");
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


