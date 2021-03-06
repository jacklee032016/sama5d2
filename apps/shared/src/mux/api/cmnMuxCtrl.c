
#include "libCmnSys.h"
#include "_cmnMux.h"

#include "mux7xx.h"

#include "muxFpga.h"

typedef	enum
{
	_SETUP_TYPE_SYSTEM = 0,	/* save and reboot */
	_SETUP_TYPE_NAME,			/* save */
	_SETUP_TYPE_RS232,			/* save and setup */
	_SETUP_TYPE_PROTOCOL,
	_SETUP_TYPE_SDP,
	_SETUP_TYPE_MEDIA,
	_SETUP_TYPE_PTP,			/* save and setup */
	_SETUP_TYPE_CONNECT,
}_SETUP_TYPE;



static unsigned short	_setupType;


#define	SETUP_SET_TYPE( _type)	\
		( _setupType |= (1<<(_type) ) )


#define	SETUP_CHECK_TYPE( _type)	\
		(( _setupType & (1<<(_type) ) ) != 0 )


#define	_checkNumU8FieldValue(dest, src, ret)	\
	if(FIELD_IS_CHANGED_U8((src)) && ((src) != *(dest)) ){ *(dest) = (src); (ret) = EXT_TRUE; }

#define	_checkNumU16FieldValue(dest, src, ret)	\
	if(FIELD_IS_CHANGED_U16((src)) && ((src) != *(dest)) ){ *(dest) = (src); (ret) = EXT_TRUE; }

#define	_checkNumU32FieldValue(dest, src, ret)	\
	if(FIELD_IS_CHANGED_U32((src)) && ((src) != *(dest)) ){ *(dest) = (src); (ret) = EXT_TRUE; }


/* comparing MAC/IP/Mask/gw/dhcp and isTx
After update, reboot is needed, so save is also needed */
static char _compareSystemCfg(EXT_RUNTIME_CFG *runCfg, EXT_RUNTIME_CFG *rxCfg)
{
	char ret = EXT_FALSE;
	
	if( rxCfg->local.ip != INVALIDATE_VALUE_U32 )
	{
		runCfg->local.ip = rxCfg->local.ip;

		ret = EXT_TRUE;
	}
	
	if(rxCfg->ipMask != INVALIDATE_VALUE_U32)
	{
		runCfg->ipMask = rxCfg->ipMask;
		ret = EXT_TRUE;
	}
	
	if(rxCfg->ipGateway != INVALIDATE_VALUE_U32)	
	{
		runCfg->ipGateway = rxCfg->ipGateway;
		ret = EXT_TRUE;
	}

#if 0	
	{
		const ip4_addr_t *mcIpAddr;
		if(!EXT_IS_TX(rawCfg) )
		{/* RX */
			mcIpAddr = (ip4_addr_t *)&rxCfg->local.ip;
			if(! ip4_addr_ismulticast(mcIpAddr))
			{
				return EXIT_FAILURE;
			}
			
			return EXIT_SUCCESS;
		}
		else
		{
			MUX_DEBUG("DHCP or local IP is not same:mode:%d=%d;IP:%d=%d;mask:%d=%d;gw:%d=%d"LWIP_NEW_LINE, 
				rawCfg->netMode, rxCfg->netMode, rawCfg->local.ip, rxCfg->local.ip, rawCfg->ipMask, rxCfg->ipMask, rawCfg->ipGateway, rxCfg->ipGateway );
			return EXIT_FAILURE;
		}
	}
#endif


	if(rxCfg->netMode != 0XFF )
	{/* check DHCP mode must only be after modify IP/mask/network */
		if( (runCfg->netMode) !=  (rxCfg->netMode) )
		{
			EXT_CFG_SET_DHCP(runCfg, (rxCfg->netMode) );
			MUX_DEBUG("DHCP set as: %s", (rxCfg->netMode)? "YES":"NO");

//			cmnSysEthernetConfig(runCfg );

			ret = EXT_TRUE;
		}
	}



	if(!MAC_ADDR_IS_NULL(&rxCfg->local.mac) )
	{
		if(! MAC_ADDR_IS_EQUAL(&runCfg->local.mac, &rxCfg->local.mac) )
		{
			MUX_DEBUG( "MAC address is not same:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x(Old)!=%.2x:%.2x:%.2x:%.2x:%.2x:%.2x(New)", 
				runCfg->local.mac.address[0], runCfg->local.mac.address[1], runCfg->local.mac.address[2], 
				runCfg->local.mac.address[3], runCfg->local.mac.address[4], runCfg->local.mac.address[5], 
				rxCfg->local.mac.address[0], rxCfg->local.mac.address[1], rxCfg->local.mac.address[2],
				rxCfg->local.mac.address[3], rxCfg->local.mac.address[4], rxCfg->local.mac.address[5]);
			memcpy(&runCfg->local.mac, &rxCfg->local.mac, EXT_MAC_ADDRESS_LENGTH);
			runCfg->isMacConfiged = EXT_TRUE;

#if 1
			/* only MAC is modified */
//			cmnSysEthernetConfig(runCfg);
#else
			/* save MAC address into u-boot env partition */
			cmnSysSaveMac2Uboot(runCfg);
#endif			
			ret = EXT_TRUE;
		}
	}

	if(ret == EXT_TRUE)
	{
		cmnSysEthernetConfig(runCfg );/* 01.14, 2020, changed */
	}

	if(strlen(rxCfg->name) )
	{
		if(!IS_STRING_EQUAL(runCfg->name, rxCfg->name) )
		{
			snprintf(runCfg->name, sizeof(runCfg->name), "%s", rxCfg->name);			
			ret = EXT_TRUE;
		}
	}

#if EXT_DIP_SWITCH_ON
//	if(_checkBoolField(&runCfg->isDipOn, rxCfg->isDipOn, EXT_TRUE) == EXIT_SUCCESS)
	_checkNumU8FieldValue(&runCfg->isDipOn, rxCfg->isDipOn, ret);
	if(ret == EXT_TRUE)
	{
		EXT_DEBUGF(EXT_IPCMD_DEBUG, "DipOn:%s"EXT_NEW_LINE, STR_BOOL_VALUE(runCfg->isDipOn) );

		if(!EXT_IS_DIP_ON(runCfg) )
		{/* DIP ON --> OFF */
//			extCmnNewDestIpEffective(runCfg, runCfg->ipMulticast);
		}
		/* when DIP is changed to ON, poll job will configure new mulitcast IP from DIP switch */
#if 0
		else
		{/* DIP OFF-->ON */
		}
#endif		
		ret = EXT_TRUE;
	}
#endif

	return ret;
}


