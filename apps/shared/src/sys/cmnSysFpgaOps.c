
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>
#include <stdint.h>

#include "libCmnSys.h"
#include "mux7xx.h"

#include "muxFpga.h"


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
	_extFpgaWriteShort(&fpga->txAddress->localPortAnc, (unsigned char *)&runCfg->local.dport);
#if EXT_FPGA_AUX_ON	
	_extFpgaWriteShort(&fpga->txAddress->localPortAuc, (unsigned char *)&runCfg->local.sport);
#endif

	EXT_DEBUGF(MUX_DEBUG_FPGA, "TX:IP:%s; portV:%d; portA:%d; portAnc:%d", cmnSysNetAddress(ip), runCfg->local.vport, runCfg->local.aport,runCfg->local.dport);

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
	_extFpgaWrite3Bytes(&fpga->rxAddress->streamAnc->ip, &vCfg->ancIp);
	_extFpgaWriteShort(&fpga->rxAddress->streamAnc->port, (unsigned char *)&vCfg->dport );

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

	if(fpga->groupAncMgr)
	{
		if(fpga->groupAncMgr->changeGroup(fpga->groupAncMgr, cmnSysNetAddress(runCfg->dest.ancIp) ) )
		{
			EXT_ERRORF("Join ANC multicast group '%s' failed", cmnSysNetAddress(runCfg->dest.ancIp));
		}
	}

//		EXT_DELAY_MS(5000);

	EXT_DEBUGF(MUX_DEBUG_FPGA,  "FPGA RX Configuration ended!");
	return EXIT_SUCCESS;
}

int sysFpgaTxReadParams(FpgaConfig *fpga)
{
	unsigned char _chValue;
	
	 EXT_RUNTIME_CFG *runCfg = fpga->runCfg;

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
		
		return EXIT_SUCCESS;
	}


	_extFpgaReadByte(&fpga->txAddress->streamVideo->rtpPayload, &_chValue);
	runCfg->runtime.rtpTypeVideo = (_chValue&0x7F);

	_extFpgaReadByte(&fpga->txAddress->streamAudio->rtpPayload, &_chValue);
	runCfg->runtime.rtpTypeAudio = (_chValue&0x7F);

	_extFpgaReadByte(&fpga->txAddress->streamAnc->rtpPayload, &_chValue);
	runCfg->runtime.rtpTypeAnc = (_chValue&0x7F);
	
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

int sysFpgaRxWriteParams(FpgaConfig *fpga)
{
	unsigned char _chValue;
	EXT_RUNTIME_CFG *runCfg = fpga->runCfg;

	if(EXT_IS_TX(runCfg))
	{
		return EXIT_FAILURE;
	}

	if(runCfg->fpgaAuto == FPGA_CFG_AUTO )
	{
		EXT_DEBUGF(MUX_DEBUG_FPGA, "RX is in auto mode");
		_chValue = EXT_FPGA_FLAGS_MCU_DISABLE;
		FPGA_I2C_WRITE(EXT_FPGA_REG_ENABLE, &_chValue, 1);
	}
	else
	{
		/* first, enable manual configuration */
		_chValue = EXT_FPGA_FLAGS_MCU_ENABLE;
		FPGA_I2C_WRITE(EXT_FPGA_REG_ENABLE, &_chValue, 1);
		
		/* then params */
		_extFpgaWriteShort(&fpga->rxAddress->media->width, (unsigned char *)&runCfg->runtime.vWidth);
		
		_extFpgaWriteShort(&fpga->rxAddress->media->height, (unsigned char *)&runCfg->runtime.vHeight);
		
		_extFpgaWriteByte(&fpga->rxAddress->media->framerate, &runCfg->runtime.vFrameRate);

#if 0
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


		_extFpgaWriteByte(&fpga->rxAddress->media->channels, &runCfg->runtime.aChannels);
		_extFpgaWriteByte(&fpga->rxAddress->media->audioRate, &runCfg->runtime.aSampleRate);
		_extFpgaWriteByte(&fpga->rxAddress->media->pktSize, &runCfg->runtime.aPktSize);


		_extFpgaWriteByte(&fpga->rxAddress->streamVideo->rtpPayload, &runCfg->runtime.rtpTypeVideo);
		_extFpgaWriteByte(&fpga->rxAddress->streamAudio->rtpPayload, &runCfg->runtime.rtpTypeAudio);
		_extFpgaWriteByte(&fpga->rxAddress->streamAnc->rtpPayload, &runCfg->runtime.rtpTypeAnc);

		
		/* third, start it */
		FPGA_I2C_WRITE(EXT_FPGA_REG_PARAM_STATUS, &_chValue, 1);

	}

	return EXIT_SUCCESS;
}


/* both for TX/RX */
/* fpgaXXX is name used in fpga module only */
int fpgaReadParamRegisters(MediaRegisterAddress *addrMedia, EXT_RUNTIME_CFG *runCfg)
{
	unsigned char _chValue;
	int i;
	
	_extFpgaReadShort(&addrMedia->width, (unsigned char *)&runCfg->runtime.vWidth);
	
	_extFpgaReadShort(&addrMedia->height, (unsigned char *)&runCfg->runtime.vHeight);
	
	_extFpgaReadByte(&addrMedia->framerate, &runCfg->runtime.vFrameRate);

#if 0
	_extFpgaReadByte(&addrMedia->colorSpace, &runCfg->runtime.vColorSpace);

	_extFpgaReadByte(&addrMedia->vDepth, &runCfg->runtime.vDepth);
#else
	{
//		unsigned char depth, unsigned char colorSpace;
		_extFpgaReadByte(&addrMedia->colorSpace, &_chValue);
		EXT_DEBUGF(MUX_DEBUG_FPGA, "VideoConfig is 0x%x", _chValue);
		runCfg->runtime.vDepth = (_chValue&0x03);
		runCfg->runtime.vColorSpace = ( (_chValue>>2) &0x07);
	}

#endif
	_extFpgaReadByte(&addrMedia->intl, &runCfg->runtime.vIsInterlaced);

	/* audio */	
	_extFpgaReadByte(&addrMedia->channels, &_chValue);
	runCfg->runtime.aChannels = 0;
	for(i=0; i< 4; i++)
	{
		if( (_chValue &(1<<i)) )
		{
			runCfg->runtime.aChannels += 4;
		}
	}
	
	_extFpgaReadByte(&addrMedia->audioRate, &runCfg->runtime.aSampleRate);
	
	_extFpgaReadByte(&addrMedia->pktSize, &runCfg->runtime.aPktSize);

	FPGA_I2C_READ(EXT_FPGA_REG_ANC_VPID, &runCfg->runtime.vpid, 1);

	return EXIT_SUCCESS;
}

