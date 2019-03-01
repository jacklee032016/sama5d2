
#include "common.h"
#include "hardware.h"
#include "pmc.h"
#include "usart.h"
#include "debug.h"
#include "ddramc.h"
#include "gpio.h"
#include "timer.h"
#include "watchdog.h"
#include "string.h"

#include "arch/at91_pmc.h"
#include "arch/at91_rstc.h"
#include "arch/at91_pio.h"
#include "arch/at91_ddrsdrc.h"
#include "arch/at91_sfr.h"
#include "muxlab_500768.h"
#include "l2cc.h"
#include "act8865.h"
#include "twi.h"
#include "arch/tz_matrix.h"
#include "matrix.h"

#include "si5351b.h"
#include "mdio_drv.h"

#include <rtk_api.h>
#include <rtk_api_ext.h>
#include <rtl8307h_types.h> 

#include <rx_lib.h> 
#include <adv7619_init.h> 


enum
{
    Y1Y0_RGB = 0,                   /* Color space in Y1Y0 of AV Infoframe */
    Y1Y0_YCBCR_422,
    Y1Y0_YCBCR_444,
    Y1Y0_INVALID
};


 unsigned char _acsamsung_edid[256UL + 1] = {
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x4C, 0x2D, 0x1F, 0x04, 0x35, 0x32, 0x59, 0x4D, 0x0B, 0x18, 0x01, 0x04, 0xC2, 0x33, 0x1D, 0x78, 0x2A, 0xEE, 0x91, 0xA3, 0x54, 0x4C, 0x99, 0x26, 0x0F, 0x50, 0x54, 0x2F, 0xCE, 0x00, 0x81, 0x80,
  0x81, 0x40, 0x81, 0x00, 0x95, 0x00, 0xB3, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x3B, 0x3D, 0x00, 0xA0, 0x80, 0x80, 0x21, 0x40, 0x30, 0x20, 0x35, 0x00, 0xFE, 0x1F, 0x11, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x38, 0x3C, 0x1E,
  0x51, 0x10, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x53, 0x79, 0x6E, 0x63, 0x4D, 0x61, 0x73, 0x74, 0x65, 0x72, 0x0A, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x48, 0x56, 0x4E, 0x51, 0x43, 0x30, 0x30,
  0x34, 0x33, 0x33, 0x0A, 0x20, 0x20, 0x00, 0xAD, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0x00
};

const unsigned char _acComplete_EDID_SEIKI_4K[256UL + 1] = {
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x4C, 0xAB, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x23, 0x17, 0x01, 0x03, 0x81, 0x56, 0x30, 0x78, 0x8A, 0xA5, 0x8E, 0xA6, 0x54, 0x4A, 0x9C, 0x26, 0x12, 0x45, 0x46, 0xAD, 0xCE, 0x00, 0x81, 0x40,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58, 0x8A, 0x00, 0x56, 0xE1, 0x31, 0x00, 0x00, 0x1E, 0x9A, 0x29, 0xA0, 0xD0, 0x51, 0x84, 0x22, 0x30,
  0x50, 0x98, 0x36, 0x00, 0x60, 0xE1, 0x31, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x32, 0x4B, 0x18, 0x3C, 0x0B, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x53, 0x45, 0x33, 0x39, 0x55, 0x59, 0x30,
  0x34, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x01, 0x65, 0x02, 0x03, 0x2A, 0x71, 0x4F, 0x06, 0x07, 0x02, 0x03, 0x15, 0x96, 0x11, 0x12, 0x13, 0x04, 0x14, 0x05, 0x1F, 0x90, 0x20, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x6D, 0x03, 0x0C, 0x00,
  0x20, 0x00, 0x08, 0x3C, 0x20, 0x40, 0x68, 0x01, 0x02, 0x03, 0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0C, 0x40, 0x55, 0x00, 0x56, 0xE1, 0x31, 0x00, 0x00, 0x18, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20, 0x58, 0x2C, 0x25, 0x00,
  0x56, 0xE1, 0x31, 0x00, 0x00, 0x9E, 0x01, 0x1D, 0x80, 0xD0, 0x72, 0x1C, 0x16, 0x20, 0x10, 0x2C, 0x25, 0x80, 0x56, 0xE1, 0x31, 0x00, 0x00, 0x9E, 0x01, 0x1D, 0x00, 0xBC, 0x52, 0xD0, 0x1E, 0x20, 0xB8, 0x28, 0x55, 0x40, 0x56, 0xE1, 0x31, 0x00,
  0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00
};


