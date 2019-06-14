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


char *cmnSysLedFindState(LED_MODE_T mode);
LED_MODE_T cmnSysLedCheck(LED_TYPE_T type);
int cmnSysLedCtrl(LED_TYPE_T type, LED_MODE_T mode);


int cmnSysSW1Check(void);
int cmnSysDipSwitchCheck(void);


int	cmnSysW1GetRomId(unsigned char *romId);

/* definitions for security chip */
#define	SC_ROM_ID_SIZE				8
#define	SC_PAGE_SIZE				32
#define	SC_PERSONNAL_SIZE			4
#define	SC_CHALLENGE_SIZE			32

#define	SC_SECRET_SIZE				32


typedef struct
{
	char		romId[CMN_NAME_LENGTH];
	
	char		pageFile[CMN_NAME_LENGTH];	
	char		keyFile[CMN_NAME_LENGTH];	
	char		macFile[CMN_NAME_LENGTH];	

	char		crcFile[CMN_NAME_LENGTH];	/* controll CRC enable/disable*/

	SC_CTRL			sc;
}SecurityFiles;


#define	IS_SECURITY_CHIP_EXIST(sc)		\
	( (sc)->isExist == EXT_TRUE )


SecurityFiles *cmnSysScInit(void);

int	cmnSysScRWPage(SecurityFiles *scf, unsigned char *page, int isRead);
int	cmnSysScRWKey(SecurityFiles *scf, unsigned char *page, int isRead);
int	cmnSysScRWMac(SecurityFiles *scf, unsigned char *page, int isRead);
int	cmnSysScCrc(SecurityFiles *scf, int isEnable);

char cmnSysScComputeMAC(SC_CTRL *sc);
int cmnSysScChallegeInit(SC_CTRL *sc);

#define	SC_PAGE_READ(scf, data)	\
	cmnSysScRWPage((scf), (data), EXT_TRUE)

#define	SC_PAGE_WRITE(scf, data)	\
	cmnSysScRWPage((scf), (data), EXT_FALSE)

#define	SC_KEY_READ(scf, data)	\
	cmnSysScRWKey((scf), (data), EXT_TRUE)

#define	SC_KEY_WRITE(scf, data)	\
	cmnSysScRWKey((scf), (data), EXT_FALSE)

#define	SC_MAC_READ(scf, data)	\
	cmnSysScRWMac((scf), (data), EXT_TRUE)

#define	SC_MAC_WRITE(scf, data)	\
	cmnSysScRWMac((scf), (data), EXT_FALSE)


#define	SC_CRC_DISABLE(scf)	\
	cmnSysScCrc((scf), EXT_FALSE)

#define	SC_CRC_ENABLE(scf)	\
	cmnSysScCrc((scf), EXT_TRUE)



int cmnSysI2cRW(int bus, unsigned char slaveAddr, unsigned int regAddr, int regAddrSize, unsigned char *buffer, int length, int isRead);
int cmnSysI2cExtRW(int bus, unsigned char chanNo, unsigned char slaveAddr, unsigned int regAddr, int regAddrSize, unsigned char *buffer, int length, int isRead);
int cmnSysI2cSensor(void);

/* I2C read/write */
#define	I2C_READ(bus, slave, offset, offSize, buf, size)		\
			cmnSysI2cRW((bus), (slave), (offset), (offSize), (unsigned char *)(buf), (size), EXT_TRUE)

#define	I2C_WRITE(bus, slave, offset, offSize, buf, size)		\
			cmnSysI2cRW((bus), (slave), (offset), (offSize), (unsigned char *)(buf), (size), EXT_FALSE)

#define	I2C_READ_SHORT(bus, ch, slave, offset, offSize, shortBuf, ret)		\
			{(ret) = I2C_READ((bus), (ch), (slave), (offset), (offSize), (shortBuf), (2)); *shortBuf = EXT_BYTE_ORDER_SHORT(*shortBuf);}

#define	I2C_READ_INTEGER(bus, ch, slave, offset, offSize, intBuf, ret)		\
			{(ret) = I2C_READ((bus), (ch), (slave), (offset), (offSize), (intBuf), (4)); *intBuf = _PP_HTONL(*intBuf);}


