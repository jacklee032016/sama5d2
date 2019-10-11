
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>
#include <stdint.h>

#include "libCmnSys.h"
#include "mux7xx.h"

#include "muxFpga.h"


static char _fpgaVersion[128];

/* read hw info, such as TX or RX, version */
static int _sysFpgaReadHwInfo(FpgaConfig 	*fpga)
{
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;

	_extFpgaReadByte(&fpga->version->version, &runCfg->runtime.version);
	_extFpgaReadByte(&fpga->version->revision, &runCfg->runtime.revision);

	if( (runCfg->runtime.version &0x80) )
	{
		runCfg->isTx = 1;
	}
	else
	{
		runCfg->isTx = 0;
	}
	MUX_DEBUG("FPGA Firmware is %s", FPGA_IS_TX(fpga)?"TX":"RX");

	EXT_DEBUGF(MUX_DEBUG_FPGA, "version:0x%x; revision:0x%x", runCfg->runtime.version, runCfg->runtime.revision);

	return EXIT_SUCCESS;
}




/* check hw info and read TX/RX */
int	sysFpgaCheck(EXT_RUNTIME_CFG *runCfg )
{
	FpgaConfig 	*fpga =  &_fpgaConfig;
	FpgaBuildTimeAddress				*buildTimeAddr = NULL;
	
	fpga->runCfg = runCfg;
	runCfg->fpgaCfg = fpga;

	if(_sysFpgaReadHwInfo(fpga) == EXIT_FAILURE)
	{
		EXT_ERRORF("Read from FPGA failed");
		return EXIT_FAILURE;
	}

	if(FPGA_IS_TX(fpga))
	{
		buildTimeAddr = fpga->txAddress->buildTime;
	}
	else
	{
		buildTimeAddr = fpga->rxAddress->buildTime;
	}
	
#if 1	
	_extFpgaReadByte(&buildTimeAddr->year, &runCfg->runtime.year);
	_extFpgaReadByte(&buildTimeAddr->month, &runCfg->runtime.month);
	_extFpgaReadByte(&buildTimeAddr->day, &runCfg->runtime.day);
	_extFpgaReadByte(&buildTimeAddr->hour, &runCfg->runtime.hour);
	_extFpgaReadByte(&buildTimeAddr->minute, &runCfg->runtime.minute);

	EXT_DEBUGF(MUX_DEBUG_FPGA, "version:0x%x; revision:0x%x, build:%2d-%2d-20%2d %2d:%2d", 
		runCfg->runtime.version, runCfg->runtime.revision, runCfg->runtime.month, runCfg->runtime.day, runCfg->runtime.year,runCfg->runtime.hour, runCfg->runtime.minute );
#endif

	return EXIT_SUCCESS;
}


char *sysFgpaVersion(void)
{
	int index = 0;
	FpgaConfig 	*fpga =  &_fpgaConfig;
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;
	MuxRunTimeParam *mediaParams = &runCfg->runtime;

#if 0	
	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, "%s, Ver:%02x", 
		((mediaParams->version&EXT_FPGA_TX_FLAGS)==0)?"RX":"TX", (mediaParams->version&(~EXT_FPGA_TX_FLAGS)) );
	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, ".%02x", mediaParams->revision);
#else
	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, "%02x.%02x", 
		(mediaParams->version&(~EXT_FPGA_TX_FLAGS)) , mediaParams->revision);
#endif

	return _fpgaVersion;
}

char *sysFgpaBuilt(void)
{
	int index = 0;
	FpgaConfig 	*fpga =  &_fpgaConfig;
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;
	MuxRunTimeParam *mediaParams = &runCfg->runtime;
	
	index += snprintf(_fpgaVersion+index, sizeof(_fpgaVersion) - index, "%02x-%02x-20%02x %02x:%02x", 
		mediaParams->month, mediaParams->day, mediaParams->year, mediaParams->hour, mediaParams->minute);

	return _fpgaVersion;
}



