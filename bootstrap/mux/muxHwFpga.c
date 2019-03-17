/*
* FPGA configuration routines across platforms
*/

#include "common.h"
#include "hardware.h"
#include "board.h"
#include "twi.h"
#include "debug.h"

#include "mux7xx.h"


unsigned char fpgaDeviceAddress(FPGA_CTRL *fpgaCtrl, int type)
{
	unsigned char  _devAddr = 0;
	
	switch(type)
	{
		case FPGA_DEV_TYPE_SYSTEM:
			_devAddr = fpgaCtrl->devAddrSystem;
			break;
		case FPGA_DEV_TYPE_RTP:
			_devAddr = fpgaCtrl->devAddrRtp;
			break;
		case FPGA_DEV_TYPE_NETWORK:
			_devAddr = fpgaCtrl->devAddrNetwork;
			break;
		case FPGA_DEV_TYPE_XADC:
			_devAddr = fpgaCtrl->devAddrXadc;
			break;
		case FPGA_DEV_TYPE_PCSPMA:
			_devAddr = fpgaCtrl->devAddrPcspma;
			break;
		case FPGA_DEV_TYPE_RLL:
			_devAddr = fpgaCtrl->devAddrRll;
			break;
			
		default:
			dbg_info("FPGA device type error %d\n", type);
			_devAddr = 0;
			break;
	}	

	return _devAddr;
}



#define	FPGA_WRITE_SYSTM_REG(fpga, reg, size, data, bytes)		\
	fpgaWriteRegister((fpga), FPGA_DEV_TYPE_SYSTEM, (reg), (size), (data), (bytes) )

#define	FPGA_WRITE_RTP_REG(fpga, reg, size, data, bytes)		\
	fpgaWriteRegister((fpga), FPGA_DEV_TYPE_RTP, (reg), (size), (data), (bytes) )

#define	FPGA_WRITE_NETWORK_REG(fpga, reg, size, data, bytes)		\
	fpgaWriteRegister((fpga), FPGA_DEV_TYPE_NETWORK, (reg), (size), (data), (bytes) )

#define	FPGA_WRITE_XADC_REG(fpga, reg, size, data, bytes)		\
	fpgaWriteRegister((fpga), FPGA_DEV_TYPE_XADC, (reg), (size), (data), (bytes) )

#define	FPGA_WRITE_PCAPMA_REG(fpga, reg, size, data, bytes)		\
	fpgaWriteRegister((fpga), FPGA_DEV_TYPE_PCSPMA, (reg), (size), (data), (bytes) )

#define	FPGA_WRITE_RLL_REG(fpga, reg, size, data, bytes)		\
	fpgaWriteRegister((fpga), FPGA_DEV_TYPE_RLL, (reg), (size), (data), (bytes) )



#define	FPGA_READ_SYSTM_REG(fpga, reg, size, data, bytes)		\
	fpgaReadRegister((fpga), FPGA_DEV_TYPE_SYSTEM, (reg), (size), (data), (bytes) )

#define	FPGA_READ_RTP_REG(fpga, reg, size, data, bytes)		\
	fpgaReadRegister((fpga), FPGA_DEV_TYPE_RTP, (reg), (size), (data), (bytes) )

#define	FPGA_READ_NETWORK_REG(fpga, reg, size, data, bytes)		\
	fpgaReadRegister((fpga), FPGA_DEV_TYPE_NETWORK, (reg), (size), (data), (bytes) )

#define	FPGA_READ_XADC_REG(fpga, reg, size, data, bytes)		\
	fpgaReadRegister((fpga), FPGA_DEV_TYPE_XADC, (reg), (size), (data), (bytes) )

#define	FPGA_READ_PCAPMA_REG(fpga, reg, size, data, bytes)		\
	fpgaReadRegister((fpga), FPGA_DEV_TYPE_PCSPMA, (reg), (size), (data), (bytes) )

#define	FPGA_READ_RLL_REG(fpga, reg, size, data, bytes)		\
	fpgaReadRegister((fpga), FPGA_DEV_TYPE_RLL, (reg), (size), (data), (bytes) )




