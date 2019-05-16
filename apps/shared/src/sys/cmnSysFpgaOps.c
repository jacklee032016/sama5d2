
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>
#include <stdint.h>

#include "libCmn.h"
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
	_fpgaRegisterWrite( fpga->txAddress->localMac, address, EXT_MAC_ADDRESS_LENGTH);

	_extFpgaWriteInteger(fpga->txAddress->localIp, &ip);

	_extFpgaWriteShort(fpga->txAddress->localPortV, (unsigned char *)&runCfg->local.vport);
	
	_extFpgaWriteShort(fpga->txAddress->localPortA, (unsigned char *)&runCfg->local.aport);
	_extFpgaWriteShort(fpga->txAddress->localPortAnc, (unsigned char *)&runCfg->local.dport);
#if EXT_FPGA_AUX_ON	
	_extFpgaWriteShort(fpga->txAddress->localPortAuc, (unsigned char *)&runCfg->local.sport);
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
	_fpgaRegisterWrite(fpga->txAddress->addrStreamVideo->addrMac, address, EXT_MAC_ADDRESS_LENGTH);

	_extFpgaWriteInteger(fpga->txAddress->addrStreamVideo->addrIp, &runCfg->dest.ip);
	_extFpgaWriteShort(fpga->txAddress->addrStreamVideo->addrPort, (unsigned char *)&runCfg->dest.vport);
	
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
	_fpgaRegisterWrite(fpga->txAddress->addrStreamAudio->addrMac, address, EXT_MAC_ADDRESS_LENGTH);

	_extFpgaWriteInteger(fpga->txAddress->addrStreamAudio->addrIp, &runCfg->dest.audioIp);
	_extFpgaWriteShort(fpga->txAddress->addrStreamAudio->addrPort, (unsigned char *)&runCfg->dest.aport);

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
	_fpgaRegisterWrite(fpga->txAddress->addrStreamAnc->addrMac, address, EXT_MAC_ADDRESS_LENGTH);

	_extFpgaWriteInteger(fpga->txAddress->addrStreamAnc->addrIp, &runCfg->dest.ancIp);
	_extFpgaWriteShort(fpga->txAddress->addrStreamAnc->addrPort, (unsigned char *)&runCfg->dest.dport);

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
	_fpgaRegisterWrite(fpga->txAddress->addrStreamAux->addrMac, address, EXT_MAC_ADDRESS_LENGTH);

	_extFpgaWriteInteger(fpga->txAddress->addrStreamAux->addrIp, &runCfg->dest.auxIp);
	_extFpgaWriteShort(fpga->txAddress->addrStreamAux->addrPort, (unsigned char *)&runCfg->dest.sport);
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
	EXT_INFOF("RX is configuring");

	/* local IP/MAC */
	_extFpgaWriteInteger(fpga->rxAddress->localIp, &ip);
//		cmnSysNetChangeByteOrderOfMac(&vCfg->mac, address);
	cmnSysNetChangeByteOrderOfMac(&runCfg->local.mac, address);
	_fpgaRegisterWrite(fpga->rxAddress->localMac, address, EXT_MAC_ADDRESS_LENGTH);

	/* dest multi Video */
	_extFpgaWrite3Bytes(fpga->rxAddress->addrStreamVideo->addrIp, &vCfg->ip);
	_extFpgaWriteShort(fpga->rxAddress->addrStreamVideo->addrPort, (unsigned char *)&vCfg->vport );

	/* dest multi Audio */
	_extFpgaWrite3Bytes(fpga->rxAddress->addrStreamAudio->addrIp, &vCfg->audioIp);
	_extFpgaWriteShort(fpga->rxAddress->addrStreamAudio->addrPort, (unsigned char *)&vCfg->aport );

	/* dest multi Anx */
	_extFpgaWrite3Bytes(fpga->rxAddress->addrStreamAnc->addrIp, &vCfg->ancIp);
	_extFpgaWriteShort(fpga->rxAddress->addrStreamAnc->addrPort, (unsigned char *)&vCfg->dport );

