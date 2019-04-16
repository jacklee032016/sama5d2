#include <common.h>
#include <debug_uart.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/atmel_sdhci.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama5d2.h>

#include <../drivers/mux/muxlabHw.h> 

//#define I2C_MUX_PCA_ADDR	(0xE0)

static struct udevice *_extSelectI2cDevice(uint busNo, uint devAddr)
{
	struct udevice *bus, *dev;
	int err;
	
	err = uclass_get_device_by_seq(UCLASS_I2C, busNo, &bus);
	if(bus == NULL || err )
	{
		printf("PCA Error: Check I2C bus %d configuration: %d.\n", busNo, err /* errno_str(err)*/ );
		return NULL;
	}
	printf("I2C bus-%d: %s.\n", busNo, bus->name);
	
	dm_i2c_probe(bus, devAddr, 0, &dev);
	if(dev == NULL || err )
	{
		printf("PCA Error: Can't found device on 0x%x of bus %s: %d.\n", devAddr, bus->name, err/* errno_str(err)*/);
	}

	return dev;
}


static struct udevice *_selectMultiplexerChan(uint busNo, u8 ch)
{
	struct udevice *bus, *dev;
#if 1
	u8 addr, data[8];
	int err;

#if 0
	uclass_get_device_by_seq(UCLASS_I2C, busNo, &bus);
	if(bus == NULL)
	{
		printf("PCA Error: Check I2C bus %d configuration.\n", busNo );
		return -ENOSYS;
	}
	printf("I2C bus-%d: %s.\n", busNo, bus->name);
	
	dm_i2c_probe(bus, I2C_MUX_PCA_ADDR, 0, &dev);
#else

	dev = _extSelectI2cDevice(busNo, I2C_MUX_PCA_ADDR);
#endif
	if(dev == NULL)
	{
		printf("PCA Error: Can't found PCA954 device on bus %d.\n", busNo);
		return dev;
	}
	
	
	/* Selecting proper channel via PCA*/
	if(ch == 0xFF )
	{
		data[0] = 0x0;
		addr = 0x0;
	}
	else
	{
		data[0] = ch;
		addr = ch; // I2C_MUX_PCA_ADDR;
	}
	
	printf("I2C bus-%d: write device %s\n", busNo, dev->name );
	err = dm_i2c_write(dev, addr, data, 0);
	if (err)
	{
		printf("PCA: failed to select proper channel.\n");
		return NULL;
	}
#else
	struct i2c_msg msg[2];
	u8 i2c_addr[3], i2c_data[4];

	uclass_get_device_by_seq(UCLASS_I2C_MUX, 0, &bus);
	if(bus == NULL)
	{
		printf("Error: Check I2C muxplexer.\n");
		return -1;
	}
	printf("I2C Muxplexoer bus: %s.\n", bus->name);
	
	dm_i2c_probe(bus, I2C_MUX_PCA_ADDR, 0, &dev);		// AT24MAC_ADDR
	if(dev == NULL)
	{
		printf("PCA Error: Can't found PCA954 device.\n");
		return -1;
	}
	
	msg[0].addr  = 0x54;
	msg[0].flags = 0;
	msg[0].len   = 3;
	msg[0].buf   = i2c_addr;
	msg[1].addr  = 0x54;
	msg[1].flags = I2C_M_STOP | I2C_M_RD;
	msg[1].len   = 4;
	msg[1].buf   = i2c_data;
	dm_i2c_xfer(dev, msg, 2);
#endif

//	printf("return 0\n");
	return dev;
}

void extRtl830xInit(void)
{
	rtk_port_link_ability_t		P5_ability; /* FPGA */
	rtk_port_link_ability_t		P6_ability; /* MCU */

//	rtk_port_link_ability_t		heac0;
//	rtk_port_link_ability_t		heac1; 

	/* 744 and 768 is different for this channel */
	if(_selectMultiplexerChan(0, EXT_I2C_PCA9554_CS_SWITCH) == NULL)
	{
		printf("Error: RTL830X is not initialized now.\n");
		return;
	}

	RTL8307H_I2C_init(); 

	rtk_port_linkAbility_get(PN_PORT5, &P5_ability );
	rtk_port_linkAbility_get(PN_PORT6, &P6_ability );

	P5_ability.link   = PORT_LINKUP;
	rtk_port_linkAbility_set(PN_PORT5, &P5_ability );
	
	P6_ability.link   = PORT_LINKUP;
	//P6_ability.speed  = PORT_SPEED_10M;
	rtk_port_linkAbility_set(PN_PORT6, &P6_ability );
	
	rtk_hec_mode_set(PN_PORT0, HEC_MODE_ENABLE);
	rtk_hec_mode_set(PN_PORT1, HEC_MODE_ENABLE); 

	printf("Port-5: BW:%s; Duplex:%s; Status:%s; %d %d %d\n", 
		(P5_ability.speed==PORT_SPEED_1000M)?"1000M":(P5_ability.speed==PORT_SPEED_100M)?"100M":"None", 
		(P5_ability.duplex==PORT_FULL_DUPLEX)?"Full":"Half", (P5_ability.link==PORT_LINKUP)?"Up":"Down", P5_ability.nway, P5_ability.txpause, P5_ability.rxpause);
	printf("Port-6: BW:%s; Duplex:%s; Status:%s; %d %d %d\n", 
		(P6_ability.speed==PORT_SPEED_1000M)?"1000M":(P6_ability.speed==PORT_SPEED_100M)?"100M":"None", 
		(P6_ability.duplex==PORT_FULL_DUPLEX)?"Full":"Half", (P6_ability.link==PORT_LINKUP)?"Up":"Down", P6_ability.nway, P6_ability.txpause, P6_ability.rxpause);
//	printf("p6 : %d %d %d %d %d %d\n", 
//		P6_ability.speed, P6_ability.duplex, P6_ability.link, P6_ability.nway, P6_ability.txpause, P6_ability.rxpause);

}