#define	I2C_WRITE_SHORT(bus, ch, slave, offset, offSize, shortBuf, ret)		\
			{ unsigned short _val = EXT_BYTE_ORDER_SHORT(*shortBuf); (ret) = I2C_WRITE((bus), (ch), (slave), (offset), (offSize), (unsigned char *)&_val, (2));}

#define	I2C_WRITE_INTEGER(bus, ch, slave, offset, offSize, intBuf, ret)		\
			{ unsigned int _val = _PP_HTONL(*intBuf); (ret) = I2C_READ((bus), (ch), (slave), (offset), (offSize), (unsigned char *)&_val, (4));}


/* I2C extended read/write: read/write with multiplexer  */
#define	I2C_EXT_READ(bus, ch, slave, offset, offSize, buf, size)		\
			cmnSysI2cExtRW((bus), (ch), (slave), (offset), (offSize), (unsigned char *)(buf), (size), EXT_TRUE)

#define	I2C_EXT_WRITE(bus, ch, slave, offset, offSize, buf, size)		\
			cmnSysI2cExtRW((bus), (ch), (slave), (offset), (offSize), (unsigned char *)(buf), (size), EXT_FALSE)


#define	I2C_EXT_READ_SHORT(bus, ch, slave, offset, offSize, shortBuf, ret)		\
			{(ret) = I2C_EXT_READ((bus), (ch), (slave), (offset), (offSize), (shortBuf), (2)); *shortBuf = EXT_BYTE_ORDER_SHORT(*shortBuf);}

#define	I2C_EXT_READ_INTEGER(bus, ch, slave, offset, offSize, intBuf, ret)		\
			{(ret) = I2C_EXT_READ((bus), (ch), (slave), (offset), (offSize), (intBuf), (4)); *intBuf = _PP_HTONL(*intBuf);}


#define	I2C_EXT_WRITE_SHORT(bus, ch, slave, offset, offSize, shortBuf, ret)		\
			{ unsigned short _val = EXT_BYTE_ORDER_SHORT(*shortBuf); (ret) = I2C_EXT_WRITE((bus), (ch), (slave), (offset), (offSize), (unsigned char *)&_val, (2));}

#define	I2C_EXT_WRITE_INTEGER(bus, ch, slave, offset, offSize, intBuf, ret)		\
			{ unsigned int _val = _PP_HTONL(*intBuf); (ret) = I2C_EXT_READ((bus), (ch), (slave), (offset), (offSize), (unsigned char *)&_val, (4));}


#define	SI5351B_READ(reg, value, length)	\
		I2C_EXT_READ(0, I2C_CHAN_4_SI5351B, EXT_I2C_DEV_SI5351B, (reg), 1, (value), (length) )

#define	SI5351B_WRITE(reg, value, length)	\
		I2C_EXT_WRITE(0, I2C_CHAN_4_SI5351B, EXT_I2C_DEV_SI5351B, (reg), 1, (value), (length) )

/* MCP47716 DAC with EEPROM*/
#define	MCP4716_READ(reg, value, length)	\
		I2C_EXT_READ(0, I2C_CHAN_4_MCP4716, EXT_I2C_DEV_MCP4617, (reg), 1, (value), (length) )

#define	MCP4716_WRITE(reg, value, length)	\
		I2C_EXT_WRITE(0, I2C_CHAN_4_MCP4716, EXT_I2C_DEV_MCP4617, (reg), 1, (value), (length) )


#define	EEPROM_READ(start, value, length)	\
		I2C_EXT_READ(0, I2C_CHAN_4_EEPROM, EXT_I2C_EEPROM_ADDRESS, (start), 2, (value), (length))

#define	EEPROM_WRITE(start, value, length)	\
		I2C_EXT_WRITE(0, I2C_CHAN_4_EEPROM, EXT_I2C_EEPROM_ADDRESS, (start), 2, (value), (length))



int	cmnSysI2cTest(void);


#endif

