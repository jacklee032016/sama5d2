
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>
#include <stdint.h>

#include "libCmnSys.h"
#include "mux7xx.h"

#include "muxFpga.h"

static struct Tx_Media_Stream_Register_Address _TxStreamVideo =
{
	.addrIp = 88,
	.addrMac = 64,
	.addrPort = 104,
	.addrRtpPayload = 37,
};

static struct Tx_Media_Stream_Register_Address _TxStreamAudio =
{
	.addrIp = 92,
	.addrMac = 70,
	.addrPort = 106,
	.addrRtpPayload = 38,
};


static struct Tx_Media_Stream_Register_Address _TxStreamAnc =
{
	.addrIp = 96,
	.addrMac = 76,
	.addrPort = 108,
	.addrRtpPayload = 39,
};

#if EXT_FPGA_AUX_ON	
static struct Tx_Media_Stream_Register_Address _TxStreamAux =
{
	.addrIp = 100,
	.addrMac = 82,
	.addrPort = 110,
	.addrRtpPayload = INVALIDATE_VALUE_U8,
};
#endif

struct Media_Register_Address _TxMediaAddress =
{
	.addrWidth = EXT_FPGA_REG_WIDTH,
	.addrHeight = EXT_FPGA_REG_HEIGHT,
	.addrFramerate = EXT_FPGA_REG_FRAMERATE,
	.addrColorSpace = EXT_FPGA_REG_SAMPLING,
	.addrVDepth = EXT_FPGA_REG_DEPTH,
	.addrIntl = EXT_FPGA_REG_INTLC_SEGM,
	
	.addrChannels = EXT_FPGA_REG_AUDIO_CHANNELS,
	.addrAudioRate = EXT_FPGA_REG_AUDIO_RATE,
	.addrPktSize = EXT_FPGA_REG_AUDIO_PKT_SIZE
	
};


static struct Tx_Register_Address _FTX_ADDRESS =
{
	.localMac = 46,
	.localIp = 52,

	.localPortV = 56,
	.localPortA = 58,
	.localPortAnc = 60,
	.localPortAuc = 62,

#if 0
	dstVideoIp	:	88,
	dstVideoMac	:	64,
	dstVideoPort	:	104,

	dstAudioIp	:	92,
	dstAudioMac	:	70,
	dstAudioPort	:	106,

	dstAncIp		:	96,
	dstAncMac	:	76,
	dstAncPort	:	108,

	dstAuxIp		:	100,
	dstAuxMac	:	82,
	dstAuxPort	:	110,

	rtpVideo		:	37,
	rtpAudio		:	38,
	rtpAnc		:	39
#endif
	/* dest address and port for every stream */
	.addrStreamVideo = &_TxStreamVideo,
	.addrStreamAudio = &_TxStreamAudio,
	.addrStreamAnc = &_TxStreamAnc,
#if EXT_FPGA_AUX_ON	
	.addrStreamAux = &_TxStreamAux,
#endif
	.addrMedia = &_TxMediaAddress

};


static struct Rx_Media_Stream_Register_Address _RxStreamVideo =
{
	.addrIp = 46,
	.addrPort = 62,
	.addrRtpPayload = 114
};

static struct Rx_Media_Stream_Register_Address _RxStreamAudio =
{
	.addrIp = 49,
	.addrPort = 76,
	.addrRtpPayload = 115
};

static struct Rx_Media_Stream_Register_Address _RxStreamAnc =
{
	.addrIp = 88,
	.addrPort = 80,
	.addrRtpPayload = 116
};

#if EXT_FPGA_AUX_ON	
static struct Rx_Media_Stream_Register_Address _RxStreamAux =
{
	.addrIp = 91,
	.addrPort = 84,
	.addrRtpPayload = 114
};
#endif

struct Media_Register_Address _RxMediaAddress =
{
	.addrWidth = EXT_FPGA_REG_WIDTH,
	.addrHeight = EXT_FPGA_REG_HEIGHT,
	.addrFramerate = EXT_FPGA_REG_FRAMERATE,
	.addrColorSpace = EXT_FPGA_REG_SAMPLING,
	.addrVDepth = EXT_FPGA_REG_DEPTH,
	.addrIntl = EXT_FPGA_REG_INTLC_SEGM,
	
	.addrChannels = EXT_FPGA_REG_AUDIO_CHANNELS,
	.addrAudioRate = EXT_FPGA_REG_AUDIO_RATE,
	.addrPktSize = EXT_FPGA_REG_AUDIO_PKT_SIZE
	
};


