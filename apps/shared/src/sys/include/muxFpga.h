
#ifndef	__MUX_FPGA_H__
#define	__MUX_FPGA_H__

 #include <arpa/inet.h>

#define	FPGA_DEBUG			1

typedef	enum
{
	IR_MOD_FREQ_30K = 0,
	IR_MOD_FREQ_33K = 1,
	IR_MOD_FREQ_36K = 2,
	IR_MOD_FREQ_38K = 3,
	IR_MOD_FREQ_40K = 4,
	IR_MOD_FREQ_56K = 5,
	IR_MOD_FREQ_INVALIDATE= 0x0F,
}IR_MOD_FREQ_T;


#if (MUX_BOARD == MUX_BOARD_774)
/* salve address */
#define	EXT_FPAG_ADDRESS_SYSTEM				(0x60>>1)	/* reset, video, audio, IR */
#define	EXT_FPAG_ADDRESS_PCSPMA_2			(0x62>>1)
#define	EXT_FPAG_ADDRESS_PTP					(0x66>>1)	/* Ptp timestamp */
#define	EXT_FPAG_ADDRESS_NETWORK			(0x68>>1)	/* ip/port and MAC */
#define	EXT_FPAG_ADDRESS_DRP_PLL				(0x6A>>1)	/* Dynamic Reconfiguration Port : Rx PLL; only RX */
#define	EXT_FPAG_ADDRESS_DRP_XADC			(0x6C>>1)	/* DRP */
#define	EXT_FPAG_ADDRESS_PCSPMA_1			(0x6E>>1)
 
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

//#define	EXT_FPGA_REG_AUDIO_PKT_SIZE				17


#define	EXT_FPGA_REG_SDI_STATUS					19

#define	EXT_FPGA_REG_ANC_VPID					25


/* when 0x00 is 'SDI DISCONNECTED; 0x01: SDI CONNECTION '*/
#define	EXT_FPGA_FLAGS_SDI_CONNECTTED			0x01


/* for both TX/RX */
/********************** TX *************/
/* system device: 0x60 */
#define	F_REG_TX_SYS_RESET						0x00
#define	F_REG_TX_SYS_VIDEO_CTRL					0x01	/*color space and depth */
#define	F_REG_TX_SYS_IR_CTRL						0x02
#define	F_REG_TX_SYS_STREAM_ENABLE				0x03

#define	F_REG_TX_SYS_IR_DEMODULATION			0x05

#define	F_REG_TX_SYS_X_ACTIVE						0x06	/* width */
#define	F_REG_TX_SYS_Y_ACTIVE						0x08	/* height */

/* statistics */
#define	F_REG_TX_SYS_STATISTICS_VIDEO			0x16
#define	F_REG_TX_SYS_STATISTICS_AUDIO			0x18

#define	F_REG_TX_SYS_STATISTICS_ANC				INVALIDATE_VALUE_U8
#define	F_REG_TX_SYS_STATISTICS_AUX				INVALIDATE_VALUE_U8

/* 10.09, 2019 */
#define	F_REG_TX_AUDIO_PKT_SIZE					0x20

#define	F_REG_TX_SYS_GBE_RX_COUNT				0x22
#define	F_REG_TX_SYS_GBE_TX_COUNT				0x24


/* from email */
#define	F_REG_TX_SYS_V_WIDTH						0x2E
#define	F_REG_TX_SYS_V_HEIGHT					0x30

#define	F_REG_TX_SYS_V_FRAMERATE					0x32
/*-- x"2"   24/1.001
-- x"3"   24
-- x"5"   25
-- x"6"   30/1.001
-- x"7"   30
-- x"9"   50
-- x"A"   60/1.001
-- x"B"   60
*/

#define	F_REG_TX_SYS_V_DIV_1001					0x33

#define	F_REG_TX_SYS_V_COLORSPACE				0x34	/* sampling */
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
#define	F_REG_TX_SYS_V_DEPTH						0x35
/*-- b"00" 8                             bits
-- b"01" 10          bits
-- b"10" 12          bits
-- b"11" 16          bits
*/
#define	F_REG_TX_SYS_V_INTLC_SEGM				0x36

#define	F_REG_TX_SYS_PARAM_STATUS				0x37	/* new params is active */

#define	F_REG_TX_SYS_V_FRAMERATE_INT			0x38


