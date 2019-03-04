#ifndef	__MUX_STRAP_CMN_H__
#define __MUX_STRAP_CMN_H__


#pragma		pack(1)

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
#include "sama5d2_xplained.h"
#include "l2cc.h"
#include "act8865.h"
#include "twi.h"
#include "arch/tz_matrix.h"
#include "matrix.h"


//#include "common.h"
//#include "hardware.h"
//#include "board.h"
//#include "twi.h"
//#include "debug.h"

//#include "muxlab_500768.h"

//#include "register_map_500768.h"

#include <rx_lib.h> 

#define I2C_ADDR_MDIO	(0x64 >> 1)		// 8 bits addr format
#define I2C_MDIO_MUX	TWI_NONE

#define	MDIO_OPS_ADDR	0x00
#define	MDIO_OPS_WRITE	0x01
#define	MDIO_OPS_READ_INC	0x02
#define	MDIO_OPS_READ	0x03

#define MDIO_OPS		0x00
#define MDIO_PRTAD		0x01
#define FPGA_DEVAD		0x02
#define FPGA_DATA		0x03
#define FPGA_DATA_MSB	0x04
#define FPGA_CONTROL	0x05
#define FPGA_STATUS		0x06


#define TLK_PRTAD	0x00
#define AQR105_PRTAD	0x00
//#define TLK_DEVAD	0x1E

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


unsigned char  mdio_init(void);

unsigned char  mdio_write_addr(unsigned char  prtad, unsigned char  devad,  unsigned int  data);
unsigned char  mdio_write_data(unsigned char  prtad, unsigned char  devad,  unsigned int  data);
unsigned char  mdio_read_data(unsigned char  prtad,  unsigned char  devad,  unsigned int  *data);
unsigned char  mdio_read_data_incr(unsigned char  prtad, unsigned char  devad, unsigned int  *data);
unsigned char  mdio_read_data_buf(unsigned int  *data);



enum
{
    Y1Y0_RGB = 0,                   /* Color space in Y1Y0 of AV Infoframe */
    Y1Y0_YCBCR_422,
    Y1Y0_YCBCR_444,
    Y1Y0_INVALID
};



int  muxSi5351bInit(void);

//ATV_ERR ADIAPI_RepRxShutdown (void);
void RxApi_HdmiHardwareInit (void);
void REP_RxSoftwareInit (void);
void REP_RxHardwareInit (void);
void HAL_PlatformInit (void);
void HAL_PlatformRXInit (void);

void fpag_init_tx(void);
void fpag_init_rx(void);
void init_adv7619(void);
void init_adv7619_B(void);
void  init_video_param(void);
void  dac_param(int  dac);
void  init_pll_drp(unsigned char mode);



//#include <rx_lib.h> 


unsigned int muxHwGetTwiBus(void);

short extSensorGetTemperatureCelsius(void);


#endif

