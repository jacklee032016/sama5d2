
#include <common.h>
#include <atmel_lcd.h>
#include <debug_uart.h>
#include <dm.h>
#include <i2c.h>
#include <version.h>
#include <video.h>
#ifdef CONFIG_DM_VIDEO
#include <video_console.h>
#endif
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/atmel_sdhci.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama5d2.h>

#include <../drivers/mux/rtl8307h/rtk_api.h>
#include <../drivers/mux/rtl8307h/rtk_api_ext.h>
#include <../drivers/mux/rtl8307h/rtl8307h_types.h> 

#include "mux7xxCompact.h"


#define I2C_MUX_PCA_ADDR	(0xE0>>1)

#define	EXT_TOSTRING(s)						#s

#define	DEBUG_LINK_PORT(port, portAbility ) \
	EXT_INFOF("\tPort-%d: %s %s;\t%s-duplex; AutoNego:%s; TxPause:%s; RxPause:%s", (port),	\
		((portAbility).link==PORT_LINKUP)?"Up":"Down", ((portAbility).speed==PORT_SPEED_1000M)?"1000M":((portAbility).speed==PORT_SPEED_100M)?"100M":"None", \
		((portAbility).duplex==PORT_FULL_DUPLEX)?"full":"half", ((portAbility).nway)?"YES":"No", ((portAbility).txpause)?"YES":"No", ((portAbility).rxpause)?"YES":"No" )

static struct udevice *_i2cBus0 = NULL;

static struct udevice	*_pcaMultiplexer = NULL;
static struct udevice	*_i2cSwitch = NULL;

static struct udevice *_extSelectI2cDevice(uint busNo, uint devAddr)
{
	struct udevice *dev;
	int err;

	if(_i2cBus0 == NULL )	
	{
		err = uclass_get_device_by_seq(UCLASS_I2C, busNo, &_i2cBus0);
		if(_i2cBus0 == NULL || err )
		{
			EXT_ERRORF("I2C Error: Check I2C bus %d configuration: %d.\n", busNo, err /* errno_str(err)*/);
			return NULL;
		}
		
		EXT_DEBUGF(RTL8307_DEBUG, "I2C bus-%d: %s found", busNo, _i2cBus0->name);
	}
	
	dm_i2c_probe(_i2cBus0, devAddr, 0, &dev);
	if(dev == NULL || err )
	{
		EXT_ERRORF("I2C Error: Can't found device on 0x%x of bus %s: %d.\n", devAddr, _i2cBus0->name, err/* errno_str(err)*/);
	}

	return dev;
}


static struct udevice *_selectMultiplexerChan(uint busNo, u8 ch, uint devAddr)
{
	struct udevice	*dev = NULL;
#if 1
	u8 addr, data[8];
	int err;

	if(_pcaMultiplexer == NULL )	
	{
		_pcaMultiplexer = _extSelectI2cDevice(busNo, I2C_MUX_PCA_ADDR);
		if(_pcaMultiplexer == NULL)
		{
			EXT_ERRORF("I2C Error: Can't found PCA954 device on bus %d.\n", busNo);
			return NULL;
		}
		EXT_DEBUGF(RTL8307_DEBUG, "I2C dev-%x: %s found", I2C_MUX_PCA_ADDR, _pcaMultiplexer->name);
	}
	
	
	/* Selecting proper channel via PCA*/
	if(ch == 0xFF )
	{
		data[0] = 0x0;
		addr = 0x0;
	}
	else
	{
		data[0] = ch+4;
		addr = ch+4;
	}
	
	EXT_DEBUGF(RTL8307_DEBUG, "I2C bus-%d: write device %s channel#%x"EXT_NEW_LINE, busNo, _pcaMultiplexer->name, addr);
	err = dm_i2c_write(_pcaMultiplexer, addr, data, 0);
	if (err)
	{
		EXT_ERRORF("I2C Error: PCA failed to select proper channel");
		return NULL;
	}

	dev = _extSelectI2cDevice(busNo, devAddr);
	if(dev == NULL)
	{
		EXT_ERRORF("I2C Error: Device#0x%x is not foundl", devAddr );
		return NULL;
	}
	EXT_DEBUGF(RTL8307_DEBUG, "I2C bus-%d: write device %s address#%x"EXT_NEW_LINE, busNo, dev->name, devAddr );

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
	