#define	F_REG_TX_SYS_AUDIO_SELECT				0x1A
#define	F_REG_TX_SYS_AUDIO_MASK					0x1B
#define	F_REG_TX_SYS_AUDIO_INPUT					0x1C

#define	F_REG_TX_SYS_VERSION						0x1D
#define	F_REG_TX_SYS_REVISION						0x1E

#define	F_REG_TX_SYS_YEAR							0x3C
#define	F_REG_TX_SYS_MONTH						0x3D
#define	F_REG_TX_SYS_DAY							0x3E
#define	F_REG_TX_SYS_HOUR						0x3F
#define	F_REG_TX_SYS_MINUTE						0x40


#define	F_REG_TX_SYS_VIDEO_FORCED				0x41
#define	F_REG_TX_SYS_VIDEO_STATUS				0x43


#define	F_REG_TX_SYS_MB_VIDEO_COLOR_DEPTH		0x2C	/* 10/12/14/16 */
#define	F_REG_TX_SYS_MB_VIDEO_COLOR_FORMAT	0x2D	/* RGB/YCbCr */

/* RTP specific: 0x66 */
#define	F_REG_TX_RTP_VIDEO_PAYLOAD_TYPE			0x00
#define	F_REG_TX_RTP_XAUI_CTRL					0x05
#define	F_REG_TX_RTP_XAUI_STATUS					0x07

#define	F_REG_TX_RTP_TIMESTAMP_VIDEO				0x08	/* 4 bytes, 32 bits */
#define	F_REG_TX_RTP_AUDIO_PAYLOAD_TYPE			0x0E	/* 12.04.2019 */
#define	F_REG_TX_RTP_TIMESTAMP_AUDIO			0x0F	/* 4 bytes, 32 bits */

#define	F_REG_TX_GRAND_MASTER_TIMESTAMP		0x14

#define	F_REG_TX_RTP_ANC_PAYLOAD_TYPE			INVALIDATE_VALUE_U8
#define	F_REG_TX_RTP_AUX_PAYLOAD_TYPE			INVALIDATE_VALUE_U8


/* Network specific: 0x68 */
#define	F_REG_TX_NET_LOCAL_MAC					0x04	/* 6 bytes */	
#define	F_REG_TX_NET_DEST_MAC					0x0A	/* 6 bytes */	

#define	F_REG_TX_NET_DEST_MAC_AUDIO				0x28	/* 6 bytes */	 /* */


#define	F_REG_TX_NET_LOCAL_VIDEO_IP				0x10
#define	F_REG_TX_NET_DEST_VIDEO_IP				0x14

#define	F_REG_TX_NET_DEST_AUDIO_IP				0x2e

#define	F_REG_TX_NET_DEST_ANC_IP					INVALIDATE_VALUE_U8
#define	F_REG_TX_NET_DEST_AUX_IP					INVALIDATE_VALUE_U8

#define	F_REG_TX_NET_LOCAL_VIDEO_PORT			0x20
#define	F_REG_TX_NET_DEST_VIDEO_PORT				0x22

#define	F_REG_TX_NET_LOCAL_AUDIO_PORT			0x24
#define	F_REG_TX_NET_DEST_AUDIO_PORT			0x26

#define	F_REG_TX_NET_LOCAL_ANC_PORT				0x28
#define	F_REG_TX_NET_DEST_ANC_PORT				0x2A

#define	F_REG_TX_NET_LOCAL_AUX_PORT				INVALIDATE_VALUE_U8
#define	F_REG_TX_NET_DEST_AUX_PORT				INVALIDATE_VALUE_U8

#define	F_REG_TX_NET_QOS_OFS						0x2C

#define	F_REG_TX_NET_MCAST_VIDEO					0x2E	/* 3 bytes, not in border of 16 bytes */
#define	F_REG_TX_NET_MCAST_AUDIO					0x31
#define	F_REG_TX_NET_MCAST_ANC					0x34
#define	F_REG_TX_NET_MCAST_AUX					0x37

#define	F_REG_TX_NET_DEST_MAC_ANC				0x3A
#define	F_REG_TX_NET_DEST_IP_ANC					0x40

#define	F_REG_TX_NET_DEST_MAC_AUX				0x44
#define	F_REG_TX_NET_DEST_IP_AUX					0x4A


/********************** RX *************/
/* system device: 0x60 */
#define	F_REG_RX_SYS_RESET						0x00
#define	F_REG_RX_SYS_VIDEO_CTRL					0x01	/* mode, and depth */
#define	F_REG_RX_SYS_IR_CTRL						0x02
#define	F_REG_RX_SYS_STREAM_ENABLE				0x03	