/* ptp */
static char _comparePtpConfig(EXT_RUNTIME_CFG *runCfg, EXT_RUNTIME_CFG *rxCfg)
{
	char ret = EXT_FALSE;
	MuxPtpConfig *ptpCfg = &runCfg->ptpConfig;
	MuxPtpConfig *rxPtpCfg = &rxCfg->ptpConfig;

	/* always enabled and SlaveOnly. 12.02, 2019 */
	_checkNumU8FieldValue(&ptpCfg->isEnable, rxPtpCfg->isEnable, ret);
	if(ret == EXT_TRUE)
	{/* from disable to enable */
	
	}
#if 0	

	_checkNumU8FieldValue(&ptpCfg->isSlaveOnly, rxPtpCfg->isSlaveOnly, ret);	
#endif

	_checkNumU8FieldValue(&ptpCfg->domain, rxPtpCfg->domain, ret);
	
	_checkNumU8FieldValue(&ptpCfg->priority1, rxPtpCfg->priority1, ret);	
	_checkNumU8FieldValue(&ptpCfg->priority2, rxPtpCfg->priority2, ret);	

	
	_checkNumU8FieldValue(&ptpCfg->clockClass, rxPtpCfg->clockClass, ret);	
	_checkNumU8FieldValue(&ptpCfg->clockAccuracy, rxPtpCfg->clockAccuracy, ret);
	
	_checkNumU16FieldValue(&ptpCfg->offsetScaledLogVariance, rxPtpCfg->offsetScaledLogVariance, ret);
	
	return ret;
}



