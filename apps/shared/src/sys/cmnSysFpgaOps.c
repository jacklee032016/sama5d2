
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>
#include <stdint.h>

#include "libCmnSys.h"
#include "mux7xx.h"

#include "muxFpga.h"
#if ARCH_ARM
#define	LONG_FORMAT	"%"
#else
#define	LONG_FORMAT	"%l"
#endif

/*
*/
#if 0
#define	FPGA_UPDATE_PTP_TIMESTAMP(fpga) \
	do{	/* write timestamp */ uint32_t fpgaTs = FPGA_GET_PTP_TIMESTAMP_VIDEO(); \
		EXT_INFOF("Write RtpTimestamp: "LONG_FORMAT"u(0x"LONG_FORMAT"x)", fpgaTs, fpgaTs); \
		_extFpgaWriteIntegerChange(&fpga->txAddress->rtpTimestampVideo, &fpgaTs); \
		fpgaTs = FPGA_GET_PTP_TIMESTAMP_AUDIO(); _extFpgaWriteIntegerChange(&fpga->txAddress->rtpTimestampAudio, &fpgaTs);  \
		}while(0)
#else
int	FPGA_UPDATE_PTP_TIMESTAMP(FpgaConfig *fpga)
{
	uint32_t readTs;
	_extFpgaReadIntegerChange(&fpga->txAddress->rtpTimestampVideo, &readTs);
	uint32_t fpgaTs = FPGA_GET_PTP_TIMESTAMP_VIDEO();
	_extFpgaWriteIntegerChange(&fpga->txAddress->rtpTimestampVideo, &fpgaTs);
	EXT_INFOF("Video: Read Timestamp: "LONG_FORMAT"u; \tWrite Timestamp: "LONG_FORMAT"u(0x"LONG_FORMAT"x), current:%llu", readTs, fpgaTs, fpgaTs, cmnGetTimeUs());

	_extFpgaReadIntegerChange(&fpga->txAddress->rtpTimestampAudio, &readTs);
	fpgaTs = FPGA_GET_PTP_TIMESTAMP_AUDIO(); 
	_extFpgaWriteIntegerChange(&fpga->txAddress->rtpTimestampAudio, &fpgaTs);
	EXT_INFOF("Audio: Read Timestamp: "LONG_FORMAT"u; \tWrite Timestamp: "LONG_FORMAT"u(0x"LONG_FORMAT"x), current:%llu", readTs, fpgaTs, fpgaTs, cmnGetTimeUs() );

	{
		struct timeval tv;

		gettimeofday(&tv,NULL);
		uint64_t _ts = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
		uint64_t _readTs;

		FPGA_I2C_READ(F_REG_TX_GRAND_MASTER_TIMESTAMP, &_readTs, 8);
		FPGA_I2C_WRITE(F_REG_TX_GRAND_MASTER_TIMESTAMP, &_ts, 8);

		EXT_INFOF("Grand Master Timestamp: \tRead: %llu; Write:%llu", _readTs, _ts );
	}

	return EXIT_SUCCESS;
}
#endif

