
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>	
#include <unistd.h>
#include <string.h>

#include "libCmnSys.h"
#include "mux7xx.h"

#define _MISSING_FUNC_FMT	"Error: Adapter does not have %s capability\n"

static int _sysI2cSetSlaveAddr(int fd, int slaveAddress, int force)
{
	/* With force, let the user read from/write to the registers
	   even when a driver is also running */
#if ARCH_ARM
	if (ioctl(fd, force ? I2C_SLAVE_FORCE : I2C_SLAVE, slaveAddress) < 0)
	{
		EXT_ERRORF("Could not set SLAVE address to 0x%02x: %m", slaveAddress);
		return EXIT_FAILURE;
	}
#else
	MUX_DEBUG_CTRL("set SLAVE address to 0x%02x", slaveAddress);
#endif
	return EXIT_SUCCESS;
}

static int _sysI2cCheckFuncs(int fd, int size, int pec)
{
#if ARCH_ARM
	unsigned long funcs;

	/* check adapter functionality */
	if (ioctl(fd, I2C_FUNCS, &funcs) < 0)
	{
		EXT_ERRORF("Could not get the adapter functionality matrix: %m");
		return EXIT_FAILURE;
	}

#if 0
	switch (size)
	{
		case I2C_SMBUS_BYTE:
			if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE))
			{
				fprintf(stderr, _MISSING_FUNC_FMT, "SMBus send byte");
				return EXIT_FAILURE;
			}
			break;

		case I2C_SMBUS_BYTE_DATA:
			if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE_DATA))
			{
				EXT_ERRORF(_MISSING_FUNC_FMT, "SMBus write byte");
				return EXIT_FAILURE;
			}
			break;

		case I2C_SMBUS_WORD_DATA:
			if (!(funcs & I2C_FUNC_SMBUS_WRITE_WORD_DATA))
			{
				EXT_ERRORF( _MISSING_FUNC_FMT, "SMBus write word");
				return EXIT_FAILURE;
			}
			break;

		case I2C_SMBUS_BLOCK_DATA:
			if (!(funcs & I2C_FUNC_SMBUS_WRITE_BLOCK_DATA))
			{
				EXT_ERRORF(_MISSING_FUNC_FMT, "SMBus block write");
				return EXIT_FAILURE;
			}
			break;
		case I2C_SMBUS_I2C_BLOCK_DATA:
			if (!(funcs & I2C_FUNC_SMBUS_WRITE_I2C_BLOCK))
			{
				EXT_ERRORF(_MISSING_FUNC_FMT, "I2C block write");
				return EXIT_FAILURE;
			}
			break;
	}

	if (pec  && !(funcs & (I2C_FUNC_SMBUS_PEC | I2C_FUNC_I2C)))
	{
		EXT_ERRORF("Warning: Adapter does not seem to support PEC\n");
	}
#endif

	if (!(funcs & I2C_FUNC_I2C))
	{
		EXT_ERRORF( _MISSING_FUNC_FMT, "I2C transfers");
		return EXIT_FAILURE;
	}
#endif

	MUX_DEBUG_CTRL("I2C transfers check OK!");

	return EXIT_SUCCESS;
}

int cmnSysI2cOpen(int i2cbus, int quiet)
{
	int fd, len;
	char fileName[CMN_NAME_LENGTH];

	len = snprintf(fileName, sizeof(fileName), I2C_DEV_HOME"/%d", i2cbus);
	if (len >= (int)sizeof(fileName) )
	{
		EXT_ERRORF("%s: path truncated", fileName);
		return EXIT_FAILURE;
	}
	
	fd = open(fileName, O_RDWR);
	if (fd < 0 && (errno == ENOENT || errno == ENOTDIR))
	{
		len = snprintf(fileName, sizeof(fileName), I2C_DEV_HOME"-%d", i2cbus);
		if (len >= (int)sizeof(fileName))
		{
			EXT_ERRORF("%s: path truncated", fileName);
			return EXIT_FAILURE;
		}
		fd = open(fileName, O_RDWR);
	}

	if (fd < 0 && !quiet)
	{
		if (errno == ENOENT)
		{
			EXT_ERRORF("Could not open file `"I2C_DEV_HOME"-%d' or `"I2C_DEV_HOME"/%d': %s", i2cbus, i2cbus, strerror(ENOENT));
		}
		else
		{
			if (errno == EACCES)
			{
				EXT_ERRORF("Run as root?");
			}
			else
			{
				EXT_ERRORF("Could not open file `%s': %s\n", fileName, strerror(errno));
			}
		}
	}

	return fd;
}

