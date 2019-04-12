
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

//#include <../drivers/mux7xx/rtl8307h/rtk_api.h>
//#include <../drivers/mux7xx/rtl8307h/rtk_api_ext.h>
//#include <../drivers/mux7xx/rtl8307h/rtl8307h_types.h> 

#include <../drivers/mux/muxlabHw.h> 

#include "mux7xxCompact.h"


#define	CONFIG_SYS_I2C_SPEED	100000


#define I2C_MUX_PCA_ADDR	(0xE0>>1)

#define	EXT_TOSTRING(s)						#s

#define	DEBUG_LINK_PORT( portAbility ) \
	printf("\tPort-%s: %s %s : %s-duplex; AutoNego:%s; TxPause:%s; RxPause:%s\n", EXT_TOSTRING(portAbility),	\
		((portAbility).link==PORT_LINKUP)?"Up":"Down", ((portAbility).speed==PORT_SPEED_1000M)?"1000M":((portAbility).speed==PORT_SPEED_100M)?"100M":"None", \
		((portAbility).duplex==PORT_FULL_DUPLEX)?"full":"half", ((portAbility).nway)?"YES":"No", ((portAbility).txpause)?"YES":"No", ((portAbility).rxpause)?"YES":"No" )


#define	__RTK_CHECK_PORT_START(port, retVal, action)		\
		{if( (retVal) != RT_ERR_OK ){ EXT_ERRORF(("Port %d %s failed: %d", (port),(action), (retVal))); \
			return -1;	}else{/*printf("Port#%d %s OK!\r\n", (port), (action));*/}}

int select_i2c_ch_pca(u8 ch)
{
	struct udevice *bus, *dev;
	u8 addr, data[8];
	int ret;

	uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	if(bus == NULL)
	{
		EXT_ERRORF(("I2C-0 is not found"));
		return -ENOSYS;
	}
	
	dm_i2c_probe(bus, I2C_MUX_PCA_ADDR, 0, &dev); // AT24MAC_ADDR
	if(dev == NULL)
	{
		EXT_ERRORF(("I2C-0 device #0x%x is not found", I2C_MUX_PCA_ADDR));
		return -ENOSYS;
	}

	/* Selecting proper channel via PCA*/
	data[0] = 0x04 | ch;
	addr = 0x04 | ch; // I2C_MUX_PCA_ADDR;
	
	ret = dm_i2c_write(dev, addr, data, 0);
	//ret = i2c_write(I2C_MUX_PCA_ADDR, 0x0, 1, &ch, 1);
	if (ret)
	{
		printf("PCA: failed to select proper channel.\n");
		return ret;
	}

#if 0
	struct i2c_msg msg[2];
	u8 i2c_addr[3], i2c_data[4];

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

	return 0;
}

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
		EXT_ERRORF(("port %d status ERROR, return : %s, \t", port, rtk_errMsg_get(ret)));
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
		EXT_ERRORF(("port %d mode ERROR, return : %s, \t", port, rtk_errMsg_get(ret)));
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


int _switchCfgOnePort(int port)
{
	rtk_port_link_ability_t		_portAbility;
	int ret;

	ret = rtk_port_linkAbility_get(port, &_portAbility );
	if(ret != RT_ERR_OK)
	{
		EXT_ERRORF(("Port#%d status failed: %s\n", port, rtk_errMsg_get(ret)  ));
		return -EINVAL;
	}

	_printPortState(port);
	_printPortAbility(&_portAbility);

	_portAbility.link   = PORT_LINKUP;
	ret = rtk_port_linkAbility_set(port, &_portAbility );
	if(ret != RT_ERR_OK)
	{
		EXT_ERRORF(("setup Port#%d failed: %s\n",  port, rtk_errMsg_get(ret) ));
//		printf("setup port 4 ERROR, return : %d\n", rtk_errMsg_get(ret));
		return -EINVAL;
	}
	
	ret = rtk_port_linkAbility_get(port, &_portAbility );
	if(ret != RT_ERR_OK)
	{
		EXT_ERRORF(("Port#%d status failed: %s\n", port, rtk_errMsg_get(ret)  ));
		return -EINVAL;
	}
	
	_printPortState(port);
	_printPortAbility(&_portAbility);

	return 0;
}

static int _extRtl8305DebugOneport(rtk_port_t port)
{
	rtk_stat_counter_t cnt;
	int count;
	int ret;
	
	ret = rtk_stat_port_get(port, STAT_IfInOctets, &cnt);
	if(ret != RT_ERR_OK)
	{
		EXT_ERRORF(("Port#%d read InOctets failed: %s\n", port, rtk_errMsg_get(ret)  ));
		return -EINVAL;
	}
	count = (int)cnt;
	printf("Port:%d: InfCount:%d ", port, count);

	ret = rtk_stat_port_get(port, STAT_IfOutOctets, &cnt);
	if(ret != RT_ERR_OK)
	{
		EXT_ERRORF(("Port#%d read OutOctets failed: %s\n", port, rtk_errMsg_get(ret)  ));
		return -EINVAL;
	}
	count = (int)cnt;
	printf("OutfCount:%d "EXT_NEW_LINE, count);

	return 0;
}

void extEtherDebug(void)
{
	_extRtl8305DebugOneport(PN_PORT4);
	_extRtl8305DebugOneport(PN_PORT5);
	_extRtl8305DebugOneport(PN_PORT6);
}



int extSwitchSetup(void)
{
//	rtk_port_link_ability_t		heac0;
//	rtk_port_link_ability_t		heac1; 

	int ret;

//	extDdrMemoryTest();
#if 1
	/* 768 */
//	select_i2c_ch_pca(3);
	/* 774 */
	select_i2c_ch_pca(2);

	RTL8307H_I2C_init(); 

	_switchCfgOnePort(PN_PORT5);
	_switchCfgOnePort(PN_PORT6);

	rtk_hec_mode_set(PN_PORT0, HEC_MODE_ENABLE);
	rtk_hec_mode_set(PN_PORT1, HEC_MODE_ENABLE); 

	ret = rtk_stat_port_reset(PN_PORT5);
	__RTK_CHECK_PORT_START(PN_PORT5, ret, "reset");
	ret = rtk_stat_port_reset(PN_PORT6);
	__RTK_CHECK_PORT_START(PN_PORT6, ret, "reset");

	ret = rtk_stat_port_start(PN_PORT5);
	__RTK_CHECK_PORT_START(PN_PORT5, ret, "start");
	ret = rtk_stat_port_start(PN_PORT6);


	extEtherDebug();
#else

	extRtl830xInit();
#endif
//	extSensorGetTemperatureCelsius();

	return 0;
}