	dm_i2c_probe(bus, I2C_MUX_PCA_ADDR, 0, &dev);
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

	return dev;
}

#define	_i2C_RW__WITH_FUNCTION		1/* otherwise read/write with i2c message */

/* only one byte register address can be supported for i2c_read and i2c_write */
int extI2CRead(unsigned char chanNo, unsigned char deviceAddress, unsigned int regAddress, int addrSize, unsigned char *val, int valSize)
{
	struct udevice	*dev = NULL;
	int err;

	dev = _selectMultiplexerChan(0, chanNo, deviceAddress);
	if(dev == NULL)
	{
		return -ENOSYS;
	}

#if _i2C_RW__WITH_FUNCTION	
	err = dm_i2c_read(dev, regAddress, val, valSize);
	if(err != 0)
	{
		EXT_ERRORF("I2C read channel %d, device %x, regAdd:%x failed: %d", chanNo, deviceAddress, regAddress, err);
	}
#else
	/* read with i2C message */
	struct i2c_msg msg[2];
	u8 _addr[3], _data[32];
	int index=0, i = 0;

	if(addrSize > 0)
	{
		_addr[index++] = regAddress>>0;
		if(addrSize > 1)
		{
			_addr[index++] = regAddress>>8;
			if(addrSize > 2)
			{
				_addr[index++] = regAddress>>16;
			}
		}
	}

	if(addrSize > 0)
	{
		msg[i].addr  = deviceAddress;
		msg[i].flags = 0;
		msg[i].len   = index;
		msg[i].buf   = _addr;
		i++;
	}
	
	msg[i].addr  = deviceAddress;
	msg[i].flags = I2C_M_STOP | I2C_M_RD;
	msg[i].len   = valSize;
	msg[i].buf   = _data;
	i++;

	err = dm_i2c_xfer(dev, msg, i);
	if(err)
	{
		EXT_ERRORF( "RTL read failed: %d", err);
		return RT_ERR_NO_ACK;
	}

	memcpy(val, _data, valSize);
#endif
	return err;
}

char extI2CWrite(unsigned char chanNo, unsigned char deviceAddress, unsigned int regAddress, int regSize, unsigned char *regVal, int valSize)
{
	struct udevice	*dev = NULL;
	int err;

	dev = _selectMultiplexerChan(0, chanNo, deviceAddress);
	if(dev == NULL)
	{
		return -ENOSYS;
	}
#if _i2C_RW__WITH_FUNCTION
	err = dm_i2c_write(dev, regAddress, regVal, valSize);
	if(err != 0)
	{
		EXT_ERRORF("I2C write channel %d, device %x, regAdd:%x failed: %d", chanNo, deviceAddress, regAddress, err);
	}
#else
	/* write with i2C message */
	uint8_t _data[32];
	struct i2c_msg msg;
	int index = 0, i;

	if(regSize > 0)
	{
		_data[index++] = regAddress>>0;
		if(regSize > 1)
		{
			_data[index++] = regAddress>>8;
			if(regSize > 2 )
			{
				_data[index++] = regAddress>>16;
			}
		}
	}

	if(index+valSize > sizeof(_data) )
	{
		valSize = sizeof(_data) - index;
	}

	for(i=0; i< valSize; i++)
	{
		_data[index++] = *(regVal+i);
	}

	msg.addr = deviceAddress;
	msg.flags = 0;//I2C_M_STOP;	/* default flags is write */
	msg.len = index;
	msg.buf = _data;
	err = dm_i2c_xfer(dev, &msg, 1);
	if(err)
	{
		EXT_ERRORF("I2C write failed: %d", err);
		return RT_ERR_NO_ACK;
	}

#endif

	return err;

}

