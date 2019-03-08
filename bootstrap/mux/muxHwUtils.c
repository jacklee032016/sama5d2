
#include "muxStrapCmn.h"

#define	EXT_I2C_PCA9554_CS_MAC				(4)	/* CS0, MAC address; not used */
#define	EXT_I2C_PCA9554_CS_SENSOR			(5)	/* CS1, sensor and EEPROM  */
#define	EXT_I2C_PCA9554_CS_FPGA				(6)	/* CS2, FPGA */
#define	EXT_I2C_PCA9554_CS_BOOTROM			(7)	/* CS3, Ethernet switch */

#define	EXT_I2C_PCA9554_CS_NONE				(0xFF) /* connect to I2C of MCU directly */

/* folllowing devices attached on I2C of LM1983 */
#define	EXT_I2C_ADDRESS_SENSOR             		(0x30 >> 1)
#define	EXT_I2C_ADDRESS_AT24MAC             		(0xAE >> 1)
#define	EXT_I2C_ADDRESS_XC7A75T             		(0xFF >> 1)	/* T.B.D. */
#define	EXT_I2C_ADDRESS_RTL8035				0x54


/* registers for LM95245 sensor */
#define	EXT_I2C_SENSOR_LOCAL_TEMP_MSB		0x00
#define	EXT_I2C_SENSOR_LOCAL_TEMP_LSB		0x30
#define	EXT_I2C_SENSOR_MANUFACTURE_ID		0xFE
#define	EXT_I2C_SENSOR_REVISION_ID			0xFF

#define PCA9544_ADDR	(0xE0>>1)

unsigned int muxHwGetTwiBus(void)
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

	
static int _extI2cPca9554Config(unsigned char channel)
{
	unsigned char	buffer[2];
	unsigned int bus;

	bus = muxHwGetTwiBus();
	buffer[0] = channel;
	if(twi_write(bus, PCA9544_ADDR, 0, 0, buffer, 1) )
	{
		dbg_info("PCA9544 Multiplexer failed....\n");
		return -1;
	}

	return 0;
}


int extI2CRead(unsigned char chanNo, unsigned char deviceAddress, unsigned int regAddress, unsigned char regAddressSize, unsigned char *regVal, unsigned char regSize)
{
	unsigned int bus;
	int ret = 0;

#if 1// EXTLAB_BOARD
	if(chanNo != EXT_I2C_PCA9554_CS_NONE)
	{
		ret = _extI2cPca9554Config(chanNo);
	}
	else
	{
		ret = _extI2cPca9554Config(0);
	}
#endif

	if(ret)
	{
		dbg_info("I2C read abort because PCA operation failed\n");
		return ret;
	}

	bus = muxHwGetTwiBus();
	if(twi_read(bus, deviceAddress, regAddress, regAddressSize, regVal, regSize))
	{
		dbg_info("I2C read failed\n");
		return -1;
	}

	return 0;
}

char extI2CWrite(unsigned char chanNo, unsigned char deviceAddress, unsigned int regAddress, unsigned char regAddressSize,  unsigned char *regVal, unsigned char regSize)
{
	unsigned int bus;
	int ret = 0;

#if 1//EXTLAB_BOARD
	if(chanNo != EXT_I2C_PCA9554_CS_NONE)
	{
		ret = _extI2cPca9554Config(chanNo);
	}
	else
	{
		ret = _extI2cPca9554Config(0);
	}
#endif
	if(ret)
	{
		dbg_info("I2C write abort because PCA operation failed\n");
		return ret;
	}

	bus = muxHwGetTwiBus();

	if(twi_write(bus, deviceAddress, regAddress, regAddressSize, regVal, regSize))
	{
		dbg_info("I2C write failed\n");
		return -1;
	}

	return 0; 
}


/*
* temperature in Celsius from LM95245 on I2C bus, byte[1] is degree, byte[0] is k*0.125
*/
short extSensorGetTemperatureCelsius(void)
{/* 11 bits: sign bit + 10 bit value */
	unsigned char regVal =0;
	short temperature = 0;
    
	extI2CRead(EXT_I2C_PCA9554_CS_SENSOR, EXT_I2C_ADDRESS_SENSOR, EXT_I2C_SENSOR_LOCAL_TEMP_MSB, 1, &regVal, 1);

	dbg_info("LM95245 Temp(MSB): %d\n", regVal);
	temperature = (regVal<<8);
	extI2CRead(EXT_I2C_PCA9554_CS_SENSOR, EXT_I2C_ADDRESS_SENSOR, EXT_I2C_SENSOR_LOCAL_TEMP_LSB, 1, &regVal, 1);

	/* only 3 bits in LSB register */
	temperature |=  (regVal>>5); //*0.125;// / 256.0;
      
	dbg_info("LM95245 Temp(LSB): %d*0.125;\n", (regVal>>5) );
	return temperature;
}