struct Rx_Register_Address _FRX_ADDRESS = 
{
	.localMac	 = 56,
	.localIp = 52,

	.localPortV = INVALIDATE_VALUE_U8,
	.localPortA = INVALIDATE_VALUE_U8,
	.localPortAnc = INVALIDATE_VALUE_U8,
#if EXT_FPGA_AUX_ON	
	.localPortAuc = INVALIDATE_VALUE_U8,
#endif

#if 0
	mulIpVideo	:	46,
	mulIpAudio	:	49,
	mulIpAnc		:	88,
	mulIpAux		:	91,

	portVideo		:	62,
	portAudio		:	76,
	portAnc		:	80,
	portAux		:	84,

	srcIp		:	64,
	srcMac		:	68,

	srcPortVideo	:	74,
	srcPortAudio	:	78,
	srcPortAnc	:	82,
	srcPortAux	:	86,

	rtpVideo		:	114,
	rtpAudio		:	115,
	rtpAnc		:	116
#endif

	.addrStreamVideo = &_RxStreamVideo,
	.addrStreamAudio = &_RxStreamAudio,
	.addrStreamAnc = &_RxStreamAnc,
#if EXT_FPGA_AUX_ON	
	.addrStreamAux = &_RxStreamAux,
#endif
	.addrMedia = &_RxMediaAddress

};



static FpgaConfig 	_fpgaConfig =
{
	.txAddress = &_FTX_ADDRESS,
	.rxAddress = &_FRX_ADDRESS,

	.addrVersion = EXT_FPGA_REG_VERSION,
	.addrRevision = EXT_FPGA_REG_REVISION,

	.addrYear = EXT_FPGA_REG_YEAR,
	.addrMonth = EXT_FPGA_REG_MONTH,
	.addrDay = EXT_FPGA_REG_DAY,
	.addrHour = EXT_FPGA_REG_HOUR,
	.addrMinute = EXT_FPGA_REG_MINUTE
};


static char _fpgaVersion[128];

/* read hw info, such as TX or RX, version */
static int _sysFpgaReadHwInfo(FpgaConfig 	*fpga)
{
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;
	
	_extFpgaReadByte(fpga->addrVersion, &runCfg->runtime.version);
	_extFpgaReadByte(fpga->addrRevision, &runCfg->runtime.revision);
	_extFpgaReadByte(fpga->addrYear, &runCfg->runtime.year);
	_extFpgaReadByte(fpga->addrMonth, &runCfg->runtime.month);
	_extFpgaReadByte(fpga->addrDay, &runCfg->runtime.day);
	_extFpgaReadByte(fpga->addrHour, &runCfg->runtime.hour);
	_extFpgaReadByte(fpga->addrMinute, &runCfg->runtime.minute);

	return EXIT_SUCCESS;
}

void _fpgaRegisterRead(unsigned char address, unsigned char *data, unsigned char size)
{
	unsigned char i;
	unsigned char *index = data;
	
	for(i=0; i< size; i++)
	{
		FPGA_I2C_READ(address+i, index+i, 1);
	}
}


/* IPAddress from Lwip, in network byte order */
void	_fpgaRegisterWrite(unsigned char baseAddr, unsigned char *data, unsigned char size)
{
	unsigned char i;
	unsigned char *index = data;

	for(i=0; i < size; i++)
	{
		FPGA_I2C_WRITE(baseAddr+i, index+i, 1);
	}

//		FPGA_I2C_READ(i, &data, 1);

	return ;//EXIT_SUCCESS;
}