int	sysFpgaTxConfig(FpgaConfig *fpga)
{
//	unsigned char		chValue;
//		const ip4_addr_t *mcIpAddr;
//		unsigned int	intValue;
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;
//	EXT_VIDEO_CONFIG *vCfg = &runCfg->dest;
	unsigned char		address[EXT_MAC_ADDRESS_LENGTH];
	
	EXT_MAC_ADDRESS	destMac, *mac;
	
	uint32_t currentIp = cmnSysNetGetIp(MUX_ETH_DEVICE);
	
	unsigned int ip = (currentIp == INVALIDATE_VALUE_U32)?runCfg->local.ip:currentIp;

	/* local */
	cmnSysNetChangeByteOrderOfMac(&runCfg->local.mac, address);
	FPGA_WRITE( &fpga->txAddress->localMac, address, EXT_MAC_ADDRESS_LENGTH);

	_extFpgaWriteInteger( &fpga->txAddress->localIp, &ip);

	_extFpgaWriteShort(&fpga->txAddress->localPortV, (unsigned char *)&runCfg->local.vport);
	
	_extFpgaWriteShort(&fpga->txAddress->localPortA, (unsigned char *)&runCfg->local.aport);
#if WITH_ANCILLIARY_STREAM
	_extFpgaWriteShort(&fpga->txAddress->localPortAnc, (unsigned char *)&runCfg->local.dport);
#endif

#if EXT_FPGA_AUX_ON	
	_extFpgaWriteShort(&fpga->txAddress->localPortAuc, (unsigned char *)&runCfg->local.sport);
#endif

#if WITH_ANCILLIARY_STREAM
	EXT_DEBUGF(MUX_DEBUG_FPGA, "TX:IP:%s; portV:%d; portA:%d; portAnc:%d", cmnSysNetAddress(ip), runCfg->local.vport, runCfg->local.aport,runCfg->local.dport);
#else
	EXT_DEBUGF(MUX_DEBUG_FPGA, "TX Local:MAC:%02x:%02x:%02x:%02x:%02x:%02x; IP:%s; portV:%d; portA:%d", 
		address[0], address[1], address[2], address[3], address[4], address[5], cmnSysNetAddress(ip), runCfg->local.vport, runCfg->local.aport);
#endif

	/* dest */
	/* video */
	if(cmnSysNetMulticastIP4Mac(runCfg->dest.ip, &destMac) == EXIT_SUCCESS)
	{/* dest MAC is multicast MAC address */
		mac = &destMac;
	}
	else
	{
		mac = &runCfg->dest.mac;
	}
	cmnSysNetChangeByteOrderOfMac(mac, address);
	FPGA_WRITE(&fpga->txAddress->streamVideo->mac, address, EXT_MAC_ADDRESS_LENGTH);

	_extFpgaWriteInteger(&fpga->txAddress->streamVideo->ip, &runCfg->dest.ip);
	_extFpgaWriteShort(&fpga->txAddress->streamVideo->port, (unsigned char *)&runCfg->dest.vport);

	EXT_DEBUGF(MUX_DEBUG_FPGA, "Video:MAC:%02x:%02x:%02x:%02x:%02x:%02x; IP:%s; port:%d", 
		address[0], address[1], address[2], address[3], address[4], address[5], cmnSysNetAddress(runCfg->dest.ip), runCfg->dest.vport);
	
	/* audio */
	if(cmnSysNetMulticastIP4Mac(runCfg->dest.audioIp, &destMac) == EXIT_SUCCESS)
	{/* dest MAC is multicast MAC address */
		mac = &destMac;
	}
	else
	{
		mac = &runCfg->dest.mac;
	}
	cmnSysNetChangeByteOrderOfMac(mac, address);
	FPGA_WRITE(&fpga->txAddress->streamAudio->mac, address, EXT_MAC_ADDRESS_LENGTH);

	_extFpgaWriteInteger(&fpga->txAddress->streamAudio->ip, &runCfg->dest.audioIp);
	_extFpgaWriteShort(&fpga->txAddress->streamAudio->port, (unsigned char *)&runCfg->dest.aport);

	EXT_DEBUGF(MUX_DEBUG_FPGA, "Audio:MAC:%02x:%02x:%02x:%02x:%02x:%02x; IP:%s; port:%d", 
		address[0], address[1], address[2], address[3], address[4], address[5], cmnSysNetAddress(runCfg->dest.audioIp), runCfg->dest.aport);

#if WITH_ANCILLIARY_STREAM
	/* ANC */
	if(cmnSysNetMulticastIP4Mac(runCfg->dest.ancIp, &destMac) == EXIT_SUCCESS)
	{/* dest MAC is multicast MAC address */
		mac = &destMac;
	}
	else
	{
		mac = &runCfg->dest.mac;
	}
	cmnSysNetChangeByteOrderOfMac(mac, address);
	FPGA_WRITE(&fpga->txAddress->streamAnc->mac, address, EXT_MAC_ADDRESS_LENGTH);

	_extFpgaWriteInteger(&fpga->txAddress->streamAnc->ip, &runCfg->dest.ancIp);
	_extFpgaWriteShort(&fpga->txAddress->streamAnc->port, (unsigned char *)&runCfg->dest.dport);
	EXT_DEBUGF(MUX_DEBUG_FPGA, "ANC:MAC:%02x:%02x:%02x:%02x:%02x:%02x; IP:%s; port:%d", 
		address[0], address[1], address[2], address[3], address[4], address[5], cmnSysNetAddress(runCfg->dest.ancIp), runCfg->dest.dport);
#endif

	/* AUX */
#if EXT_FPGA_AUX_ON	
	if(cmnSysNetMulticastIP4Mac(runCfg->dest.auxIp, &destMac) == EXIT_SUCCESS)
	{/* dest MAC is multicast MAC address */
		mac = &destMac;
	}
	else
	{
		mac = &runCfg->dest.mac;
	}
	cmnSysNetChangeByteOrderOfMac(mac, address);
	FPGA_WRITE(&fpga->txAddress->streamAux->addrMac, address, EXT_MAC_ADDRESS_LENGTH);

	_extFpgaWriteInteger(&fpga->txAddress->streamAux->ip, &runCfg->dest.auxIp);
	_extFpgaWriteShort(&fpga->txAddress->streamAux->port, (unsigned char *)&runCfg->dest.sport);
#endif/* AUX */

	/* enable all streams */
	address[0] = INVALIDATE_VALUE_U8;
	_extFpgaWriteByte(&fpga->txAddress->enable, address);

	address[0] = 0x07;
	FPGA_I2C_WRITE(0x1A, address, 1);

	address[0] = INVALIDATE_VALUE_U8;
	FPGA_I2C_WRITE(0x1B, address, 1);

	return EXIT_SUCCESS;
}