#define	F_REG_RX_SYS_IR_DEMODULATION			0x05

/* read only , not used now */
#define	F_REG_RX_SYS_X_ACTIVE					0x06	/* width */
#define	F_REG_RX_SYS_Y_ACTIVE					0x08	/* height */

/* statistics */
#define	F_REG_RX_SYS_STATISTICS_VIDEO			0x16
#define	F_REG_RX_SYS_STATISTICS_AUDIO			0x18

#define	F_REG_RX_SYS_STATISTICS_ANC				INVALIDATE_VALUE_U8
#define	F_REG_RX_SYS_STATISTICS_AUX				INVALIDATE_VALUE_U8

#define	F_REG_RX_SYS_GBE_RX_COUNT				0x22
#define	F_REG_RX_SYS_GBE_TX_COUNT				0x24

#define	F_REG_RX_SYS_AUDIO_SELECT				0x1A
#define	F_REG_RX_SYS_AUDIO_MASK					0x1B
#define	F_REG_RX_SYS_AUDIO_INPUT					0x1C

#define	F_REG_RX_SYS_VERSION						0x1D
#define	F_REG_RX_SYS_REVISION						0x1E

#define	F_REG_RX_SYS_YEAR							66
#define	F_REG_RX_SYS_MONTH						67
#define	F_REG_RX_SYS_DAY							68
#define	F_REG_RX_SYS_HOUR						69
#define	F_REG_RX_SYS_MINUTE						70

#define	F_REG_RX_SYS_V_WIDTH						71
#define	F_REG_RX_SYS_V_HEIGHT					73

#define	F_REG_RX_SYS_V_FRAMERATE				75
#define	F_REG_RX_SYS_V_COLORSPACE				76		/* sampling */
#define	F_REG_RX_SYS_V_DEPTH						77
#define	F_REG_RX_SYS_V_INTLC_SEGM				78
#define	F_REG_RX_SYS_V_PARAM_UPDATE				79


#define	F_REG_RX_SYS_MB_VIDEO_COLOR_DEPTH		0x2C	/* 10/12/14/16 */
#define	F_REG_RX_SYS_MB_VIDEO_COLOR_FORMAT	0x2D	/* RGB/YCbCr */

/* RTP specific: 0x66 */
#define	F_REG_RX_RTP_VIDEO_PAYLOAD_TYPE			0x00
#define	F_REG_RX_RTP_XAUI_CTRL					0x05
#define	F_REG_RX_RTP_XAUI_STATUS					0x07
#define	F_REG_RX_RTP_TIMESTAMP_VIDEO			0x08	/* 4 bytes, 32 bits */
#define	F_REG_RX_RTP_TIMESTAMP_AUDIO			0x0F	/* 4 bytes, 32 bits */

#define	F_REG_RX_RTP_AUDIO_PAYLOAD_TYPE			INVALIDATE_VALUE_U8
#define	F_REG_RX_RTP_ANC_PAYLOAD_TYPE			INVALIDATE_VALUE_U8
#define	F_REG_RX_RTP_AUX_PAYLOAD_TYPE			INVALIDATE_VALUE_U8


/******** Network specific: 0x68 ********************/
/*
*  For RX, only local MAC, local IP, local video port, local audio port, and multicast video/audio IP/port are configured 
*/
#define	F_REG_RX_NET_LOCAL_MAC					0x04	/* 6 bytes */	
#define	F_REG_RX_NET_DEST_MAC					0x0A	/* 6 bytes */	

#define	F_REG_RX_NET_LOCAL_VIDEO_IP				0x10
#define	F_REG_RX_NET_DEST_VIDEO_IP				0x14

#define	F_REG_RX_NET_LOCAL_AUDIO_IP				F_REG_TX_NET_LOCAL_VIDEO_IP
#define	F_REG_RX_NET_DEST_AUDIO_IP				0x18


#define	F_REG_RX_NET_LOCAL_VIDEO_PORT			0x20
#define	F_REG_RX_NET_DEST_VIDEO_PORT			0x22

#define	F_REG_RX_NET_LOCAL_AUDIO_PORT			0x24
#define	F_REG_RX_NET_DEST_AUDIO_PORT			0x26

