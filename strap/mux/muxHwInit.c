
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

#if 0
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
	dbg_printf("rest FPGA on pin %d\n", FPGA_RST_N);
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

	dbg_printf("read FPGA Done %d\n", FPGA_DONE);
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
			dbg_printf("Timeout in waiting FPGA Done\n");
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
	dbg_printf("Reload FPGA firmware on pin %d...\n", FPGA_PROGRAM);
	/* start load FPGA firmware */
	pio_set_gpio_output(FPGA_PROGRAM, 1);
	pio_set_value(FPGA_PROGRAM, 0);
	udelay(500);
	pio_set_value(FPGA_PROGRAM, 1);

	return  _extBspFpgaWaitDone(8);
}


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
	int count = 0;
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

	count = pio_configure(pio_pins);
	EXT_INFOF(("%d input pins are configured", count));

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

	// init M500768 hw specific
	dbg_printf(EXT_OS_NAME);

	muxSi5351bHwInit();

#if	(MUX_BOARD == MUX_BOARD_768)
//	AQR105_hw_reset();
#endif
	_hdmiHwReset();
	_fpgaHwReset();

	// read DIP switch value
	btnCfg = _cfgReadPins();
	dbg_info("DIP SW: %x; RX_SELECT: %x \n", btnCfg&0x0F, (btnCfg>>4)&0x03);
	
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
#endif
	

#if WITH_MEM_TEST
	dbg_info("DDR Memory Testing...\n");
	ddr_memory_test();
#endif

	muxFpgaReset(fpgaCtrl);

	extSensorGetTemperatureCelsius();

	return 0;
}

