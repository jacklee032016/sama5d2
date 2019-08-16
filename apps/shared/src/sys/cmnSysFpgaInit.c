
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
	FpgaVersionAddress		*verAddr = NULL;

	if(FPGA_IS_TX(fpga) )
	{
		verAddr = fpga->txAddress->version;
	}
	else
	{
		verAddr = fpga->rxAddress->version;
	}
	
	_extFpgaReadByte(&verAddr->version, &runCfg->runtime.version);
	_extFpgaReadByte(&verAddr->revision, &runCfg->runtime.revision);
	_extFpgaReadByte(&verAddr->year, &runCfg->runtime.year);
	_extFpgaReadByte(&verAddr->month, &runCfg->runtime.month);
	_extFpgaReadByte(&verAddr->day, &runCfg->runtime.day);
	_extFpgaReadByte(&verAddr->hour, &runCfg->runtime.hour);
	_extFpgaReadByte(&verAddr->minute, &runCfg->runtime.minute);

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
	
//	unsigned char baseAddr = address->offset;

	for(i=0; i < size; i++)
	{
//		I2C_EXT_READ( address->device->bus,address->device->bus,address->device->bus, address+i, 1, index+i, 1);
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
	index += snprintf(data+index, size-index, "\t%s: \tIP:%s;", label, cmnSysNetAddress(intValue));
	index += snprintf(data+index, size-index, "\tMAC:" );
	index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x;", address[0], address[1], address[2], address[3], address[4], address[5]);
	index += snprintf(data+index, size-index, "\tPort:%d;\tRtpType:%d", port, _chVal);

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
	index += snprintf(data+index, size-index, "\t%s :\tIP:%s;\tPort:%d;\tRtpType:%d", label, cmnSysNetAddress(intValue), port, _chVal);

	return index;
}


int sysFgpaRegistersDebug( char *data, unsigned int size)
{
	int index = 0;
//	unsigned char val;
	unsigned short port = 0;
	unsigned int	intValue;
	EXT_MAC_ADDRESS destMac;
	unsigned char		address[EXT_MAC_ADDRESS_LENGTH], _chVal;
	
	MediaRegisterAddress		*addrMedia = NULL;

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
		addrMedia = fpga->txAddress->media;
		
		_extFpgaReadInteger(&fpga->txAddress->localIp, (unsigned char *)&intValue);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"Local :"EXT_NEW_LINE"\tIP\t: %s", cmnSysNetAddress(intValue));

		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tMAC\t:" );
		FPGA_READ(&fpga->txAddress->localMac,  (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
		cmnSysNetChangeByteOrderOfMac(&destMac, address);
		index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x:", address[0], address[1], address[2], address[3], address[4], address[5]);

		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tPort\t: Video:" );
		_extFpgaReadShort(&fpga->txAddress->localPortV,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;", port);

		index += snprintf(data+index, size-index," Audio:" );
		_extFpgaReadShort(&fpga->txAddress->localPortA,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;", port);

		index += snprintf(data+index, size-index, " ANC:" );
		_extFpgaReadShort(&fpga->txAddress->localPortAnc,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;", port);

#if EXT_FPGA_AUX_ON	
		index += snprintf(data+index, size-index, " AUX:" );
		_extFpgaReadShort(&fpga->txAddress->localPortAuc,  (unsigned char *)&port);
		index += snprintf(data+index, size-index, "%d;"EXT_NEW_LINE, port);
#endif

		index += snprintf(data+index, size-index, EXT_NEW_LINE"Dest :"EXT_NEW_LINE);
		index += _debugOneStreamTx("Video", fpga->txAddress->streamVideo, data+index, size-index);

		/* audio */
		index += _debugOneStreamTx("Audio", fpga->txAddress->streamAudio, data+index, size-index);

		index += _debugOneStreamTx("ANC", fpga->txAddress->streamAnc, data+index, size-index);

#if EXT_FPGA_AUX_ON	
		index += _debugOneStreamTx("AUX", fpga->txAddress->streamAux, data+index, size-index);
#endif		
		FPGA_I2C_READ(EXT_FPGA_REG_SDI_STATUS, &_chVal, 1);
		index += snprintf(data+index, size-index, "LockReg :%02x;"EXT_NEW_LINE, _chVal );
	}
	else
	{
		addrMedia = fpga->rxAddress->media;

		_extFpgaReadInteger(&fpga->rxAddress->localIp, (unsigned char *)&intValue);
		index += snprintf(data+index, size-index, EXT_NEW_LINE"Local :"EXT_NEW_LINE"\tIP\t: %s", cmnSysNetAddress(intValue));

		index += snprintf(data+index, size-index, EXT_NEW_LINE"\tMAC\t:" );
		FPGA_READ(&fpga->rxAddress->localMac,  (unsigned char *)&destMac, EXT_MAC_ADDRESS_LENGTH);
		cmnSysNetChangeByteOrderOfMac(&destMac, address);
		index += snprintf(data+index, size-index, " %02x:%02x:%02x:%02x:%02x:%02x:", address[0], address[1], address[2], address[3], address[4], address[5]);

		index += _debugOneStreamRx("Video", fpga->rxAddress->streamVideo, data+index, size-index);

		index += _debugOneStreamRx("Audio", fpga->rxAddress->streamAudio, data+index, size-index);

		index += _debugOneStreamRx("ANC", fpga->rxAddress->streamAnc, data+index, size-index);

#if EXT_FPGA_AUX_ON	
		index += _debugOneStreamRx("AUX", fpga->rxAddress->streamAux, data+index, size-index);
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

	FPGA_I2C_READ(EXT_FPGA_REG_ENABLE, &_chVal, 1);
	FPGA_I2C_READ(EXT_FPGA_REG_PARAM_STATUS, address, 1);
	index += snprintf(data+index, size-index, "EnableReg :%02x;\tUpdateRgr:%02x"EXT_NEW_LINE, _chVal, address[0] );
	
	return index;
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
		FPGA_I2C_READ(EXT_FPGA_REG_ENABLE, &value, 1);
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

