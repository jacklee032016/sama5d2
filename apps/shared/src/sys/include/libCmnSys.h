#ifndef  __LIB_CMN_SYS_H__
#define	__LIB_CMN_SYS_H__


#include "libCmn.h"
#include "libMux.h"


#define	EXT_DELAY_S(sec)		cmn_delay((sec)*1000)
#define	EXT_DELAY_MS(ms)		cmn_delay((ms))
#define	EXT_DELAY_US(us)		cmn_usleep((us))


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


#define		MUX_ETH_DEVICE				"eth0"

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

#define INFO_FRAME_PORT			6000

// int multicast_ip_change_flag;

struct _CmnMultiGroup;

typedef struct _CmnMultiGroup
{
	char			groupAddress[CMN_NAME_LENGTH];
	char			devName[CMN_NAME_LENGTH];

	uint32_t		address;
	int			ifIndex;	/* index of net interface */
	int			socket;

	int		(*changeGroup)(struct _CmnMultiGroup *group, char *newAddress);
}CmnMultiGroup;


int cmnSysI2cWrite(int deviceID, unsigned char offset, unsigned char *buffer, int length);
int cmnSysI2cRead(int deviceID, unsigned char offset, unsigned char *buffer, int length);


int extI2CRead(unsigned char chanNo, unsigned char deviceAddress, unsigned int regAddress, unsigned char regAddressSize, unsigned char *regVal, unsigned char regSize);
int extI2CWrite(unsigned char chanNo, unsigned char deviceAddress, unsigned int regAddress, unsigned char regAddressSize,  unsigned char *regVal, unsigned char regSize);


/* system network */
uint32_t cmnSysNetGetIp(char * hwName);
uint32_t cmnSysNetGetMask(char *hwName);

uint32_t cmnSysNetGetDefaultGw(char *hwName);
int cmnSysNetSetGateway(char *gateway, char *devName);


char *cmnSysNetAddress( uint32_t address);
uint32_t cmnSystemNetIp(char *ip);

int cmnSysNetGetMacAddress(char *hwName, EXT_MAC_ADDRESS *mac);


CmnMultiGroup *cmnSysNetMGroupInit(const char *devName, char *groupIp);
void cmnSysNetMGroupDestory(CmnMultiGroup *_group);

int cmnSysNetMulticastIP4Mac(uint32_t	ipAddress, EXT_MAC_ADDRESS *macAddress);
void cmnSysNetChangeByteOrderOfMac(EXT_MAC_ADDRESS *mac, unsigned char *address);


/* system control routines */
int	cmnSysCtrlBlinkPowerLED(char	isEnable);
int	cmnSysCtrlDelayReset(unsigned short delayMs, void *data);
int	cmnSysCtrlDelayReboot(unsigned short delayMs, void *data);


int cmnSysRs232Init(EXT_RUNTIME_CFG *runCfg);
int cmnSysRs232Config(EXT_RUNTIME_CFG *runCfg);
int cmnSysRs232Write(unsigned char *data, unsigned short size);

int	sysFpgaCheck(EXT_RUNTIME_CFG *runCfg );

int	sysFpgaInit(EXT_RUNTIME_CFG *runCfg );


int cmnSysJsonUpdate(MuxMain *muxMain);


#endif