int	sysFpgaRxConfig(FpgaConfig *fpga)
{
	unsigned char		chValue;
//		const ip4_addr_t *mcIpAddr;
//		unsigned int	intValue;
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;
	EXT_VIDEO_CONFIG *vCfg = &runCfg->dest;
	unsigned char		address[EXT_MAC_ADDRESS_LENGTH];
	
	uint32_t currentIp = cmnSysNetGetIp(MUX_ETH_DEVICE);
	
	unsigned int ip = (currentIp == INVALIDATE_VALUE_U32)?runCfg->local.ip:currentIp;
#if 0
	if(_netIfIpAddr->addr != IPADDR_ANY)
	{
		return EXIT_SUCCESS;
	}
#endif
	EXT_INFOF("RX is configuring...");

	/* local IP/MAC */
	_extFpgaWriteInteger(&fpga->rxAddress->localIp, &ip);
//		cmnSysNetChangeByteOrderOfMac(&vCfg->mac, address);
	cmnSysNetChangeByteOrderOfMac(&runCfg->local.mac, address);
	FPGA_WRITE(&fpga->rxAddress->localMac, address, EXT_MAC_ADDRESS_LENGTH);

	/* dest multi Video */
	_extFpgaWrite3Bytes(&fpga->rxAddress->streamVideo->ip, &vCfg->ip);
	_extFpgaWriteShort(&fpga->rxAddress->streamVideo->port, (unsigned char *)&vCfg->vport );

	/* dest multi Audio */
	_extFpgaWrite3Bytes(&fpga->rxAddress->streamAudio->ip, &vCfg->audioIp);
	_extFpgaWriteShort(&fpga->rxAddress->streamAudio->port, (unsigned char *)&vCfg->aport );

	/* dest multi Anx */
#if WITH_ANCILLIARY_STREAM
	_extFpgaWrite3Bytes(&fpga->rxAddress->streamAnc->ip, &vCfg->ancIp);
	_extFpgaWriteShort(&fpga->rxAddress->streamAnc->port, (unsigned char *)&vCfg->dport );
#endif

#if EXT_FPGA_AUX_ON	
	/* dest multi Aux */
	_extFpgaWrite3Bytes(&fpga->rxAddress->streamAux->ip, &vCfg->auxIp);
	_extFpgaWriteShort(&fpga->rxAddress->streamAnc->port, (unsigned char *)&vCfg->sport );
#endif


#if  1
#define	_USING_OR_OP		1
	/* OR_Op must be used to make RTK switch chipset working. 09.21, 2018 */
	/* RX version fromo 09.04, 2018 */
	/* reset */
#if _USING_OR_OP
	FPGA_I2C_READ(EXT_FPGA_REG_ETHERNET_RESET, &chValue, 1);
	EXT_DEBUGF(MUX_DEBUG_FPGA, "RX Read %x from register 0x%x", chValue, EXT_FPGA_REG_ETHERNET_RESET);
	chValue = (chValue | (1<<1));
#else		
	chValue = 0x02;
#endif
	EXT_DEBUGF(MUX_DEBUG_FPGA, "RX Write %x to register 0x%x: reset FPGA", chValue, EXT_FPGA_REG_ETHERNET_RESET);
	FPGA_I2C_READ(EXT_FPGA_REG_ETHERNET_RESET, &chValue, 1);

		/* release reset */
#if _USING_OR_OP
	FPGA_I2C_READ(EXT_FPGA_REG_ETHERNET_RESET, &chValue, 1);
	EXT_DEBUGF(MUX_DEBUG_FPGA, "RX ReRead %x from register 0x%x", chValue, EXT_FPGA_REG_ETHERNET_RESET);
	chValue = chValue & 0xFD;
#else		
	chValue = 0x00;
#endif
	EXT_DEBUGF(MUX_DEBUG_FPGA,  "TX ReWrite %x to register 0x%x: release reset of FPGA", chValue, EXT_FPGA_REG_ETHERNET_RESET);
	FPGA_I2C_WRITE(EXT_FPGA_REG_ETHERNET_RESET, &chValue, 1);
#endif

#if 0	
		ip4_addr_t *mcIpAddr = (ip4_addr_t *)&vCfg->ip;
		if( ip4_addr_ismulticast(mcIpAddr) )
		{/* join */
			if( (_netIfIpAddr->addr == IPADDR_ANY) )
			{
				EXT_DEBUGF(EXT_DBG_ON,  ("netif is not available, IGMP group can't join now"));
			}
			else
			{
//				printf("Send IGMP JOIN"LWIP_NEW_LINE);
				ret = extLwipGroupMgr(runCfg, runCfg->dest.ip, EXT_TRUE);
			}
		}
#endif

	if(fpga->groupVideoMgr)
	{
		if(fpga->groupVideoMgr->changeGroup(fpga->groupVideoMgr, cmnSysNetAddress(runCfg->dest.ip) ) )
		{
			EXT_ERRORF("Join video multicast group '%s' failed", cmnSysNetAddress(runCfg->dest.ip));
		}
	}
	
	if(fpga->groupAudioMgr )
	{
		if(fpga->groupAudioMgr->changeGroup(fpga->groupAudioMgr, cmnSysNetAddress(runCfg->dest.audioIp) ) )
		{
			EXT_ERRORF("Join audio multicast group '%s' failed", cmnSysNetAddress(runCfg->dest.audioIp));
		}
	}

#if WITH_ANCILLIARY_STREAM
	if(fpga->groupAncMgr)
	{
		if(fpga->groupAncMgr->changeGroup(fpga->groupAncMgr, cmnSysNetAddress(runCfg->dest.ancIp) ) )
		{
			EXT_ERRORF("Join ANC multicast group '%s' failed", cmnSysNetAddress(runCfg->dest.ancIp));
		}
	}
#endif

//		EXT_DELAY_MS(5000);

	/* enable all streams */
	chValue = INVALIDATE_VALUE_U8;
	_extFpgaWriteByte(&fpga->rxAddress->enable, &chValue);


	EXT_DEBUGF(MUX_DEBUG_FPGA,  "FPGA RX Configuration ended!");
	return EXIT_SUCCESS;
}