/* dest IP and ports */
static char _compareRs232Config(EXT_RUNTIME_CFG *runCfg, EXT_RUNTIME_CFG *rxCfg)
{
	char ret = EXT_FALSE;//, _ret = EXT_FALSE;
	
	_checkNumU32FieldValue(&runCfg->rs232Cfg.baudRate, rxCfg->rs232Cfg.baudRate, ret);	

	_checkNumU8FieldValue(&runCfg->rs232Cfg.charLength, rxCfg->rs232Cfg.charLength, ret);	

	_checkNumU8FieldValue(&runCfg->rs232Cfg.stopbits, rxCfg->rs232Cfg.stopbits, ret);	

	_checkNumU8FieldValue(&runCfg->rs232Cfg.parityType, rxCfg->rs232Cfg.parityType, ret);	
	
	return ret;
}



/* mac, ip, video/audio port of peer */
static char _compareMediaCfg(EXT_RUNTIME_CFG *runCfg, EXT_RUNTIME_CFG *rxCfg)
{
//	const ip4_addr_t *mcIpAddr;
	char ret = EXT_FALSE;

	if(FIELD_IS_CHANGED_U8(rxCfg->fpgaAuto) )
	{
		if( (runCfg->fpgaAuto) !=  (rxCfg->fpgaAuto) )
		{
			EXT_DEBUGF(DEBUG_SYS_CTRL, FIELD_SYS_CFG_MEDIA_AUTO" changed from %s to %s", 
				CMN_FIND_MEDIA_MODE(runCfg->fpgaAuto), CMN_FIND_MEDIA_MODE(rxCfg->fpgaAuto) );
			runCfg->fpgaAuto = rxCfg->fpgaAuto;
			
			ret = EXT_TRUE;
		}
	}

	/* video */
	_checkNumU16FieldValue(&runCfg->runtime.vWidth, rxCfg->runtime.vWidth, ret);	

	_checkNumU16FieldValue(&runCfg->runtime.vHeight, rxCfg->runtime.vHeight, ret);	

	_checkNumU8FieldValue(&runCfg->runtime.vFrameRate, rxCfg->runtime.vFrameRate, ret);	


	_checkNumU8FieldValue(&runCfg->runtime.vDepth, rxCfg->runtime.vDepth, ret);	

	_checkNumU8FieldValue(&runCfg->runtime.vColorSpace, rxCfg->runtime.vColorSpace, ret);	

	_checkNumU8FieldValue(&runCfg->runtime.vIsInterlaced, rxCfg->runtime.vIsInterlaced, ret);	

//	_checkNumU8FieldValue(&runCfg->runtime.vIsSegmented, rxCfg->runtime.vIsSegmented, ret);	

	/* audio */
	_checkNumU8FieldValue(&runCfg->runtime.aSampleRate, rxCfg->runtime.aSampleRate, ret);	

	_checkNumU8FieldValue(&runCfg->runtime.aChannels, rxCfg->runtime.aChannels, ret);	

	_checkNumU8FieldValue(&runCfg->runtime.aDepth, rxCfg->runtime.aDepth, ret);	

	_checkNumU8FieldValue(&runCfg->runtime.aPktSize, rxCfg->runtime.aPktSize, ret);	


	_checkNumU8FieldValue(&runCfg->runtime.ttlVideo, rxCfg->runtime.ttlVideo, ret);	
	_checkNumU8FieldValue(&runCfg->runtime.ttlAudio, rxCfg->runtime.ttlAudio, ret);	

	_checkNumU8FieldValue(&runCfg->runtime.rtpTypeVideo, rxCfg->runtime.rtpTypeVideo, ret);	
	_checkNumU8FieldValue(&runCfg->runtime.rtpTypeAudio, rxCfg->runtime.rtpTypeAudio, ret);	
	_checkNumU8FieldValue(&runCfg->runtime.rtpTypeAnc, rxCfg->runtime.rtpTypeAnc, ret);	

	return ret;
}



