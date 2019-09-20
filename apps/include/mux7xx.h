/*
* definitions about board dependent
* include from libMux.h, definitions for MuxMain
*/

#ifndef	__MUX_7XX_H__
#define	__MUX_7XX_H__

/* board type, so libMux and libCmnSys can be customized */
#define	EXT_BOARD_TYPE			MUX_BOARD_TYPE_774

#if 1
#define	MUX_ATMEL_XPLAINED		1
#define	MUX_BOARD_768				2
#define	MUX_BOARD_774				3
#define	MUX_BOARD_767				4

//#define	MUX_BOARD				MUX_BOARD_774
//#define	MUX_BOARD				MUX_ATMEL_XPLAINED
#endif


#define	EEPROM_AT24MAC402_ADDRESS			(0xBE >> 1)	/* Evaludation board AT24MAC, 4Kb */
#define	EEPROM_24LC32A_ADDRESS				(0xAE >> 1)	/* 4Kb, on channel-1, shared with Sensor */
#define	EEPROM_24AA02E48T_ADDRESS			(0xA0 >> 1)	/* 2Kb, on channel-0 */


#define	MUX_NAME				"Mux"

#define	I2C_CHAN_4_BASIC					4

#define	I2C_CHAN_NO_0						(I2C_CHAN_4_BASIC+0)
#define	I2C_CHAN_NO_1						(I2C_CHAN_4_BASIC+1)
#define	I2C_CHAN_NO_2						(I2C_CHAN_4_BASIC+2)
#define	I2C_CHAN_NO_3						(I2C_CHAN_4_BASIC+3)

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


#if	(MUX_BOARD == MUX_ATMEL_XPLAINED)
	#define	EXT_SYSTEM						"Sama5d2Xpld"
	#define	EXT_PRODUCT_NAME				"Sama5d2Xpld"
	#define	EXT_MODEL						"Xplained"
	
	#define	I2C_CHAN_4_SENSOR				0
	#define	I2C_CHAN_4_SWITCH			0
	
	#define	EXT_I2C_DEV_SENSOR			(0x00 >> 1)
#elif	(MUX_BOARD == MUX_BOARD_768)
	#define	EXT_SYSTEM						MUX_NAME"768"
	#define	EXT_PRODUCT_NAME				"HdmiOverIP-768"
	#define	EXT_MODEL						"500768"
	
	#define	I2C_CHAN_4_SENSOR				I2C_CHAN_NO_1
	#define	I2C_CHAN_4_SWITCH			I2C_CHAN_NO_3
	
	#define	EXT_I2C_DEV_SENSOR			(0x30 >> 1)
#elif (MUX_BOARD == MUX_BOARD_774)
	#define	EXT_SYSTEM						MUX_NAME"774"
	#define	EXT_PRODUCT_NAME				"SdiOverIP-774"
	#define	EXT_MODEL						"500774"

	#define	I2C_CHAN_4_SI5351B			I2C_CHAN_NO_0
	#define	I2C_CHAN_4_PCM3060			I2C_CHAN_NO_0				/* Audio codec */
	
	#define	I2C_CHAN_4_SENSOR				I2C_CHAN_NO_1
	#define	I2C_CHAN_4_EEPROM			I2C_CHAN_NO_1
	#define	I2C_CHAN_4_MCP4716			I2C_CHAN_NO_1				/* DAC */
	#define	I2C_CHAN_4_EP91A6				I2C_CHAN_NO_1				/* HDMI repeator */
	
	#define	I2C_CHAN_4_SWITCH			I2C_CHAN_NO_2				/* ethernet switch */

	#define	I2C_CHAN_4_AQLX107GK7		I2C_CHAN_NO_3				/* AQLX107GK7, FPGA */

	
	#define	EXT_I2C_DEV_SENSOR			(0x90 >> 1)					/* RX only; RX 0x30 */
	#define	EXT_I2C_DEV_PCM3060			(0x8C >> 1)					/* RX only; RX 0x30 */

	#define	EXT_I2C_DEV_MCP4617			(0xC0 >> 1)					/* MCP4716 */
	#define	EXT_I2C_DEV_SI5351B			(0xC0 >> 1)					/* Si5351B Clock generator */
	#define	EXT_I2C_DEV_EP91A6			(0xCB >> 1)					/* EP91A6SQ, HDMI repeator */

	#define	EXT_I2C_EEPROM_ADDRESS		EEPROM_24LC32A_ADDRESS		/* 24LC32A */




#elif (MUX_BOARD == MUX_BOARD_767)
	#define	EXT_SYSTEM						MUX_NAME"767"
	#define	EXT_PRODUCT_NAME				"SdiOverIP-767"
	#define	EXT_MODEL						"500767"
	
	#define	I2C_CHAN_4_SENSOR				(I2C_CHAN_4_BASIC+1)
	#define	I2C_CHAN_4_SWITCH			(I2C_CHAN_4_BASIC+3)

	#define	EXT_I2C_DEV_SENSOR             	(0x30 >> 1)

#else
#error 	Not support board definition
#endif


/* debug option, in EXT_DEBUG_XXXX  */
#define	EXT_DEBUG_INIT				EXT_DBG_ON



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
	LED_TYPE_LINK,
	LED_TYPE_UNKNOWN
}LED_TYPE_T;


typedef	struct _LedCtrl
{
	int					fd;
	LED_MODE_T			ledMode;
}HwLedCtrl;


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


#define	MUX_PCT2075_REG_TEMP		0x00
#define	MUX_PCT2075_REG_TOS		0x03


#define	MUX_PCT2075_TOS_DEFAULT		0x5000

#define	DEBUG_SYS_NET					EXT_DBG_ON

#define	DEBUG_SYS_CTRL				EXT_DBG_ON


#define	EXT_RS232_DEBUG				EXT_DBG_ON


struct DATA_BUFFER
{
	unsigned char		buffer[1024];
	int				size;

	int				writeIndex;
	int				readIndex;
};




#endif