#define	F_REG_RX_NET_LOCAL_ANC_PORT				0x28
#define	F_REG_RX_NET_DEST_ANC_PORT				0x2A

#define	F_REG_RX_NET_DEST_AUX_PORT				INVALIDATE_VALUE_U8

#define	F_REG_RX_NET_QOS_OFS						0x2C

#define	F_REG_RX_NET_MCAST_VIDEO					0x2E	/* 3 bytes, not in border of 16 bytes */
#define	F_REG_RX_NET_MCAST_AUDIO				0x31
#define	F_REG_RX_NET_MCAST_ANC					0x34
#define	F_REG_RX_NET_MCAST_AUX					0x37


#define	F_REG_RX_NET_DEST_MAC_ANC				0x3A
#define	F_REG_RX_NET_DEST_IP_ANC					0x40
#define	F_REG_RX_NET_DEST_MAC_AUX				0x44
#define	F_REG_RX_NET_DEST_IP_AUX					0x4A





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

#else
#error 	Not support board definition
#endif


typedef	struct
{
	uint8_t		bus;
	uint8_t		channel;
	uint8_t 		slaveAddress;
#if FPGA_DEBUG
	char			*name;
#endif
}FpgaI2cDevice;

typedef	struct
{
	FpgaI2cDevice	*device;

	uint8_t			offset;
#if FPGA_DEBUG
	char				*name;
#endif
}FpgaI2cAddress;


typedef struct 
{
	FpgaI2cAddress		ip;
	FpgaI2cAddress		mac;	/* this field not used in RX */
	FpgaI2cAddress		port;
	FpgaI2cAddress		rtpPayload;

	FpgaI2cAddress		count;	/* RX or TX packet */

}StreamRegisterAddress;


typedef struct 
{
	/* video */
	FpgaI2cAddress		width;
	FpgaI2cAddress		height;
	FpgaI2cAddress		framerate;
	FpgaI2cAddress		colorSpace;
	FpgaI2cAddress		vDepth;
	FpgaI2cAddress		intl;

	/* audio */
	FpgaI2cAddress		channels;
	FpgaI2cAddress		audioRate;
	FpgaI2cAddress		pktSize;

	FpgaI2cAddress		paramStatus;	/* for TX, this is Params Status; for RX, this is Params Update */
	
}MediaRegisterAddress;


typedef	struct
{
	FpgaI2cAddress			version;
	FpgaI2cAddress			revision;

}FpgaVersionAddress;


typedef	struct
{
	FpgaI2cAddress			year;
	FpgaI2cAddress			month;
	FpgaI2cAddress			day;
	FpgaI2cAddress			hour;
	FpgaI2cAddress			minute;
}FpgaBuildTimeAddress;



typedef	struct 
{
	FpgaI2cAddress			reset;
	FpgaI2cAddress			enable;

	FpgaI2cAddress			rtpTimestampVideo;
	FpgaI2cAddress			rtpTimestampAudio;

	FpgaI2cAddress			irCtrl;
	FpgaI2cAddress			irDemodulation;

	FpgaI2cAddress			gbeRxCount;
	FpgaI2cAddress			gbeTxCount;
	
	/* local address and port */
	FpgaI2cAddress			localMac;
	FpgaI2cAddress			localIp;

	FpgaI2cAddress			localPortV;
	FpgaI2cAddress			localPortA;
	FpgaI2cAddress			localPortAnc;
#if EXT_FPGA_AUX_ON	
	FpgaI2cAddress			localPortAuc;
#endif

	/* dest address and port for every stream */
	StreamRegisterAddress		*streamVideo;
	StreamRegisterAddress		*streamAudio;
	StreamRegisterAddress		*streamAnc;
#if EXT_FPGA_AUX_ON	
	StreamRegisterAddress		*streamAux;
#endif

	MediaRegisterAddress		*media;

	FpgaBuildTimeAddress		*buildTime;
}TxRegisterMap;