/* check hw info and read TX/RX */
int	sysFpgaCheck(EXT_RUNTIME_CFG *runCfg )
{
	FpgaConfig 	*fpga =  &_fpgaConfig;
	
	fpga->runCfg = runCfg;
	runCfg->fpgaCfg = fpga;

	if(_sysFpgaReadHwInfo(fpga) == EXIT_FAILURE)
	{
		EXT_ERRORF("Read from FPGA failed");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int	sysFpgaInit(EXT_RUNTIME_CFG *runCfg )
{
	FpgaConfig 	*fpga =  &_fpgaConfig;
	char	value;
	
	fpga->runCfg = runCfg;
	runCfg->fpgaCfg = fpga;
	
//	_fpgaRegisterWrite(EXT_FPGA_REG_PORT_AUDIO, (unsigned char *)&runCfg->local.aport, 2);

	
	if(! IS_SECURITY_CHIP_EXIST(runCfg->muxMain) || cmnSysScValidate(runCfg->muxMain->scf)== EXIT_FAILURE)
	{
		value = EXT_FPGA_FLAGS_DISABLE_ALL;	
		_extFpgaWriteByte(EXT_FPGA_REG_ENABLE, &value);
		return EXIT_FAILURE;
	}

	/*configure dest, only for TX */
	if(EXT_IS_TX(runCfg) )
	{
		fpga->opProtocolCtrl = sysFpgaTxConfig;
		fpga->opMediaRead = sysFpgaTxReadParams;
		fpga->opMediaWrite = sysFpgaTxWriteParams;
	}
	else
	{/* RX */
		fpga->opProtocolCtrl = sysFpgaRxConfig;
		fpga->opMediaRead = sysFpgaRxReadParams;
		fpga->opMediaWrite = sysFpgaRxWriteParams;

		/* only one group address now */
		fpga->groupVideoMgr = cmnSysNetMGroupInit(MUX_ETH_DEVICE, NULL);
		if(runCfg->dest.ip != runCfg->dest.audioIp)
		{
			fpga->groupAudioMgr = cmnSysNetMGroupInit(MUX_ETH_DEVICE, NULL);
		}

		if(runCfg->dest.ip != runCfg->dest.ancIp && runCfg->dest.audioIp != runCfg->dest.ancIp)
		{
			fpga->groupAncMgr = cmnSysNetMGroupInit(MUX_ETH_DEVICE, NULL);
		}
	}

	fpga->opProtocolCtrl(fpga);

	fpga->opMediaWrite(fpga);

	return EXIT_SUCCESS;
}


char *sysFgpaVersion(FpgaConfig 	*fpga)
{
	int index = 0;
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;
	MuxRunTimeParam *mediaParams = &runCfg->runtime;
	
//	_extFpgaReadVer(mediaParams);

	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, " %s, Ver:%02x", 
		((mediaParams->version&EXT_FPGA_TX_FLAGS)==0)?"RX":"TX", (mediaParams->version&(~EXT_FPGA_TX_FLAGS)) );
	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, ".%02x, ", mediaParams->revision);

	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, "(Build %02x ", mediaParams->month);
	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, "%02x ", mediaParams->day);
	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, "%02x ", mediaParams->year);
	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, "%02x:", mediaParams->hour);
	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, "%02x)", mediaParams->minute);

	return _fpgaVersion;
}



