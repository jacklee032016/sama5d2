/*
* hardware definitions, bus and registers, etc.
*/

#ifndef	__MUX_7XX_HW_H__
#define	__MUX_7XX_HW_H__

#include "mux7xxCompact.h"

#define	EXT_I2C_DEV_PCA9544			(0xE0>>1)
#define	EXT_I2C_DEV_SI5351B			(0xC0>>1)
#define	EXT_I2C_DEV_MDIO				(0x64 >> 1)		// 8 bits addr format


#define	EXT_I2C_DEV_FPGA_SYSTEM		(0x60>>1)	/* 0x30 */
#define	EXT_I2C_DEV_FPGA_RTP			(0x66>>1)	/* 0x33 */
#define	EXT_I2C_DEV_FPGA_NET			(0x68>>1)	/* 0x34 */
#define	EXT_I2C_DEV_FPGA_DRP_PLL		(0x6A>>1)	/* 0x35, RX only */
#define	EXT_I2C_DEV_FPGA_DRP_XADC	(0x6c>>1)	/* 0x36, Dynamic Reconfiguration Port */
#define	EXT_I2C_DEV_FPGA_PCSPMA		(0x6E>>1)	/* 0x37 */


#if	(MUX_BOARD == MUX_ATMEL_XPLAINED)
	#define	EXT_I2C_DEV_SENSOR			(0x30 >> 1)
#elif	(MUX_BOARD == MUX_BOARD_768)
	#define	EXT_I2C_DEV_SENSOR			(0x30 >> 1)
#elif (MUX_BOARD == MUX_BOARD_774)
	#define	EXT_I2C_DEV_SENSOR			(0x90 >> 1)
#elif (MUX_BOARD == MUX_BOARD_767)
	#define	EXT_I2C_DEV_SENSOR			(0x30 >> 1)
#else
#error 	Not support board definition
#endif


#define	EXT_I2C_SENSOR_LOCAL_TEMP_MSB		0x00
#define	EXT_I2C_SENSOR_LOCAL_TEMP_LSB		0x30
#define	EXT_I2C_SENSOR_MANUFACTURE_ID		0xFE
#define	EXT_I2C_SENSOR_REVISION_ID			0xFF


/* bus of multiplexer */
#define	MUX_BUS_SI5351B						0
#define	MUX_BUS_LM9524						1
#define	MUX_BUS_MCP4706						1

#define	MUX_BUS_INVALIDATE					0xFF
#define	MUX_BUS_MDIO							MUX_BUS_INVALIDATE


/******  FPGA registers ******************/

#define	MUX_FPGA_REG_RESET					0x0		/* 0 */
#define	MUX_FPGA_REG_VIDEO_CTRL				0x1		/* 1 */
#define	MUX_FPGA_REG_IR_CTRL					0x2		/* 2 */
#define	MUX_FPGA_REG_STREAM_ENBALE			0x3		/* 3 */

#define	MUX_FPGA_REG_VERSION					0x1D	/* 29 */

#define	MUX_FPGA_REG_SPI_PATH_SEL			0x20	/* 32 */


/* network registers */
#define	MUX_FPGA_REG_MAC_LOCAL				0x04
#define	MUX_FPGA_REG_MAC_DEST				0x0A	/* 10*/

#define	MUX_FPGA_REG_IP_VIDEO_LOCAL			0x10	/* 16*/
#define	MUX_FPGA_REG_IP_VIDEO_DEST			0x14	/* 20*/
#define	MUX_FPGA_REG_IP_AUDIO_DEST			0x18	/* 24*/

#define	MUX_FPGA_REG_PORT_VIDEO_LOCAL		0x20	/* 32 */
#define	MUX_FPGA_REG_PORT_VIDEO_DEST		0x22	/* 34 */

#define	MUX_FPGA_REG_PORT_AUDIO_LOCAL		0x24	/* 36 */
#define	MUX_FPGA_REG_PORT_AUDIO_DEST		0x26	/* 38 */

#define	MUX_FPGA_REG_PORT_ANC_LOCAL		0x28	/* 40 */
#define	MUX_FPGA_REG_PORT_ANC_DEST			0x2A	/* 42 */



#define	EXT_HTONS(x)	((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
#define	EXT_NTOHS(x)	EXT_HTONS(x)

#define	EXT_HTONL(x)	((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))
                     
#define	EXT_NTOHL(x)	EXT_HTONL(x)


#define IP_ADDR_IS_MULTICAST( addr)  \
		((addr & EXT_HTONL(0xf0000000UL)) == EXT_HTONL(0xe0000000UL))


#define	INVALIDATE_VALUE_U8						0xFF
#define	INVALIDATE_VALUE_U16						0xFFFF
#define	INVALIDATE_VALUE_U32						0xFFFFFFFF