typedef	struct 
{
	FpgaI2cAddress			reset;
	FpgaI2cAddress			enable;
	
	FpgaI2cAddress			rtpTimestampVideo;
	FpgaI2cAddress			rtpTimestampAudio;

	FpgaI2cAddress			irCtrl;
	FpgaI2cAddress			irDemodulation;

	FpgaI2cAddress			gbeRxCount;
	FpgaI2cAddress			gbeTxCount;
	
	FpgaI2cAddress			localMac;
	FpgaI2cAddress			localIp;

	/* for RX, these are not used */
	FpgaI2cAddress			localPortV;
	FpgaI2cAddress			localPortA;
	FpgaI2cAddress			localPortAnc;
	FpgaI2cAddress			localPortAux;

	/* dest address and port for every stream */
	StreamRegisterAddress		*streamVideo;
	StreamRegisterAddress		*streamAudio;
	StreamRegisterAddress		*streamAnc;
#if EXT_FPGA_AUX_ON	
	StreamRegisterAddress		*streamAux;
#endif

	MediaRegisterAddress		*media;

	FpgaBuildTimeAddress		*buildTime;
}RxRegisterMap;

struct	_FpgaConfig;

typedef	int	(*FpgaCtrlHandler)(struct	_FpgaConfig *_fpgaCfg);


typedef	struct	_FpgaConfig
{
	EXT_RUNTIME_CFG				*runCfg;

	FpgaVersionAddress			*version;
	
	TxRegisterMap				*txAddress;
	RxRegisterMap					*rxAddress;

	FpgaCtrlHandler				opProtocolCtrl;		/* IP address, port of all streams */
	FpgaCtrlHandler				opMediaWrite;		/* media params except address and port */
	FpgaCtrlHandler				opMediaRead;		/* read media params except address and port, for SDP and display */

	CmnMultiGroup 				*groupVideoMgr;
	CmnMultiGroup 				*groupAudioMgr;
	CmnMultiGroup 				*groupAncMgr;
}FpgaConfig;


#define	FPGA_IS_TX(fpgaCfg)		\
			EXT_IS_TX((fpgaCfg)->runCfg)
		
/* used to debug, accessing with address directly */
/* 8 bits address, and 8 bit data */
#define	FPGA_I2C_WRITE(address, val, size)		\
	I2C_EXT_WRITE(0, EXT_I2C_PCA9554_CS_NONE, EXT_FPAG_ADDRESS_SYSTEM, (address), 1, (val), size)

//	extI2CWrite(EXT_I2C_PCA9554_CS_NONE, EXT_I2C_ADDRESS_FPGA, (address), 1, (val), size)

#define	FPGA_I2C_READ(address, val, size)		\
	I2C_EXT_READ(0, EXT_I2C_PCA9554_CS_NONE, EXT_FPAG_ADDRESS_SYSTEM, (address), 1, (val), size)
//	extI2CRead(EXT_I2C_PCA9554_CS_NONE, EXT_I2C_ADDRESS_FPGA, (address), 1, (val), size)


#define	FPGA_I2C_4BYTE_WRITE(address, val, size)		\
	I2C_EXT_WRITE(0, EXT_I2C_PCA9554_CS_NONE, EXT_FPAG_ADDRESS_SYSTEM, (address), 1, (val), size)

//	extI2CWrite(EXT_I2C_PCA9554_CS_NONE, EXT_I2C_ADDRESS_FPGA, (address), 1, (val), size)

#define	FPGA_I2C_4BYTE_READ(address, val, size)		\
	I2C_EXT_READ(0, EXT_I2C_PCA9554_CS_NONE, EXT_FPAG_ADDRESS_SYSTEM, (address), 1, (val), size)

/* can be used to implment on different boards */


