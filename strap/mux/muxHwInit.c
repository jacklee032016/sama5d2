
#include "common.h"
#include "hardware.h"
#include "board.h"
#include "twi.h"
#include "gpio.h"
#include "debug.h"

#include "timer.h"	/* delay */
#include "pmc.h"

#include "arch/at91_pio.h"	/* for AT91C_PIN_PC pin definition */
#include "string.h"

#include "mux7xx.h"

#if 0
static void pwm_init(void)
{
	const struct pio_desc pwm_pins[] =
	{
		{"PWMIR", CONFIG_SYS_PWM_IR, 0, PIO_DEFAULT, PIO_PERIPH_D},
		{(char *)0, 0, 0, PIO_DEFAULT, PIO_PERIPH_A},
	};

	pio_configure(pwm_pins);		
//	pmc_sam9x5_enable_periph_clk(CONFIG_SYS_DBGU_ID);
}

static void RTL8307H_hw_reset(void)
{
	/* reset realtek ethernet switch */
	pio_set_gpio_output(ETH_RST_N, 1);
	pio_set_value(ETH_RST_N, 0);
	udelay(500);
	pio_set_value(ETH_RST_N, 1);
}
#endif

static void _hdmiHwReset(void)
{
#if 0
	pio_set_gpio_output(EXPL_COMP_ENA, 1);

	/* reset both HDMI chips */
	pio_set_gpio_output(EXPL_RST_N, 1);
	pio_set_gpio_output(HDMI_RST_N, 1);
	pio_set_value(EXPL_RST_N, 0);
	pio_set_value(HDMI_RST_N, 0);
	udelay(5000);
	pio_set_value(EXPL_RST_N, 1);
	pio_set_value(HDMI_RST_N, 1);
#else
	/* reset HDMI chip */
	pio_set_gpio_output(HDMI_RST_N, 1);
	pio_set_value(HDMI_RST_N, 0);
	udelay(5000);
	pio_set_value(HDMI_RST_N, 1);
#endif	
}

static void _fpgaHwReset(void)
{
	EXT_DEBUGF(EXT_BOOTSTRAP_DEBUG, "rest FPGA on pin %d", FPGA_RST_N );
	/* reset FPGA chip */
	pio_set_gpio_output(FPGA_RST_N, 1);
	pio_set_value(FPGA_RST_N, 0);
	udelay(5000);
	pio_set_value(FPGA_RST_N, 1);
}

static int  _extBspFpgaWaitDone(unsigned int  seconds)
{
//	char data;
	unsigned int	timeout;
	int  done = 0;
	
	timeout = seconds;

	EXT_DEBUGF(EXT_BOOTSTRAP_DEBUG, "read FPGA Done %d", FPGA_DONE);
	while (1)
	{
		done = pio_get_value(FPGA_DONE);
		if (done== 0)
		{
			dbg_printf("L");
		}
		else
		{
			dbg_printf("H, OK!");
			break;
		}
		
		if (timeout == 0)
		{
			EXT_ERRORF("Timeout in waiting FPGA Done");
			return done;
		}

		timeout--;
		mdelay(1000);
	}
	
	dbg_printf("\n");

	return done;
}


static int _muxFpgaReload(void)
{
	EXT_DEBUGF(EXT_BOOTSTRAP_DEBUG, "Reload FPGA firmware on pin %d...", FPGA_PROGRAM);
	EXT_INFOF("Reload FPGA firmware on pin %d...", FPGA_PROGRAM);
	/* start load FPGA firmware */
	pio_set_gpio_output(FPGA_PROGRAM, 1);
	pio_set_value(FPGA_PROGRAM, 0);
	udelay(500);
	pio_set_value(FPGA_PROGRAM, 1);

	return  _extBspFpgaWaitDone(8);
}

/* make every LED on */
static void _ledReset(void)
{
	/* set LED pins to output */
	pio_set_gpio_output(LED_VIDEO, 1);
	pio_set_gpio_output(LED_ACT, 1);
	pio_set_gpio_output(LED_POWER, 1);
	pio_set_gpio_output(LED_LINK, 1);
}