#define	EXT_INVALIDATE_STRING_TYPE				0xFFFF



#define	FIELD_INVALIDATE_U8( field) 					((field) = INVALIDATE_VALUE_U8)
#define	FIELD_INVALIDATE_U16(field) 					((field) = INVALIDATE_VALUE_U16)
#define	FIELD_INVALIDATE_U32(field)					((field) = INVALIDATE_VALUE_U32)

#define	FIELD_IS_CHANGED_U8(field)					((field) != INVALIDATE_VALUE_U8)
#define	FIELD_IS_CHANGED_U16(field)				((field) != INVALIDATE_VALUE_U16)
#define	FIELD_IS_CHANGED_U32(field)				((field) != INVALIDATE_VALUE_U32)



typedef	enum _EXT_VIDEO_MODE
{
	EXT_VIDEO_MODE_YCBCR442  = 0,
	EXT_VIDEO_MODE_YCBCR422  = 1,
	EXT_VIDEO_MODE_YCBCR420  = 2,
	EXT_VIDEO_MODE_YCBCR_RAW  = 3,
	EXT_VIDEO_MODE_RGB444  = 4,
	EXT_VIDEO_MODE_RESERVER  = 5,
	EXT_VIDEO_MODE_Y4K_420  = 6,
	EXT_VIDEO_MODE_RGB_RAW  = 7,
	EXT_VIDEO_MODE_UNKNOWN
}EXT_VIDEO_MODE;


#define	EXT_VIDEO_MODE

typedef	enum  _EXT_VIDEO_DEPTH
{
	EXT_VIDEO_DEPTH_8BIT = 0,
	EXT_VIDEO_DEPTH_10BIT = 1,
	EXT_VIDEO_DEPTH_12BIT = 2,
	EXT_VIDEO_DEPTH_16BIT = 3,
	EXT_VIDEO_DEPTH_UNKNOWN
}EXT_VIDEO_DEPTH;


#define	FPAG_VIDEO_CONFIG_REG(mod, pixSize)	(((mod)<<2) | ((pixSize)<<0))


#define	EXT_BOOTSTRAP_DEBUG		EXT_DBG_ON// EXT_DBG_ON

#include "mux7xxSysParams.h"


typedef	struct
{/* must as 'unsigned', otherwise it output FFFFFFFa1 when print as char */
	unsigned char		address[EXT_MAC_ADDRESS_LENGTH];
}EXT_MAC_ADDRESS;


#define	MAC_ADDR_IS_BOARDCAST(macAddr)		\
	( (macAddr)->address[0]==0xFF && (macAddr)->address[1]==0xFF && (macAddr)->address[4]==0xFF && (macAddr)->address[5]==0xFF  )

#define	MAC_ADDR_IS_EQUAL(macAdd1, macAdd2 )	\
	( (macAdd1)->address[0]==(macAdd2)->address[0] && \
		(macAdd1)->address[1]==(macAdd2)->address[1] && \
		(macAdd1)->address[2]==(macAdd2)->address[2] && \
		(macAdd1)->address[3]==(macAdd2)->address[3] && \
		(macAdd1)->address[4]==(macAdd2)->address[4] && \
		(macAdd1)->address[5]==(macAdd2)->address[5]  )

#define	MAC_ADDR_IS_NULL(macAddr)		\
	( (macAddr)->address[0]==0 && (macAddr)->address[1]==0 && (macAddr)->address[4]==0 && (macAddr)->address[5]==0  )



typedef enum  _FPGA_DEV_TYPE
{
	FPGA_DEV_TYPE_SYSTEM = 0,
	FPGA_DEV_TYPE_RTP,
	FPGA_DEV_TYPE_NETWORK,
	FPGA_DEV_TYPE_XADC,
	FPGA_DEV_TYPE_PCSPMA,
	FPGA_DEV_TYPE_RLL,
	FPGA_DEV_TYPE_UNKOWN,
}FPGA_DEV_TYPE;

typedef	struct  _FPGA_CTRL
{
	unsigned int		bus;
	
	unsigned char		devAddrSystem;
	unsigned char		devAddrRtp;
	unsigned char		devAddrNetwork;
	unsigned char		devAddrXadc;
	unsigned char		devAddrPcspma;
	unsigned char		devAddrRll;

	int				isTx;
}FPGA_CTRL;


unsigned char fpgaDeviceAddress(FPGA_CTRL *fpgaCtrl, int type);


int fpgaWriteRegister(FPGA_CTRL *fpgaCtrl, int type, unsigned int regAddr, unsigned char addrSize, unsigned char *data, unsigned int bytes);
int fpgaReadRegister(FPGA_CTRL *fpgaCtrl, int type, unsigned int regAddr, unsigned char addrSize, unsigned char *data, unsigned int bytes);



#endif

