
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef	X86
#include <errno.h>
#endif

#include "extSysParams.h"

#define	_DEFAULT_API_ROOT			"api"

static unsigned char		_readBuffer[EXT_RW_BUFFER_SIZE];
static unsigned char		_writeBuffer[EXT_RW_BUFFER_SIZE];

/* used in bootloader, RTOS and Linux simhost; after TX/RX is determined, this function is called */
void cmnSysCfgFromFactory( EXT_RUNTIME_CFG *cfg )
{
	cfg->magic[0] = EXT_MAGIC_VALUE_A;
	cfg->magic[1] = EXT_MAGIC_VALUE_B;

	sprintf(cfg->product, "%s", EXT_MUX_PRODUCT_NAME);
	sprintf(cfg->name, "%s", EXT_MUX_PRODUCT_NAME);
	sprintf(cfg->model, "%s", EXT_MUX_MODEL);

#if 0
	/* read from muxMain every time when it starts */
	cfg->version.major = BL_VERSION_MAJOR;
	cfg->version.minor = BL_VERSION_MINOR;
	cfg->version.revision = BL_VERSION_REVISION;
#endif

	snprintf(cfg->user, EXT_USER_SIZE, "%s", EXT_USER);
	snprintf(cfg->password, EXT_PASSWORD_SIZE, "%s", EXT_PASSWORD);

	snprintf(cfg->superUser, EXT_USER_SIZE, "%s", EXT_SUPER_USER);
	snprintf(cfg->superPassword, EXT_PASSWORD_SIZE, "%s", EXT_SUPER_PASSWORD);

#if 1
	cfg->isMCast = EXT_TRUE;
#else
	cfg->isMCast = EXT_FALSE;
#endif
	cfg->ipMulticast = CFG_MAKE_IP_ADDRESS(MCAST_DEFAULT_IPADDR0, MCAST_DEFAULT_IPADDR1, MCAST_DEFAULT_IPADDR2, MCAST_DEFAULT_IPADDR3);

	cfg->isUpdate = EXT_FALSE;
#if EXT_DIP_SWITCH_ON
	cfg->isDipOn = EXT_TRUE;
#endif
	cfg->isMacConfiged = EXT_FALSE;

	cfg->sfpCfg = EXT_SFP_CFG_FIRST;
	cfg->isConvert = EXT_TRUE;
	
#if 0
	cfg->macAddress.address[0] = ETHERNET_CONF_ETHADDR0;
	cfg->macAddress.address[1] = ETHERNET_CONF_ETHADDR1;
	cfg->macAddress.address[2] = ETHERNET_CONF_ETHADDR2;
	cfg->macAddress.address[3] = ETHERNET_CONF_ETHADDR3;
	cfg->macAddress.address[5] = ETHERNET_CONF_ETHADDR5;
#endif

	cfg->local.mac.address[0] = ETHERNET_CONF_ETHADDR0;
	cfg->local.mac.address[1] = ETHERNET_CONF_ETHADDR1;
	cfg->local.mac.address[2] = ETHERNET_CONF_ETHADDR2;
	cfg->local.mac.address[3] = ETHERNET_CONF_ETHADDR3;
	cfg->local.mac.address[5] = ETHERNET_CONF_ETHADDR5;

	cfg->dest.mac.address[0] = ETHERNET_CONF_ETHADDR0;
	cfg->dest.mac.address[1] = ETHERNET_CONF_ETHADDR1;
	cfg->dest.mac.address[2] = ETHERNET_CONF_ETHADDR2;
	cfg->dest.mac.address[3] = ETHERNET_CONF_ETHADDR3;
	cfg->dest.mac.address[5] = ETHERNET_CONF_ETHADDR5;
	
	if(EXT_IS_TX(cfg) )
	{
		cfg->local.mac.address[4] = ETHERNET_CONF_ETHADDR4_TX;
		cfg->dest.mac.address[4] = ETHERNET_CONF_ETHADDR4_RX;
		
		cfg->local.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_TX);
		cfg->local.vport = EXT_MEDIA_PORT_TX_VIDEO;
		cfg->local.aport = EXT_MEDIA_PORT_TX_AUDIO;
		cfg->local.dport = EXT_MEDIA_PORT_TX_DATA;
#if EXT_FPGA_AUX_ON	
		cfg->local.sport = EXT_MEDIA_PORT_TX_STREA;
#endif

		cfg->sdpUriAudio.type = HC_REQ_SDP_AUDIO;
		cfg->sdpUriAudio.ip = CFG_MAKE_IP_ADDRESS( ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_TX);
		cfg->sdpUriAudio.port = EXT_SDP_SVR_PORT;
		snprintf(cfg->sdpUriAudio.uri, sizeof(cfg->sdpUriAudio.uri), _DEFAULT_API_ROOT"/%s", EXT_WEBPAGE_SDP_AUDIO);

		cfg->sdpUriVideo.type = HC_REQ_SDP_VIDEO;
		cfg->sdpUriVideo.ip = CFG_MAKE_IP_ADDRESS( ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_TX );
		cfg->sdpUriVideo.port = EXT_SDP_SVR_PORT;
		snprintf(cfg->sdpUriVideo.uri, sizeof(cfg->sdpUriVideo.uri), _DEFAULT_API_ROOT"/%s", EXT_WEBPAGE_SDP_VIDEO);

		cfg->sdpUriAnc.type = HC_REQ_SDP_ANC;
		cfg->sdpUriAnc.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_TX );
		cfg->sdpUriAnc.port = EXT_SDP_SVR_PORT;
		snprintf(cfg->sdpUriAnc.uri, sizeof(cfg->sdpUriAnc.uri), _DEFAULT_API_ROOT"/%s", EXT_WEBPAGE_SDP_ANC);


		cfg->restUrl.type = HC_REQ_JSON;
		cfg->restUrl.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_TX );
		cfg->restUrl.port = EXT_SDP_SVR_PORT;
		snprintf(cfg->restUrl.uri, sizeof(cfg->restUrl.uri), "%s", EXT_WEBPAGE_API_SERVICE);

		if(cfg->isMCast == EXT_FALSE)
		{
			cfg->dest.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_RX );
			cfg->dest.audioIp = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_RX+1);
			cfg->dest.ancIp = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_RX+2);