#if RTL8307_DEBUG
static void _printPortAbility(rtk_port_link_ability_t *ability)
{
	switch(ability->speed)
	{
		case PORT_SPEED_10M:
			printf("\t10M\t");
			break;
		case PORT_SPEED_100M:
			printf("\t100M\t");
			break;
		case PORT_SPEED_1000M:
			printf("\t1000M\t");
			break;
		default:
			printf("Invalidate Speed\t");
			break;
	}
	
	switch(ability->duplex)
	{
		case PORT_HALF_DUPLEX:
			printf("HALF_DUPLEX\t");
			break;
		case PORT_FULL_DUPLEX:
			printf("FULL_DUPLEX\t");
			break;
		default:
			printf("Invalidate Duplex\t");
			break;
	}
	
	switch(ability->link)
	{
		case PORT_LINKDOWN:
			printf("LINK_DOWN\t");
			break;
		case PORT_LINKUP:
			printf("LINK_UP\t");
			break;
		default:
			printf("Invalidate Link Status\t");
			break;
	}  

	printf("NWAY=%i\r\n\r\n", ability->nway);
}

static void _printPortState(rtk_port_t port)
{
	rtk_port_link_status_t	portStatus = {0};
	rtk_mode_ext_t mode;
	int ret;

	printf("Port#%d\r\n", port);
	
	ret = rtk_port_linkStatus_get(port, &portStatus);
	if(ret != RT_ERR_OK)
	{
		EXT_ERRORF("port %d status ERROR, return : %s, \t", port, rtk_errMsg_get(ret));
		return;
	}

	if(portStatus.link==PORT_LINKUP)
	{
		printf("\tSpeed:");
		switch(portStatus.speed)
		{
			case  PORT_SPEED_1000M:
				printf("1000M\r\n");
				break;
			case  PORT_SPEED_100M:
				printf("100M\r\n");
				break;
			case  PORT_SPEED_10M:
				printf("10M\r\n");
				break;
			default:
				printf("Unknown\r\n");
				break;
		}

		printf("\tDuplex:%s\r\n", (portStatus.duplex == PORT_FULL_DUPLEX)?"Full":"Half");
		printf("\tPause:%s\r\n", (portStatus.txpause == ENABLED )?"Enable":"Disable");
		printf("\tAsymetric Pause:%s\r\n", (portStatus.rxpause == ENABLED)?"Enable":"Disable");

	}
	else
	{
		printf("\tLink Down\r\n");
	}

	ret = rtk_port_mode_get(port, &mode);
	if(ret != RT_ERR_OK)
	{
		EXT_ERRORF("port %d mode ERROR, return : %s, \t", port, rtk_errMsg_get(ret));
		return;
	}

	printf("\t");
	switch(mode)
	{
		case MODE_EXT_RGMII_NODELAY:
			printf("MODE_EXT_RGMII_NODELAY");
			break;

		case MODE_EXT_RGMII_RXDELAY_ONLY:
			printf("MODE_EXT_RGMII_RXDELAY_ONLY");
			break;
		case MODE_EXT_RGMII_TXDELAY_ONLY:
			printf("MODE_EXT_RGMII_TXDELAY_ONLY");
			break;
		case MODE_EXT_RGMII_RXTXDELAY:
			printf("MODE_EXT_RGMII_RXTXDELAY");
			break;
		case MODE_EXT_TMII_MII_MAC:
			printf("MODE_EXT_TMII_MII_MAC");
			break;
		case MODE_EXT_MII_PHY:
			printf("MODE_EXT_MII_PHY");
			break;
		case MODE_EXT_TMII_PHY:
			printf("MODE_EXT_TMII_PHY");
			break;
		case MODE_EXT_RMII_INPUT:
			printf("MODE_EXT_RMII_INPUT");
			break;
		case MODE_EXT_RMII_OUTPUT:
			printf("MODE_EXT_RMII_OUTPUT");
			break;
		case MODE_EXT_HEAC:
			printf("MODE_EXT_HEAC");
			break;
		case MODE_EXT_FAST_ETHERNET:
			printf("MODE_EXT_FAST_ETHERNET");
			break;
		case MODE_EXT_GIGA:
			printf("MODE_EXT_GIGA");
			break;
		default:
			printf("Unknown mode");
			break;
	}

	printf("\r\n");
}
#endif