int extI2CRead(uint busNo, uint8_t channo,  uint devAddr, uint regAddr, uint regSize, uint *regVal, uint valueSize)
{    
	struct udevice *bus, *dev;
	struct i2c_msg msg[2];
	u8 _regAddr[4], regData[8];
	int err;
	int ret;

	if(channo == EXT_I2C_PCA9554_CS_NONE )
	{
		dev = _extSelectI2cDevice(busNo, devAddr);
		if(err)
		{	
//		printf("Error: RTL830X is not initialized now.\n");
			return err;
		}
	}
	else
	{
		dev = _selectMultiplexerChan(busNo, channo);
	}
#if 0	
	dm_i2c_probe(bus, I2C_MUX_PCA_ADDR, 0, &dev);		// AT24MAC_ADDR
#else
#endif

	if(dev == NULL)
	{
		printf("Error: dev 0x%x is not found on channel %d bus 0x%x.\n", devAddr, busNo, channo);
		return -ENOSYS;
	}

	if(regSize >= 1)
	{
		_regAddr[0] = regAddr>>0;
		if(regSize >= 2)
		{
			_regAddr[1] = regAddr>>8;
		
			if(regSize >= 3)
			{
				_regAddr[2] = regAddr>>16;
				
				if(regSize >= 4)
				{
					_regAddr[3] = regAddr>>24;
				}
			}	
		}
	}

	msg[0].addr  = devAddr;
	msg[0].flags = 0;
	msg[0].len   = (regSize>4)?4:regSize;
	msg[0].buf   = _regAddr;
	
	msg[1].addr  = devAddr;
	msg[1].flags = I2C_M_STOP | I2C_M_RD;
	msg[1].len   = valueSize;
	msg[1].buf   = regData;
	
	dm_i2c_xfer(dev, msg, 2);

	if (regVal != NULL)
	{
		*regVal = 0;
		for (int i = 0; i < valueSize; i++)
		{
			*regVal = (regData[i]<<(i*8)) | *regVal;
		} 
	}

	return 0;
}

int extI2CWrite( uint switch_addr, uint reg_val)
{
#if 0
	struct udevice *bus, *dev;
	u8 	i2c_addr, i2c_data[8];
	int err;
	int ret;

	uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	
	dm_i2c_probe(bus, (RTL8305 | DEVICE_ID), 0, &dev);		// AT24MAC_ADDR

	i2c_addr = switch_addr>>0;
	i2c_data[0] = switch_addr>>0;
	i2c_data[1] = switch_addr>>8;
	i2c_data[2] = switch_addr>>16;

	i2c_data[3] = reg_val>>0;
	i2c_data[4] = reg_val>>8;
	i2c_data[5] = reg_val>>16;
	i2c_data[6] = reg_val>>24;

	err = dm_i2c_write(dev, i2c_addr, i2c_data, 7);
#endif

	return 0; 
}


short extSensorGetTemperatureCelsius(void)
{/* 11 bits: sign bit + 10 bit value */
	uint regVal =0;
	short temperature = 0;
    
	if(extI2CRead(0, EXT_I2C_PCA9554_CS_SENSOR, EXT_I2C_ADDRESS_SENSOR, EXT_I2C_SENSOR_LOCAL_TEMP_MSB, 1, &regVal, 1) )
		return -EINVAL;

	printf("LM95245 Temp(MSB): %d\n", regVal);
	temperature = (regVal<<8);
	if(extI2CRead(0, EXT_I2C_PCA9554_CS_SENSOR, EXT_I2C_ADDRESS_SENSOR, EXT_I2C_SENSOR_LOCAL_TEMP_LSB, 1, &regVal, 1) )
		return -EINVAL;

	/* only 3 bits in LSB register */
	temperature |=  (regVal>>5); //*0.125;// / 256.0;
      
	printf("LM95245 Temp(LSB): %d*0.125;\n", (regVal>>5) );
	return temperature;
}