STATIC CONSTANT RX_PIN Pin[] = 
{
    RX_LLC_OUT_PIN,     RX_SYNC_OUT_PINS,  
    RX_PIX_OUT_PINS,    RX_AUD_OUT_PINS,
    RX_INT2_OUT_PIN,
    (RX_PIN)0xff 
};


static void AQR105_hw_reset(void)
{
	pio_set_gpio_output(XAUI_RX_DC_RST, 0);
	pio_set_gpio_output(XAUI_RESET, 0);
	pio_set_value(XAUI_RESET, 1);
	udelay(500);
	pio_set_value(XAUI_RESET, 0);
}

static void RTL8307H_hw_reset(void)
{
	/* reset realtek ethernet switch */
	pio_set_gpio_output(ETH_RST_N, 1);
	pio_set_value(ETH_RST_N, 0);
	udelay(500);
	pio_set_value(ETH_RST_N, 1);
}

static void HDMI_hw_reset(void)
{
	pio_set_gpio_output(EXPL_COMP_ENA, 1);

	/* reset both HDMI chips */
	pio_set_gpio_output(EXPL_RST_N, 1);
	pio_set_gpio_output(HDMI_RST_N, 1);
	pio_set_value(EXPL_RST_N, 0);
	pio_set_value(HDMI_RST_N, 0);
	udelay(5000);
	pio_set_value(EXPL_RST_N, 1);
	pio_set_value(HDMI_RST_N, 1);
}

static void FPGA_reload(void)
{
	/* reset both HDMI chips */
	pio_set_gpio_output(FPGA_PROGRAM, 1);
	pio_set_value(FPGA_PROGRAM, 0);
	udelay(500);
	pio_set_value(FPGA_PROGRAM, 1);
}


static void LED_RESET(void)
{
	/* reset both HDMI chips */
	pio_set_gpio_output(LED_VIDEO, 1);
	pio_set_gpio_output(LED_ACT, 1);
	pio_set_gpio_output(LED_POWER, 1);
	pio_set_gpio_output(LED_LINK, 1);
}

unsigned read_dipsw(void)
{
const struct pio_desc pio_pins[] = {
	{"SW1", DIPSW_01, 1, PIO_DEFAULT, PIO_INPUT},
	{"SW2", DIPSW_02, 1, PIO_DEFAULT, PIO_INPUT},
	{"SW3", DIPSW_03, 1, PIO_DEFAULT, PIO_INPUT},
	{"SW4", DIPSW_04, 1, PIO_DEFAULT, PIO_INPUT},
	{"SEL1", RX_SELECT1, 1, PIO_DEFAULT, PIO_INPUT},
	{"SEL2", RX_SELECT2, 1, PIO_DEFAULT, PIO_INPUT},
	{(char *)0, 0, 0, PIO_DEFAULT, PIO_INPUT},
	};

//	pio_set_gpio_input(RX_SELECT2, PIO_PULLUP);

	pio_configure(pio_pins);
	pmc_sam9x5_enable_periph_clk(AT91C_ID_PIOA);
	pmc_sam9x5_enable_periph_clk(AT91C_ID_PIOB);
	pmc_sam9x5_enable_periph_clk(AT91C_ID_PIOC);
	pmc_sam9x5_enable_periph_clk(AT91C_ID_PIOD);
	

	/* reset both HDMI chips */
	return( (pio_get_value(DIPSW_01) ?0x01:0) |
		(pio_get_value(DIPSW_02) ?0x02:0) |
		(pio_get_value(DIPSW_03) ?0x04:0) |
		(pio_get_value(DIPSW_04) ?0x08:0) |
		(pio_get_value(RX_SELECT1) ?0x10:0) |
		(pio_get_value(RX_SELECT2) ?0x20:0) );
}