#if EXT_FPGA_AUX_ON	
	/* dest multi Aux */
	_extFpgaWrite3Bytes(fpga->rxAddress->addrStreamAux->addrIp, &vCfg->auxIp);
	_extFpgaWriteShort(fpga->rxAddress->addrStreamAnc->addrPort, (unsigned char *)&vCfg->sport );
#endif


#if  1
#define	_USING_OR_OP		1
	/* OR_Op must be used to make RTK switch chipset working. 09.21, 2018 */
	/* RX version fromo 09.04, 2018 */
	/* reset */
#if _USING_OR_OP
	_extFpgaReadByte(EXT_FPGA_REG_ETHERNET_RESET, &chValue);
	EXT_DEBUGF(EXT_DBG_ON, ("RX Read %x from register 0x%x", chValue, EXT_FPGA_REG_ETHERNET_RESET));
	chValue = (chValue | (1<<1));
#else		
	chValue = 0x02;
#endif
	EXT_DEBUGF(EXT_DBG_ON, ("RX Write %x to register 0x%x: reset FPGA", chValue, EXT_FPGA_REG_ETHERNET_RESET));
	_extFpgaWriteByte(EXT_FPGA_REG_ETHERNET_RESET, &chValue);

		/* release reset */
#if _USING_OR_OP
	_extFpgaReadByte(EXT_FPGA_REG_ETHERNET_RESET, &chValue);
	EXT_DEBUGF(EXT_DBG_ON, ("RX ReRead %x from register 0x%x", chValue, EXT_FPGA_REG_ETHERNET_RESET));
	chValue = chValue & 0xFD;
#else		
	chValue = 0x00;
#endif
	EXT_DEBUGF(EXT_DBG_ON,  ("TX ReWrite %x to register 0x%x: release reset of FPGA", chValue, EXT_FPGA_REG_ETHERNET_RESET));
	_extFpgaWriteByte(EXT_FPGA_REG_ETHERNET_RESET, &chValue);
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

	EXT_DEBUGF(EXT_DBG_ON,  ("FPGA RX Configuration ended!"));
	return EXIT_SUCCESS;
}


/* both for TX/RX */
/* fpgaXXX is name used in fpga module only */
int fpgaReadParamRegisters(struct Media_Register_Address *addrMedia, EXT_RUNTIME_CFG *runCfg)
{
	unsigned char _chValue;
	int i;
	
	_extFpgaReadShort(addrMedia->addrWidth, (unsigned char *)&runCfg->runtime.vWidth);
	
	_extFpgaReadShort(addrMedia->addrHeight, (unsigned char *)&runCfg->runtime.vHeight);
	
	_extFpgaReadByte(addrMedia->addrFramerate, &runCfg->runtime.vFrameRate);

	_extFpgaReadByte(addrMedia->addrColorSpace, &runCfg->runtime.vColorSpace);

	_extFpgaReadByte(addrMedia->addrVDepth, &runCfg->runtime.vDepth);

	_extFpgaReadByte(addrMedia->addrIntl, &runCfg->runtime.vIsInterlaced);

	/* audio */	
	_extFpgaReadByte(addrMedia->addrChannels, &_chValue);
	runCfg->runtime.aChannels = 0;
	for(i=0; i< 4; i++)
	{
		if( (_chValue &(1<<i)) )
		{
			runCfg->runtime.aChannels += 4;
		}
	}
	
	_extFpgaReadByte(addrMedia->addrAudioRate, &runCfg->runtime.aSampleRate);
	
	_extFpgaReadByte(addrMedia->addrPktSize, &runCfg->runtime.aPktSize);

	_extFpgaReadByte(EXT_FPGA_REG_ANC_VPID, &runCfg->runtime.vpid);

	return EXIT_SUCCESS;
}