int _switchCfgOnePort(int port)
{
	rtk_port_link_ability_t		_portAbility;
	int ret;

	ret = rtk_port_linkAbility_get(port, &_portAbility );
	if(ret != RT_ERR_OK)
	{
		EXT_ERRORF("Port#%d status failed: %s\n", port, rtk_errMsg_get(ret) );
		return -EINVAL;
	}

#if RTL8307_DEBUG
	_printPortState(port);
	_printPortAbility(&_portAbility);
#endif

	_portAbility.link   = PORT_LINKUP;
	ret = rtk_port_linkAbility_set(port, &_portAbility );
	if(ret != RT_ERR_OK)
	{
		EXT_ERRORF("setup Port#%d failed: %s\n",  port, rtk_errMsg_get(ret) );
//		printf("setup port 4 ERROR, return : %d\n", rtk_errMsg_get(ret));
		return -EINVAL;
	}
	
	ret = rtk_port_linkAbility_get(port, &_portAbility );
	if(ret != RT_ERR_OK)
	{
		EXT_ERRORF("Port#%d status failed: %s\n", port, rtk_errMsg_get(ret) );
		return -EINVAL;
	}
	
#if RTL8307_DEBUG
	_printPortState(port);
	_printPortAbility(&_portAbility);
#else
	DEBUG_LINK_PORT(port, _portAbility);
#endif

	return 0;
}


int _extSensorGetTemperatureCelsius(void)
{/* 11 bits: sign bit + 10 bit value */
	unsigned char regVal =0;
	unsigned short temp;

	unsigned int bus;
	bus = I2C_CHAN_4_SENSOR;

	/* test hardware */
	/* write pointer register for TOS: figure 12 in p18; and then read read TOS value. actually pointer register is the address  */
	regVal = 0x03;
	if(extI2CRead(bus, EXT_I2C_DEV_SENSOR, regVal, 1, (unsigned char *)&temp, 2) )
	{
		EXT_ERRORF("Sensor Tos read failed on #0x%x", EXT_I2C_DEV_SENSOR);
		return -1;
	}
	else
	{
		EXT_DEBUGF(EXT_DBG_ON, "\tSensor ToS: %x", temp);
	}
	
	temp = EXT_BYTE_ORDER_SHORT(temp);
	/* table 7, page 9 */
	if(temp == 0x5000 ||temp == 0x6E00 ||temp == 0xFB00 )
	{
		EXT_INFOF("\tSensor PCT2075 is OK. Tos: 0x%x", temp);
	}
	else
	{
		EXT_ERRORF("Sensor PCT2075 test failed. Tos: 0x%x", temp);
	}

	/* read temperature */
	/* write pointer register for temperature: figure 12 in p18.  */
	regVal = 0;
	if(extI2CRead(bus, EXT_I2C_DEV_SENSOR, regVal, 1, (unsigned char *)&temp, 2) )
	{
		EXT_ERRORF("Sensor Temp read failed on #0x%x", EXT_I2C_DEV_SENSOR);
		return -1;
	}
	else
	{
		EXT_DEBUGF(EXT_DBG_ON, "Sensor Temp: 0x%x", temp);
	}

	temp = (temp>>5);
	temp = EXT_BYTE_ORDER_SHORT(temp);
	EXT_INFOF("\tSensor Temp: %d*0.125;", temp );

	return 0;
}