int muxFpgaInit(FPGA_CTRL *fpgaCtrl)
{
	unsigned char  _chData;
	unsigned char	_chArray[16];
	unsigned short port;

	_chData =FPAG_VIDEO_CONFIG_REG(EXT_VIDEO_MODE_RGB_RAW, EXT_VIDEO_DEPTH_8BIT);
	// Set the video format as: bits(4:2) = RGB Raw, bits(1:0) = 8-bits per pixel
	FPGA_WRITE_SYSTM_REG(fpgaCtrl, MUX_FPGA_REG_VIDEO_CTRL, 1, &_chData, 1);
	

  // for debug
//	twi_addr = 0;
//	data[0] = 0xff;
//	data[1] = 0xfe;
//	data[2] = 0xfd;
//	data[3] = 0xfc;
	FPGA_READ_SYSTM_REG(fpgaCtrl, MUX_FPGA_REG_RESET, 1, _chArray, 4);
	EXT_INFOF(("FPGA(60.0) %x %x %x %x ", _chArray[0], _chArray[1], _chArray[2], _chArray[3] ));

//	twi_addr = 4;
	_chArray[0] = (fpgaCtrl->isTx)? 0x10: 0x11;
	_chArray[1] = 0x00;
	_chArray[2] = 0x00;
	_chArray[3] = 0xE5;
	_chArray[4] = 0xA0;
	_chArray[5] = 0x00;
	// Set the 48-bit Local MAC address 
//	twi_write(bus, twi_dev, twi_addr, 1, data, 6);
	FPGA_READ_NETWORK_REG(fpgaCtrl, MUX_FPGA_REG_MAC_LOCAL, 1, _chArray, 6);

//	twi_addr = 10;
	_chArray[0] = (fpgaCtrl->isTx)?0x11:0x10;
	// Set the 48-bit Destination MAC address
//	twi_write(bus, twi_dev, twi_addr, 1, data, 6);
	FPGA_READ_NETWORK_REG(fpgaCtrl, MUX_FPGA_REG_MAC_DEST, 1, _chArray, 6);

	/* local/dest IP */
	_chArray[0] = (fpgaCtrl->isTx)?ETHERNET_CONF_IPADDR3_TX:ETHERNET_CONF_IPADDR3_RX;
	_chArray[1] = ETHERNET_CONF_IPADDR2_TX;
	_chArray[2] = ETHERNET_CONF_IPADDR1;
	_chArray[3] = ETHERNET_CONF_IPADDR0;
	FPGA_READ_NETWORK_REG(fpgaCtrl, MUX_FPGA_REG_IP_VIDEO_LOCAL, 1, _chArray, 4);

	_chArray[0] = (fpgaCtrl->isTx)?ETHERNET_CONF_IPADDR3_RX:ETHERNET_CONF_IPADDR3_TX;
	FPGA_READ_NETWORK_REG(fpgaCtrl, MUX_FPGA_REG_IP_VIDEO_DEST, 1, _chArray, 4);

	/* video port */
	port =  (fpgaCtrl->isTx)?EXT_MEDIA_PORT_TX_VIDEO:EXT_MEDIA_PORT_RX_VIDEO;
	port = EXT_NTOHS(port);
	FPGA_READ_NETWORK_REG(fpgaCtrl, MUX_FPGA_REG_PORT_VIDEO_LOCAL, 1, (unsigned char *)&port, 2);

	port =  (fpgaCtrl->isTx)?EXT_MEDIA_PORT_RX_VIDEO:EXT_MEDIA_PORT_TX_VIDEO;
	port = EXT_NTOHS(port);
	FPGA_READ_NETWORK_REG(fpgaCtrl, MUX_FPGA_REG_PORT_VIDEO_DEST, 1, (unsigned char *)&port, 2);

	/* audio port */
	port =  (fpgaCtrl->isTx)?EXT_MEDIA_PORT_TX_AUDIO:EXT_MEDIA_PORT_RX_AUDIO;
	port = EXT_NTOHS(port);
	FPGA_READ_NETWORK_REG(fpgaCtrl, MUX_FPGA_REG_PORT_AUDIO_LOCAL, 1, (unsigned char *)&port, 2);

	port =  (fpgaCtrl->isTx)?EXT_MEDIA_PORT_RX_AUDIO:EXT_MEDIA_PORT_TX_AUDIO;
	port = EXT_NTOHS(port);
	FPGA_READ_NETWORK_REG(fpgaCtrl, MUX_FPGA_REG_PORT_AUDIO_DEST, 1, (unsigned char *)&port, 2);

	/* anc port */
	port =  (fpgaCtrl->isTx)?EXT_MEDIA_PORT_TX_DATA:EXT_MEDIA_PORT_RX_DATA;
	port = EXT_NTOHS(port);
	FPGA_READ_NETWORK_REG(fpgaCtrl, MUX_FPGA_REG_PORT_ANC_LOCAL, 1, (unsigned char *)&port, 2);
	
	port =  (fpgaCtrl->isTx)?EXT_MEDIA_PORT_RX_DATA:EXT_MEDIA_PORT_TX_DATA;
	port = EXT_NTOHS(port);
	FPGA_READ_NETWORK_REG(fpgaCtrl, MUX_FPGA_REG_PORT_ANC_DEST, 1, (unsigned char *)&port, 2);

	return 0;
}


