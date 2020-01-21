
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>	/* PRi64*/

#include <linux/reboot.h>
#include <sys/reboot.h>

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

	/* write video/audio payload type */
	_extFpgaWriteByte(&fpga->txAddress->streamVideo->rtpPayload, &runCfg->runtime.rtpTypeVideo);
	_extFpgaWriteByte(&fpga->txAddress->streamAudio->rtpPayload, &	runCfg->runtime.rtpTypeAudio);

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
//	_extFpgaWrite3Bytes(&fpga->rxAddress->streamVideo->ip, &vCfg->ip);
	_extFpgaWriteInteger(&fpga->rxAddress->streamVideo->ip, &vCfg->ip);
	_extFpgaWriteShort(&fpga->rxAddress->streamVideo->port, (unsigned char *)&vCfg->vport );

	/* dest multi Audio */
//	_extFpgaWrite3Bytes(&fpga->rxAddress->streamAudio->ip, &vCfg->audioIp);
	_extFpgaWriteInteger(&fpga->rxAddress->streamAudio->ip, &vCfg->audioIp);
	_extFpgaWriteShort(&fpga->rxAddress->streamAudio->port, (unsigned char *)&vCfg->aport );

	/* dest multi Anx */
#if WITH_ANCILLIARY_STREAM
//	_extFpgaWrite3Bytes(&fpga->rxAddress->streamAnc->ip, &vCfg->ancIp);
	_extFpgaWriteInteger(&fpga->rxAddress->streamAnc->ip, &vCfg->ancIp);
	_extFpgaWriteShort(&fpga->rxAddress->streamAnc->port, (unsigned char *)&vCfg->dport );
#endif

#if EXT_FPGA_AUX_ON	
	/* dest multi Aux */
//	_extFpgaWrite3Bytes(&fpga->rxAddress->streamAux->ip, &vCfg->auxIp);
	_extFpgaWriteInteger(&fpga->rxAddress->streamAux->ip, &vCfg->auxIp);
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
		EXT_INFOF("Join video multicast group '%s'", cmnSysNetAddress(runCfg->dest.ip));
	}
	
	if(fpga->groupAudioMgr )
	{
		if(fpga->groupAudioMgr !=  fpga->groupVideoMgr )
		{
//			EXT_INFOF("Join audio multicast group '%s'...", cmnSysNetAddress(runCfg->dest.audioIp));
			if(fpga->groupAudioMgr->changeGroup(fpga->groupAudioMgr, cmnSysNetAddress(runCfg->dest.audioIp) ) )
			{
				EXT_ERRORF("Join audio multicast group '%s' failed", cmnSysNetAddress(runCfg->dest.audioIp));
			}
			EXT_INFOF("Join audio multicast group '%s'", cmnSysNetAddress(runCfg->dest.audioIp));
		}
#if 0
		else
		{
			EXT_ERRORF("audio/video multicast group mgr is same");
		}
#endif		
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
//	unsigned char _chValue;
	
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

#if 0	/* use these params to configure FPGA */
		FIELD_INVALIDATE_U8(runCfg->runtime.rtpTypeVideo);
		FIELD_INVALIDATE_U8(runCfg->runtime.rtpTypeAudio);
		FIELD_INVALIDATE_U8(runCfg->runtime.rtpTypeAnc);
#endif

		FIELD_INVALIDATE_U8(runCfg->runtime.vpid );
		
//		return EXIT_SUCCESS;
	}


#if 0
	/* not read payload type, only write to fpga. 12.04, 2019 */
	_extFpgaReadByte(&fpga->txAddress->streamVideo->rtpPayload, &_chValue);
	runCfg->runtime.rtpTypeVideo = (_chValue&0x7F);

	_extFpgaReadByte(&fpga->txAddress->streamAudio->rtpPayload, &_chValue);
	runCfg->runtime.rtpTypeAudio = (_chValue&0x7F);
	
#if WITH_ANCILLIARY_STREAM
	_extFpgaReadByte(&fpga->txAddress->streamAnc->rtpPayload, &_chValue);
	runCfg->runtime.rtpTypeAnc = (_chValue&0x7F);
#endif

#endif

	/* write timestamp */
//	FPGA_UPDATE_PTP_TIMESTAMP(fpga);