/* SDP IP and ports */
static char _compareSdpConfig(EXT_RUNTIME_CFG *runCfg, EXT_RUNTIME_CFG *rxCfg)
{
	char ret = EXT_FALSE;

	if(rxCfg->sdpUriVideo.ip != INVALIDATE_VALUE_U32 && rxCfg->sdpUriVideo.ip != runCfg->sdpUriVideo.ip )
	{
		 runCfg->sdpUriVideo.ip = rxCfg->sdpUriVideo.ip;
		ret = EXT_TRUE;
	}
	_checkNumU16FieldValue(&runCfg->sdpUriVideo.port, rxCfg->sdpUriVideo.port, ret);
	if(( !IS_STRING_NULL_OR_ZERO(rxCfg->sdpUriVideo.uri) ) &&(!IS_STRING_EQUAL(rxCfg->sdpUriVideo.uri, runCfg->sdpUriVideo.uri) ))
	{
		snprintf(runCfg->sdpUriVideo.uri, sizeof(runCfg->sdpUriVideo.uri), "%s", rxCfg->sdpUriVideo.uri);
		ret = EXT_TRUE;
	}


	if(rxCfg->sdpUriAudio.ip != INVALIDATE_VALUE_U32 && rxCfg->sdpUriAudio.ip != runCfg->sdpUriAudio.ip )
	{
		 runCfg->sdpUriAudio.ip = rxCfg->sdpUriAudio.ip;
		ret = EXT_TRUE;
	}
	_checkNumU16FieldValue(&runCfg->sdpUriAudio.port, rxCfg->sdpUriAudio.port, ret);	
	if(( !IS_STRING_NULL_OR_ZERO(rxCfg->sdpUriAudio.uri) ) && (!IS_STRING_EQUAL(rxCfg->sdpUriAudio.uri, runCfg->sdpUriAudio.uri) ))
	{
		snprintf(runCfg->sdpUriAudio.uri, sizeof(runCfg->sdpUriAudio.uri), "%s", rxCfg->sdpUriAudio.uri);
		ret = EXT_TRUE;
	}

	if(rxCfg->sdpUriAnc.ip != INVALIDATE_VALUE_U32 && rxCfg->sdpUriAnc.ip != runCfg->sdpUriAnc.ip )
	{
		 runCfg->sdpUriAnc.ip = rxCfg->sdpUriAnc.ip;
		ret = EXT_TRUE;
	}
	_checkNumU16FieldValue(&runCfg->sdpUriAnc.port, rxCfg->sdpUriAnc.port, ret);	
	if(( !IS_STRING_NULL_OR_ZERO(rxCfg->sdpUriAnc.uri) ) && (!IS_STRING_EQUAL(rxCfg->sdpUriAnc.uri, runCfg->sdpUriAnc.uri) ))
	{
		snprintf(runCfg->sdpUriAnc.uri, sizeof(runCfg->sdpUriAnc.uri), "%s", rxCfg->sdpUriAnc.uri);
		ret = EXT_TRUE;
	}

	return ret;
}