int muxFpgaReset(FPGA_CTRL *fpgaCtrl)
{
	unsigned char	version[16];

	unsigned char  _chData;
//	unsigned int  _intRegAddr;

//	twi_dev = EXT_I2C_DEV_FPGA_SYSTEM;

	// sw reset fpga global
	_chData = 0x80;	// reset FPGA
	// Assert global reset
//	twi_write(fpgaCtrl->bus, twi_dev, twi_addr, 1, &_chData, 1);	
	FPGA_WRITE_SYSTM_REG(fpgaCtrl, MUX_FPGA_REG_RESET, 1, &_chData, 1);
	
	_chData = 0x0;	// clear reset
	// De-assert global reset
//	twi_write(bus, twi_dev, twi_addr, 1, &twi_data, 1);
	FPGA_WRITE_SYSTM_REG(fpgaCtrl, MUX_FPGA_REG_RESET, 1, &_chData, 1);

	// sw reset sub modules
	_chData = 0x7f;	// reset all FPGA sub modules
	// Assert reset to each FPGA sub-system: XAUI, Video, PCS/PMA, 1GB Ethernet, HDMI/Microblaze
//	twi_write(fpgaCtrl->bus, twi_dev, twi_addr, 1, &twi_data, 1);
	FPGA_WRITE_SYSTM_REG(fpgaCtrl, MUX_FPGA_REG_RESET, 1, &_chData, 1);
	_chData = 0x0;		// clear reset
	// De-assert reset to each FPGA sub-system: XAUI, Video, PCS/PMA, 1GB Ethernet, HDMI/Microblaze
//	twi_write(fpgaCtrl->bus, twi_dev, twi_addr, 1, &twi_data, 1);
	FPGA_WRITE_SYSTM_REG(fpgaCtrl, MUX_FPGA_REG_RESET, 1, &_chData, 1);

	// reset PCS/PMA
#if (MUX_BOARD == MUX_BOARD_774)
//	twi_dev = EXT_I2C_DEV_FPGA_PCSPMA;
//	twi_addr = 0;
	_chData = 0x12;		// set reset
	// Assert reset to PCS and PMA
//	twi_write(fpgaCtrl->bus, twi_dev, twi_addr, 1, &twi_data, 1);
	FPGA_WRITE_PCAPMA_REG(fpgaCtrl, 0, 1, &_chData, 1);
	_chData = 0x0;		// clear reset
	// De-assert reset to PCS and PMA
//	twi_write(fpgaCtrl->bus, twi_dev, twi_addr, 1, &twi_data, 1);
	FPGA_WRITE_PCAPMA_REG(fpgaCtrl, 0, 1, &_chData, 1);
#endif

//	twi_dev = EXT_I2C_DEV_FPGA_SYSTEM;
	// disable all stream (video, audio, etc)
//	twi_addr = 3;
	_chData = 0x0;
	// Disable all streams: Video, Audio, SIO, 1GB Ethernet, Ancillary, and Auxiliary	
//	twi_write(fpgaCtrl->bus, twi_dev, twi_addr, 1, &twi_data, 1);
	FPGA_WRITE_SYSTM_REG(fpgaCtrl, MUX_FPGA_REG_STREAM_ENBALE, 1, &_chData, 1);

	//read reg 03 value, if not 0, disable it again.
#if 0
	if(twi_read(fpgaCtrl->bus, twi_dev, twi_addr, 1, &twi_data, 1) )
	{
		dbg_info("read reg %x %x failed\n", twi_addr, twi_data);
	}
#endif

	FPGA_READ_SYSTM_REG(fpgaCtrl, MUX_FPGA_REG_STREAM_ENBALE, 1, &_chData, 1);
	if(_chData != 0x0)
	{
		EXT_ERRORF(("After reset all streams, streams register is %x. Reset all streams again %x"));
		_chData = 0x0;
		FPGA_WRITE_SYSTM_REG(fpgaCtrl, MUX_FPGA_REG_STREAM_ENBALE, 1, &_chData, 1);
	}
	else
	{
		EXT_INFOF(("After reset all streams, streams register is OK!" ));
	}


	// read fpga version,revision
	if(FPGA_READ_SYSTM_REG(fpgaCtrl, MUX_FPGA_REG_VERSION, 1, version, 3))
	{
		EXT_ERRORF(("FPGA version read failed" ));
	}
	else
	{
		EXT_INFOF(("FPGA version %x %x %x", version[0],version[1],version[2] ));
	}

	fpgaCtrl->isTx = ((version[0] & 0x80) == 0)?0:1;	
	EXT_INFOF(("FPGA %s unit", (fpgaCtrl->isTx)?"TX":"RX" ));
	
	int dac = (fpgaCtrl->isTx)?0:1000;
	if(fpgaCtrl->isTx)
	{
		si5351bTxCfg();
	}
	
	dac_param(dac);
	muxFpgaInit(fpgaCtrl);
	

	if(fpgaCtrl->isTx)
	{
		muxTxInitHdmi();
	}
	else
	{
		muxRxInitVideoParam();
	}

#if 0
	if ((version[0] & 0x80) == 0)
	{
		dbg_info("FPGA RX unit \n");
		dac_param(1000);
		muxFpgaRxInit();

		dbg_info("HDMI RX unit \n");
		 muxRxInitVideoParam();
	}
	else
	{
		si5351bTxCfg();
		dbg_info("FPGA TX unit \n");
		dac_param(0);
		muxFpgaTxInit();
		
		dbg_info("HDMI TX unit \n");
		muxTxInitHdmi();
	}
#endif

	return 0;
}

