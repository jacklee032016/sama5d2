/*
* definitions about board dependent
* include from libMux.h, definitions for MuxMain
*/

#ifndef	__MUX_7XX_H__
#define	__MUX_7XX_H__

/* board type, so libMux and libCmnSys can be customized */
#define	EXT_BOARD_TYPE			MUX_BOARD_TYPE_774


/* debug option, in EXT_DEBUG_XXXX  */
#define	EXT_DEBUG_INIT				EXT_DBG_ON


#include "extSysParams.h"


typedef	enum
{
	LED_MODE_OFF	 	=	0,
	LED_MODE_ON		=	1,
	LED_MODE_BLINK	= 2
}LED_MODE_T;

typedef	enum
{
	LED_TYPE_POWER	=	0,
	LED_TYPE_VIDEO	,
	LED_TYPE_ACT,
	LED_TYPE_UNKNOWN
}LED_TYPE_T;


typedef	struct _LedCtrl
{
	int					fd;
	LED_MODE_T			ledMode;
}HwLedCtrl;

typedef	struct _ButtonCtrl
{
	int					fd;
}HwButtonCtrl;

typedef	struct _WatchdogCtrl
{
	int				fd;
	int				timeout;
}HwWatchdogCtrl;


typedef	enum _RS232_BAUDRATE
{
	RS232_BR_300 = 300,
	RS232_BR_600 = 600,
	RS232_BR_1200 =  1200,
	RS232_BR_2400 = 2400,
	RS232_BR_4800 =  4800,
	RS232_BR_9600 =  9600,
	RS232_BR_19200 = 19200,
	RS232_BR_38400 =  38400,
	RS232_BR_57600 = 57600,
	RS232_BR_115200 =  115200,
	RS232_BR_INVALIDATE = MUX_INVALIDATE_INTEGER
}RS232_BAUDRATE;

typedef enum _RS232_DATABITS
{
	RS232_DB_5 =  5,
	RS232_DB_6 =  6,
	RS232_DB_7 = 7,
	RS232_DB_8 = 8,
	RS232_DB_INVALIDATE = MUX_INVALIDATE_INTEGER
}RS232_DATABITS;

typedef enum _RS232_STOPBITS
{
	RS232_SB_1 = 1,
	RS232_SB_2 = 2,
	RS232_SB_INVALIDATE = MUX_INVALIDATE_INTEGER
}RS232_STOPBITS;

typedef	struct _Rs232Ctrl
{
	int					fd;

	RS232_BAUDRATE		baud;
	RS232_DATABITS		databits;
	RS232_STOPBITS		stopbits;
	char					parity;
}HwRs232Ctrl;



#define	DEBUG_SYS_NET		EXT_DBG_ON



/* following devices attached on I2C of MCU directly */
#define	EXT_I2C_PCA9554_ADDRESS				(0xE0 >> 1)

#define	EXT_I2C_ADDRESS_LM1983				(0xCA >> 1)		/* ADDR pin is Tie low */
#define	EXT_I2C_ADDRESS_LM1983_FLOAT		(0xCE >> 1)
#define	EXT_I2C_ADDRESS_FPGA					(0x60 >> 1)
#define	EXT_I2C_ADDRESS_FPGA_B				(0x62 >> 1)


#define	EXT_I2C_PCA9554_CS_MAC				(4)	/* CS0, MAC address; not used */
#define	EXT_I2C_PCA9554_CS_SENSOR			(5)	/* CS1, sensor and EEPROM  */
#define	EXT_I2C_PCA9554_CS_FPGA				(6)	/* CS2, FPGA */
#define	EXT_I2C_PCA9554_CS_BOOTROM			(7)	/* CS3, Ethernet switch */

#define	EXT_I2C_PCA9554_CS_NONE				(0xFF) /* connect to I2C of MCU directly */


#endif