#if EXT_FPGA_AUX_ON	
			cfg->dest.auxIp = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_RX+3);
#endif
		}
		else
		{
			cfg->dest.ip = CFG_MAKE_IP_ADDRESS(MCAST_DEFAULT_IPADDR0, MCAST_DEFAULT_IPADDR1, MCAST_DEFAULT_IPADDR2, MCAST_DEFAULT_IPADDR3);
			cfg->dest.audioIp = CFG_MAKE_IP_ADDRESS(MCAST_DEFAULT_IPADDR0, MCAST_DEFAULT_IPADDR1, MCAST_DEFAULT_IPADDR2, MCAST_DEFAULT_IPADDR3+1);
			cfg->dest.ancIp = CFG_MAKE_IP_ADDRESS(MCAST_DEFAULT_IPADDR0, MCAST_DEFAULT_IPADDR1, MCAST_DEFAULT_IPADDR2, MCAST_DEFAULT_IPADDR3+2);
#if EXT_FPGA_AUX_ON	
			cfg->dest.auxIp = CFG_MAKE_IP_ADDRESS(MCAST_DEFAULT_IPADDR0, MCAST_DEFAULT_IPADDR1, MCAST_DEFAULT_IPADDR2, MCAST_DEFAULT_IPADDR3+3);
#endif
		}
		cfg->ipGateway = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_GATEWAY_ADDR0, ETHERNET_CONF_GATEWAY_ADDR1, ETHERNET_CONF_GATEWAY_ADDR2_TX, ETHERNET_CONF_GATEWAY_ADDR3);

		if(IP4_ADDR_IS_MULTICAST(cfg->dest.ip))
		{
			extTxMulticastIP2Mac(cfg);
		}

		cfg->dest.vport = EXT_MEDIA_PORT_RX_VIDEO;
		cfg->dest.aport = EXT_MEDIA_PORT_RX_AUDIO;
		cfg->dest.dport = EXT_MEDIA_PORT_RX_DATA;