int sysFpgaTxReadParams(FpgaConfig *fpga)
{
	unsigned char _chValue;
	

	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;
	EXT_INFOF("Read cfg:%p", runCfg);

#if 0
	FPGA_I2C_READ(EXT_FPGA_REG_SDI_STATUS, (unsigned char *)&_chValue, 1);
#if 0
	if(_chValue == 0x0f)
	{
		EXT_ERRORF("Lock %02x from register 0x%x, shifted. Reset I2C", _chValue, EXT_FPGA_REG_SDI_STATUS);
		extI2cReset();
		_extFpgaReadShort(EXT_FPGA_REG_SDI_STATUS, (unsigned char *)&_chValue);
	}
#endif

	EXT_DEBUGF(MUX_DEBUG_FPGA, "Lock %02x from register 0x%x", _chValue, EXT_FPGA_REG_SDI_STATUS);
	if(!_chValue)
#endif

	{
		FIELD_INVALIDATE_U16(runCfg->runtime.vWidth);
		FIELD_INVALIDATE_U16(runCfg->runtime.vHeight);

		FIELD_INVALIDATE_U8(runCfg->runtime.vFrameRate);
		FIELD_INVALIDATE_U8(runCfg->runtime.vColorSpace);
		FIELD_INVALIDATE_U8(runCfg->runtime.vDepth);
		FIELD_INVALIDATE_U8(runCfg->runtime.vIsInterlaced);
		
		FIELD_INVALIDATE_U8(runCfg->runtime.aChannels);
		FIELD_INVALIDATE_U8(runCfg->runtime.aSampleRate);
		FIELD_INVALIDATE_U8(runCfg->runtime.aPktSize);

		FIELD_INVALIDATE_U8(runCfg->runtime.rtpTypeVideo);
		FIELD_INVALIDATE_U8(runCfg->runtime.rtpTypeAudio);
		FIELD_INVALIDATE_U8(runCfg->runtime.rtpTypeAnc);

		FIELD_INVALIDATE_U8(runCfg->runtime.vpid );
		
//		return EXIT_SUCCESS;
	}


	_extFpgaReadByte(&fpga->txAddress->streamVideo->rtpPayload, &_chValue);
	runCfg->runtime.rtpTypeVideo = (_chValue&0x7F);

	_extFpgaReadByte(&fpga->txAddress->streamAudio->rtpPayload, &_chValue);
	runCfg->runtime.rtpTypeAudio = (_chValue&0x7F);

#if WITH_ANCILLIARY_STREAM
	_extFpgaReadByte(&fpga->txAddress->streamAnc->rtpPayload, &_chValue);
	runCfg->runtime.rtpTypeAnc = (_chValue&0x7F);
#endif

	/* write timestamp */
//	FPGA_UPDATE_PTP_TIMESTAMP(fpga);

	_chValue = 0x40; /* 10.30, 2019. TX write to select audio channel from Audio input, not I2S */
	_extFpgaWriteByte(&fpga->txAddress->media->channels, &runCfg->runtime.aChannels);

	sysFpgaVideoForced();
	
	return fpgaReadParamRegisters(fpga->txAddress->media, runCfg);
}