//	_chValue = 0x40; /* 10.30, 2019. TX write to select audio channel from Audio input, not I2S */
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

/* refer to _txUpdateParams() */
static int	_rxUpdateAudioParams(EXT_RUNTIME_CFG *runCfg, MediaRegisterAddress *mediaAddrs)
{
	/* 11.04, 2019, change as following */
//	unsigned char _chVal;
	unsigned char _aDpeth, _aSampleRate, _aCh, _aPktSize;
	
	/* aDepth store 24/16bit. eg. SDP data format after SDP parsing */
	if(runCfg->runtime.aDepth != 24)
	{
		MUX_ERROR("Audio: depth: %d, not 24bit depth, no support audio depth", runCfg->runtime.aDepth );
		_aDpeth = 3;/*24*/
	}
	else
	{
		_aDpeth = 3;/*24*/
	}
	

	/* aSample store 2/3/5, eg. FPGA data format after SDP parsing, */
	if(runCfg->runtime.aSampleRate != EXT_A_RATE_44K && 
		runCfg->runtime.aSampleRate != EXT_A_RATE_48K && 
		runCfg->runtime.aSampleRate != EXT_A_RATE_96K  )
	{
		MUX_ERROR("Audio:sample rate: %d, no support sample rate", runCfg->runtime.aSampleRate );
		_aSampleRate = EXT_A_RATE_48K;
	}
	else
	{
		_aSampleRate = runCfg->runtime.aSampleRate; /* 2: 44.1K; 3: 48K; 5: 96K */
	}

	/* aChannel save result from SDP */
	if(runCfg->runtime.aChannels != 8  )
	{
//		MUX_ERROR("Audio: channel: %d, no support channel number", runCfg->runtime.aChannels);
		_aCh = runCfg->runtime.aChannels -1;
	}
	else
	{
		_aCh = runCfg->runtime.aChannels -1;
	}

	if(runCfg->runtime.aPktSize != EXT_A_PKT_SIZE_1MS && 
		runCfg->runtime.aPktSize != EXT_A_PKT_SIZE_125US   )
	{
		MUX_ERROR("Audio:PacketSize: %d, no support Packet Size", runCfg->runtime.aPktSize );
		_aPktSize = EXT_A_PKT_SIZE_1MS;
	}
	else
	{
		_aPktSize = runCfg->runtime.aPktSize;
	}

#if 1
	_extFpgaWriteByte(&mediaAddrs->channels, &_aCh );
	_extFpgaWriteByte(&mediaAddrs->audioRate, &_aSampleRate);
	_extFpgaWriteByte(&mediaAddrs->pktSize, &_aPktSize);
#else
	_chVal = (_aDpeth<<6)|(_aSampleRate<<3)|_aCh;

	FPGA_I2C_WRITE(F_REG_RX_SYS_AUDIO_SELECT, &_chVal, 1);
#endif


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


		_extFpgaWriteByte(&fpga->rxAddress->streamVideo->rtpPayload, &runCfg->runtime.rtpTypeVideo);
		_extFpgaWriteByte(&fpga->rxAddress->streamAudio->rtpPayload, &runCfg->runtime.rtpTypeAudio);

#if WITH_ANCILLIARY_STREAM
		_extFpgaWriteByte(&fpga->rxAddress->streamAnc->rtpPayload, &runCfg->runtime.rtpTypeAnc);
#endif
		
		/* third, start it */
		EXT_DEBUGF(MUX_DEBUG_FPGA, "RX write:%dx%d, fps:%d; cs:%d; depth:%d; intlc:%d; ", 
			runCfg->runtime.vWidth, runCfg->runtime.vHeight, runCfg->runtime.vFrameRate, runCfg->runtime.vColorSpace, runCfg->runtime.vDepth, runCfg->runtime.vIsInterlaced);

		_rxUpdateAudioParams(runCfg, fpga->rxAddress->media );

		//FPGA_I2C_WRITE(EXT_FPGA_REG_PARAM_STATUS, &_chValue, 1);
		_chValue = INVALIDATE_VALUE_U8;
		_extFpgaWriteByte(&fpga->rxAddress->media->paramStatus, &_chValue);

	}

	return EXIT_SUCCESS;
}