#if EXT_FPGA_AUX_ON	
		cfg->dest.sport = EXT_MEDIA_PORT_RX_STREA;
#endif
	}
	else
	{/* RX */
		/* for RX, 
		* unicast: dest and local are same
		* multicast: local is MCU, dest is multicast 
		*/
		cfg->local.mac.address[4] = ETHERNET_CONF_ETHADDR4_RX;
		cfg->dest.mac.address[4] = ETHERNET_CONF_ETHADDR4_RX;

		cfg->local.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_RX, ETHERNET_CONF_IPADDR3_RX);
		cfg->local.vport = EXT_MEDIA_PORT_RX_VIDEO;
		cfg->local.aport = EXT_MEDIA_PORT_RX_AUDIO;
		cfg->local.dport = EXT_MEDIA_PORT_RX_DATA;
#if EXT_FPGA_AUX_ON	
		cfg->local.sport = EXT_MEDIA_PORT_RX_STREA;
#endif

		cfg->sdpUriAudio.type = HC_REQ_SDP_AUDIO;
		cfg->sdpUriAudio.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_TX);
		cfg->sdpUriAudio.port = EXT_SDP_SVR_PORT;
		snprintf(cfg->sdpUriAudio.uri, sizeof(cfg->sdpUriAudio.uri), _DEFAULT_API_ROOT"/%s", EXT_WEBPAGE_SDP_AUDIO);

		cfg->sdpUriVideo.type = HC_REQ_SDP_VIDEO;
		cfg->sdpUriVideo.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_TX);
		cfg->sdpUriVideo.port = EXT_SDP_SVR_PORT;
		snprintf(cfg->sdpUriVideo.uri, sizeof(cfg->sdpUriVideo.uri), _DEFAULT_API_ROOT"/%s", EXT_WEBPAGE_SDP_VIDEO);

		cfg->sdpUriAnc.type = HC_REQ_SDP_ANC;
		cfg->sdpUriAnc.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_TX );
		cfg->sdpUriAnc.port = EXT_SDP_SVR_PORT;
		snprintf(cfg->sdpUriAnc.uri, sizeof(cfg->sdpUriAnc.uri), _DEFAULT_API_ROOT"/%s", EXT_WEBPAGE_SDP_ANC);


		cfg->restUrl.type = HC_REQ_JSON;
		cfg->restUrl.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_TX);
		cfg->restUrl.port = EXT_SDP_SVR_PORT;
		snprintf(cfg->restUrl.uri, sizeof(cfg->restUrl.uri), "%s", EXT_WEBPAGE_API_SERVICE);
		
		cfg->htmlUrl.type = HC_REQ_HTML;
		cfg->htmlUrl.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_TX, ETHERNET_CONF_IPADDR3_TX);
		cfg->htmlUrl.port = EXT_SDP_SVR_PORT;
		snprintf(cfg->restUrl.uri, sizeof(cfg->restUrl.uri), "%s", EXT_WEBPAGE_MEDIA);

		if(cfg->isMCast == EXT_FALSE)
		{
			cfg->dest.ip = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_RX, ETHERNET_CONF_IPADDR3_RX );
			cfg->dest.audioIp = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_RX, ETHERNET_CONF_IPADDR3_RX+1);
			cfg->dest.ancIp = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_RX, ETHERNET_CONF_IPADDR3_RX+2);
#if EXT_FPGA_AUX_ON	
			cfg->dest.auxIp = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, ETHERNET_CONF_IPADDR2_RX, ETHERNET_CONF_IPADDR3_RX+3);