int extSwitchSetup(void)
{
	int err;
#if RTL8307_DEBUG
	rtk_hec_mode_t	hecMode;
#endif

	EXT_INFOF(""BOARD_NAME " is initializing ..." );
#if	(MUX_BOARD == MUX_ATMEL_XPLAINED)
	return 0;
#endif
	EXT_INFOF("\tMUX Channels: switch:#%d; sensor:#%d;", I2C_CHAN_4_SWITCH, I2C_CHAN_4_SENSOR );

	_extSensorGetTemperatureCelsius();

	RTL8307H_I2C_init(); 

#if RTL8307_DEBUG
	err= _switchCfgOnePort(PN_PORT0);
	if(err < 0)
	{
		return err;
	}
	
	err = _switchCfgOnePort(PN_PORT1);
	if(err < 0)
	{
		return err;
	}

	err = _switchCfgOnePort(PN_PORT2);
	if(err < 0)
	{
		return err;
	}
	
	err = _switchCfgOnePort(PN_PORT3);
	if(err < 0)
	{
		return err;
	}

	err = _switchCfgOnePort(PN_PORT4);
	if(err < 0)
	{
		return err;
	}
#endif

	err = _switchCfgOnePort(PN_PORT5);
	if(err < 0)
	{
		return err;
	}
	
	err = _switchCfgOnePort(PN_PORT6);
	if(err < 0)
	{
		return err;
	}

	err = rtk_hec_mode_set(PN_PORT0, HEC_MODE_ENABLE);
	if(err != RT_ERR_OK)
	{
		EXT_ERRORF("Port#%d status failed: %s\n", PN_PORT0, rtk_errMsg_get(err) );
		return -ENODATA;
	}
#if RTL8307_DEBUG
	err = rtk_hec_mode_get(PN_PORT0, &hecMode);
	if(err != RT_ERR_OK)
	{
		EXT_ERRORF("Port#%d status failed: %s\n", PN_PORT0, rtk_errMsg_get(err));
		return -ENODATA;
	}
	EXT_INFOF("PN_PORT0 mode: %s", (hecMode)?"Enable":"Disable");
#endif

	err = rtk_hec_mode_set(PN_PORT1, HEC_MODE_ENABLE); 
	if(err != RT_ERR_OK)
	{
		EXT_ERRORF("Port#%d status failed: %s\n", PN_PORT1, rtk_errMsg_get(err) );
		return -ENODATA;
	}
	
#if RTL8307_DEBUG
	err = rtk_hec_mode_get(PN_PORT1, &hecMode);
	if(err != RT_ERR_OK)
	{
		EXT_ERRORF("Port#%d status failed: %s\n", PN_PORT1, rtk_errMsg_get(err) );
		return -ENODATA;
	}
	EXT_INFOF("PN_PORT1 mode: %s", (hecMode)?"Enable":"Disable");
#endif

#if 0	
	P6_ability.link   = PORT_LINKUP;
	P6_ability.speed  = PORT_SPEED_100M;// | PORT_SPEED_10M;
	P6_ability.duplex    = PORT_FULL_DUPLEX;// | PORT_HALF_DUPLEX;
	P6_ability.nway = 1; /* disable auto-negotiation */
	ret = rtk_port_linkAbility_set(PN_PORT6, &P6_ability );
	if(ret != RT_ERR_OK)
	{
		printf("setup port 6 ERROR, return : %d\n", ret);
//		printf("setup port 6 ERROR, return : %s\n", rtk_errMsg_get(ret));
	}
	printf("setup port 6, return : %d\n", ret);
#endif

//	EXT_INFOF(("switch configuration ended!\n"));

	return 0;
}

void RTL8307H_I2C_init(void)
{ 
	if(_i2cSwitch == NULL)
	{
		_i2cSwitch = _selectMultiplexerChan(0, I2C_CHAN_4_SWITCH, EXT_I2C_ADDRESS_RTL8035);
	}
}

/*3 bytes address of RTL driver into I2C address in 4 bytes */
#if 1
#define	RTL_ADDR(i2cAddr, rtlAddr)	\
	{	(i2cAddr) = 0; \
		(i2cAddr) = ( ((rtlAddr)&&0x000000ffUL) | \
			 ((rtlAddr)&& 0x0000ff00UL)|	\
			 ((rtlAddr)&& 0x00ff0000UL)); }

#else
#define	RTL_ADDR(i2cAddr, rtlAddr)	\
	{	(i2cAddr) = 0; \
		(i2cAddr) = ( ( ((rtlAddr)&&0x000000ffUL)<<16) | \
			 (((rtlAddr)&& 0x0000ff00UL)<<8)|	\
			 (((rtlAddr)&& 0x00ff0000UL)>>8) ); (rtlAddr) = ((rtlAddr) >>8);}
#endif

/* Function Name:
 *      RTL8307H_I2C_READ
 * Description:
 *      Read the data with address switch_addr. 
 *      The MSB of *reg_val (i.e. (*reg_val >> 31) & 0x1) corresponds to bits 31:31
 *      in the register datasheet
 * Input:
 *      switch_addr               -  switch address
 * Output:     
 *      reg_val                   -  Register value
 * Return: 
 *      RT_ERR_OK  
 *      RT_ERR_NO_ACK
 * Note:
 *      None
 */