#define TEST_SYS_SDRAM_PTRN		0x5AA53CC3

#define TEST_SYS_SDRAM_PRINT 1

static int _extDdrMemoryTestOneArea(unsigned long start, unsigned long end )
{
	unsigned long dram_start;
	unsigned long dram_end;
	unsigned long dram_point;
	int test_res;

	dram_point = 0;
	dram_start = start;
	dram_end = end;

	dbg_printf("Test memory zone from %x -- %x....." EXT_NEW_LINE, start, end);

	dbg_printf("\tWriting....." EXT_NEW_LINE"\t", start, end);
	for(dram_point = dram_start; dram_point < dram_end; dram_point = dram_point + sizeof(unsigned long))
	{
		*(volatile unsigned long*)dram_point = dram_point^TEST_SYS_SDRAM_PTRN;
		if(dram_point%0x100000 == 0)
		{
			dbg_printf(".");
		}
	}
	dbg_printf(EXT_NEW_LINE);
	

	dbg_printf("\tChecking....." EXT_NEW_LINE"\t", start, end);
	for(dram_point = dram_start; dram_point < dram_end; dram_point = dram_point + sizeof(unsigned long))
	{
		if(((*(volatile unsigned long*)dram_point)^TEST_SYS_SDRAM_PTRN) != dram_point)
		{
			dbg_info("DDR Memory Test Failed at addr %x data: %x vs %x\n", dram_point, *(volatile unsigned long*)dram_point, dram_start^TEST_SYS_SDRAM_PTRN);
			dbg_info("next: %x %x %x %x %x %x %x %x\n", 
				*(volatile unsigned long*)(dram_point+sizeof(unsigned long)), *(volatile unsigned long*)(dram_point+2*sizeof(unsigned long)), 
				*(volatile unsigned long*)(dram_point+3*sizeof(unsigned long)), *(volatile unsigned long*)(dram_point+4*sizeof(unsigned long)),
				*(volatile unsigned long*)(dram_point+5*sizeof(unsigned long)), *(volatile unsigned long*)(dram_point+6*sizeof(unsigned long)),
				*(volatile unsigned long*)(dram_point+7*sizeof(unsigned long)), *(volatile unsigned long*)(dram_point+8*sizeof(unsigned long)));
			test_res = 1;
			break;
		}
		if(dram_point%0x100000 == 0)
		{
			dbg_printf("*");
		}
	}
	dbg_printf(EXT_NEW_LINE);

	if(test_res == 0)
	{
		dbg_info("DDR Memory Test from %x to %x Passed.\n", dram_start, dram_end);
	}

	return 0;	
}


int extDdrMemoryTest(void)
{
	int _res;

#define TEST_SYS_SDRAM_BASE		0x20000000
#define TEST_SYS_SDRAM_SIZE		0x10000000	/* 256M */


#define	MEM_SIZE_96M		0x6000000
#define	MEM_SIZE_112M		0x7000000

#define	MEM_SIZE_512M		(0x20000000 - 0x20) //0x1000000)
#define	MEM_SIZE_256M		(0x10000000) //0x1000000)
#define	MEM_SIZE_128M		(0x8000000) //0x1000000)
#if 0
	_res = _extDdrMemoryTestOneArea(TEST_SYS_SDRAM_BASE + 0x0000000 , TEST_SYS_SDRAM_BASE + 0x6000000);
	
	_res += _extDdrMemoryTestOneArea(TEST_SYS_SDRAM_BASE + 0x8000000, TEST_SYS_SDRAM_BASE + 0x8000000 + MEM_SIZE_112M);//0xF000000);
#else
	_res += _extDdrMemoryTestOneArea(AT91C_BASE_DDRCS, AT91C_BASE_DDRCS + MEM_SIZE_128M);
#endif
	return _res;
}