/* dest IP and ports */
static char _compareProtocolConfig(EXT_RUNTIME_CFG *runCfg, EXT_RUNTIME_CFG *rxCfg)
{
	char ret = EXT_FALSE;

#if 0
	_checkNumU8FieldValue(&runCfg->isMCast, rxCfg->isMCast, ret);	
	{
		EXT_DEBUGF(EXT_IPCMD_DEBUG, ("isMcast:%s"LWIP_NEW_LINE, STR_BOOL_VALUE(runCfg->isMCast)) );
		ret = EXT_TRUE;
	}
#endif

//	if( _checkIntegerField(&runCfg->dest.ip, rxCfg->dest.ip) == EXIT_SUCCESS)
	if(rxCfg->dest.ip != INVALIDATE_VALUE_U32 && rxCfg->dest.ip != runCfg->dest.ip )
	{
//		runCfg->ipMulticast = rxCfg->dest.ip;
		EXT_DEBUGF(EXT_IPCMD_DEBUG, "New video McastAddress:%s"EXT_NEW_LINE, cmnSysNetAddress(rxCfg->dest.ip) );
#if EXT_DIP_SWITCH_ON
		if(!EXT_IS_DIP_ON(runCfg) )
		{/* FPGA maybe configured twice: furst here; second, in setupParams() */
//			extCmnNewDestIpEffective( runCfg, runCfg->ipMulticast);
		}
//		else

		{/* config type: system, so only save, not reconfigure FPGA */
		}
#endif		
//		extLwipGroupMgr(runCfg, runCfg->dest.ip, EXT_FALSE);
		runCfg->dest.ip = rxCfg->dest.ip;
		ret = EXT_TRUE;
	}

	if(rxCfg->dest.audioIp != INVALIDATE_VALUE_U32 && rxCfg->dest.audioIp != runCfg->dest.audioIp )
	{
		EXT_DEBUGF(EXT_IPCMD_DEBUG, "New Audio McastAddress:%s"EXT_NEW_LINE, cmnSysNetAddress(rxCfg->dest.ip) );
//		extLwipGroupMgr(runCfg, runCfg->dest.audioIp, EXT_FALSE);
		
		runCfg->dest.audioIp = rxCfg->dest.audioIp;
		ret = EXT_TRUE;
	}

	if(rxCfg->dest.ancIp != INVALIDATE_VALUE_U32 && rxCfg->dest.ancIp != runCfg->dest.ancIp )
	{
		EXT_DEBUGF(EXT_IPCMD_DEBUG, "New ANC McastAddress:%s"EXT_NEW_LINE, cmnSysNetAddress(rxCfg->dest.ip) );
//		extLwipGroupMgr(runCfg, runCfg->dest.ancIp, EXT_FALSE);

		runCfg->dest.ancIp = rxCfg->dest.ancIp;
		ret = EXT_TRUE;
	}

#if EXT_FPGA_AUX_ON	
	if(rxCfg->dest.auxIp != INVALIDATE_VALUE_U32 && rxCfg->dest.auxIp != runCfg->dest.auxIp )
	{
		EXT_DEBUGF(EXT_IPCMD_DEBUG, "New AUX McastAddress:%s"EXT_NEW_LINE, cmnSysNetAddress(rxCfg->dest.ip) );
		runCfg->dest.auxIp = rxCfg->dest.auxIp;
		ret = EXT_TRUE;
	}
#endif

	_checkNumU16FieldValue(&runCfg->dest.vport, rxCfg->dest.vport, ret);	

	_checkNumU16FieldValue(&runCfg->dest.aport, rxCfg->dest.aport, ret);	

	_checkNumU16FieldValue(&runCfg->dest.dport, rxCfg->dest.dport, ret);	

	_checkNumU8FieldValue(&runCfg->runtime.ttlVideo, rxCfg->runtime.ttlVideo, ret);	
	_checkNumU8FieldValue(&runCfg->runtime.ttlAudio, rxCfg->runtime.ttlAudio, ret);	

	_checkNumU8FieldValue(&runCfg->runtime.rtpTypeVideo, rxCfg->runtime.rtpTypeVideo, ret);	
	_checkNumU8FieldValue(&runCfg->runtime.rtpTypeAudio, rxCfg->runtime.rtpTypeAudio, ret);	

#if EXT_FPGA_AUX_ON	
	_checkNumU16FieldValue(&runCfg->dest.sport, rxCfg->dest.sport, ret);	
#endif

	return ret;
}