#endif
		}
		else
		{
			cfg->dest.ip = CFG_MAKE_IP_ADDRESS(MCAST_DEFAULT_IPADDR0, MCAST_DEFAULT_IPADDR1, MCAST_DEFAULT_IPADDR2, MCAST_DEFAULT_IPADDR3);
			cfg->dest.audioIp = CFG_MAKE_IP_ADDRESS(MCAST_DEFAULT_IPADDR0, MCAST_DEFAULT_IPADDR1, MCAST_DEFAULT_IPADDR2, MCAST_DEFAULT_IPADDR3+1);
			cfg->dest.ancIp = CFG_MAKE_IP_ADDRESS(MCAST_DEFAULT_IPADDR0, MCAST_DEFAULT_IPADDR1, MCAST_DEFAULT_IPADDR2, MCAST_DEFAULT_IPADDR3+2);
#if EXT_FPGA_AUX_ON	
			cfg->dest.auxIp = CFG_MAKE_IP_ADDRESS(MCAST_DEFAULT_IPADDR0, MCAST_DEFAULT_IPADDR1, MCAST_DEFAULT_IPADDR2, MCAST_DEFAULT_IPADDR3+3);
#endif
		}
		cfg->ipGateway = CFG_MAKE_IP_ADDRESS( ETHERNET_CONF_GATEWAY_ADDR0, ETHERNET_CONF_GATEWAY_ADDR1, ETHERNET_CONF_GATEWAY_ADDR2_RX, ETHERNET_CONF_GATEWAY_ADDR3);
		if(IP4_ADDR_IS_MULTICAST(cfg->dest.ip))
		{
			extTxMulticastIP2Mac(cfg);
		}
		
		cfg->dest.vport = EXT_MEDIA_PORT_RX_VIDEO;
		cfg->dest.aport = EXT_MEDIA_PORT_RX_AUDIO;
		cfg->dest.dport = EXT_MEDIA_PORT_RX_DATA;
#if EXT_FPGA_AUX_ON	
		cfg->dest.sport = EXT_MEDIA_PORT_RX_STREA;
#endif
	}

	cfg->ipSvr811 = CFG_MAKE_IP_ADDRESS( ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1, CONF_SVR811_IPADDR2, CONF_SVR811_IPADDR3 );
	cfg->portSvr811 = CONF_SVR811_PORT;
	
	cfg->fpgaAuto = FPGA_CFG_SDP;
	
	cfg->runtime.aChannels = 12;
	cfg->runtime.aSampleRate= EXT_A_RATE_48K;
	cfg->runtime.aDepth = 24;
	cfg->runtime.aPktSize = EXT_A_PKT_SIZE_125US;
	
	cfg->runtime.vWidth = 1920;
	cfg->runtime.vHeight = 1080;
	cfg->runtime.vFrameRate = EXT_V_FRAMERATE_T_30;
	cfg->runtime.vColorSpace = EXT_V_COLORSPACE_YCBCR_444;
	cfg->runtime.vDepth = EXT_V_DEPTH_10;
	cfg->runtime.vIsInterlaced = EXT_VIDEO_INTLC_INTERLACED;

	cfg->runtime.ttlVideo = SDP_P_MEDIA_TTL_VIDEO;
	cfg->runtime.ttlAudio = SDP_P_MEDIA_TTL_AUDIO;
	
	cfg->runtime.rtpTypeVideo = SDP_P_MEDIA_FORMAT_VIDEO;
	cfg->runtime.rtpTypeAudio = SDP_P_MEDIA_FORMAT_AUDIO;
	cfg->runtime.rtpTypeAnc = SDP_P_MEDIA_FORMAT_ANC;
	cfg->runtime.vpid = SDP_P_MEDIA_VP_ID;

	cfg->runtime.rtpTypeVideo = SDP_P_MEDIA_FORMAT_VIDEO;
	
//	memcpy(&cfg->videoMacLocal, &cfg->macAddress, EXT_MAC_ADDRESS_LENGTH);
//	cfg->videoIpLocal = cfg->ipAddress;

	/* little endian */
	cfg->ipMask = CFG_MAKE_IP_ADDRESS(ETHERNET_CONF_NET_MASK0, ETHERNET_CONF_NET_MASK1, ETHERNET_CONF_NET_MASK2, ETHERNET_CONF_NET_MASK3);

	/* RS232: 57600, even, 6 bit, 2 stop-bits */
	cfg->rs232Cfg.baudRate = EXT_BAUDRATE_115200;
	cfg->rs232Cfg.charLength = EXT_RS232_CHAR_LENGTH_8;	/* 6 bits */
	cfg->rs232Cfg.parityType = EXT_RS232_PARITY_NONE;
	cfg->rs232Cfg.stopbits = EXT_RS232_STOP_BITS_1;

