#ifndef  __LIB_CMN_SYS_H__
#define	__LIB_CMN_SYS_H__


#include "libCmn.h"
#include "libMux.h"

#define	EXT_DEBUG_TIMESTAMP				1

#define	EXT_CMN_SYS_HW_DEBUG			EXT_DBG_ON

#define	EXT_DELAY_S(sec)		cmn_delay((sec)*1000)
#define	EXT_DELAY_MS(ms)		cmn_delay((ms))
#define	EXT_DELAY_US(us)		cmn_usleep((us))


#define		SYS_REBOOT_DELAY_MS				3000	/* for delay both reboot and reset */
#define		SYS_REBOOT_DELAY_MS_4_BTN		1000	/* for delay of reset after button has pressed about 6 seconds */

#define		SYS_NET_CONFIG_FILE				RUN_HOME_DIR"/etc/network/interfaces"


#define		DEV_HOME				RUN_HOME_DIR_TEMP"/dev"
#define		LED_HOME				RUN_HOME_DIR_TEMP"/sys/class/leds"

#define		I2C_DEV_HOME			RUN_HOME_DIR_TEMP"/dev/i2c"


#define		MUX_DEV_RS232			DEV_HOME"/ttyS1"
#define		MUX_DEV_WATCHDOG	DEV_HOME"/watchdog"
#define		MUX_DEV_EXP_RESET	DEV_HOME"/exprst"
#define		MUX_DEV_HDMI_RESET	DEV_HOME"/hdmirst"	/* no in 774 */

#define		MUX_DEV_I2C_0			DEV_HOME"/i2c-0"
#define		MUX_DEV_I2C_1			DEV_HOME"/i2c-1"


#define		MUX_ETH_DEVICE				"eth0"

#define		MAX_SPI_WRITE 64

//#define		SPI_0_FILENAME					DEV_HOME"/spidev32766.0"
#define		SPI_0_FILENAME					DEV_HOME"/spidev0.0"

#define SPI_TST_ADDR		0x0001
#define SPI_TST_D0			0x5A
#define SPI_TST_D1			0xC3


#define    INPUT_EVENT_SW1    				DEV_HOME"/input/event0"
#define    INPUT_EVENT_DIP_SWITCH    		DEV_HOME"/input/event1"


/* button event */
typedef	enum _BTN_EVENT_T
{
	BTN_EVENT_NONE,
	BTN_EVENT_PRESSED,
	BTN_EVENT_RELEASED,
	BTN_EVENT_UNKNOWN
}BTN_EVENT_T;


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

int cmnSysGetPidByName(char *progName);
int cmnSysSigSend(pid_t pid, int sigid, int opcode);
int cmnSysKillProcess(char *name);
void cmnSysForkCmd(const char *cmd);

void rs232StartRx(void);
void rs232StopRx(void);


int cmnSysRs232Init(EXT_RUNTIME_CFG *runCfg);
int cmnSysRs232Config(EXT_RUNTIME_CFG *runCfg);
int cmnSysRs232Write(unsigned char *data, unsigned short size);
int cmnSysRs232Read(unsigned char *data, unsigned short size, int timeoutMs);

int	sysFpgaCheck(EXT_RUNTIME_CFG *runCfg );

int	sysFpgaInit(EXT_RUNTIME_CFG *runCfg );

int sysFpgaVideoForced(void);
int sysFpgaCfgSfpWhenBoot(void);

int	sysFpgaRefresh(void );

char *sysFgpaVersion(void);
char *sysFgpaBuilt(void);

int	sysFpgaReadIrDemodulation(unsigned char *freq);

int sysFpgaTxPollUpdateParams(void *data);


int cmnSysJsonUpdate(MuxMain *muxMain);


char *cmnSysLedFindState(LED_MODE_T mode);
LED_MODE_T cmnSysLedCheck(LED_TYPE_T type);
int cmnSysLedCtrl(LED_TYPE_T type, LED_MODE_T mode);


#define	CMN_SYS_LED_CTRL(led, mode)	\
			cmnSysLedCtrl((led), (mode)	)

/* LED signal, send signal to FPGA, 11.25, 2019 */
#define	CMN_SYS_LED_SIGNAL_CTRL( mode)	\
			CMN_SYS_LED_CTRL(LED_TYPE_SIGNAL, (mode)	)

#define	CMN_SYS_LED_SIGNAL_OFF( mode)	\
			CMN_SYS_LED_SIGNAL_CTRL(LED_MODE_OFF)

#define	CMN_SYS_LED_SIGNAL_ON( mode)	\
			CMN_SYS_LED_SIGNAL_CTRL(LED_MODE_ON)


/* LED power, used by IP command to blink */
#define	CMN_SYS_LED_POWER_CTRL( mode)	\
			CMN_SYS_LED_CTRL(LED_TYPE_POWER, (mode)	)

#define	CMN_SYS_LED_POWER_OFF( mode)	\
			CMN_SYS_LED_POWER_CTRL(LED_MODE_OFF)

#define	CMN_SYS_LED_POWER_ON( mode)	\
			CMN_SYS_LED_POWER_CTRL(LED_MODE_ON)

#define	CMN_SYS_LED_POWER_BLINK( mode)	\
			CMN_SYS_LED_POWER_CTRL(LED_MODE_BLINK)

/* LED video */
#define	CMN_SYS_LED_VIDEO_CTRL( mode)	\
			CMN_SYS_LED_CTRL(LED_TYPE_VIDEO, (mode))

#define	CMN_SYS_LED_VIDEO_OFF( mode)	\
			CMN_SYS_LED_VIDEO_CTRL(LED_MODE_OFF)