int sysFpgaTxReadParams(FpgaConfig *fpga)
{
	unsigned char _chValue;
	
	 EXT_RUNTIME_CFG *runCfg = fpga->runCfg;

	_extFpgaReadShort(EXT_FPGA_REG_SDI_STATUS, (unsigned char *)&_chValue);
#if 0
	if(_chValue == 0x0f)
	{
		EXT_ERRORF(("Lock %02x from register 0x%x, shifted. Reset I2C", _chValue, EXT_FPGA_REG_SDI_STATUS));
		extI2cReset();
		_extFpgaReadShort(EXT_FPGA_REG_SDI_STATUS, (unsigned char *)&_chValue);
	}
#endif

	EXT_DEBUGF(EXT_DBG_ON, ("Lock %02x from register 0x%x", _chValue, EXT_FPGA_REG_SDI_STATUS));
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


	_extFpgaReadByte(fpga->txAddress->addrStreamVideo->addrRtpPayload, &_chValue);
	runCfg->runtime.rtpTypeVideo = (_chValue&0x7F);

	_extFpgaReadByte(fpga->txAddress->addrStreamAudio->addrRtpPayload, &_chValue);
	runCfg->runtime.rtpTypeAudio = (_chValue&0x7F);

	_extFpgaReadByte(fpga->txAddress->addrStreamAnc->addrRtpPayload, &_chValue);
	runCfg->runtime.rtpTypeAnc = (_chValue&0x7F);
	
	return fpgaReadParamRegisters(fpga->txAddress->addrMedia, runCfg);
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
		_chValue = EXT_FPGA_FLAGS_MCU_DISABLE;
		_extFpgaWriteByte(EXT_FPGA_REG_ENABLE, &_chValue);
	}
	else
	{
		/* first, enable manual configuration */
		_chValue = EXT_FPGA_FLAGS_MCU_ENABLE;
		_extFpgaWriteByte(EXT_FPGA_REG_ENABLE, &_chValue);
		
		/* then params */
		_extFpgaWriteShort(fpga->rxAddress->addrMedia->addrWidth, (unsigned char *)&runCfg->runtime.vWidth);
		
		_extFpgaWriteShort(fpga->rxAddress->addrMedia->addrHeight, (unsigned char *)&runCfg->runtime.vHeight);
		
		_extFpgaWriteByte(fpga->rxAddress->addrMedia->addrFramerate, &runCfg->runtime.vFrameRate);

		_extFpgaWriteByte(fpga->rxAddress->addrMedia->addrColorSpace, &runCfg->runtime.vColorSpace);

		_extFpgaWriteByte(fpga->rxAddress->addrMedia->addrVDepth, &runCfg->runtime.vDepth);

		_extFpgaWriteByte(fpga->rxAddress->addrMedia->addrIntl, &runCfg->runtime.vIsInterlaced);


		_extFpgaWriteByte(fpga->rxAddress->addrMedia->addrChannels, &runCfg->runtime.aChannels);
		_extFpgaWriteByte(fpga->rxAddress->addrMedia->addrAudioRate, &runCfg->runtime.aSampleRate);
		
		_extFpgaWriteByte(fpga->rxAddress->addrMedia->addrPktSize, &runCfg->runtime.aPktSize);


		_extFpgaWriteByte(fpga->rxAddress->addrStreamVideo->addrRtpPayload, &runCfg->runtime.rtpTypeVideo);
		_extFpgaWriteByte(fpga->rxAddress->addrStreamAudio->addrRtpPayload, &runCfg->runtime.rtpTypeAudio);
		_extFpgaWriteByte(fpga->rxAddress->addrStreamAnc->addrRtpPayload, &runCfg->runtime.rtpTypeAnc);

		
		/* third, start it */
		_extFpgaWriteByte(EXT_FPGA_REG_PARAM_STATUS, &_chValue);

	}

	return EXIT_SUCCESS;
}