#ifdef  ARM
	cfg->netMode = EXT_TRUE;
#else
	cfg->netMode = EXT_FALSE;
#endif

	cfg->httpPort = EXT_HTTP_SVR_PORT;

	extNmosIdGenerate(&cfg->nodeID, cfg);
	extNmosIdGenerate(&cfg->deviceID, cfg);

	cfg->isConfigFpga = EXT_TRUE;
	memset(&cfg->firmUpdateInfo, 0, sizeof(EXT_FM_UPDATE));

	muxPtpDefaultConfig(&cfg->ptpConfig );
//	muxPtpDefaultConfig(&cfg->ptpRuntime);

#ifdef	X86
	cfg->sc.isExist = EXT_TRUE;
#endif

	cfg->endMagic[0] = EXT_MAGIC_VALUE_B;
	cfg->endMagic[1] = EXT_MAGIC_VALUE_A;
}


void extCfgInitAfterReadFromFlash(EXT_RUNTIME_CFG *runCfg)
{
	runCfg->bufRead = _readBuffer;
	runCfg->bufWrite = _writeBuffer;
	runCfg->bufLength = sizeof(_readBuffer);
	
	runCfg->runtime.reset = 0;
	runCfg->runtime.reboot = 0;
	runCfg->runtime.blink = 0;

//	memset(runCfg->hexData, 0, sizeof(runCfg->hexData) );
}

char extTxMulticastIP2Mac(EXT_RUNTIME_CFG *runCfg)
{
#if 0
	const ip4_addr_t *mcIpAddr = (ip4_addr_t *)&runCfg->dest.ip;
	if( ip4_addr_ismulticast(mcIpAddr) )
	{/* Hash IP multicast address to MAC address.*/
		runCfg->dest.mac.address[0] = LL_IP4_MULTICAST_ADDR_0;
		runCfg->dest.mac.address[1] = LL_IP4_MULTICAST_ADDR_1;
		runCfg->dest.mac.address[2] = LL_IP4_MULTICAST_ADDR_2;
		runCfg->dest.mac.address[3] = ip4_addr2(mcIpAddr) & 0x7f;
		runCfg->dest.mac.address[4] = ip4_addr3(mcIpAddr);
		runCfg->dest.mac.address[5] = ip4_addr4(mcIpAddr);

		return EXIT_SUCCESS;
	}
#endif
	return EXIT_FAILURE;
}

void	extUuidGenerate(EXT_UUID_T *uuid, EXT_RUNTIME_CFG *runCfg)
{
//	srand( runCfg->currentTimestamp);

	unsigned long random = rand();
	memcpy(&uuid->uuid[0], &random, sizeof(unsigned long));
	
	random = rand();
	memcpy(&uuid->uuid[sizeof(unsigned long)], &random, sizeof(unsigned long));

	random = rand();
	memcpy(&uuid->uuid[8], &random, 2);

	memcpy(&uuid->uuid[10], &runCfg->local.mac, 6);

//	printf("%d bytes random long\n\r", sizeof(unsigned long));
}


void	extNmosIdGenerate(MuxNmosID *nmosId, EXT_RUNTIME_CFG *runCfg)
{
	runCfg->currentTimestamp = 0;//sys_now();

	extUuidGenerate(&nmosId->uuid, runCfg);

	nmosId->version.seconds = runCfg->currentTimestamp/1000L;
	nmosId->version.nanoSeconds = runCfg->currentTimestamp * 1000L;

//	printf("%d bytes random long\n\r", sizeof(unsigned long));
}