#define	CMN_SYS_LED_VIDEO_ON( mode)	\
			CMN_SYS_LED_VIDEO_CTRL(LED_MODE_ON)

#define	CMN_SYS_LED_VIDEO_BLINK( mode)	\
			CMN_SYS_LED_VIDEO_CTRL(LED_MODE_BLINK)

/* LED act */
#define	CMN_SYS_LED_ACT_CTRL( mode)	\
			CMN_SYS_LED_CTRL(LED_TYPE_ACT, (mode))

#define	CMN_SYS_LED_ACT_OFF( mode)	\
			CMN_SYS_LED_ACT_CTRL(LED_MODE_OFF)

#define	CMN_SYS_LED_ACT_ON( mode)	\
			CMN_SYS_LED_ACT_CTRL(LED_MODE_ON)

#define	CMN_SYS_LED_ACT_BLINK( mode)	\
			CMN_SYS_LED_ACT_CTRL(LED_MODE_BLINK)

/* LED link */
#define	CMN_SYS_LED_LINK_CTRL( mode)	\
			CMN_SYS_LED_CTRL(LED_TYPE_LINK, (mode))

#define	CMN_SYS_LED_LINK_OFF( mode)	\
			CMN_SYS_LED_LINK_CTRL(LED_MODE_OFF)

#define	CMN_SYS_LED_LINK_ON( mode)	\
			CMN_SYS_LED_LINK_CTRL(LED_MODE_ON)

#define	CMN_SYS_LED_LINK_BLINK( mode)	\
			CMN_SYS_LED_LINK_CTRL(LED_MODE_BLINK)



int cmnSysSW1CheckEvent(void);
int cmnSysDipSwitchCheck(void);


int	cmnSysW1GetRomId(unsigned char *romId);


SecurityFiles *cmnSysScInit(void);
int cmnSysScValidate(SecurityFiles *scf);
int cmnSysScWriteKey(SecurityFiles *scf, unsigned char  *secret, int size);


int	cmnSysScRWPage(SecurityFiles *scf, unsigned char *page, int isRead);
int	cmnSysScRWKey(SecurityFiles *scf, unsigned char *page, int isRead);
int	cmnSysScRWMac(SecurityFiles *scf, unsigned char *page, int isRead);
int	cmnSysScCrc(SecurityFiles *scf, int isEnable);

char cmnSysScComputeMAC(SC_CTRL *sc);
int cmnSysScChallengeInit(SC_CTRL *sc);

#if ARCH_ARM
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
#else
#define	SC_PAGE_READ(scf, data)		(EXIT_SUCCESS)

#define	SC_PAGE_WRITE(scf, data)	(EXIT_SUCCESS)

#define	SC_KEY_READ(scf, data)		(EXIT_SUCCESS)

#define	SC_KEY_WRITE(scf, data)		(EXIT_SUCCESS)

#define	SC_MAC_READ(scf, data)		(EXIT_SUCCESS)

#define	SC_MAC_WRITE(scf, data)		(EXIT_SUCCESS)

#endif


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


#define	EP91A6_READ(reg, value, length)	\
		I2C_EXT_READ(0, I2C_CHAN_4_EP91A6, EXT_I2C_DEV_EP91A6, (reg), 1, (value), (length) )

#define	EP91A6_WRITE(reg, value, length)	\
		I2C_EXT_WRITE(0, I2C_CHAN_4_EP91A6, EXT_I2C_DEV_EP91A6, (reg), 1, (value), (length) )


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

cJSON *cmnMuxThreadsInfo(MuxMain *muxMain);


int	sysFpgaReadVideoStatus(void);
int	sysFpgaReadSfpStatus(void);

#define	MAC_DEBUG_FORMAT(address)	\
			(" %02x:%02x:%02x:%02x:%02x:%02x:", address[0], address[1], address[2], address[3], address[4], address[5])

#if 1
/* N25Q512A and N25Q256A */
#define	SPI_FLASH_SECTOR_SIZE						0x10000	/* 65536, 64K */
#else
#define	SPI_FLASH_SECTOR_SIZE							(0x10000/2)	/* 65536, 64K */
#endif

int	cmnSysSpiTest(void);

int	cmnSysRawSpiFlashInit(void);
void	cmnSysRawSpiFlashClose(int fd);
int	cmnSysRawSpiFlashWriteFile(int flashFd, char *filename, uint32_t startAddr);
int	cmnSysRawSpiFlashRead(int flashFd, uint32_t startAddr, uint32_t pageCount);
int cmnSysRawSpiFlashEraseSectors(int flashFd, int sectorCount, uint32_t startAddr);


int	cmnSysI2cTxReadAudioParams(unsigned char *sampleRate, unsigned char *channels , unsigned char *depth);

int cmnSysI2cSi5351VcxoPllCheck(void);
int cmnSysI2cSi5351VcxoControl(unsigned long clkFreq);


/* in unit of micro second, eg. 1/90000 */
#define	FPGA_TIMESTAMP_UNIT_VIDEO			11111

#define	FPGA_TIMESTAMP_UNIT_AUDIO			20833


#define	FPGA_GET_PTP_TIMESTAMP_VIDEO()		\
		(uint32_t)(cmnGetTimeUs()*1000/FPGA_TIMESTAMP_UNIT_VIDEO)


#define	FPGA_GET_PTP_TIMESTAMP_AUDIO()		\
		(uint32_t)(cmnGetTimeUs()*1000/FPGA_TIMESTAMP_UNIT_AUDIO)


int	extSysNetInit(MuxMain		*muxMain);

int32_t	extPtpInit(EXT_RUNTIME_CFG		*runCfg);

#endif