static unsigned _cfgReadPins(void)
{
	const struct pio_desc pio_pins[] =
	{
		{"SW1", DIPSW_01, 1, PIO_DEFAULT, PIO_INPUT},
		{"SW2", DIPSW_02, 1, PIO_DEFAULT, PIO_INPUT},
		{"SW3", DIPSW_03, 1, PIO_DEFAULT, PIO_INPUT},
		{"SW4", DIPSW_04, 1, PIO_DEFAULT, PIO_INPUT},
		{"SEL1", RX_SELECT1, 1, PIO_DEFAULT, PIO_INPUT},
		{"SEL2", RX_SELECT2, 1, PIO_DEFAULT, PIO_INPUT},
		{"FPDN", FPGA_DONE, 1, PIO_DEFAULT, PIO_INPUT},
		{(char *)0, 0, 0, PIO_DEFAULT, PIO_INPUT},
	};

//	pio_set_gpio_input(RX_SELECT2, PIO_PULLUP);

#ifndef __EXT_RELEASE__
	int count = 0;

	count = pio_configure(pio_pins);
	EXT_DEBUGF(EXT_BOOTSTRAP_DEBUG, "%d input pins are configured", count);
#else
	pio_configure(pio_pins);
#endif
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

static FPGA_CTRL _fpgaCtrl;


int muxHwInit(void)
{
	unsigned char  btnCfg = 0;
	
	FPGA_CTRL *fpgaCtrl = &_fpgaCtrl;
	
	memset(fpgaCtrl, 0, sizeof(FPGA_CTRL));
	fpgaCtrl->bus = muxHwTwiBus();
	fpgaCtrl->devAddrSystem = EXT_I2C_DEV_FPGA_SYSTEM;
	fpgaCtrl->devAddrRtp= EXT_I2C_DEV_FPGA_RTP;
	fpgaCtrl->devAddrNetwork = EXT_I2C_DEV_FPGA_NET;
	fpgaCtrl->devAddrXadc = EXT_I2C_DEV_FPGA_DRP_XADC;
	fpgaCtrl->devAddrPcspma = EXT_I2C_DEV_FPGA_PCSPMA;
	fpgaCtrl->devAddrRll = EXT_I2C_DEV_FPGA_DRP_PLL;

	EXT_INFOF( BOARD_NAME " is initializing ...");
	// init M500768 hw specific
#if	(MUX_BOARD == MUX_ATMEL_XPLAINED)
	EXT_INFOF(EXT_OS_NAME);
	return 0;
#endif

	muxSi5351bHwInit();

	TRACE();
	
//	RTL8307H_hw_reset();
#if	(MUX_BOARD == MUX_BOARD_768)
//	AQR105_hw_reset();
#endif
	_hdmiHwReset();
	_fpgaHwReset();

	// read DIP switch value
	btnCfg = _cfgReadPins();
	EXT_INFOF("DIP SW: %x; RX_SELECT: %x", btnCfg&0x0F, (btnCfg>>4)&0x03);
	
	_ledReset();
	_muxFpgaReload();

#if	(MUX_BOARD == MUX_BOARD_768)
//	pwm_init();	
#endif

#if	(MUX_BOARD == MUX_BOARD_768)
//	if(mdioInit() )
	{
//		return -1;
	}
#endif

	/* after reload, LEDs off */
#if	(MUX_BOARD == MUX_BOARD_768)
	pio_set_gpio_output(LED_VIDEO, 0);
	pio_set_gpio_output(LED_ACT, 0);
	pio_set_gpio_output(LED_LINK, 0);
#elif (MUX_BOARD == MUX_BOARD_774)
	pio_set_value(LED_VIDEO, 0);
	pio_set_value(LED_ACT, 0);
	pio_set_value(LED_LINK, 0);

	/* flash from FPGA to MUC, 08.22, 2019 */
	EXT_INFOF("Flash switch to MCU");
	pio_set_gpio_output(FPGA_FLASH_SWITCH, 1);
	pio_set_value(FPGA_FLASH_SWITCH, 1);
#endif
	

#if WITH_MEM_TEST
	dbg_info("DDR Memory Testing...\n");
	ddr_memory_test();
#endif

	muxFpgaReset(fpgaCtrl);

	extSensorGetTemperatureCelsius();

	return 0;
}

