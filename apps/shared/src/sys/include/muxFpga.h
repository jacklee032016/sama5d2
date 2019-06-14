
#ifndef	__MUX_FPGA_H__
#define	__MUX_FPGA_H__

 #include <arpa/inet.h>
 
/* registers */
#define	EXT_FPGA_REG_ETHERNET_RESET			0

#define	EXT_FPGA_REG_ENABLE					3

#define	EXT_FPGA_FLAGS_MCU_ENABLE			0x43	/* for other vendors' product, and enable SDP configuration */
#define	EXT_FPGA_FLAGS_MCU_DISABLE			0x05	/* for muxlab product */
#define	EXT_FPGA_FLAGS_DISABLE_ALL			0x00	/* disable for TX and RX */


#define	EXT_FPGA_REG_POWER_LED				5	/* -- 0 - LED is on, 1 - LED is flashing*/

#define	EXT_FPGA_REG_WIDTH					6	/* 2 bytes */
#define	EXT_FPGA_REG_HEIGHT					8	/* 2 bytes */
#define	EXT_FPGA_REG_FRAMERATE				10	/* 4 bits */
/*-- x"2"   24/1.001
-- x"3"   24
-- x"5"   25
-- x"6"   30/1.001
-- x"7"   30
-- x"9"   50
-- x"A"   60/1.001
-- x"B"   60
*/
#define EXT_FPGA_REG_SAMPLING					11	/* 4 bits */
/*-- "0000" YCbCr-4:2:2
-- "0001" YCbCr-4:4:4
-- "0010" RGB
-- "0011" YCbCr-4:2:0
-- "0100" XYZ
-- "0101" KEY
-- "1000" CLYCbCr-4:2:2
-- "1001" CLYCbCr-4:4:4
-- "1011" CLYCbCr-4:2:0
*/
#define	EXT_FPGA_REG_DEPTH					12	/* 2 bits (1:0) */
/*-- b"00" 8                             bits
-- b"01" 10          bits
-- b"10" 12          bits
-- b"11" 16          bits
*/
#define EXT_FPGA_REG_INTLC_SEGM				13	/* 2 bits interlac bit 0 , segmented bit 1 */


#define EXT_FPGA_REG_PARAM_STATUS				14

#define	EXT_FPGA_FLAGS_PARAM_USABLE				0x01

/* audio */
#define	EXT_FPGA_REG_AUDIO_CHANNELS				15
#define	EXT_FPGA_REG_AUDIO_RATE					16
#define	EXT_FPGA_REG_AUDIO_PKT_SIZE				17


#define	EXT_FPGA_REG_SDI_STATUS					19

#define	EXT_FPGA_REG_ANC_VPID					25


/* when 0x00 is 'SDI DISCONNECTED; 0x01: SDI CONNECTION '*/
#define	EXT_FPGA_FLAGS_SDI_CONNECTTED			0x01


/* for both TX/RX */
#define	EXT_FPGA_REG_VERSION						29

#define	EXT_FPGA_TX_FLAGS							0x80	/* in version info */

#define	EXT_FPGA_REG_REVISION						30
#define	EXT_FPGA_REG_MODEL						31

#define	EXT_FPGA_REG_YEAR							32
#define	EXT_FPGA_REG_MONTH						33
#define	EXT_FPGA_REG_DAY							34
#define	EXT_FPGA_REG_HOUR						35
#define	EXT_FPGA_REG_MINUTE						36


#define	EXT_FPGA_REG_IP							52
#define	EXT_FPGA_REG_MAC							56
#define	EXT_FPGA_REG_PORT_VIDEO					62
#define	EXT_FPGA_REG_PORT_AUDIO					76

#define	EXT_FPGA_REG_PORT_ANC_DT				80
#define	EXT_FPGA_REG_PORT_ANC_ST				84

/* for only TX */
#define	EXT_FPGA_REG_DEST_IP						64
#define	EXT_FPGA_REG_DEST_MAC					68
#define	EXT_FPGA_REG_DEST_PORT_VIDEO			74
#define	EXT_FPGA_REG_DEST_PORT_AUDIO			78

#define	EXT_FPGA_REG_DEST_PORT_ANC_DT			82
#define	EXT_FPGA_REG_DEST_PORT_ANC_ST			86


struct Tx_Media_Stream_Register_Address
{
	uint8_t		addrIp;
	uint8_t		addrMac;
	uint8_t		addrPort;
	uint8_t		addrRtpPayload;
};

struct Rx_Media_Stream_Register_Address
{
	uint8_t		addrIp;
	uint8_t		addrPort;
	uint8_t		addrRtpPayload;
};


struct Media_Register_Address
{
	uint8_t		addrWidth;
	uint8_t		addrHeight;
	uint8_t		addrFramerate;
	uint8_t		addrColorSpace;
	uint8_t		addrVDepth;
	uint8_t		addrIntl;
	
	uint8_t		addrChannels;
	uint8_t		addrAudioRate;
	uint8_t		addrPktSize;
};

struct Tx_Register_Address
{
	/* local address and port */
	uint8_t		localMac;
	uint8_t		localIp;

	uint8_t		localPortV;
	uint8_t		localPortA;
	uint8_t		localPortAnc;
	uint8_t		localPortAuc;

