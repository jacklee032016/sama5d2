/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2015, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __SAMA5D2_XPLAINED_H__
#define __SAMA5D2_XPLAINED_H__

/*
 * PMC Settings
 */
#define BOARD_MAINOSC		12000000

/* PCK: 498MHz, MCK: 166MHz */
#ifdef CONFIG_BUS_SPEED_166MHZ
#define BOARD_PLLA_MULA		82
#endif
/* SAMA5D23 - SAMA5D28
Frequency Monitor limitations
PCK: 348MHz, MCK: 116MHz */
#ifdef CONFIG_BUS_SPEED_116MHZ
#define BOARD_PLLA_MULA		57
#endif

#define BOARD_PCK		((unsigned long)((BOARD_MAINOSC * (BOARD_PLLA_MULA + 1)) / 2))
#define BOARD_MCK		((unsigned long)((BOARD_MAINOSC * (BOARD_PLLA_MULA + 1)) / 2 / 3))

#define BOARD_CKGR_PLLA		(AT91C_CKGR_SRCA | AT91C_CKGR_OUTA_0)
#define BOARD_PLLACOUNT		(0x3F << 8)
#define BOARD_MULA		((AT91C_CKGR_MULA << 2) & (BOARD_PLLA_MULA << 18))
#define BOARD_DIVA		(AT91C_CKGR_DIVA & 1)

#define BOARD_PRESCALER_MAIN_CLOCK	(AT91C_PMC_PLLADIV2_2 \
					| AT91C_PMC_MDIV_3 \
					| AT91C_PMC_CSS_MAIN_CLK)

#define BOARD_PRESCALER_PLLA		(AT91C_PMC_H32MXDIV_H32MXDIV2 \
					| AT91C_PMC_PLLADIV2_2 \
					| AT91C_PMC_MDIV_3 \
					| AT91C_PMC_CSS_PLLA_CLK)

#ifdef CONFIG_BUS_SPEED_166MHZ
#define MASTER_CLOCK		166000000
#endif
#ifdef CONFIG_BUS_SPEED_116MHZ
#define MASTER_CLOCK		116000000
#endif

#define PLLA_SETTINGS		(BOARD_CKGR_PLLA | \
				BOARD_PLLACOUNT | \
				BOARD_MULA | \
				BOARD_DIVA)

/*
 * DBGU Settings
 */
#define	USART_BASE	AT91C_BASE_UART1
#define CONFIG_SYS_DBGU_RXD_PIN		AT91C_PIN_PD(2)
#define CONFIG_SYS_DBGU_TXD_PIN		AT91C_PIN_PD(3)
#define CONFIG_SYS_DBGU_ID		AT91C_ID_UART1

/*
 * DataFlash Settings
 */
#define CONFIG_SYS_SPI_CLOCK	AT91C_SPI_CLK
#define CONFIG_SYS_SPI_MODE	SPI_MODE3

#if defined(CONFIG_SPI_BUS0)
#define CONFIG_SYS_BASE_SPI	AT91C_BASE_SPI0
#define CONFIG_SYS_ID_SPI	AT91C_ID_SPI0

#if defined(CONFIG_SPI0_IOSET_1)
#define CONFIG_SYS_SPI_PCS	AT91C_PIN_PA(17)
#elif defined(CONFIG_SPI0_IOSET_2)
#define CONFIG_SYS_SPI_PCS	AT91C_PIN_PA(30)
#endif

#elif defined(CONFIG_SPI_BUS1)

#define CONFIG_SYS_BASE_SPI	AT91C_BASE_SPI1
#define CONFIG_SYS_ID_SPI	AT91C_ID_SPI1

#if defined(CONFIG_SPI1_IOSET_1)
#define CONFIG_SYS_SPI_PCS	AT91C_PIN_PC(4)
#elif defined(CONFIG_SPI1_IOSET_2)
#define CONFIG_SYS_SPI_PCS	AT91C_PIN_PA(25)
#elif defined(CONFIG_SPI1_IOSET_3)
#define CONFIG_SYS_SPI_PCS	AT91C_PIN_PD(28)
#endif
#endif /* #if defined(CONFIG_SPI_BUS1) */

#if defined(CONFIG_QSPI)
#if defined(CONFIG_QSPI_BUS0)
#define	CONFIG_SYS_BASE_QSPI		AT91C_BASE_QSPI0
#define	CONFIG_SYS_BASE_QSPI_MEM	AT91C_BASE_QSPI0_MEM
#define	CONFIG_SYS_ID_QSPI		AT91C_ID_QSPI0

#elif defined(CONFIG_QSPI_BUS1)

#define	CONFIG_SYS_BASE_QSPI		AT91C_BASE_QSPI1
#define	CONFIG_SYS_BASE_QSPI_MEM	AT91C_BASE_QSPI1_MEM
#define	CONFIG_SYS_ID_QSPI		AT91C_ID_QSPI1
#endif

#endif

/*
 * SDHC Settings
 */
#ifdef CONFIG_SDHC0
#define CONFIG_SYS_BASE_SDHC	AT91C_BASE_SDHC0
#define CONFIG_SYS_ID_SDHC	AT91C_ID_SDMMC0
#endif

#ifdef CONFIG_SDHC1
#define CONFIG_SYS_BASE_SDHC	AT91C_BASE_SDHC1
#define CONFIG_SYS_ID_SDHC	AT91C_ID_SDMMC1
#endif


#define DIPSW_01				AT91C_PIN_PA(18)
#define DIPSW_02				AT91C_PIN_PA(19)
#define DIPSW_03				AT91C_PIN_PA(20)
#define DIPSW_04				AT91C_PIN_PA(21)

#define CONFIG_SYS_PWM_IR		AT91C_PIN_PB(1)


#define FPGA_PROGRAM			AT91C_PIN_PC(12)
#define FPGA_DONE				AT91C_PIN_PC(13)

#define RX_SELECT1				AT91C_PIN_PC(14)
#define RX_SELECT2				AT91C_PIN_PC(15)
#define POWER_1V_OK			AT91C_PIN_PC(16)


#define XAUI_RESET				AT91C_PIN_PC(17)
#define XAUI_RX_DC_RST			AT91C_PIN_PC(18)

#define EXPL_RST_N				AT91C_PIN_PC(19)
#define HDMI_RST_N				AT91C_PIN_PC(20)
#define ETH_RST_N				AT91C_PIN_PC(21)

#define EXPL_COMP_ENA			AT91C_PIN_PC(24)


#define LED_VIDEO				AT91C_PIN_PB(0)
#define LED_ACT					AT91C_PIN_PB(4)
#define LED_POWER				AT91C_PIN_PB(5)
#define LED_LINK					AT91C_PIN_PB(7)


#define  ADV7619_BUS			2


#endif