int _debugOneStreamTx(char *label, StreamRegisterAddress *streamAddr, char *data, unsigned int size )
{
	int index = 0;
	unsigned short port = 0;
	unsigned int	intValue;

	EXT_MAC_ADDRESS destMac;
	unsigned char		address[EXT_MAC_ADDRESS_LENGTH], _chVal;
	
	_extFpgaReadInteger(&streamAddr->ip, (unsigned char *)&intValue);
	FPGA_READ(&streamAddr->mac, (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
	cmnSysNetChangeByteOrderOfMac(&destMac, address);
	_extFpgaReadShort(&streamAddr->port,  (unsigned char *)&port);
	_extFpgaReadByte(&streamAddr->rtpPayload, &_chVal);
	_chVal = (_chVal&0x7F);
	index += snprintf(data+index, size-index, "\t%s\t: IP:%s;", label, cmnSysNetAddress(intValue));
	index += snprintf(data+index, size-index, "\tMAC:" );
	index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x;", address[0], address[1], address[2], address[3], address[4], address[5]);
	index += snprintf(data+index, size-index, "\tPort:%d;\tRtpType:%d"EXT_NEW_LINE, port, _chVal);

	return index;
}

int _debugOneStreamRx(char *label, StreamRegisterAddress *streamAddr, char *data, unsigned int size )
{
	int index = 0;
	unsigned short port = 0;
	unsigned int	intValue;
	unsigned char	_chVal;

	_extFpgaRead3Bytes(&streamAddr->ip, &intValue);
	_extFpgaReadShort(&streamAddr->port,  (unsigned char *)&port);
	_extFpgaReadByte(&streamAddr->rtpPayload, &_chVal);
	_chVal = (_chVal&0x7F);
	index += snprintf(data+index, size-index, "\t%s\t:\tIP:%s;\tPort:%d;\tRtpType:%d"EXT_NEW_LINE, label, cmnSysNetAddress(intValue), port, _chVal);

	return index;
}


int sysFpgaRegistersDebug( char *data, unsigned int size)
{
	int index = 0;
//	unsigned char val;
	unsigned short port = 0;
	unsigned int	intValue;
	EXT_MAC_ADDRESS destMac;
	unsigned char		address[EXT_MAC_ADDRESS_LENGTH], _chVal;
	
	MediaRegisterAddress		*addrMedia = NULL;
	FpgaConfig 	*fpga =  &_fpgaConfig;
	unsigned short videoCount, audioCount;
	
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;
	if(runCfg == NULL)
	{
		index += snprintf(data+index, size-index, "FPGA is not initialized now"EXT_NEW_LINE);
		return EXIT_FAILURE;
	}

	index += snprintf(data+index, size-index, "Version:%s; Built: %s"EXT_NEW_LINE, sysFgpaVersion(), sysFgpaBuilt() );
	index += snprintf(data+index, size-index, "%s configuration: ", EXT_IS_TX(runCfg)?"TX":"RX");

	if(EXT_IS_TX(runCfg) )
	{
		addrMedia = fpga->txAddress->media;
		
		_extFpgaReadInteger(&fpga->txAddress->localIp, (unsigned char *)&intValue);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"Local :"EXT_NEW_LINE"\tIP\t: %s", cmnSysNetAddress(intValue));

		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tMAC\t:" );
		FPGA_READ(&fpga->txAddress->localMac,  (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
		cmnSysNetChangeByteOrderOfMac(&destMac, address);
		index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4], address[5]);

		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tPort\t: Video:" );
		_extFpgaReadShort(&fpga->txAddress->localPortV,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;", port);

		index += snprintf(data+index, size-index," Audio:" );
		_extFpgaReadShort(&fpga->txAddress->localPortA,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;", port);

#if WITH_ANCILLIARY_STREAM
		index += snprintf(data+index, size-index, " ANC:" );
		_extFpgaReadShort(&fpga->txAddress->localPortAnc,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;", port);
#endif

#if EXT_FPGA_AUX_ON	
		index += snprintf(data+index, size-index, " AUX:" );
		_extFpgaReadShort(&fpga->txAddress->localPortAuc,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;"EXT_NEW_LINE, port);
#endif

		index += snprintf(data+index, size-index, EXT_NEW_LINE"Dest :"EXT_NEW_LINE);
		index += _debugOneStreamTx("Video", fpga->txAddress->streamVideo, data+index, size-index);

		/* audio */
		index += _debugOneStreamTx("Audio", fpga->txAddress->streamAudio, data+index, size-index);

#if WITH_ANCILLIARY_STREAM
		index += _debugOneStreamTx("ANC", fpga->txAddress->streamAnc, data+index, size-index);
#endif

#if EXT_FPGA_AUX_ON	
		index += _debugOneStreamTx("AUX", fpga->txAddress->streamAux, data+index, size-index);
#endif		

		index += snprintf(data+index, size-index, EXT_NEW_LINE"Statistics\t:" );
		_extFpgaReadShort(&fpga->rxAddress->streamVideo->count,  (unsigned char *)&videoCount);
		_extFpgaReadShort(&fpga->rxAddress->streamAudio->count,  (unsigned char *)&audioCount);
		index += snprintf(data+index, size-index, "Video: %d;\tAudio: %d;", videoCount, audioCount);

		_extFpgaReadShort(&fpga->rxAddress->gbeRxCount,  (unsigned char *)&videoCount);
		_extFpgaReadShort(&fpga->rxAddress->gbeTxCount,  (unsigned char *)&audioCount);
		index += snprintf(data+index, size-index, "\tGBE: tx: %d;\trx: %d"EXT_NEW_LINE, audioCount, videoCount);

		FPGA_I2C_READ(EXT_FPGA_REG_SDI_STATUS, &_chVal, 1);
		index += snprintf(data+index, size-index, "\tLockReg :%02x;"EXT_NEW_LINE, _chVal );

	}
	else
	{
		
		addrMedia = fpga->rxAddress->media;

		_extFpgaReadInteger(&fpga->rxAddress->localIp, (unsigned char *)&intValue);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"Local :"EXT_NEW_LINE"\tIP\t: %s", cmnSysNetAddress(intValue));

		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tMAC\t:" );
		FPGA_READ(&fpga->rxAddress->localMac,  (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
		cmnSysNetChangeByteOrderOfMac(&destMac, address);
		index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4], address[5]);

		index += snprintf(data+index, size-index, EXT_NEW_LINE"Streams\t:"EXT_NEW_LINE );
		index += _debugOneStreamRx("Video", fpga->rxAddress->streamVideo, data+index, size-index);

		index += _debugOneStreamRx("Audio", fpga->rxAddress->streamAudio, data+index, size-index);

#if WITH_ANCILLIARY_STREAM
		index += _debugOneStreamRx("ANC", fpga->rxAddress->streamAnc, data+index, size-index);
#endif

#if EXT_FPGA_AUX_ON	
		index += _debugOneStreamRx("AUX", fpga->rxAddress->streamAux, data+index, size-index);
#endif

		index += snprintf(data+index, size-index, EXT_NEW_LINE"Statistics\t:" );
		_extFpgaReadShort(&fpga->rxAddress->streamVideo->count,  (unsigned char *)&videoCount);
		_extFpgaReadShort(&fpga->rxAddress->streamAudio->count,  (unsigned char *)&audioCount);
		index += snprintf(data+index, size-index, "Video: %d;\tAudio: %d;", videoCount, audioCount);

		_extFpgaReadShort(&fpga->rxAddress->gbeRxCount,  (unsigned char *)&videoCount);
		_extFpgaReadShort(&fpga->rxAddress->gbeTxCount,  (unsigned char *)&audioCount);
		index += snprintf(data+index, size-index, "\tGBE: tx: %d;\trx: %d"EXT_NEW_LINE, audioCount, videoCount);

	}

	fpgaReadParamRegisters(addrMedia, runCfg);

	index += snprintf(data+index, size-index, EXT_NEW_LINE"Video :\tWxH:%hux%hu; ", runCfg->runtime.vWidth, runCfg->runtime.vHeight);
	index += snprintf(data+index, size-index, "FPS:%s; ", CMN_FIND_V_FPS_4_REST(runCfg->runtime.vFrameRate));
	index += snprintf(data+index, size-index, "Depth:%d; ", CMN_INT_FIND_NAME_V_DEPTH(runCfg->runtime.vDepth) );
	index += snprintf(data+index, size-index, "ColorSpace:%s; ", CMN_FIND_V_COLORSPACE(runCfg->runtime.vColorSpace));
	index += snprintf(data+index, size-index, "%s;"EXT_NEW_LINE, (runCfg->runtime.vIsInterlaced == EXT_VIDEO_INTLC_INTERLACED)?"Interlaced":"Progressive");

	index += snprintf(data+index, size-index, "Audio :\tChannels:%d; Sample Rate:%d; Depth: %d; Pkt Size: %s"EXT_NEW_LINE, 
		runCfg->runtime.aChannels, runCfg->runtime.aSampleRate, runCfg->runtime.aDepth, CMN_FIND_A_PKTSIZE(runCfg->runtime.aPktSize) );

#if WITH_ANCILLIARY_STREAM
	index += snprintf(data+index, size-index, "RTP Payload :\tVideo:%d; Audio:%d; ANC: %d"EXT_NEW_LINE, 
		runCfg->runtime.rtpTypeVideo, runCfg->runtime.rtpTypeAudio, runCfg->runtime.rtpTypeAnc );
#else
	index += snprintf(data+index, size-index, "RTP Payload :\tVideo:%d; Audio:%d"EXT_NEW_LINE, 
		runCfg->runtime.rtpTypeVideo, runCfg->runtime.rtpTypeAudio );
#endif

	FPGA_I2C_READ(EXT_FPGA_REG_ENABLE, &_chVal, 1);
	FPGA_I2C_READ(EXT_FPGA_REG_PARAM_STATUS, address, 1);
	index += snprintf(data+index, size-index, "EnableReg :%02x;\tUpdateRgr:%02x"EXT_NEW_LINE, _chVal, address[0] );
	
	return index;
}

/* read params from registers */
int	sysFpgaRefresh(void )
{
	FpgaConfig 	*fpga =  &_fpgaConfig;

	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;
	if(!runCfg || !fpga->opMediaRead )
	{
		return EXIT_SUCCESS;
	}

	if(EXT_IS_TX(runCfg) )
	{
		fpga->opMediaRead(fpga);

	}

		{
			char 	buf[4096];
			sysFpgaRegistersDebug(buf, sizeof(buf));

			printf(buf);
		}
	
	return EXIT_SUCCESS;
}

int	sysFpgaInit(EXT_RUNTIME_CFG *runCfg )
{
	FpgaConfig 	*fpga =  &_fpgaConfig;
//	char	value;
	
	fpga->runCfg = runCfg;
	runCfg->fpgaCfg = fpga;
	
//	_fpgaRegisterWrite(EXT_FPGA_REG_PORT_AUDIO, (unsigned char *)&runCfg->local.aport, 2);

#if 0	
	if(! IS_SECURITY_CHIP_EXIST(runCfg->muxMain) || cmnSysScValidate(runCfg->muxMain->scf)== EXIT_FAILURE)
	{
		value = EXT_FPGA_FLAGS_DISABLE_ALL;	
		FPGA_I2C_READ(EXT_FPGA_REG_ENABLE, &value, 1);
		return EXIT_FAILURE;
	}
#endif

	sysFpgaCheck(runCfg);

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

	if(EXT_IS_TX(runCfg) )
	{
		fpga->opMediaRead(fpga);
	}
	else
	{
		fpga->opMediaWrite(fpga);
	}

	{
		char 	buf[4096];
		sysFpgaRegistersDebug(buf, sizeof(buf));

		printf(buf);
	}

	return EXIT_SUCCESS;
}


int	sysFpgaReadVideoStatus(void)
{
	FpgaI2cAddress		*reg;
	FpgaConfig 	*fpga =  &_fpgaConfig;
	unsigned short _shortValue;


	if(fpga->runCfg == NULL)
		return EXT_FALSE;
	
	if(EXT_IS_TX(fpga->runCfg) )
	{
		reg = &fpga->txAddress->media->width;
	}
	else
	{
		reg = &fpga->rxAddress->media->width;
	}

	_extFpgaReadShort(reg, &_shortValue);

	if(_shortValue)
	{
		return EXT_TRUE;
	}

	return EXT_FALSE;
}


int	sysFpgaReadFpsStatus(void)
{
	FpgaI2cAddress		*reg, *reg2;
	FpgaConfig 	*fpga =  &_fpgaConfig;
	unsigned short rxCount, txCount;


	if(fpga->runCfg == NULL)
		return EXT_FALSE;
	
	if(EXT_IS_TX(fpga->runCfg) )
	{
		reg = &fpga->txAddress->gbeRxCount;
		reg2 = &fpga->txAddress->gbeTxCount;
	}
	else
	{
		reg = &fpga->rxAddress->gbeRxCount;
		reg2 = &fpga->rxAddress->gbeTxCount;
	}

	_extFpgaReadShort(reg, &rxCount);
	_extFpgaReadShort(reg2, &txCount);

	if( (rxCount!= INVALIDATE_VALUE_U16) &&(txCount != INVALIDATE_VALUE_U16) && (rxCount+txCount)!= 0 )
	{
		return EXT_TRUE;
	}

	return EXT_FALSE;
}


int	sysFpgaReadIrDemodulation(unsigned char *freq)
{
	FpgaConfig 	*fpga =  &_fpgaConfig;
	FpgaI2cAddress		*regIrDemodulation;

	if(fpga->opProtocolCtrl == NULL)
	{
		EXT_ERRORF("FPGA is not initialized");

		return EXIT_FAILURE;
	}

	if(EXT_IS_TX(fpga->runCfg) )
	{
		regIrDemodulation = &fpga->txAddress->irDemodulation;
	}
	else
	{
		regIrDemodulation = &fpga->rxAddress->irDemodulation;
	}

	_extFpgaReadByte(regIrDemodulation, freq);

	return EXIT_SUCCESS;
}


int	sysFpgaWriteIrCtrl(unsigned char isIrPowerOn, unsigned char isIrOut, unsigned char freq)
{
	FpgaConfig 	*fpga =  &_fpgaConfig;
	FpgaI2cAddress		*regIrCtrl;
	unsigned char value, tmp;

	if(fpga->opProtocolCtrl == NULL)
	{
		EXT_ERRORF("FPGA is not initialized");
		return EXIT_FAILURE;
	}

	value = ((isIrPowerOn==0)?0x00:0x40) |((isIrOut==0)?0x00:0x10)|(freq&0x0F); 
	value |= 0x20; /* ??? */

	if(EXT_IS_TX(fpga->runCfg) )
	{
		regIrCtrl = &fpga->txAddress->irCtrl;
	}
	else
	{
		regIrCtrl = &fpga->rxAddress->irCtrl;
	}

	_extFpgaWriteByte(regIrCtrl, &value);

	_extFpgaReadByte(regIrCtrl, &tmp);

	EXT_DEBUGF(MUX_DEBUG_FPGA, "IR Control:0x%x(write); 0x%x(read)", value, tmp );

	return EXIT_SUCCESS;
}