/* offset can be 1, 2 or 3 bytes: EEPROM is 2 bytes offset */
int cmnSysI2cRW(int bus, unsigned char slaveAddr, unsigned int regAddr, int regAddrSize, unsigned char *buffer, int length, int isRead)
{
	int ret = EXIT_SUCCESS;
	int fd, i = 0, _len = 0;
	int addIndex = 0;
	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg	msgs[2];
	unsigned char data[2][64];


	MUX_DEBUG_CTRL("I2C %s on: Bus:%d; Slave:0x%x; Reg:0x%x,size:%d; BufSize:%d", (isRead)?"read":"write", bus, slaveAddr, regAddr, regAddrSize, length);
	fd = cmnSysI2cOpen(bus, EXT_FALSE);
	if(fd <= 0)
	{
		return EXIT_FAILURE;
	}

	if(_sysI2cSetSlaveAddr(fd, slaveAddr, EXT_TRUE) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	
	if(_sysI2cCheckFuncs(fd, 0, 0) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	/* address of register, for both read and write */
	if(regAddrSize >= 1)
	{
		data[i][addIndex++] = ((regAddr)&0xFF);
		if( regAddrSize >= 2)
		{
			data[i][addIndex++] = ((regAddr>>8)&0xFF);

			if( regAddrSize >= 3)
			{
				data[i][addIndex++] = ((regAddr>>16)&0xFF);
			}
		}
	}

	if(addIndex != regAddrSize)
	{
		EXT_ERRORF("Set length offset address as %d, is not %d", addIndex, regAddrSize);
		return EXIT_FAILURE;
	}
	
	if(isRead)
	{/* read:  */
		_len = addIndex;
//		memcpy(data[i+1], buffer, (length<sizeof(data[i]))?length: (sizeof(data[0])));
	}
	else
	{/* write */
		_len = length + addIndex;
		if(length >0 )
		{/* buffer can be null when writing */
			memcpy(&data[i][addIndex], buffer, (_len <sizeof(data[i]))?length: (sizeof(data[0])-addIndex) );
		}
	}
	
	msgs[i].addr = slaveAddr;
	msgs[i].len = _len;
	msgs[i].flags = 0;
	msgs[i].buf = data[i];

	if(isRead)
	{
		i++;
		msgs[i].addr = slaveAddr;
		msgs[i].len = length;
		msgs[i].flags = I2C_M_RD;
		msgs[i].buf = data[i];
	}
	
	i++;
	rdwr.nmsgs = i;
	rdwr.msgs = msgs;

#if ARCH_ARM
	ret = ioctl(fd, I2C_RDWR, &rdwr);
#else
	ret = write(fd, (void *)&rdwr, sizeof(struct i2c_rdwr_ioctl_data));
	if(isRead)
	{
		int _value = rand();
		memcpy(data[1], &_value, sizeof(int));
	}
#endif
	if (ret < 0)
	{
		MUX_ERROR("I2C %s failed at Salve Address %02x, offset %d, data length %d: %m", (isRead)?"read":"write", slaveAddr, regAddr, length);
		ret = EXIT_FAILURE;
	}
	else
	{
		if(isRead)
		{
			memcpy(buffer, data[1], length);
		}
		ret = EXIT_SUCCESS;
	}
	
	close(fd);

	return ret;
}

int cmnSysI2cExtRW(int bus, unsigned char chanNo, unsigned char slaveAddr, unsigned int regAddr, int regAddrSize, unsigned char *buffer, int length, int isRead)
{
	if(cmnSysI2cRW(bus, EXT_I2C_PCA9554_ADDRESS, (unsigned int)chanNo, 1, NULL, 0, EXT_FALSE))
	{/* channo as first byte in buffer of msg while no data is writen */
		EXT_ERRORF("I2C Mulplexer on channel %d at bus %d failed", chanNo, bus);
		return EXIT_FAILURE;
	}

	return cmnSysI2cRW(bus, slaveAddr, regAddr, regAddrSize, buffer, length, isRead);
}


int cmnSysI2cSensor(void)
{
	unsigned short _shValue;
	unsigned int offset = MUX_PCT2075_REG_TOS;
	int ret;

	EXT_INFOF("Test Sensor...");
	I2C_EXT_READ_SHORT(0, I2C_CHAN_4_SENSOR, EXT_I2C_DEV_SENSOR, offset, 1, &_shValue, ret);
	if(ret == EXIT_FAILURE)
	{
		EXT_ERRORF("\tI2C read offset 0x%x from sensor", offset);
		return EXIT_FAILURE;
	}

	if(_shValue == MUX_PCT2075_TOS_DEFAULT)
	{
		EXT_INFOF("\tTos of sensor is OK: 0x%04x", _shValue);
	}
	else
	{
		EXT_ERRORF("\tRead Tos of Sensor failed: 0x%04x", _shValue);
	}

	offset = MUX_PCT2075_REG_TEMP;
	I2C_EXT_READ_SHORT(0, I2C_CHAN_4_SENSOR, EXT_I2C_DEV_SENSOR, offset, 1, &_shValue, ret);
	if(ret == EXIT_FAILURE)
	{
		EXT_ERRORF("\tI2C read offset 0x%x from sensor", offset);
		return EXIT_FAILURE;
	}

	EXT_INFOF("\tTemperature of sensor is %f degree(0x%04x)", (_shValue>>5)*0.125, _shValue);

	return EXIT_SUCCESS;
}

#define	EEPROM_24LC32A_PAGE_SIZE						(32)		/* page of 32 byte for whole 4KB (32kbit) */

#define	EXT_I2C_EEPROM_PAGE_SIZE						EEPROM_24LC32A_PAGE_SIZE

static const uint8_t test_data_tx[] =
{
	/** SAM TWI EEPROM EXAMPLE */
	'M', 'u', 'x', 'L', 'a', 'b', '-', 'A',
	'N', '7', '6', '7', 'M', '-','O', 'K',
	'P', 'a', 'g', 'e', '-', 'N', 'o', ',',
	'2'
};

#define TEST_DATA_LENGTH  (sizeof(test_data_tx) / sizeof(uint8_t))

/* startAddress is at page border*/
int extEepromWrite(unsigned int startAddress, unsigned char *value, unsigned int size)
{
	unsigned int i;
	unsigned int pages = (size+EXT_I2C_EEPROM_PAGE_SIZE-1)/EXT_I2C_EEPROM_PAGE_SIZE;
	unsigned char left = EXT_I2C_EEPROM_PAGE_SIZE;
	unsigned int start = 0;
	int ret = EXIT_FAILURE;

	for(i=0; i< pages; i++)
	{/* the offset address is send together, don't send it independently */
		left = ((size - start)>EXT_I2C_EEPROM_PAGE_SIZE)?EXT_I2C_EEPROM_PAGE_SIZE:(size - start);
		if(i>0)
		{
			EXT_DELAY_US(1500);
		}
#if 1
		EXT_DEBUGF(EXT_DBG_ON, "\tEEPROM Write No.%d page at 0x%x, length %d", i, start, left );
#endif
		ret = EEPROM_WRITE(startAddress+start, value+start, left);
		start += EXT_I2C_EEPROM_PAGE_SIZE;
	}
	
#if 1//def BSP_BIST_DEBUG
	EXT_DEBUGF(EXT_DBG_ON, "\tEEPROM write at 0x%08X:", startAddress);
	CMN_HEX_DUMP(value, size, "\tEEPROM write");
#endif

	return ret;
}


int i2cTestEeprom(void)
{
	uint8_t datarx[TEST_DATA_LENGTH];
	unsigned int address = 0;
	unsigned int size = TEST_DATA_LENGTH;
	unsigned int i;

	//	extEepromRead(0, datarx, TEST_DATA_LENGTH);
	
	EXT_INFOF("EEPROM test...");
	/* write and readback 25 bytes, acrossing 2 pages in EEPROM */
	extEepromWrite(address,(unsigned char *) test_data_tx, size);

	EXT_DELAY_US(2500);/* if no delay, read will failed with "Remote I/O error" */


	EEPROM_READ(address, datarx, size);
#if 1// def BSP_BIST_DEBUG
	EXT_DEBUGF(EXT_DBG_ON, "\tEEPROM Read at 0x%08X:"EXT_NEW_LINE, address);
	CMN_HEX_DUMP(datarx, size, "\tEEPROM Read");
#endif
	
	for(i=0; i< size; i++)
	{
		if(datarx[i] != test_data_tx[i])
		{
			EXT_ERRORF("\tNo.%d bytes is different, %x!=%x", (int)i, datarx[i],  test_data_tx[i]);
			return EXIT_FAILURE;
		}
	}

	EXT_INFOF("\tEEPROM write and read test OK!");
	return EXIT_SUCCESS;
}


/* registers configued in bootstrap */
typedef struct Si5351RegCfg
{
	unsigned char		addr;	/* 8 bit address. page 17. only 8 bit register address for Si5351B. JL. 06.14, 2019 */
	unsigned char		val;
}Si5351RegCfg;

#if	(MUX_BOARD == MUX_ATMEL_XPLAINED)

#elif	(MUX_BOARD == MUX_BOARD_768)

Si5351RegCfg const _si5351bRegCfgs[] =
{
	{ 15,0x00},
	{ 16,0x0F},
	{ 17,0x0F},
	{ 18,0x0F},
	{ 19,0x0F},
	{ 20,0x0F},
	{ 21,0x4F},
	{ 22,0x2F},
	{ 23,0x0F},
	{ 24,0x00},
	{ 25,0x00},
	{ 26,0x0C},
	{ 27,0x35},
	{ 28,0x00},
	{ 29,0x0E},
	{ 30,0x38},
	{ 31,0x00},
	{ 32,0x04},
	{ 33,0x68},
	{ 34,0x31},
	{ 35,0x2D},
	{ 36,0x00},
	{ 37,0x0F},
	{ 38,0xD1},
	{ 39,0x11},
	{ 40,0x18},
	{ 41,0xC3},
	{ 42,0xEC},
	{ 43,0x85},
	{ 44,0x00},
	{ 45,0x0C},
	{ 46,0x29},
	{ 47,0xA0},
	{ 48,0xD8},
	{ 49,0xB3},
	{ 50,0x00},
	{ 51,0x7D},
	{ 52,0x00},
	{ 53,0x0E},
	{ 54,0xE5},
	{ 55,0x00},
	{ 56,0x00},
	{ 57,0x2F},
	{ 58,0x01},
	{ 59,0x77},
	{ 60,0x00},
	{ 61,0x0D},
	{ 62,0x04},
	{ 63,0x00},
	{ 64,0x01},
	{ 65,0x24},
	{ 66,0x00},
	{ 67,0x7D},
	{ 68,0x00},
	{ 69,0x0E},
	{ 70,0xE5},
	{ 71,0x00},
	{ 72,0x00},
	{ 73,0x2F},
	{ 74,0x0C},
	{ 75,0x35},
	{ 76,0x00},
	{ 77,0x0E},
	{ 78,0x38},
	{ 79,0x00},
	{ 80,0x04},
	{ 81,0x68},
	{ 82,0x00},
	{ 83,0x01},
	{ 84,0x00},
	{ 85,0x1F},
	{ 86,0x00},
	{ 87,0x00},
	{ 88,0x00},
	{ 89,0x00},
	{ 90,0x06},
	{ 91,0x42},
	{ 92,0x00},
	{149,0x00},
	{150,0x00},
	{151,0x00},
	{152,0x00},
	{153,0x00},
	{154,0x00},
	{155,0x00},
	{156,0x00},
	{157,0x00},
	{158,0x00},
	{159,0x00},
	{160,0x00},
	{161,0x00},
	{162,0x26},
	{163,0x58},
	{164,0x01},
	{165,0x00},
	{166,0x00},
	{167,0x00},
	{168,0x00},
	{169,0x00},
	{170,0x00},
	};

#elif (MUX_BOARD == MUX_BOARD_774)
Si5351RegCfg const _si5351bRegCfgs[] =
{
	{ 0x02, 0x13 },
//	{ 0x03, 0x00 },
	{ 0x07, 0x01 },
	{ 0x0F, 0x00 },
	{ 0x10, 0x8C },
	{ 0x11, 0x8C },
	{ 0x12, 0x0F },
	{ 0x0013, 0x0F },
	{ 0x0014, 0x0F },
	{ 0x0015, 0x0F },
	{ 0x0016, 0x0F },
	{ 0x0017, 0x2F },
	{ 0x001A, 0x00 },
	{ 0x001B, 0x19 },
	{ 0x001C, 0x00 },
	{ 0x001D, 0x0F },
	{ 0x001E, 0xD1 },
	{ 0x001F, 0x00 },
	{ 0x0020, 0x00 },
	{ 0x0021, 0x17 },
	{ 0x0022, 0x00 },
	{ 0x0023, 0x01 },
	{ 0x0024, 0x00 },
	{ 0x0025, 0x10 },
	{ 0x0026, 0x00 },
	{ 0x0027, 0x00 },
	{ 0x0028, 0x00 },
	{ 0x0029, 0x00 },
	{ 0x003A, 0x00 },
	{ 0x003B, 0x01 },
	{ 0x003C, 0x00 },
	{ 0x003D, 0x0E },
	{ 0x003E, 0x80 },
	{ 0x003F, 0x00 },
	{ 0x0040, 0x00 },
	{ 0x0041, 0x00 },
	{ 0x0042, 0x00 },
	{ 0x0043, 0x08 },
	{ 0x0044, 0x00 },
	{ 0x0045, 0x10 },
	{ 0x0046, 0x90 },
	{ 0x0047, 0x00 },
	{ 0x0048, 0x00 },
	{ 0x0049, 0x00 },
	{ 0x004A, 0x00 },
	{ 0x004B, 0x19 },
	{ 0x004C, 0x00 },
	{ 0x004D, 0x0F },
	{ 0x004E, 0xD1 },
	{ 0x004F, 0x00 },
	{ 0x0050, 0x00 },
	{ 0x0051, 0x17 },
	{ 0x0052, 0x02 },
	{ 0x0053, 0x00 },
	{ 0x0054, 0x00 },
	{ 0x0055, 0x22 },
	{ 0x0056, 0x41 },
	{ 0x0057, 0x00 },
	{ 0x0058, 0x00 },
	{ 0x0059, 0x80 },
	{ 0x005A, 0x06 },
	{ 0x005B, 0x12 },
	{ 0x0095, 0x00 },
	{ 0x0096, 0x00 },
	{ 0x0097, 0x00 },
	{ 0x0098, 0x00 },
	{ 0x0099, 0x00 },
	{ 0x009A, 0x00 },
	{ 0x009B, 0x00 },
	{ 0x00A2, 0x00 },
	{ 0x00A3, 0x00 },
	{ 0x00A4, 0x00 },
	{ 0x00B7, 0x92 },
};

#elif (MUX_BOARD == MUX_BOARD_767)
#else
#error 	Not support board definition
#endif


#define SI5351_REG_MAX  (sizeof(_si5351bRegCfgs) / sizeof(Si5351RegCfg))

int	cmnSysI2cTestSi5351B(void)
{
	int i;
	unsigned int regAddr;
	unsigned char value;

	EXT_INFOF(EXT_NEW_LINE"Si5351B test..." );
	for(i=0; i< SI5351_REG_MAX; i++ )
	{
		regAddr = (unsigned int)_si5351bRegCfgs[i].addr;
		
		if(SI5351B_READ(regAddr, &value, 1) == EXIT_FAILURE)
		{
			EXT_ERRORF("SI5351B read register #%d at address 0x%X failed", i, regAddr);
			return EXIT_FAILURE;
		}
		
		if(value != _si5351bRegCfgs[i].val)
		{
			EXT_ERRORF("Register#%d should be 0x%x, but is 0x%x", i, _si5351bRegCfgs[i].val, value);
			return EXIT_FAILURE;
		}
		printf(".");
	}
	printf(EXT_NEW_LINE);

	EXT_INFOF("\tSi5351B test OK: %d registers have been tested"EXT_NEW_LINE, i );
	
	return EXIT_SUCCESS;
}


int	cmnSysI2cTestEp91a6(void)
{
	unsigned int ID=0xA3947A17;	/* */
	unsigned int readId;
	int ret;

	EXT_INFOF("");
	EXT_INFOF("\tCheck EP91A6SQ...");
	ret = EP91A6_READ(0, &readId, sizeof(int));
	if(ret == EXIT_FAILURE)
	{
		EXT_ERRORF("EP91A6SQ read failed");
		return EXIT_FAILURE;
	}

	EXT_INFOF("\t\tEP91A6SQ check %s: ID is 0x%8X", (readId==ID)?"OK":"Failed", readId);
	
	return EXIT_SUCCESS;
}


int	cmnSysI2cTestMcp4716(void)
{
#define	MCP4617_CMD_RESET		0x06
#define	MCP4617_CMD_WAKEUP		0x09

	unsigned int regAddr;
	unsigned char value;
	unsigned char data[32];
	int ret;

	EXT_INFOF("MCP4716 test...");
	regAddr = 0x00;
	value = MCP4617_CMD_WAKEUP;
	ret = MCP4716_WRITE(regAddr, &value, 1);
	if(ret == EXIT_FAILURE)
	{
		EXT_ERRORF("MCP4716 wakeup failed");
		return EXIT_FAILURE;
	}

	EXT_INFOF("\tMCP4716 wakeup OK!");

	ret = MCP4716_READ(regAddr, data, sizeof(data));
	if(ret == EXIT_FAILURE)
	{
		EXT_ERRORF("\tMCP4716 read data failed");
		return EXIT_FAILURE;
	}

	EXT_DEBUGF(EXT_DBG_ON, "\tRead %d bytes from 0x%08X:", (int)sizeof(data), regAddr);
	CMN_HEX_DUMP(data, sizeof(data), "\tMCP4716 read");

	EXT_INFOF("\tMCP4716 test OK");
	
	return EXIT_SUCCESS;
}


int	cmnSysI2cTest(void)
{
	unsigned char audioRate, audioChannel, audioDepth;
	cmnSysI2cSensor();

	i2cTestEeprom();
	
	cmnSysI2cTestSi5351B();

	cmnSysI2cTestMcp4716();

	cmnSysI2cTestEp91a6();

	EXT_DEBUGF(EXT_DBG_ON, "\tRead audio parameters from HDMI repeater....");
	cmnSysI2cTxReadAudioParams(&audioRate, &audioChannel, &audioDepth );

	EXT_DEBUGF(EXT_DBG_ON, "\t\tTest SI5351B registers....");
	cmnSysI2cSi5351VcxoPllCheck();
	return EXIT_SUCCESS;
}