	/* dest address and port for every stream */
	struct Tx_Media_Stream_Register_Address	*addrStreamVideo;
	struct Tx_Media_Stream_Register_Address	*addrStreamAudio;
	struct Tx_Media_Stream_Register_Address	*addrStreamAnc;
#if EXT_FPGA_AUX_ON	
	struct Tx_Media_Stream_Register_Address	*addrStreamAux;
#endif
	struct Media_Register_Address		*addrMedia;
};


struct Rx_Register_Address
{
	uint8_t		localMac;
	uint8_t		localIp;

	/* for RX, these are not used */
	uint8_t		localPortV;
	uint8_t		localPortA;
	uint8_t		localPortAnc;
	uint8_t		localPortAux;

	/* dest address and port for every stream */
	struct Rx_Media_Stream_Register_Address	*addrStreamVideo;
	struct Rx_Media_Stream_Register_Address	*addrStreamAudio;
	struct Rx_Media_Stream_Register_Address	*addrStreamAnc;
#if EXT_FPGA_AUX_ON	
	struct Rx_Media_Stream_Register_Address	*addrStreamAux;
#endif
	struct Media_Register_Address		*addrMedia;
};

struct	_FpgaConfig;

typedef	int	(*FpgaCtrlHandler)(struct	_FpgaConfig *_fpgaCfg);


typedef	struct	_FpgaConfig
{
	struct Tx_Register_Address		*txAddress;
	struct Rx_Register_Address		*rxAddress;


	uint8_t			addrVersion;
	uint8_t			addrRevision;

	uint8_t			addrYear;
	uint8_t			addrMonth;
	uint8_t			addrDay;
	uint8_t			addrHour;
	uint8_t			addrMinute;


	EXT_RUNTIME_CFG				*runCfg;

	FpgaCtrlHandler				opProtocolCtrl;		/* IP address, port of all streams */
	FpgaCtrlHandler				opMediaWrite;		/* media params except address and port */
	FpgaCtrlHandler				opMediaRead;		/* read media params except address and port, for SDP and display */

	CmnMultiGroup 				*groupVideoMgr;
	CmnMultiGroup 				*groupAudioMgr;
	CmnMultiGroup 				*groupAncMgr;
}FpgaConfig;


/* 8 bits address, and 8 bit data */
#define	FPGA_I2C_WRITE(address, val, size)		\
	I2C_EXT_WRITE(0, EXT_I2C_PCA9554_CS_NONE, EXT_I2C_ADDRESS_FPGA, (address), 1, (val), size)

//	extI2CWrite(EXT_I2C_PCA9554_CS_NONE, EXT_I2C_ADDRESS_FPGA, (address), 1, (val), size)

#define	FPGA_I2C_READ(address, val, size)		\
	I2C_EXT_READ(0, EXT_I2C_PCA9554_CS_NONE, EXT_I2C_ADDRESS_FPGA, (address), 1, (val), size)
//	extI2CRead(EXT_I2C_PCA9554_CS_NONE, EXT_I2C_ADDRESS_FPGA, (address), 1, (val), size)

#define	_extFpgaReadByte(address, value)	\
	FPGA_I2C_READ((address), (value), 1)


#define	_extFpgaReadShortChanging(address, shortVal) \
{	_fpgaRegisterRead((address), (shortVal), 2); \
	 *((unsigned short *)shortVal) = ntohs(*((unsigned short *)shortVal)); }


#define	_extFpgaReadShort(address, shortVal) \
{	_fpgaRegisterRead((address), (shortVal), 2); }

#define	_extFpgaRead3Bytes(address, intVal) \
{	unsigned char *p = (unsigned char *)(intVal); *intVal=0;  _fpgaRegisterRead((address), p, 3); *intVal = (ntohl(*intVal)|(239) ) ; }


#define	_extFpgaReadInteger(address, intVal) \
{	_fpgaRegisterRead((address), intVal, 4); *((unsigned int *)intVal) = ntohl(*((unsigned int *)intVal)); }



#define	_extFpgaWriteByte(address, value)	\
	FPGA_I2C_WRITE((address), (value), 1)


#define	_extFpgaWriteShortChanging(address, shortVal) \
{	unsigned short _value = ntohs(*((unsigned short *)(shortVal))); _fpgaRegisterWrite((address), (unsigned char *)&_value, 2); }


#define	_extFpgaWriteShort(address, shortVal) \
{	_fpgaRegisterWrite((address), shortVal, 2); }


#define	_extFpgaWrite3Bytes(address, intVal) \
{	unsigned int _value = ntohl(*((unsigned int *)(intVal))); unsigned char *p = (unsigned char *)&_value; _fpgaRegisterWrite((address), p, 3); }


#define	_extFpgaWriteInteger(address, intVal) \
{	unsigned int _value = ntohl(*((unsigned int *)(intVal))); _fpgaRegisterWrite((address), (unsigned char *)&_value, 4); }


void _fpgaRegisterRead(unsigned char address, unsigned char *data, unsigned char size);
void	_fpgaRegisterWrite(unsigned char baseAddr, unsigned char *data, unsigned char size);

int fpgaReadParamRegisters(struct Media_Register_Address *addrMedia, EXT_RUNTIME_CFG *runCfg);


int	sysFpgaTxConfig(FpgaConfig *fpga);
int	sysFpgaRxConfig(FpgaConfig *fpga);
int sysFpgaTxReadParams(FpgaConfig *fpga);
int sysFpgaRxReadParams(FpgaConfig *fpga);
int sysFpgaTxWriteParams(FpgaConfig *fpga);
int sysFpgaRxWriteParams(FpgaConfig *fpga);

#endif