int32 RTL8307H_I2C_READ(uint32 switch_addr, uint32 *reg_val)
{    
	int err;

#if 0
	uint8_t data[4];
	/* dm_i2c_chip only support 1 byte offset_len (refer to dm_i2c_probe in i2c_uclass.c), so only 1 byte address can be used in dm_i2c_read and dm_i2c_write 
      * Jack Lee, 04.16, 2019
      */
	int addr = 0;
	RTL_ADDR(addr, switch_addr);
/*
	if( extI2CRead(I2C_CHAN_4_SWITCH, EXT_I2C_ADDRESS_RTL8035, (addr), (unsigned char *)&val, 4) != 0)
	{
		return RT_ERR_BUSYWAIT_TIMEOUT;
	}
*/	
	err = dm_i2c_read(_i2cSwitch, addr, (unsigned char *)data, 4);
	if(err != 0)
	{
		EXT_ERRORF(( "RTL read failed: %d", err));
	}

	for (int i = 0; i < 4; i++)
	{
		*reg_val = (data[i]<<(i*8)) | *reg_val;
	} 
	
	//*reg_val = EXT_HTONL(val);

	return RT_ERR_OK;
#else	
	struct i2c_msg msg[2];
	u8 _addr[3], _data[4];

	_addr[0] = switch_addr>>0;
	_addr[1] = switch_addr>>8;
	_addr[2] = switch_addr>>16;

	msg[0].addr  = EXT_I2C_ADDRESS_RTL8035;
	msg[0].flags = 0;
	msg[0].len   = 3; /* 3 bytes for rtl830x */
	msg[0].buf   = _addr;
	
	msg[1].addr  = EXT_I2C_ADDRESS_RTL8035;
	msg[1].flags = I2C_M_STOP | I2C_M_RD;
	msg[1].len   = 4;
	msg[1].buf   = _data;

	err = dm_i2c_xfer(_i2cSwitch, msg, 2);
	if(err)
	{
		EXT_ERRORF( "RTL read failed: %d", err);
		return RT_ERR_NO_ACK;
	}

	if (reg_val != NULL)
	{
		*reg_val = 0;
		for (int i = 0; i < 4; i++)
		{
			*reg_val = (_data[i]<<(i*8)) | *reg_val;
		} 
	}
#endif
	return RT_ERR_OK;
}

int32 RTL8307H_I2C_WRITE(uint32 switch_addr, uint32 reg_val)
{
	int err;

#if 1
#if 0
	int val = EXT_HTONL(reg_val);
	int addr = 0;
	RTL_ADDR(addr, switch_addr);
	
	data[0] = reg_val>>0;
	data[1] = reg_val>>8;
	data[2] = reg_val>>16;
	data[3] = reg_val>>24;

/*	
	if( extI2CWrite(I2C_CHAN_4_SWITCH, EXT_I2C_ADDRESS_RTL8035, (addr),(unsigned char *) &val, 4) != 0)
	{
		return RT_ERR_BUSYWAIT_TIMEOUT;
	}
*/
	err = dm_i2c_write(_i2cSwitch, addr, (unsigned char *)data, 4);
	if(err != 0)
	{
		EXT_ERRORF(( "RTL write failed: %d", err));
	}
#else
	uint8_t _data[7];
	struct i2c_msg msg;
	
	_data[0] = switch_addr>>0;
	_data[1] = switch_addr>>8;
	_data[2] = switch_addr>>16;
	_data[3] = reg_val>>0;
	_data[4] = reg_val>>8;
	_data[5] = reg_val>>16;
	_data[6] = reg_val>>24;


	msg.addr = EXT_I2C_ADDRESS_RTL8035;
	msg.flags = 0;//I2C_M_STOP;
	msg.len = 7;
	msg.buf = _data;
	err = dm_i2c_xfer(_i2cSwitch, &msg, 1);
	if(err)
	{
		EXT_ERRORF("RTL read failed: %d", err);
		return RT_ERR_NO_ACK;
	}

#endif

#else	
	struct udevice *dev;
	u8 	i2c_addr, i2c_data[8];
//	uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	dm_i2c_probe(_i2cBus0, (EXT_I2C_ADDRESS_RTL8035), 0, &dev);		// AT24MAC_ADDR
	if(dev == NULL)
	{
		EXT_ERRORF(("Can found switch " ));
		return -ENOSYS;
	}

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

	return RT_ERR_OK;
}