int sysFgpaRegistersDebug( char *data, unsigned int size)
{
	int index = 0;
//	unsigned char val;
	unsigned short port = 0;
	unsigned int	intValue;
	EXT_MAC_ADDRESS destMac;
	unsigned char		address[EXT_MAC_ADDRESS_LENGTH], _chVal;
	struct Media_Register_Address		*addrMedia = NULL;

	FpgaConfig 	*fpga =  &_fpgaConfig;
	
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;
	if(runCfg == NULL)
	{
		index += snprintf(data+index, size-index, "FPGA is not initialized now"EXT_NEW_LINE);
		return EXIT_FAILURE;
	}

	index += snprintf(data+index, size-index, "%s;"EXT_NEW_LINE, sysFgpaVersion(fpga) );
	index += snprintf(data+index, size-index, "%s configuration: ", EXT_IS_TX(runCfg)?"TX":"RX");

	if(EXT_IS_TX(runCfg) )
	{
		addrMedia = fpga->txAddress->addrMedia;
		_extFpgaReadInteger(fpga->txAddress->localIp, (unsigned char *)&intValue);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"Local :"EXT_NEW_LINE"\tIP\t: %s", cmnSysNetAddress(intValue));

		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tMAC\t:" );
		_fpgaRegisterRead(fpga->txAddress->localMac,  (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
		cmnSysNetChangeByteOrderOfMac(&destMac, address);
		index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x:", address[0], address[1], address[2], address[3], address[4], address[5]);

		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tPort\t: Video:" );
		_extFpgaReadShort(fpga->txAddress->localPortV,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;", port);

		index += snprintf(data+index, size-index," Audio:" );
		_extFpgaReadShort(fpga->txAddress->localPortA,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;", port);

		index += snprintf(data+index, size-index, " ANC:" );
		_extFpgaReadShort(fpga->txAddress->localPortAnc,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;", port);

#if EXT_FPGA_AUX_ON	
		index += snprintf(data+index, size-index, " AUX:" );
		_extFpgaReadShort(fpga->txAddress->localPortAuc,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;"EXT_NEW_LINE, port);
#endif

		_extFpgaReadInteger(fpga->txAddress->addrStreamVideo->addrIp, (unsigned char *)&intValue);
		_fpgaRegisterRead(fpga->txAddress->addrStreamVideo->addrMac, (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
		cmnSysNetChangeByteOrderOfMac(&destMac, address);
		_extFpgaReadShort(fpga->txAddress->addrStreamVideo->addrPort,  (unsigned char *)&port);
		_extFpgaReadByte(fpga->txAddress->addrStreamVideo->addrRtpPayload, &_chVal);
		_chVal = (_chVal&0x7F);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"Dest :"EXT_NEW_LINE"\tVideo: \tIP:%s;", cmnSysNetAddress(intValue));
		index += snprintf(data+index, size-index, "\tMAC:" );
		index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x;", address[0], address[1], address[2], address[3], address[4], address[5]);
		index += snprintf(data+index, size-index, "\tPort:%d;\tRtpType:%d", port, _chVal);

		/* audio */
		_extFpgaReadInteger(fpga->txAddress->addrStreamAudio->addrIp, (unsigned char *)&intValue);
		_fpgaRegisterRead(fpga->txAddress->addrStreamAudio->addrMac, (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
		cmnSysNetChangeByteOrderOfMac(&destMac, address);
		_extFpgaReadShort(fpga->txAddress->addrStreamAudio->addrPort,  (unsigned char *)&port);
		_extFpgaReadByte(fpga->txAddress->addrStreamAudio->addrRtpPayload, &_chVal);
		_chVal = (_chVal&0x7F);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tAudio: \tIP:%s;", cmnSysNetAddress(intValue));
		index += snprintf(data+index, size-index, "\tMAC:" );
		index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x;", address[0], address[1], address[2], address[3], address[4], address[5]);
		index += snprintf(data+index, size-index, "\tPort:%d;\tRtpType:%d", port, _chVal);

		_extFpgaReadInteger(fpga->txAddress->addrStreamAnc->addrIp, (unsigned char *)&intValue);
		_fpgaRegisterRead(fpga->txAddress->addrStreamAnc->addrMac, (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
		cmnSysNetChangeByteOrderOfMac(&destMac, address);
		_extFpgaReadShort(fpga->txAddress->addrStreamAnc->addrPort,  (unsigned char *)&port);
		_extFpgaReadByte(fpga->txAddress->addrStreamAnc->addrRtpPayload, &_chVal);
		_chVal = (_chVal&0x7F);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tANC : \tIP:%s;", cmnSysNetAddress(intValue));
		index += snprintf(data+index, size-index, "\tMAC:" );
		index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x;", address[0], address[1], address[2], address[3], address[4], address[5]);
		index += snprintf(data+index, size-index, "\tPort:%d;\tRtpType:%d", port, _chVal);

#if EXT_FPGA_AUX_ON	
		_extFpgaReadInteger(fpga->txAddress->addrStreamAux->addrIp, (unsigned char *)&intValue);
		_fpgaRegisterRead(fpga->txAddress->addrStreamAux->addrMac, (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
		cmnSysNetChangeByteOrderOfMac(&destMac, address);
		_extFpgaReadShort(fpga->txAddress->addrStreamAux->addrPort,  (unsigned char *)&port);
		_extFpgaReadByte(fpga->txAddress->addrStreamAux->addrRtpPayload, &_chVal);
		_chVal = (_chVal&0x7F);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tAUX : \tIP:%s;", cmnSysNetAddress(intValue));
		index += snprintf(data+index, size-index, "\tMAC:" );
		index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x;", address[0], address[1], address[2], address[3], address[4], address[5]);
		index += snprintf(data+index, size-index, "\tPort:%d;\tRtpType:%d", port, _chVal);
#endif		
		_extFpgaReadByte(EXT_FPGA_REG_SDI_STATUS, &_chVal);
		index += snprintf(data+index, size-index, "LockReg :%02x;"EXT_NEW_LINE, _chVal );
	}
	else
	{
		addrMedia = fpga->rxAddress->addrMedia;

		_extFpgaReadInteger(fpga->rxAddress->localIp, (unsigned char *)&intValue);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"Local :"EXT_NEW_LINE"\tIP\t: %s", cmnSysNetAddress(intValue));

		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tMAC\t:" );
		_fpgaRegisterRead(fpga->rxAddress->localMac,  (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
		cmnSysNetChangeByteOrderOfMac(&destMac, address);
		index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x:", address[0], address[1], address[2], address[3], address[4], address[5]);

		_extFpgaRead3Bytes(fpga->rxAddress->addrStreamVideo->addrIp, &intValue);
		_extFpgaReadShort(fpga->rxAddress->addrStreamVideo->addrPort,  (unsigned char *)&port);
		_extFpgaReadByte(fpga->rxAddress->addrStreamVideo->addrRtpPayload, &_chVal);
		_chVal = (_chVal&0x7F);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"Dest :"EXT_NEW_LINE"\tVideo :\tIP:%s;\tPort:%d;\tRtpType:%d", cmnSysNetAddress(intValue), port, _chVal);

		_extFpgaRead3Bytes(fpga->rxAddress->addrStreamAudio->addrIp, &intValue);
		_extFpgaReadShort(fpga->rxAddress->addrStreamAudio->addrPort,  (unsigned char *)&port);
		_extFpgaReadByte(fpga->rxAddress->addrStreamAudio->addrRtpPayload, &_chVal);
		_chVal = (_chVal&0x7F);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tAudio :\tIP:%s;\tPort:%d;\tRtpType:%d", cmnSysNetAddress(intValue), port, _chVal);

		_extFpgaRead3Bytes(fpga->rxAddress->addrStreamAnc->addrIp, &intValue);
		_extFpgaReadShort(fpga->rxAddress->addrStreamAnc->addrPort,  (unsigned char *)&port);
		_extFpgaReadByte(fpga->rxAddress->addrStreamAnc->addrRtpPayload, &_chVal);
		_chVal = (_chVal&0x7F);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tANC : \tIP:%s;\tPort:%d;\tRtpType:%d", cmnSysNetAddress(intValue), port, _chVal);

#if EXT_FPGA_AUX_ON	
		_extFpgaRead3Bytes(fpga->rxAddress->addrStreamAux->addrIp, &intValue);
		_extFpgaReadShort(fpga->rxAddress->addrStreamAux->addrPort,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tAUX : \tIP:%s;\tPort:%d;", cmnSysNetAddress(intValue), port);
#endif

	}

	fpgaReadParamRegisters(addrMedia, runCfg);

	index += snprintf(data+index, size-index, EXT_NEW_LINE"Video :\tWxH:%hux%hu; ", runCfg->runtime.vWidth, runCfg->runtime.vHeight);
	index += snprintf(data+index, size-index, "FPS:%s; ", CMN_FIND_V_FPS_4_REST(runCfg->runtime.vFrameRate));
	index += snprintf(data+index, size-index, "Depth:%d; ", CMN_INT_FIND_NAME_V_DEPTH(runCfg->runtime.vDepth) );
	index += snprintf(data+index, size-index, "ColorSpace:%s; ", CMN_FIND_V_COLORSPACE(runCfg->runtime.vColorSpace));
	index += snprintf(data+index, size-index, "%s;"EXT_NEW_LINE, (runCfg->runtime.vIsInterlaced == EXT_VIDEO_INTLC_INTERLACED)?"Interlaced":"Progressive");

	index += snprintf(data+index, size-index, "Audio :\tChannels:%d; Sample Rate:%d; Pkt Size: %s"EXT_NEW_LINE, 
		runCfg->runtime.aChannels, runCfg->runtime.aSampleRate, CMN_FIND_A_PKTSIZE(runCfg->runtime.aPktSize) );

	index += snprintf(data+index, size-index, "RTP Payload :\tVideo:%d; Audio:%d; ANC: %d"EXT_NEW_LINE, 
		runCfg->runtime.rtpTypeVideo, runCfg->runtime.rtpTypeAudio, runCfg->runtime.rtpTypeAnc );

	_extFpgaReadByte(EXT_FPGA_REG_ENABLE, &_chVal);
	_extFpgaReadByte(EXT_FPGA_REG_PARAM_STATUS, address);
	index += snprintf(data+index, size-index, "EnableReg :%02x;\tUpdateRgr:%02x"EXT_NEW_LINE, _chVal, address[0] );
	
	return index;
}