char extSysCompareParams(EXT_RUNTIME_CFG *runCfg, EXT_RUNTIME_CFG *rxCfg)
{
//	DEBUG_CFG_PARAMS();
	int isSfpChanged = EXT_FALSE;
	int isForceStreamChanged = EXT_FALSE;

	if(_compareSystemCfg(runCfg, rxCfg) == EXT_TRUE)
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "System params changing" );
		SETUP_SET_TYPE(_SETUP_TYPE_SYSTEM);
	}

	if(_compareProtocolConfig(runCfg, rxCfg) == EXT_TRUE)
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "Protocol params changing" );
		SETUP_SET_TYPE(_SETUP_TYPE_PROTOCOL);
	}
	
	if(_compareMediaCfg(runCfg, rxCfg) == EXT_TRUE)
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "Media params changing" );
		SETUP_SET_TYPE(_SETUP_TYPE_MEDIA);
	}


	if(_compareSdpConfig(runCfg, rxCfg) == EXT_TRUE)
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "SDP params changing" );
		SETUP_SET_TYPE(_SETUP_TYPE_SDP);
	}

	if(_comparePtpConfig(runCfg, rxCfg) == EXT_TRUE)
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "PTP params changing" );
		SETUP_SET_TYPE(_SETUP_TYPE_PTP);
	}

	if(_compareRs232Config(runCfg, rxCfg) == EXT_TRUE)
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "RS232 params changing" );
		SETUP_SET_TYPE(_SETUP_TYPE_RS232);
	}

	if(FIELD_IS_CHANGED_U8(rxCfg->runtime.blink) && (rxCfg->runtime.blink != runCfg->runtime.blink) ) 
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "LED POWER new state: %s", (rxCfg->runtime.blink)?"Blink":"Off" );
		runCfg->runtime.blink = rxCfg->runtime.blink;
		cmnSysCtrlBlinkPowerLED(runCfg->runtime.blink);
	}

	if(FIELD_IS_CHANGED_U8(rxCfg->runtime.reset) && (rxCfg->runtime.reset != runCfg->runtime.reset) ) 
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "Reset %d", rxCfg->runtime.reset);
		runCfg->runtime.reset = rxCfg->runtime.reset;
		cmnSysCtrlDelayReset(SYS_REBOOT_DELAY_MS, runCfg);
	}

	if(FIELD_IS_CHANGED_U8(rxCfg->runtime.reboot) && (rxCfg->runtime.reboot != runCfg->runtime.reboot) ) 
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "Reboot %d", rxCfg->runtime.reboot );
		runCfg->runtime.reboot = rxCfg->runtime.reboot;
		cmnSysCtrlDelayReboot(SYS_REBOOT_DELAY_MS, runCfg);
	}

	if(! IS_STRING_NULL_OR_ZERO(rxCfg->hexData) )
	{
		memcpy(runCfg->hexData, rxCfg->hexData, sizeof(rxCfg->hexData) );
	}

	if(EXT_DEBUG_REST_IS_ENABLE(runCfg) || EXT_DEBUG_SDP_IS_ENABLE(runCfg))
	{
		printf(EXT_NEW_LINE"After configured, Runtime Configuration:"EXT_NEW_LINE);
		cmnMuxCfgDebugData(runCfg);
	}

	/* system params for FPGA */
	if(FIELD_IS_CHANGED_U8(rxCfg->sfpCfg ) && (rxCfg->sfpCfg != runCfg->sfpCfg ) ) 
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "SFP Configuration %d", rxCfg->sfpCfg);
		runCfg->sfpCfg = rxCfg->sfpCfg;
		isSfpChanged = EXT_TRUE;
	}

	if(FIELD_IS_CHANGED_U8(rxCfg->isConvert ) && (rxCfg->isConvert != runCfg->isConvert ) ) 
	{
		EXT_DEBUGF(DEBUG_SYS_CTRL, "ForceStrean Configuration %d", rxCfg->isConvert);
		runCfg->isConvert = rxCfg->isConvert;
		isForceStreamChanged = EXT_TRUE;
	}

	if(isForceStreamChanged == EXT_TRUE || isSfpChanged == EXT_TRUE )
	{
		cmnSysCfgSave(runCfg, EXT_CFG_MAIN);
		sysFpgaVideoForced();
	}

	return EXIT_SUCCESS;
}