void extSysClearConfig(EXT_RUNTIME_CFG *rxCfg)
{
#ifndef	IPADDR_NONE
#define IPADDR_NONE         ((uint32_t)0xffffffffUL)
#endif

	memset(rxCfg, 0, sizeof(EXT_RUNTIME_CFG));

	FIELD_INVALIDATE_U8(rxCfg->isMCast);
	FIELD_INVALIDATE_U8(rxCfg->netMode);
	FIELD_INVALIDATE_U8(rxCfg->fpgaAuto);


#if EXT_DIP_SWITCH_ON
	FIELD_INVALIDATE_U8(rxCfg->isDipOn);
#endif
	FIELD_INVALIDATE_U32(rxCfg->rs232Cfg.baudRate);
	FIELD_INVALIDATE_U8(rxCfg->rs232Cfg.charLength);
	FIELD_INVALIDATE_U8(rxCfg->rs232Cfg.parityType);
	FIELD_INVALIDATE_U8(rxCfg->rs232Cfg.stopbits);

	FIELD_INVALIDATE_U16(rxCfg->runtime.vWidth);
	FIELD_INVALIDATE_U16(rxCfg->runtime.vHeight);
	FIELD_INVALIDATE_U8(rxCfg->runtime.vFrameRate);
	FIELD_INVALIDATE_U8(rxCfg->runtime.vColorSpace);
	FIELD_INVALIDATE_U8(rxCfg->runtime.vDepth);
	FIELD_INVALIDATE_U8(rxCfg->runtime.vIsInterlaced);
//	FIELD_INVALIDATE_U8(rxCfg->runtime.vIsSegmented);

	FIELD_INVALIDATE_U8(rxCfg->runtime.aChannels);
	FIELD_INVALIDATE_U8(rxCfg->runtime.aSampleRate);
	FIELD_INVALIDATE_U8(rxCfg->runtime.aDepth);
	FIELD_INVALIDATE_U8(rxCfg->runtime.aPktSize);

	FIELD_INVALIDATE_U8(rxCfg->runtime.isConnect);

	FIELD_INVALIDATE_U8(rxCfg->runtime.reset);
	FIELD_INVALIDATE_U8(rxCfg->runtime.reboot);
	FIELD_INVALIDATE_U8(rxCfg->runtime.blink);

	FIELD_INVALIDATE_U8(rxCfg->runtime.ttlVideo);
	FIELD_INVALIDATE_U8(rxCfg->runtime.ttlAudio);

	FIELD_INVALIDATE_U8(rxCfg->runtime.rtpTypeVideo);
	FIELD_INVALIDATE_U8(rxCfg->runtime.rtpTypeAudio);
	FIELD_INVALIDATE_U8(rxCfg->runtime.rtpTypeAnc);
	
	rxCfg->sdpUriVideo.ip = IPADDR_NONE;
	FIELD_INVALIDATE_U16(rxCfg->sdpUriVideo.port);

	rxCfg->sdpUriAudio.ip = IPADDR_NONE;
	FIELD_INVALIDATE_U16(rxCfg->sdpUriAudio.port);

	rxCfg->sdpUriAnc.ip = IPADDR_NONE;
	FIELD_INVALIDATE_U16(rxCfg->sdpUriAnc.port);
	
	rxCfg->local.ip = IPADDR_NONE;

	rxCfg->ipGateway = IPADDR_NONE;
	rxCfg->ipMask = IPADDR_NONE;
	rxCfg->dest.ip = IPADDR_NONE;
	
	rxCfg->dest.audioIp = IPADDR_NONE;
	rxCfg->dest.ancIp = IPADDR_NONE;
#if EXT_FPGA_AUX_ON	
	rxCfg->dest.auxIp = IPADDR_NONE;
	FIELD_INVALIDATE_U16(rxCfg->dest.sport);
#endif

	FIELD_INVALIDATE_U16(rxCfg->dest.vport);
	FIELD_INVALIDATE_U16(rxCfg->dest.aport);
	FIELD_INVALIDATE_U16(rxCfg->dest.dport);

//	_setupType = 0;
}