static void pwm_init(void)
{
	const struct pio_desc pwm_pins[] = {
		{"PWMIR", CONFIG_SYS_PWM_IR, 0, PIO_DEFAULT, PIO_PERIPH_D},
		{(char *)0, 0, 0, PIO_DEFAULT, PIO_PERIPH_A},
	};

	pio_configure(pwm_pins);		
//	pmc_sam9x5_enable_periph_clk(CONFIG_SYS_DBGU_ID);
}


void hw_config_late(void)
{
	int timeout;
	unsigned int mdio_page;
	unsigned int mdio_addr;
	unsigned int mdio_data;

	//unsigned char	version[32];
	//unsigned char	buffer[4];
	unsigned char	data[16];
	unsigned int bus;
	//unsigned int chan = 2;

	//unsigned int	RxRev;
	//RX_OP_MODE 	OpMode = RX_OPMODE_HDMI;
	//RX_PIX_BUS_INFO PixBusInfo;

	BOOL  Encrypted;
	RX_DEV_STATE DevState[2];
	unsigned char  bRet;
	RX_OP_MODE_CFG  OpModeCfg;

	unsigned char  twi_data;
	unsigned char  twi_addr;
	unsigned char  twi_dev;
	UINT16	Noise =0;
	UINT16	Calib =0;
	unsigned char  cfg;

	// init M500768 hw specific
	init_si5351b();

	AQR105_hw_reset();
	//RTL8307H_hw_reset();
	HDMI_hw_reset();

	LED_RESET();
	FPGA_reload();
	udelay(200000);	// wait for FPGA to finish loading
	

	pio_set_gpio_output(LED_VIDEO, 0);
	pio_set_gpio_output(LED_ACT, 0);
	pio_set_gpio_output(LED_LINK, 0);
	

	pwm_init();	
	

#if 0
	bus = 0;
	buffer[0] = 0x04 | ADV7619_BUS;	// set mux for si5351b 
	twi_write(bus, PCA9544_ADDR, 0, 0, buffer, 1);

	dbg_info("init ADV7619 \n");
	//ADIAPI_RxGetChipRevision (version);
//	dbg_info(version);

	ADIAPI_RxSetDeviceIndex(chan);
	ADIAPI_RxInit(OpMode);
	ADIAPI_RxHdmiAssertHpd(PORT_ALL, FALSE);
	ADIAPI_RepRxShutdown();     /* Turn off RX device */
	REP_RxSoftwareInit();       /* Initialize Repeater software (RX part) */
	RxApi_HdmiHardwareInit();   /* Initialize RX hw for HDMI operation */
	REP_RxHardwareInit();       /* Initialize RX hw for repeater operation */

	HAL_PlatformRXInit();      /* Initialize platform-dependent RX settings */

//	ATV_I2CWriteTable (init_tbl_4k, 0);
                                        
	bRet = ADIAPI_RxHdmiSetSelectedPort ( PORT_B , TRUE);
	ADIAPI_RxSetAudioInterface(RX_AUD_IF_AUTO );

	bRet = ADIAPI_RxHdmiSetInternalEdid( _acComplete_EDID_SEIKI_4K , 0x0020 , NULL , 0 , FALSE);
//	bRet = ADIAPI_RxHdmiSetInternalEdid( _acsamsung_edid , 0 , NULL , 0 , TRUE);

	bRet = ADIAPI_RxHdmiEnableInternalEdid(PORT_ALL , FALSE);
	ADIAPI_RxHdmiAssertHpd ( PORT_B , TRUE);


#if 0
	ADIAPI_RxSetDeviceIndex(chan);
	ADIAPI_RxInit( OpMode);

		//Rx4K2KResSettingTable
//	ATV_I2CWriteTable (init_tbl_4k, 0);
//	ATV_I2CWriteTable (init_tbl_HD, 0);


	ADIAPI_RxHdmiAssertHpd ( PORT_B , FALSE);
	ADIAPI_RxManagePower( RX_ALL_SECTIONS, TRUE);
	ADIAPI_RxGetChipRevision(&RxRev);
	ADIAPI_RxChipReset( RX_SDP_MAIN);
	ADIAPI_RxChipReset( RX_EDID_REP_MPU);

	bRet = ADIAPI_RxHdmiSetSelectedPort ( PORT_B , TRUE);
	ADIAPI_RxSetAudioInterface(RX_AUD_IF_AUTO );

	bRet = ADIAPI_RxHdmiEnableInternalEdid(PORT_ALL , FALSE);

//	bRet = ADIAPI_RxHdmiSetInternalEdid( _acComplete_EDID_SEIKI_4K , 0x0020 , NULL , 0 , FALSE);
	bRet = ADIAPI_RxHdmiSetInternalEdid( _acsamsung_edid , 0 , NULL , 0 , TRUE);

	bRet = ADIAPI_RxHdmiSetHdcpI2CBase ( 0x74 ) ;

#if 0
	ATV_I2CWriteField8(VRX_REPEATER_MAP_ADDR, 0x77, 0xFF, 0x0, 0x00);
	ATV_I2CWriteField8(VRX_REPEATER_MAP_ADDR, 0x52, 0xFF, 0x0, 0x20);
	ATV_I2CWriteField8(VRX_REPEATER_MAP_ADDR, 0x53, 0xFF, 0x0, 0x00);
	ATV_I2CWriteField8(VRX_REPEATER_MAP_ADDR, 0x70, 0xFF, 0x0, 0x9E);
#endif

//	bRet = ADIAPI_RxHdmiEnableInternalEdidN(chan, PORT_ALL , TRUE);

//	ADIAPI_RxSetEnabledEvents (EventList, sizeof(EventList)/sizeof(RX_EVENT), TRUE);

	//RX_3_XTAL_CLK , RX_15_XTAL_CLK,  RX_63_XTAL_CLK, 	RX_ACT_UNTIL_CLR
//	ADIAPI_RxConfigInt( RX_INT1_OUT_PIN , RX_ACT_UNTIL_CLR /*RX_63_XTAL_CLK*/ , RX_LOW_LEVEL );

	// Enable Hotplug signal
//	msDelay(50);

//	ADIAPI_RxSetDeviceIndex(chan);
	ADIAPI_RxHdmiAssertHpd ( PORT_B , TRUE);

	PixBusInfo.BitWidth = RX_BUS_BITS_8;
	PixBusInfo.Format = RX_BUS_MODE5; // RX_BUS_MODE0;
	PixBusInfo.Mode = RX_BUS_OP_MODE_444SDR;
	PixBusInfo.ChOrder = RX_BUS_OP_CH_GBR;
	PixBusInfo.CrCbSwap = 0;
	PixBusInfo.BitReorder = 0;
//	ADIAPI_RxSetOpPixFormat( &PixBusInfo, &OpMode);

#endif

		//Rx4K2KResSettingTable
//	ATV_I2CWriteTable (init_tbl_4k, 0);
//	ATV_I2CWriteTable (init_tbl_HD, 0);

	ADIAPI_RxSetInpVid4K2K(TRUE);

	//ADIAPI_RxCpSetManGainFilter(RX_4K_LINES);
	//ADIAPI_RxCpConfigHsyncLckMsrmt(100, RX_256_LINES);
	//ADIAPI_RxCpSetNoiseCalibWindow (10, RX_4K_LINES);

	ATV_I2CWriteTable(init_clk_gt170, 0);

#if 1
	VRX_set_OP_FORMAT_SEL(0x54);
	VRX_set_VIDEO_CORE_ENABLE_MAN_EN(1);
	VRX_set_VIDEO_FREQ_LIMIT_DISABLE(1);
	VRX_set_AUDIO_FREQ_LIMIT_DISABLE(1);

	VRX_set_MAN_OP_CLK_SEL(0x2);
	VRX_set_MAN_OP_CLK_SEL_EN(1);
#endif

#endif



#if 1
	timeout = 1000 / 50;	// 5 sec delay max
	mdio_page=0x1;  mdio_addr=0xcc02;
	mdio_write_addr(AQR105_PRTAD, mdio_page, mdio_addr);	 // set reg addr

	while (--timeout!=0) {
		mdio_read_data(AQR105_PRTAD, mdio_page, &mdio_data);	// read data
		if ((mdio_data & 0x03) == 0x01) break;		
		udelay(50000);
		dbg_info(".");
	}
#endif

	mdio_page=0x1e;  mdio_addr=0x0020;
	mdio_write_addr(AQR105_PRTAD, mdio_page, mdio_addr);	 	// set reg addr
	mdio_read_data(AQR105_PRTAD, mdio_page, &mdio_data);		// read data
	dbg_info("\nAQR105 Firmware version %x.%x\n", (mdio_data>>8), (mdio_data&0xff) );


	mdio_page=0x1e;  mdio_addr=0xC820;
	mdio_write_addr(AQR105_PRTAD, mdio_page, mdio_addr);      	// set reg addr
	mdio_read_data(AQR105_PRTAD, mdio_page, &mdio_data);     	// read data
	dbg_info("AQR105 Temperature %d.%d\n", (mdio_data>>8), (mdio_data&0xff) );

	// change LED behaviour
	mdio_page=0x1e;  mdio_addr=0xC430; mdio_data= 0x000F;        	// blink on TX & RX, strech activity by 100 ms
	mdio_write_addr(AQR105_PRTAD, mdio_page, mdio_addr);      	// set reg addr
	mdio_write_data(AQR105_PRTAD, mdio_page, mdio_data);      	// read data

	mdio_page=0x1e;  mdio_addr=0xC431; mdio_data= 0x0083;        	// Link 10G, strech activity by 100 ms
	mdio_write_addr(AQR105_PRTAD, mdio_page, mdio_addr);      	// set reg addr
	mdio_write_data(AQR105_PRTAD, mdio_page, mdio_data);      	// read data

	// read DIP switch value
	cfg = read_dipsw();
	dbg_info("DIP SW: %x \n", cfg);



	bus = 0;
	twi_dev = (0x60>>1);
	twi_addr = 0;

	// reset fpga global
	twi_data = 0x80;	// reset FPGA
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_data = 0x0;	// clear reset
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);

	// reset sub modules
	twi_data = 0x7f;	// reset all FPGA sub modules
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	twi_data = 0x0;		// clear reset
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);


	// disable all stream (video, audio, etc)
	twi_addr = 3;
	//twi_data = 0x0f;	
	twi_data = 0x0;	
	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);

	//read reg 03 value, if not 0, disable it again.
	twi_read(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	if(twi_data != 0x0)
	{
		twi_data = 0x0;
		twi_addr = 3;	
		twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
		dbg_info("set reg 03 to 0 again %x\n");
	}
	else
		dbg_info("read 03 reg %x\n", twi_data);


	// read fpga version,revision
	twi_addr = 29;
	twi_read(bus, twi_dev, twi_addr, 1, &data, 3);
	dbg_info("FPGA version %x %x %x\n", data[0],data[1],data[2] );


	if ((data[0] & 0x80) == 0)
	{
		dbg_info("FPGA RX unit \n");
		dac_param(1000);

		fpag_init_rx();

		dbg_info("HDMI RX unit \n");
		init_video_param();
		Sil9136_init();

	}
	else
	{
		dbg_info("FPGA TX unit \n");
		dac_param(0);
		init_adv7619();
		init_adv7619_B();
		fpag_init_tx();
	}



//	while (1) {
//		ADIAPI_RxHouseKeeping ();
//		udelay(500);
//	}
}