/* call wen chip configuration is changed; not data is send. For example, change RS232 will call this function, but send data to RS232 will not
*  Because read reply data is needed after write data, so delay maybe needed for data operation
*/
static int _cmnSysConfigCtrl(EXT_RUNTIME_CFG *runCfg, EXT_RUNTIME_CFG *rxCfg)
{
	FpgaConfig 	*fpga = (FpgaConfig 	*)runCfg->fpgaCfg;
	MuxMain *muxMain = runCfg->muxMain;
	
//	EXT_DEBUGF(EXT_DBG_ON, ("config options:0x%x (0x%x)"LWIP_NEW_LINE, _setupType, SETUP_CHECK_TYPE(_SETUP_TYPE_RS232) ) );

	if(fpga== NULL)
	{
		EXT_ERRORF("FPGA is not initialized");
		return EXIT_FAILURE;
	}

	/* save configuration, and reboot to make it active */
	//if(needReboot || hasNewMedia  || needSave)
	if( SETUP_CHECK_TYPE(_SETUP_TYPE_SYSTEM) )	
	{
		cmnSysCfgSave(runCfg, EXT_CFG_MAIN);
		EXT_DEBUGF(EXT_DBG_ON, "New system configuration, saving configuration and reboot" );
	}

	if( SETUP_CHECK_TYPE(_SETUP_TYPE_SDP) )	
	{
		cmnSysCfgSave(runCfg, EXT_CFG_MAIN);
//		bspCmdReboot(NULL, NULL, 0);
		EXT_DEBUGF(EXT_DBG_ON, "New SDP parameters" );
	}

	//if(needReboot)
	if(SETUP_CHECK_TYPE(_SETUP_TYPE_RS232) || SETUP_CHECK_TYPE(_SETUP_TYPE_NAME))
	{
		cmnSysCfgSave(runCfg, EXT_CFG_MAIN);
		if(SETUP_CHECK_TYPE(_SETUP_TYPE_RS232) )
		{
			if(MUX_MAIN_IS_DEBUG_MSG(muxMain))
			{
				MUX_INFO("RS232 new configuration: bitrate:%d; parity:%s; databits:%d; stopbits:%d", 
					runCfg->rs232Cfg.baudRate, CMN_FIND_RS_PARITY(runCfg->rs232Cfg.parityType), runCfg->rs232Cfg.charLength, runCfg->rs232Cfg.stopbits);
			}
			cmnSysRs232Config(runCfg);
		}
		EXT_DEBUGF(EXT_DBG_ON, "RS232 save and setup" );
	}
	
	//if(hasNewMedia)
	if(SETUP_CHECK_TYPE(_SETUP_TYPE_PROTOCOL) )
	{
//		extIgmpGroupMgr(rxCfg, EXT_FALSE);
//		memcpy(&runCfg->dest,  &rxCfg->dest, sizeof(EXT_VIDEO_CONFIG));

		cmnSysCfgSave(runCfg, EXT_CFG_MAIN);

		fpga->opProtocolCtrl(fpga);
		EXT_DEBUGF(EXT_DBG_ON, "FPGA configuration Protocol");
	}
	
	if( SETUP_CHECK_TYPE(_SETUP_TYPE_MEDIA) )
	{
		cmnSysCfgSave(runCfg, EXT_CFG_MAIN);
		fpga->opMediaWrite(fpga);
		EXT_DEBUGF(EXT_DBG_ON, "FPGA configuration Media" );
	}

	if( SETUP_CHECK_TYPE(_SETUP_TYPE_PTP) )	
	{
		cmnMuxSavePtpConfig(muxMain);
		
		cmnSysCfgSave(runCfg, EXT_CFG_MAIN);
//		bspCmdReboot(NULL, NULL, 0);
		EXT_DEBUGF(EXT_DBG_ON, "New PTP parameters" );
	}

#if 0
	/* connect command */
	if(FIELD_IS_CHANGED(rxCfg->runtime.isConnect))
	{
		EXT_DEBUGF(EXT_DBG_ON, ("IsConnect changed: %d(Runtime:%d)", rxCfg->runtime.isConnect, runCfg->runtime.isConnect) );
//		if( parser->runCfg->runtime.isConnect != (rxCfg->runtime.isConnect-1) )
		{
			extCmdConnect( runCfg, rxCfg->runtime.isConnect-1 );
		}
	}
#endif

	cmnSysJsonUpdate(runCfg->muxMain);

	return EXIT_SUCCESS;
}