#if 0//FPGA_DEBUG
#define	FPGA_WRITE(addr, val, size)		\
	do{int ret; EXT_ASSERT( (((addr)!=NULL) && ((addr)->device != NULL)), "FPGA address ' %s' is null", #addr); ret = I2C_EXT_WRITE((addr)->device->bus, (addr)->device->channel, (addr)->device->slaveAddress, (addr)->offset, 1, (val), (size)); \
		MUX_DEBUG("FPGA write on %d.%d.0x%x(%s), offset 0x%x(%s), value:%d: %s",(addr)->device->bus, (addr)->device->channel, (addr)->device->slaveAddress, (addr)->device->name, (addr)->offset, (addr)->name, *val, (ret==EXIT_SUCCESS)?"OK":"Failed");\
		}while(0)
		


#define	FPGA_READ(addr, val, size)		\
		do{int ret; EXT_ASSERT( (((addr)!=NULL) && ((addr)->device != NULL)), "FPGA address ' %s' is null", #addr); ret = I2C_EXT_READ((addr)->device->bus, (addr)->device->channel, (addr)->device->slaveAddress, (addr)->offset, 1, (val), (size)); \
			MUX_DEBUG("FPGA read on %d.%d.0x%x(%s), offset 0x%x(%s): %s :0x%x", (addr)->device->bus, (addr)->device->channel, (addr)->device->slaveAddress, (addr)->device->name, (addr)->offset, (addr)->name, (ret==EXIT_SUCCESS)?"OK":"Failed", *val);\
			}while(0)

#else
#define	FPGA_WRITE(addr, val, size)		\
	do{int ret; ret = I2C_EXT_WRITE((addr)->device->bus, (addr)->device->channel, (addr)->device->slaveAddress, (addr)->offset, 1, (val), (size)); \
		if(ret==EXIT_FAILURE){ MUX_ERROR("FPGA write on %d.%d.0x%x(%s), offset 0x%x(%s), value:%d: FAILED",(addr)->device->bus, (addr)->device->channel, (addr)->device->slaveAddress, (addr)->device->name, (addr)->offset, (addr)->name, *val);} \
		}while(0)
		


#define	FPGA_READ(addr, val, size)		\
		do{int ret;  ret = I2C_EXT_READ((addr)->device->bus, (addr)->device->channel, (addr)->device->slaveAddress, (addr)->offset, 1, (val), (size)); \
			if(ret==EXIT_FAILURE) {MUX_ERROR("FPGA read on %d.%d.0x%x(%s), offset 0x%x(%s): FAILED", (addr)->device->bus, (addr)->device->channel, (addr)->device->slaveAddress, (addr)->device->name, (addr)->offset, (addr)->name);}\
			}while(0)


#endif



#define	_extFpgaReadByte(address, value)	\
			FPGA_READ((address), (value), 1)

//	FPGA_I2C_READ((address), (value), 1)


#define	_extFpgaReadShortChanging(address, shortVal) \
	{	FPGA_READ((address), (shortVal), 2); \
	 *((unsigned short *)shortVal) = ntohs(*((unsigned short *)shortVal)); }


#define	_extFpgaReadShort(address, shortVal) \
	{	FPGA_READ((address), (shortVal), 2); }

#define	_extFpgaRead3Bytes(address, intVal) \
	{	unsigned char *p = (unsigned char *)(intVal); *intVal=0;  FPGA_READ((address), p, 3); *intVal = (ntohl(*intVal)|(239) ) ; }


#define	_extFpgaReadIntegerChange(address, intVal) \
		{FPGA_READ((address), intVal, 4); }

#define	_extFpgaReadInteger(address, intVal) \
		{FPGA_READ((address), intVal, 4); *((unsigned int *)intVal) = ntohl(*((unsigned int *)intVal)); }



#define	_extFpgaWriteByte(address, value)	\
		FPGA_WRITE((address), (value), 1)

//		FPGA_I2C_WRITE((address), (value), 1)


#define	_extFpgaWriteShortChanging(address, shortVal) \
	{	unsigned short _value = ntohs(*((unsigned short *)(shortVal))); FPGA_WRITE((address), (unsigned char *)&_value, 2); }


#define	_extFpgaWriteShort(address, shortVal) \
	{	FPGA_WRITE((address), shortVal, 2); }


#define	_extFpgaWrite3Bytes(address, intVal) \
	{	unsigned int _value = ntohl(*((unsigned int *)(intVal))); unsigned char *p = (unsigned char *)&_value; FPGA_WRITE((address), p, 3); }


#define	_extFpgaWriteIntegerChange(address, intVal) \
	{	unsigned int _value = (*((unsigned int *)(intVal))); FPGA_WRITE((address), (unsigned char *)&_value, 4); }


#define	_extFpgaWriteInteger(address, intVal) \
	{	unsigned int _value = ntohl(*((unsigned int *)(intVal))); FPGA_WRITE((address), (unsigned char *)&_value, 4); }


int fpgaReadParamRegisters(MediaRegisterAddress *addrMedia, EXT_RUNTIME_CFG *runCfg);


int	sysFpgaTxConfig(FpgaConfig *fpga);
int	sysFpgaRxConfig(FpgaConfig *fpga);
int sysFpgaTxReadParams(FpgaConfig *fpga);
int sysFpgaRxReadParams(FpgaConfig *fpga);
int sysFpgaTxWriteParams(FpgaConfig *fpga);
int sysFpgaRxWriteParams(FpgaConfig *fpga);



extern	FpgaConfig 	_fpgaConfig;


#endif

