
#ifndef	__MUX_7XX_H__
#define	__MUX_7XX_H__

#define	IP_ADDRESS_IN_NET_ORDER		1

/* LED */
#define VIDEO_LED_MAGIC 'V'
#define VIDEO_LED_ON  	_IO(VIDEO_LED_MAGIC,0)  
#define VIDEO_LED_OFF 	_IO(VIDEO_LED_MAGIC,1)

#define POWER_LED_MAGIC 'P'
#define POWER_LED_ON  	_IO(POWER_LED_MAGIC, 0)  
#define POWER_LED_OFF 	_IO(POWER_LED_MAGIC, 1)

#define ACT_LED_MAGIC 'A'
#define ACT_LED_ON  	_IO(ACT_LED_MAGIC, 0)  
#define ACT_LED_OFF 	_IO(ACT_LED_MAGIC, 1)


/* button */
#define BUTTON_MAGIC	'B'
#define READ_BUTTON  	_IOR(BUTTON_MAGIC, 1, unsigned char *)


#define		DEV_HOME				"/dev"

#define		MUX_DEV_BUTTON		DEV_HOME"/muxbutton"
#define		MUX_DEV_SWITCH		DEV_HOME"/muxswitch"
#define		MUX_DEV_LED			DEV_HOME"/powerled"

#define		MUX_DEV_RS232			DEV_HOME"/ttyS1"
#define		MUX_DEV_WATCHDOG	DEV_HOME"/watchdog"
#define		MUX_DEV_EXP_RESET	DEV_HOME"/exprst"
#define		MUX_DEV_HDMI_RESET	DEV_HOME"/hdmirst"	/* no in 774 */

#define		MUX_DEV_I2C_0			DEV_HOME"/i2c-0"
#define		MUX_DEV_I2C_1			DEV_HOME"/i2c-1"


#define MAX_SPI_WRITE 64

#define		SPI_0_FILENAME				DEV_HOME"/spidev32766.0"

#define SPI_TST_ADDR 0x0001
#define SPI_TST_D0 0x5A
#define SPI_TST_D1 0xC3


#define		SET_LED_BLINK(muxMain)		((muxMain)->ledCtrl.ledMode = LED_MODE_BLINK)

#define		LINUX_SYS_ROUTE_PROC			"/proc/net/route"

typedef struct
{
	unsigned char Cmd_Code;
	unsigned char Src_Addr;
	unsigned char Dest_Addr;
} CEC_Cmd_Data;

unsigned int ASCII_2_HEX(unsigned char * ASCIIBuffer);

#define TX_COM_MSG_MAX_DATA_SIZE 508
#define TX_COM_PACKET_SIZE  TX_COM_MSG_MAX_DATA_SIZE + 4

#define INFO_FRAME_PORT 6000

// int multicast_ip_change_flag;


#include "extSysParams.h"



typedef	enum
{
	LED_MODE_OFF	 	=	0,
	LED_MODE_ON		=	1,
	LED_MODE_BLINK	= 2
}LED_MODE_T;

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



#endif