void cmnMuxCfgDebugData(EXT_RUNTIME_CFG *cfg)
{
	printf("%s"EXT_NEW_LINE"\tproduct:%s; name:%s; model:%s; IP:%s"EXT_NEW_LINE, MUX_REST_URI_SYSTEM, 
		cfg->product, cfg->name, cfg->model, cmnSysNetAddress(cfg->local.ip));
	printf("\tnetmask:%s; MAC:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X"EXT_NEW_LINE, 
		cmnSysNetAddress(cfg->ipMask), cfg->local.mac.address[0], cfg->local.mac.address[1], cfg->local.mac.address[2], 
			cfg->local.mac.address[3], cfg->local.mac.address[4], cfg->local.mac.address[5]);
	printf("\tgateway:%s; isDhcp:%d; isDipOn:%d; reboot:%d; reset:%d; blink:%d"EXT_NEW_LINE, 
		cmnSysNetAddress(cfg->ipGateway), cfg->netMode, cfg->isDipOn, cfg->runtime.reboot, cfg->runtime.reset, cfg->runtime.blink );

/* video */	
	printf("%s"EXT_NEW_LINE"\tIP:%s; port:%d; TTL:%d; Payload:%d; sdp:%s"EXT_NEW_LINE, MUX_REST_URI_VIDEO, 
		cmnSysNetAddress(cfg->dest.ip), cfg->dest.vport, cfg->runtime.ttlVideo, cfg->runtime.rtpTypeVideo, cfg->sdpUriVideo.uri );
	printf("\twidth:%d; height:%d; fps:%d; colorspce:%d; depth=%d, interlaced:%d"EXT_NEW_LINE, 
		cfg->runtime.vWidth, cfg->runtime.vHeight, cfg->runtime.vFrameRate, cfg->runtime.vColorSpace, cfg->runtime.vDepth, cfg->runtime.vIsInterlaced);
	
	printf("%s"EXT_NEW_LINE"\tIP:%s; port:%d; TTL:%d; Payload:%d; sdp:%s"EXT_NEW_LINE, MUX_REST_URI_AUDIO, 
		cmnSysNetAddress(cfg->dest.audioIp), cfg->dest.aport, cfg->runtime.ttlAudio, cfg->runtime.rtpTypeAudio, cfg->sdpUriAudio.uri );
	printf("\tChans:%d; pktSize:%d; sampleRate:%d; depth:%d"EXT_NEW_LINE, cfg->runtime.aChannels, cfg->runtime.aPktSize, cfg->runtime.aSampleRate, cfg->runtime.aDepth );

#if WITH_ANCILLIARY_STREAM
	printf("%s"EXT_NEW_LINE"\tIP:%s; port:%d; sdp:%s"EXT_NEW_LINE, 
		MUX_REST_URI_ANC, cmnSysNetAddress(cfg->dest.ancIp), cfg->dest.dport, cfg->sdpUriAnc.uri );

	printf("\%s:%s; "EXT_NEW_LINE, FIELD_SYS_CFG_MEDIA_AUTO, CMN_FIND_MEDIA_MODE(cfg->fpgaAuto) );
//	printf("Audio:Chans:%d; depth:%d; pktSize:%d; sampleRate:%d", cfg->runtime.aChannels, cfg->runtime.aDepth, cfg->runtime.aPktSize, cfg->runtime.aSampleRate));
#endif

	printf("%s"EXT_NEW_LINE"\tVideo IP:%s; port:%d; URI:%s"EXT_NEW_LINE,
		MUX_REST_URI_SDP, cmnSysNetAddress(cfg->sdpUriVideo.ip), cfg->sdpUriVideo.port, cfg->sdpUriVideo.uri);
	printf("\tAudio IP:%s; port:%d; URI:%s"EXT_NEW_LINE, cmnSysNetAddress(cfg->sdpUriAudio.ip), cfg->sdpUriAudio.port, cfg->sdpUriAudio.uri);
#if WITH_ANCILLIARY_STREAM
	printf("\tANC IP:%s; port:%d; URI:%s"EXT_NEW_LINE, cmnSysNetAddress(cfg->sdpUriAnc.ip), cfg->sdpUriAnc.port, cfg->sdpUriAnc.uri);
#endif

	/* RS232 */
	printf("%s"EXT_NEW_LINE"\tbitrate:%d; parity:%s; databits:%d; stopbits:%d"EXT_NEW_LINE, 
				MUX_REST_URI_RS232,	cfg->rs232Cfg.baudRate, CMN_FIND_RS_PARITY(cfg->rs232Cfg.parityType), cfg->rs232Cfg.charLength, cfg->rs232Cfg.stopbits);
}

int cmnMuxSystemConfig(EXT_RUNTIME_CFG *runCfg, 	EXT_RUNTIME_CFG	 *rxCfg)
{
	if(1)//MUX_MAIN_IS_DEBUG_MSG(runCfg->muxMain))
	{
		EXT_INFOF("Existed data:");
		cmnMuxCfgDebugData(runCfg);

		EXT_INFOF("Received/parsed data:");
		cmnMuxCfgDebugData(rxCfg);
	}

	extSysCompareParams(runCfg, rxCfg);

	return _cmnSysConfigCtrl(runCfg, rxCfg);
}

