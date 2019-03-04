// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou.Yang <wenyou.yang@atmel.com>
 */

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

#include <dm.h>
#include <i2c.h>

#include <../drivers/mux/rtl8307h/rtk_api.h>
#include <../drivers/mux/rtl8307h/rtk_api_ext.h>
#include <../drivers/mux/rtl8307h/rtl8307h_types.h> 


extern void at91_pda_detect(void);

DECLARE_GLOBAL_DATA_PTR;

static void board_usb_hw_init(void)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTB, 10, 1);
}

#define I2C_MUX_PCA_ADDR	(0xE0>>1)
//#define I2C_MUX_PCA_ADDR	(0xE0)

static int _select_i2c_ch_pca(u8 ch)
{
	struct udevice *bus, *dev;
	u8 addr, data[8];
	int err;
//	struct i2c_msg msg[2];
//	u8 i2c_addr[3], i2c_data[4];

	uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	if(bus == NULL)
	{
		printf("PCA Error: Check I2C configuration.\n");
		return -1;
	}
	printf("I2C bus: %s.\n", bus->name);
	
	dm_i2c_probe(bus, I2C_MUX_PCA_ADDR, 0, &dev);		// AT24MAC_ADDR
	if(dev == NULL)
	{
		printf("PCA Error: Can't found PCA954 device.\n");
		return -1;
	}
	

	/* Selecting proper channel via PCA*/
	data[0] = 0x04 | ch;
	addr = 0x04 | ch; // I2C_MUX_PCA_ADDR;
	
	printf("I2C bus: %s write device %s\n", bus->name, dev->name );
	err = dm_i2c_write(dev, addr, data, 0);
	//ret = i2c_write(I2C_MUX_PCA_ADDR, 0x0, 1, &ch, 1);
	if (err) {
		printf("PCA: failed to select proper channel.\n");
		return err;
	}
#if 0
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
	return 0;
}

static void rtl830x_init(void)
{
	rtk_port_link_ability_t		P5_ability; /* FPGA */
	rtk_port_link_ability_t		P6_ability; /* MCU */

//	rtk_port_link_ability_t		heac0;
//	rtk_port_link_ability_t		heac1; 

	if(_select_i2c_ch_pca(3) )
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


#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_DM_VIDEO
	at91_video_show_board_info();
#endif
	at91_pda_detect();

	rtl830x_init();
	return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
static void board_uart1_hw_init(void)
{
	atmel_pio4_set_a_periph(AT91_PIO_PORTD, 2, ATMEL_PIO_PUEN_MASK);	/* URXD1 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTD, 3, 0);	/* UTXD1 */

	at91_periph_clk_enable(ATMEL_ID_UART1);
}

void board_debug_uart_init(void)
{
	board_uart1_hw_init();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif

	return 0;
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_USB
	board_usb_hw_init();
#endif

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#define AT24MAC_MAC_OFFSET	0x9a

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_I2C_EEPROM
	at91_set_ethaddr(AT24MAC_MAC_OFFSET);
#endif

	return 0;
}
#endif

/* SPL */
#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
}

static void ddrc_conf(struct atmel_mpddrc_config *ddrc)
{
#if WITH_MUXLAB_BOARD
	/* MuxLab */
	ddrc->md = (ATMEL_MPDDRC_MD_DBW_16_BITS | ATMEL_MPDDRC_MD_DDR3_SDRAM);
#else
	ddrc->md = (ATMEL_MPDDRC_MD_DBW_32_BITS | ATMEL_MPDDRC_MD_DDR3_SDRAM);
#endif

	ddrc->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_14 |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS5 |
		    ATMEL_MPDDRC_CR_DIC_DS |
		    ATMEL_MPDDRC_CR_DIS_DLL |
		    ATMEL_MPDDRC_CR_NB_8BANKS |
		    ATMEL_MPDDRC_CR_DECOD_INTERLEAVED |
		    ATMEL_MPDDRC_CR_UNAL_SUPPORTED);

	ddrc->rtr = 0x511;

	ddrc->tpr0 = (6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |
		      3 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |
		      4 << ATMEL_MPDDRC_TPR0_TWR_OFFSET |
		      9 << ATMEL_MPDDRC_TPR0_TRC_OFFSET |
		      3 << ATMEL_MPDDRC_TPR0_TRP_OFFSET |
		      4 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET |
		      4 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET |
		      4 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET);

	ddrc->tpr1 = (27 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET |
		      29 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET |
		      0 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET |
		      3 << ATMEL_MPDDRC_TPR1_TXP_OFFSET);

	ddrc->tpr2 = (0 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET |
		      0 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      0 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      4 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET);
}

void mem_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct atmel_mpddr *mpddrc = (struct atmel_mpddr *)ATMEL_BASE_MPDDRC;
	struct atmel_mpddrc_config ddrc_config;
	u32 reg;

	ddrc_conf(&ddrc_config);

	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	writel(AT91_PMC_DDR, &pmc->scer);

	reg = readl(&mpddrc->io_calibr);
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_RDIV;
	reg |= ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_55;
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_TZQIO;
	reg |= ATMEL_MPDDRC_IO_CALIBR_TZQIO_(100);
	writel(reg, &mpddrc->io_calibr);

	writel(ATMEL_MPDDRC_RD_DATA_PATH_SHIFT_TWO_CYCLE,
	       &mpddrc->rd_data_path);

	ddr3_init(ATMEL_BASE_MPDDRC, ATMEL_BASE_DDRCS, &ddrc_config);

	writel(0x3, &mpddrc->cal_mr4);
	writel(64, &mpddrc->tim_cal);
}

void at91_pmc_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 tmp;

	/*
	 * while coming from the ROM code, we run on PLLA @ 492 MHz / 164 MHz
	 * so we need to slow down and configure MCKR accordingly.
	 * This is why we have a special flavor of the switching function.
	 */
	tmp = AT91_PMC_MCKR_PLLADIV_2 |
	      AT91_PMC_MCKR_MDIV_3 |
	      AT91_PMC_MCKR_CSS_MAIN;
	at91_mck_init_down(tmp);

	tmp = AT91_PMC_PLLAR_29 |
	      AT91_PMC_PLLXR_PLLCOUNT(0x3f) |
	      AT91_PMC_PLLXR_MUL(82) |
	      AT91_PMC_PLLXR_DIV(1);
	at91_plla_init(tmp);

	writel(0x0 << 8, &pmc->pllicpr);

	tmp = AT91_PMC_MCKR_H32MXDIV |
	      AT91_PMC_MCKR_PLLADIV_2 |
	      AT91_PMC_MCKR_MDIV_3 |
	      AT91_PMC_MCKR_CSS_PLLA;
	at91_mck_init(tmp);
}
#endif