int sysFpgaRxReadParams(FpgaConfig *fpga)
{
	if(! EXT_IS_TX(fpga->runCfg))
	{
		EXT_ERRORF("RX can't read params");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}


int sysFpgaTxWriteParams(FpgaConfig *fpga)
{
	if(EXT_IS_TX(fpga->runCfg))
	{
		EXT_ERRORF("TX can't write params");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int	_rxUpdateAudioParams(EXT_RUNTIME_CFG *runCfg)
{
	/* 11.04, 2019, change as following */
	unsigned char _chVal;

	unsigned char depth = ((_chVal>> 6) & 0x03);
	runCfg->runtime.aDepth = (depth ==0x03)? 24:0;
	unsigned char sampleRate = ( (_chVal>>3) &0x07);
	runCfg->runtime.aSampleRate; /* 2: 44.1K; 3: 48K; 5: 96K */
//	unsigned char 
	runCfg->runtime.aChannels = (_chVal&0x07)+1;

	FPGA_I2C_READ(F_REG_RX_SYS_AUDIO_SELECT, &_chVal, 1);

	return EXIT_SUCCESS;
}


int sysFpgaRxWriteParams(FpgaConfig *fpga)
{
	unsigned char _chValue;
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;

	if(EXT_IS_TX(runCfg))
	{
		return EXIT_FAILURE;
	}

#if WITH_ANCILLIARY_STREAM
	if(runCfg->fpgaAuto == FPGA_CFG_AUTO )
	{
		EXT_DEBUGF(MUX_DEBUG_FPGA, "RX is in auto mode");
		_chValue = EXT_FPGA_FLAGS_MCU_DISABLE;
		FPGA_I2C_WRITE(EXT_FPGA_REG_ENABLE, &_chValue, 1);
	}
	else
#endif
	{
		/* first, enable manual configuration */
		_chValue = EXT_FPGA_FLAGS_MCU_ENABLE;
		FPGA_I2C_WRITE(EXT_FPGA_REG_ENABLE, &_chValue, 1);
		
		/* then params */
		_extFpgaWriteShort(&fpga->rxAddress->media->width, (unsigned char *)&runCfg->runtime.vWidth);
		
		_extFpgaWriteShort(&fpga->rxAddress->media->height, (unsigned char *)&runCfg->runtime.vHeight);
		
		_extFpgaWriteByte(&fpga->rxAddress->media->framerate, &runCfg->runtime.vFrameRate);

#if 1
		_extFpgaWriteByte(&fpga->rxAddress->media->colorSpace, &runCfg->runtime.vColorSpace);
		_extFpgaWriteByte(&fpga->rxAddress->media->vDepth, &runCfg->runtime.vDepth);
#else
		{
			_chValue = (runCfg->runtime.vColorSpace<<2)|runCfg->runtime.vDepth;
			_extFpgaWriteByte(&fpga->rxAddress->media->colorSpace, &_chValue);
			EXT_DEBUGF(MUX_DEBUG_FPGA, "RX write CideoCfg 0x%x", _chValue );
		}
#endif

		_extFpgaWriteByte(&fpga->rxAddress->media->intl, &runCfg->runtime.vIsInterlaced);

#if 0
		_extFpgaWriteByte(&fpga->rxAddress->media->channels, &runCfg->runtime.aChannels);
		_extFpgaWriteByte(&fpga->rxAddress->media->audioRate, &runCfg->runtime.aSampleRate);
		_extFpgaWriteByte(&fpga->rxAddress->media->pktSize, &runCfg->runtime.aPktSize);
#else

#endif

		_extFpgaWriteByte(&fpga->rxAddress->streamVideo->rtpPayload, &runCfg->runtime.rtpTypeVideo);
		_extFpgaWriteByte(&fpga->rxAddress->streamAudio->rtpPayload, &runCfg->runtime.rtpTypeAudio);

#if WITH_ANCILLIARY_STREAM
		_extFpgaWriteByte(&fpga->rxAddress->streamAnc->rtpPayload, &runCfg->runtime.rtpTypeAnc);
#endif
		
		/* third, start it */
		//FPGA_I2C_WRITE(EXT_FPGA_REG_PARAM_STATUS, &_chValue, 1);
		_chValue = INVALIDATE_VALUE_U8;
		_extFpgaWriteByte(&fpga->rxAddress->media->paramStatus, &_chValue);

		EXT_DEBUGF(MUX_DEBUG_FPGA, "RX write:%dx%d, fps:%d; cs:%d; depth:%d; intlc:%d; ", 
			runCfg->runtime.vWidth, runCfg->runtime.vHeight, runCfg->runtime.vFrameRate, runCfg->runtime.vColorSpace, runCfg->runtime.vDepth, runCfg->runtime.vIsInterlaced);

	}

	return EXIT_SUCCESS;
}


int	_txUpdateParams(EXT_RUNTIME_CFG *runCfg)
{
	/* 11.04, 2019, change as following */
#if 1	
	unsigned char _chVal;
	FPGA_I2C_READ(F_REG_TX_SYS_AUDIO_SELECT, &_chVal, 1);

	unsigned char depth = ((_chVal>> 6) & 0x03);
	runCfg->runtime.aDepth = (depth ==0x03)? 24:0;
	unsigned char sampleRate = ( (_chVal>>3) &0x07);
	runCfg->runtime.aSampleRate = sampleRate; //(sampleRate==0x03)?EXT_A_RATE_48K:0; 
//	unsigned char 
	runCfg->runtime.aChannels = (_chVal&0x07)+1;

	cmnSysI2cTxReadAudioParams(&runCfg->runtime.aSampleRate, &runCfg->runtime.aChannels, &runCfg->runtime.aDepth);

#else
	unsigned char _audioSelect = 0, sampleFreq, depth;
	cmnSysI2cTxReadAudioParams(&runCfg->runtime.aSampleRate, &runCfg->runtime.aChannels, &runCfg->runtime.aDepth);
	
	depth = (runCfg->runtime.aDepth==24)?3:(runCfg->runtime.aDepth==20)?2:(runCfg->runtime.aDepth==16)?1:INVALIDATE_VALUE_U8;
	sampleFreq = runCfg->runtime.aSampleRate;
	_audioSelect = (runCfg->runtime.aChannels-1) | (sampleFreq<<3) | (depth<<6);
	EXT_INFOF("Audio Depth: %d; SampleRate:%d; Channel: %d; audioSelect:0x%x", 
		runCfg->runtime.aDepth, runCfg->runtime.aSampleRate, runCfg->runtime.aChannels, _audioSelect);

	FPGA_I2C_WRITE(F_REG_TX_SYS_AUDIO_SELECT, &_audioSelect, 1);
#endif
	return EXIT_SUCCESS;
}



/* both for TX/RX */
/* fpgaXXX is name used in fpga module only */
int fpgaReadParamRegisters(MediaRegisterAddress *addrMedia, EXT_RUNTIME_CFG *runCfg)
{
#if (MUX_BOARD == MUX_BOARD_767 )
	unsigned char _chValue;
	int i;
#endif

	_extFpgaReadShort(&addrMedia->width, (unsigned char *)&runCfg->runtime.vWidth);
	
	_extFpgaReadShort(&addrMedia->height, (unsigned char *)&runCfg->runtime.vHeight);
	
	_extFpgaReadByte(&addrMedia->framerate, &runCfg->runtime.vFrameRate);

	EXT_DEBUGF(MUX_DEBUG_FPGA, "Video:Width:%d; Height:%d; fps:%d", 
		runCfg->runtime.vWidth, runCfg->runtime.vHeight, runCfg->runtime.vFrameRate);

#if 1
	_extFpgaReadByte(&addrMedia->colorSpace, &runCfg->runtime.vColorSpace);
	_extFpgaReadByte(&addrMedia->vDepth, &runCfg->runtime.vDepth);
	EXT_DEBUGF(MUX_DEBUG_FPGA, "depth:%d(0x%x); ColorSpace:%s(0x%x)", 
		CMN_INT_FIND_NAME_V_DEPTH(runCfg->runtime.vDepth), runCfg->runtime.vDepth, 
		CMN_FIND_V_COLORSPACE(runCfg->runtime.vColorSpace), runCfg->runtime.vColorSpace);
#else
	{
//		unsigned char depth, unsigned char colorSpace;
		_extFpgaReadByte(&addrMedia->colorSpace, &_chValue);
		runCfg->runtime.vDepth = (_chValue&0x03);
		runCfg->runtime.vColorSpace = ( (_chValue>>2) &0x07);
	}
	EXT_DEBUGF(MUX_DEBUG_FPGA, "VideoConfig is 0x%x; depth:%d(0x%x); ColorSpace:%s(0x%x)", 
		_chValue, CMN_INT_FIND_NAME_V_DEPTH(runCfg->runtime.vDepth), runCfg->runtime.vDepth, 
		CMN_FIND_V_COLORSPACE(runCfg->runtime.vColorSpace), runCfg->runtime.vColorSpace);
#endif

	_extFpgaReadByte(&addrMedia->intl, &runCfg->runtime.vIsInterlaced);

#if (MUX_BOARD == MUX_BOARD_767 )
	/* audio */	
	_extFpgaReadByte(&addrMedia->channels, &_chValue);
	EXT_DEBUGF(MUX_DEBUG_FPGA, "AudioChannel is 0x%x", _chValue);
	runCfg->runtime.aChannels = 0;
	for(i=0; i< 4; i++)
	{
		if( (_chValue &(1<<i)) )
		{
			runCfg->runtime.aChannels += 4;
		}
	}
	
	_extFpgaReadByte(&addrMedia->audioRate, &runCfg->runtime.aSampleRate);
	
#elif (MUX_BOARD == MUX_BOARD_774)
	if(EXT_IS_TX(runCfg))
	{
		_txUpdateParams(runCfg);
	}
	else
	{
		EXT_ERRORF("RX not support audio read now!!!");
	}

#else
#error 	Not support board definition
#endif
	
#if 1
	/* only for test, 09.26. 2019 */
	_extFpgaReadByte(&addrMedia->pktSize, &runCfg->runtime.aPktSize );
#else
	runCfg->runtime.aPktSize = EXT_A_PKT_SIZE_1MS;
#endif
	EXT_DEBUGF(MUX_DEBUG_FPGA, "Audio: Rate: %d; PktSize:%d", runCfg->runtime.aSampleRate, runCfg->runtime.aPktSize);

#if 0
	FPGA_I2C_READ(EXT_FPGA_REG_ANC_VPID, &runCfg->runtime.vpid, 1);
#endif

	return EXIT_SUCCESS;
}


int sysFpgaTxPollUpdateParams(void *data)
{
#define	NEW_PARAM_AVAILABLE(chValue)		(chValue==0x01)

	FpgaConfig 	*fpga =  (FpgaConfig 	*)data;

	EXT_ASSERT((fpga != NULL), "FPAG failed: %p", data);
		
	unsigned char _chValue;
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;


//	FPGA_UPDATE_PTP_TIMESTAMP(fpga);

	if(! EXT_IS_TX(runCfg))
	{
		return EXIT_FAILURE;
	}

	sysFpgaWritePtpTimestamp(runCfg);


	_extFpgaReadByte(&fpga->txAddress->media->paramStatus, &_chValue);
//	EXT_DEBUGF(MUX_DEBUG_FPGA, "params status:0x%x", _chValue);
	if( NEW_PARAM_AVAILABLE(_chValue) )
	{
		EXT_DEBUGF(MUX_DEBUG_FPGA, "New params is available now");

		/* update params */
		_txUpdateParams(runCfg);

		/* clear register */
		_chValue = INVALIDATE_VALUE_U8;
		_extFpgaWriteByte(&fpga->txAddress->media->paramStatus, &_chValue);

		SYS_UPDATE_SESSION_ID();
	}

	return EXIT_SUCCESS;
}


int sysFpgaWritePtpTimestamp(void 	*data)
{
	struct timeval tv;
	
	FpgaConfig 	*fpga =  (FpgaConfig 	*)data;

	EXT_ASSERT((fpga != NULL), "FPAG failed: %p", data);

	gettimeofday(&tv,NULL);
	uint64_t _ts = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
	uint64_t _readTs;

	FPGA_I2C_READ(F_REG_TX_GRAND_MASTER_TIMESTAMP, &_readTs, 8);
	CMN_SYS_LED_SIGNAL_ON();
	I2C_EXT_WRITE(0, EXT_I2C_PCA9554_CS_NONE, EXT_FPAG_ADDRESS_PTP, F_REG_TX_GRAND_MASTER_TIMESTAMP, 1, &_ts, 8);
	CMN_SYS_LED_SIGNAL_OFF();

	EXT_INFOF("Grand Master Timestamp: \tRead: %llu; Write:%llu", _readTs, _ts );

	return EXIT_SUCCESS;
}