static int _extDdrMemoryTestOneArea(unsigned long start, unsigned long end )
{
	unsigned long dram_start;
	unsigned long dram_end;
	unsigned long dram_point;
	int test_res;

	dram_point = 0;
	dram_start = start;
	dram_end = end;

	printf("Test memory zone from %lx -- %lx....." EXT_NEW_LINE, start, end);

	printf("\tWriting....." EXT_NEW_LINE"\t");
	for(dram_point = dram_start; dram_point < dram_end; dram_point = dram_point + sizeof(unsigned long))
	{
//		*(volatile unsigned long*)dram_point = dram_point^TEST_SYS_SDRAM_PTRN;
		*(volatile unsigned long*)dram_point = dram_point;
		if(dram_point%0x100000 == 0)
		{
			printf(".");
		}
	}
	printf(EXT_NEW_LINE);
	

	printf("\tChecking....." EXT_NEW_LINE"\t");
	for(dram_point = dram_start; dram_point < dram_end; dram_point = dram_point + sizeof(unsigned long))
	{
//		if(((*(volatile unsigned long*)dram_point)^TEST_SYS_SDRAM_PTRN) != dram_point)
		if(((*(volatile unsigned long*)dram_point)) != dram_point)
		{
//			printf("DDR Memory Test Failed at addr %x data: %x vs %x\n", dram_point, *(volatile unsigned long*)dram_point, dram_start^TEST_SYS_SDRAM_PTRN);
			printf("DDR Memory Test Failed at addr %lx data: %lx vs %lx\n", dram_point, *(volatile unsigned long*)dram_point, dram_start);
			printf("next: %lx %lx %lx %lx %lx %lx %lx %lx\n", 
				*(volatile unsigned long*)(dram_point+sizeof(unsigned long)), *(volatile unsigned long*)(dram_point+2*sizeof(unsigned long)), 
				*(volatile unsigned long*)(dram_point+3*sizeof(unsigned long)), *(volatile unsigned long*)(dram_point+4*sizeof(unsigned long)),
				*(volatile unsigned long*)(dram_point+5*sizeof(unsigned long)), *(volatile unsigned long*)(dram_point+6*sizeof(unsigned long)),
				*(volatile unsigned long*)(dram_point+7*sizeof(unsigned long)), *(volatile unsigned long*)(dram_point+8*sizeof(unsigned long)));
			test_res = 1;
			break;
		}
		if(dram_point%0x100000 == 0)
		{
			printf("*");
		}
	}
	printf(EXT_NEW_LINE);

	if(test_res == 0)
	{
		printf("DDR Memory Test from %lx to %lx Passed.\n", dram_start, dram_end);
	}

	return 0;	
}


int extDdrMemoryTest(void)
{
	int _res;

#define TEST_SYS_SDRAM_SIZE		0x10000000	/* 256M */


#define	MEM_SIZE_96M		0x6000000
#define	MEM_SIZE_112M		0x7000000

#define	MEM_SIZE_512M		(0x20000000 - 0x20) //0x1000000)
#define	MEM_SIZE_256M		(0x10000000) //0x1000000)
#define	MEM_SIZE_128M		(0x8000000) //0x1000000)
#if 1
	_res = _extDdrMemoryTestOneArea(CONFIG_SYS_SDRAM_BASE, CONFIG_SYS_SDRAM_BASE + MEM_SIZE_96M);
	
	_res += _extDdrMemoryTestOneArea(CONFIG_SYS_SDRAM_BASE + 0x8000000, CONFIG_SYS_SDRAM_BASE + 0x8000000 + MEM_SIZE_112M);//0xF000000);
#else
	_res += _extDdrMemoryTestOneArea(CONFIG_SYS_SDRAM_BASE, CONFIG_SYS_SDRAM_BASE + MEM_SIZE_128M);
#endif
	return _res;
}