static int	_txUpdateParams(EXT_RUNTIME_CFG *runCfg)
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
	if(runCfg->isConvert )
	{
		runCfg->runtime.vColorSpace = EXT_V_COLORSPACE_YCBCR_422;
		runCfg->runtime.vDepth = EXT_V_DEPTH_10;
	}
	else
	{
		_extFpgaReadByte(&addrMedia->colorSpace, &runCfg->runtime.vColorSpace);
		_extFpgaReadByte(&addrMedia->vDepth, &runCfg->runtime.vDepth);
	}
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
//		EXT_ERRORF("RX not support audio read now!!!");
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


#if EXT_DEBUG_TIMESTAMP
extern	FILE *offsetFp;
#endif

int sysFpgaWritePtpTimestamp(EXT_RUNTIME_CFG *runCfg)
{
	struct timeval tv;
	unsigned char		isPtpLocked = 0;
		
	MuxPtpRuntime *ptpRuntime = &runCfg->ptpRuntime;	

	if(muxPtpPoll(ptpRuntime, runCfg->ptpConfig.domain) == EXIT_SUCCESS)
	{
		if(ptpRuntime->portState == 9) /* SLAVE state*/
		{
			isPtpLocked = 1;
		}
	}
	
//	FpgaConfig 	*fpga =  (FpgaConfig 	*)data;

//	EXT_ASSERT((fpga != NULL), "FPAG failed: %p", data);

	gettimeofday(&tv,NULL);
	uint64_t _ts = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
	uint64_t _readTs;

	FPGA_I2C_READ(F_REG_TX_GRAND_MASTER_TIMESTAMP, &_readTs, 8);
	CMN_SYS_LED_SIGNAL_ON();
	I2C_EXT_WRITE(0, EXT_I2C_PCA9554_CS_NONE, EXT_FPAG_ADDRESS_PTP, F_REG_TX_GRAND_MASTER_TIMESTAMP, 1, &_ts, 8);

	I2C_EXT_WRITE(0, EXT_I2C_PCA9554_CS_NONE, EXT_FPAG_ADDRESS_PTP, F_REG_TX_PTP_LOCKED, 1, &isPtpLocked, 1);
	CMN_SYS_LED_SIGNAL_OFF();

#if EXT_DEBUG_TIMESTAMP
	if(isPtpLocked )
	{
		char timestampStr[32];
		struct tm *ptm;

		ptm = localtime(&tv.tv_sec);
//		strftime(timestampStr, sizeof(timestampStr), "%Y-%m-%d %H:%M:%S", ptm);
		strftime(timestampStr, sizeof(timestampStr), "%H:%M:%S", ptm);
		
		EXT_INFOF("%s: %" PRId64, timestampStr, ptpRuntime->offset );
		if(offsetFp)
		{
			fprintf(offsetFp, "%s: %" PRId64"\n", timestampStr, ptpRuntime->offset);
			fflush(offsetFp);
		}
		else
		{
			EXT_ERRORF("offset file is not opened");
		}
	}
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

	if(EXT_IS_PTP_ENABLED(runCfg) )
	{
		sysFpgaWritePtpTimestamp(runCfg);
	}

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

/* align seq_req bit of RESET register */
static void _sysFpgaVideoEnable(FpgaI2cAddress *enableReg, unsigned char isEnble)
{	
	unsigned char _chValue;
//	byte data_write[1];
	
	_extFpgaReadByte(enableReg, &_chValue);
//	I2C_Read_feedback(0x60, 0x0, data_write, 1);	
	if(isEnble)
	{
		_chValue &= ~0x20;
	}
	else
	{
		_chValue |= 0x20;	/* write 1 to begin pulse align */
	}
//	I2C_Write_feedback(0x60, 0x0, data_write, 1);
	_extFpgaWriteByte(enableReg, &_chValue);
}


#define	DISPLAY_RESYNC(reg)	\
	do{_sysFpgaVideoEnable(reg, EXT_FALSE); \
		_sysFpgaVideoEnable(reg, EXT_TRUE); }while(0)


static void _sysFpgaDisplayCtrl(FpgaI2cAddress *reg, unsigned char isBlank)
{
	unsigned char _chValue;

	_extFpgaReadByte(reg, &_chValue);
	if(isBlank )
	{
		_chValue |= 0x80;
	}
	else
	{
		_chValue &= ~0x80;
	}

	_extFpgaWriteByte(reg, &_chValue);
}


struct VcxoStatus
{
	int		dac;
	int		newDac;
	int		dacI;
	int		initFlag;

	unsigned int		xTemp;
	unsigned int		yTemp;
	unsigned int		width;
	unsigned int		height;
	unsigned int		pixelDelta;

	int		value;
};

struct VcxoStatus		_vcxoStatus=
{
	0,
	0,
	0,
	0,
	0
};

#define __min(a,b)		(((a)<(b))?(a):(b))
#define __max(a,b)		(((a)>(b))?(a):(b))

#define	CLIPPING(lo, a, hi)		__max((lo), __min((a), (hi)))

#define		DAC_MAX				2047
#define		DAC_MIN				-(DAC_MAX)
#define		DAC_MAX_STEP			(DAC_MAX/16)

#define		DAC_I_SH				4
#define		PIXEL_DELTA_SH			3



int  cmnSysDacWriteVoltage(int  dac)
{	
	unsigned char   data[2];	
	unsigned int  udac;
	unsigned char offAddress = 0;
	int ret;

	dac = CLIPPING(DAC_MIN, dac, DAC_MAX);
	udac = 0x7ff + dac;
	/* p.50, figure 6-1
	* command 2-bit: b7-b6 = 00
	* power down 2-bit: b5-b4 = 00
	* data bit: 4-bit: only 2 bit for 4716,
	*/
#if 0	
	unsigned char   cmd;
	unsigned char   pd;
	cmd = 0;
	pd = 0;
	data[0] = ((cmd & 0X03) << 6) | ((pd & 0X03) << 4) | ((udac >> 8) & 0x0f);
#endif
	data[0] = ((udac >> 8) & 0x0f);
	data[1] = udac;
	//printf("data[0] = 0x%d, data[1] = 0x%d\n", data[0], data[1]);	

	ret = MCP4716_WRITE(offAddress, data, 2);
	if(ret == EXIT_FAILURE)
	{
		EXT_ERRORF("MCP4716 write voltage failed");
	}

	return ret;	
}

/* return EXT_TRUE: param changed */
static int	_checkRxParams(FpgaConfig 	*fpga)
{
	struct VcxoStatus *vcxo = &_vcxoStatus;

	unsigned int pixDeltaCount;
	unsigned short width, height;

	_extFpgaReadShort(&fpga->rxAddress->xActive, &width); /* X */
	_extFpgaReadShort(&fpga->rxAddress->yActive, &height); /* Y */


	_extFpgaReadInteger(&fpga->rxAddress->pixelCount, &pixDeltaCount);

	if( /* xTemp!= vcxo->xTemp || yTemp!= vcxo->yTemp || */ width!= vcxo->width || height != vcxo->height || pixDeltaCount != vcxo->pixelDelta )
	{
#if 0
		EXT_DEBUGF(EXT_DBG_ON, "RX params changed: xTime:%d(%d); yTime:%d(%d); xActive: %d(%d); yActive:%d(%d); pixelDelta:%d(%d)", 
			xTemp, vcxo->xTemp, yTemp, vcxo->yTemp, width, vcxo->width, height, vcxo->height, pixDeltaCount, vcxo->pixelDelta );
		
		vcxo->xTemp = xTemp;
		vcxo->yTemp = yTemp;
#else
		EXT_DEBUGF(EXT_DBG_ON, "RX params changed: xActive: %d(%d); yActive:%d(%d); pixelDelta:%d(%d)", 
			width, vcxo->width, height, vcxo->height, pixDeltaCount, vcxo->pixelDelta );

#endif
		vcxo->width = width;
		vcxo->height = height;
		vcxo->pixelDelta = pixDeltaCount;

		return EXT_TRUE;
	}

	return EXT_FALSE;
}


/* read params for tuning frequency of clock generator, for RX only */
int sysFpgaReadClockParams(void *data)
{
	FpgaConfig 	*fpga =  (FpgaConfig 	*)data;
	struct VcxoStatus *vcxo = &_vcxoStatus;

//	unsigned int	xFront, xPulse, xBack;
//	unsigned int	yFront, yPulse, yBack;
	unsigned int xTotal, yTotal, total;
	int pixelDeltaCount;

	EXT_ASSERT((fpga != NULL), "FPAG failed: %p", data);
	EXT_ASSERT((fpga->runCfg!= NULL), "FPAG Initialization failed: %p", data);
		
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;

	if( EXT_IS_TX(runCfg))
	{
		return EXIT_SUCCESS;
	}

	if(_checkRxParams( fpga) == EXT_FALSE)
	{
		return EXIT_SUCCESS;
	}
	pixelDeltaCount = vcxo->pixelDelta;

#if 0
	xFront = (vcxo->xTemp >> 0)  & 0x7FF;
	xPulse = (vcxo->xTemp >> 11) & 0x3FF;
	xBack  = (vcxo->xTemp >> 21) & 0x3FF;

	yFront = (vcxo->yTemp >> 0)  & 0x7FF;
	yPulse = (vcxo->yTemp >> 11) & 0x3FF;
	yBack  = (vcxo->yTemp >> 21) & 0x3FF;
#endif

	xTotal = vcxo->width;// + (xFront + xPulse + xBack);
	yTotal = vcxo->height;// + (yFront + yPulse + yBack);		
	
	total = xTotal*yTotal;

#if 0
	EXT_DEBUGF(EXT_DBG_ON, "X: %d=%d+%d+%d+%d; Y: %d=%d+%d+%d+%d; Total: %d, deltaCount:%d", 
		xTotal, vcxo->width, xFront, xPulse, xBack	, 
		yTotal, vcxo->height, yFront, yPulse, yBack, 
		total, pixelDeltaCount);
#else
	EXT_DEBUGF(EXT_DBG_ON, "X: %d; Y: %d; Total: %d, deltaCount:%d", 
		xTotal, yTotal, total, pixelDeltaCount);
#endif	
	if (pixelDeltaCount < total ) 
	{
		// wrap around when pix_delta is just pas the zero
		if (pixelDeltaCount > (total/2) ) 
		{/* negative value */
			pixelDeltaCount = pixelDeltaCount - total;
		}

		vcxo->newDac = CLIPPING(DAC_MIN, (pixelDeltaCount>>PIXEL_DELTA_SH),  DAC_MAX);
		vcxo->dac = CLIPPING(vcxo->dac-DAC_MAX_STEP, vcxo->newDac, vcxo->dac+DAC_MAX_STEP); // limit DAC steps    

		//signal integration when proportionnal DAC value is in range
		vcxo->dacI = CLIPPING(DAC_MIN<<DAC_I_SH, (vcxo->dacI + vcxo->newDac), DAC_MAX<<DAC_I_SH);
		vcxo->value = vcxo->dac + (vcxo->dacI >> DAC_I_SH);
		cmnSysDacWriteVoltage(vcxo->value);

		// reset HDMI out engine and resync to start of stream
		if ((pixelDeltaCount > (4*xTotal)) || (-pixelDeltaCount > (4*xTotal)) || abs(vcxo->value)>=DAC_MAX ) 
		{
			if(vcxo->initFlag == 0)
			{
				DISPLAY_RESYNC(&fpga->rxAddress->reset );

				//reset dac
				vcxo->dac = 0;
				vcxo->dacI = 0;
				cmnSysDacWriteVoltage(0);
				
				usleep(30000);   // wait for 1 frame before enabling display
				_sysFpgaDisplayCtrl(&fpga->rxAddress->sfpCtrl, EXT_FALSE);

				EXT_INFOF("fine tune enable video");
				vcxo->initFlag = 30; // 30x 100 msec = 3 second
			}
			else 
			{
				if(vcxo->initFlag != 0)
				{
					vcxo->initFlag--;
				}
			}
		}
		else 
		{
			vcxo->initFlag = 30; // adjust for 3 second delay to reset, 30x 100 msec
		}
	}					


	return EXIT_SUCCESS;
}

int sysFpgaPollThreadInit(void *data)
{
	FpgaConfig 	*fpga =  (FpgaConfig 	*)data;
	struct VcxoStatus *vcxo = &_vcxoStatus;
	
	EXT_ASSERT((fpga != NULL), "FPAG failed: %p", data);

	EXT_ASSERT((fpga->runCfg!= NULL), "FPAG Initialization failed: %p", data);

	if(EXT_IS_TX(fpga->runCfg) )
	{
		memset(vcxo, 0, sizeof(struct VcxoStatus));
	}

	return EXIT_SUCCESS;
}

