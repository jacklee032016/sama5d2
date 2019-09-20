/*
* JSON object and EXT_RUNTIME_CONFIG
*/

#include "libCmnSys.h"

#if 1
#define	CJSON_REPLACE_STRING(obj, strKey, strValue)	\
	{if(cJSON_ReplaceItemInObject((obj), (strKey), cJSON_CreateString((strValue)) )==0){MUX_ERROR("Replace string with key '%s', value '%s' failed", (strKey), (strValue));} }


#define	CJSON_REPLACE_INTEGRE(obj, strKey, intValue)	\
	{if(cJSON_ReplaceItemInObject((obj), (strKey), cJSON_CreateNumber((intValue)) ) == 0){MUX_ERROR("Replace integer with key '%s', value '%d' failed", (strKey), (intValue));}}
#else

#define	CJSON_REPLACE_STRING(obj, strKey, strValue)	\
	cJSON_ReplaceItemInObject((obj), (strKey), cJSON_CreateString((strValue)) )


#define	CJSON_REPLACE_INTEGRE(obj, strKey, intValue)	\
	cJSON_ReplaceItemInObject((obj), (strKey), cJSON_CreateNumber((intValue)) )
#endif

static int _cmnSysJsonUpdateOthers(EXT_RUNTIME_CFG *runCfg, cJSON *obj)
{
	CJSON_REPLACE_INTEGRE(obj, FIELD_OTHERS_AUTHEN, runCfg->muxMain->isAuthen );
	CJSON_REPLACE_INTEGRE(obj, FIELD_OTHERS_DEBUG_REST, runCfg->muxMain->debugRest);
	CJSON_REPLACE_INTEGRE(obj, FIELD_OTHERS_DEBUG_IP_CMD, runCfg->muxMain->debugCmd);
	CJSON_REPLACE_INTEGRE(obj, FIELD_OTHERS_DEBUG_SDP, runCfg->muxMain->debugSdp );
	if(cJSON_GetObjectItem(obj, FIELD_OTHERS_SDP_STATUS))
	{
		cJSON_ReplaceItemInObject(obj, FIELD_OTHERS_SDP_STATUS, cmnMuxThreadsInfo(runCfg->muxMain) );
	}
	else
	{
		cJSON_AddItemToObjectCS(obj, FIELD_OTHERS_SDP_STATUS, cmnMuxThreadsInfo(runCfg->muxMain) );
	}
	
	if(cJSON_GetObjectItem(obj, FIELD_OTHERS_THREADS))
	{
		cJSON_ReplaceItemInObject(obj, FIELD_OTHERS_THREADS, cmnMuxSdpStatus());
	}
	else
	{
		cJSON_AddItemToObjectCS(obj, FIELD_OTHERS_THREADS, cmnMuxSdpStatus());
	}

	return EXIT_SUCCESS;
}

static int _cmnSysJsonUpdateIR(EXT_RUNTIME_CFG *runCfg, cJSON *obj)
{
	/* data, use same field names as RS232 */
	CJSON_REPLACE_STRING(obj, FIELD_RS232_DATA,  "" );
	CJSON_REPLACE_INTEGRE(obj, FIELD_RS232_IS_FEEDBACK, 0);
	CJSON_REPLACE_INTEGRE(obj, FIELD_RS232_WAIT_TIME, 0);
	return EXIT_SUCCESS;
}

static int _cmnSysJsonUpdateSecurity(EXT_RUNTIME_CFG *runCfg, cJSON *obj)
{
	CJSON_REPLACE_STRING(obj, FIELD_SECURITY_GET_ID, "");
	CJSON_REPLACE_INTEGRE(obj, FIELD_SECURITY_GET_STATUS, 0 );
	CJSON_REPLACE_STRING(obj, FIELD_SECURITY_SET_KEY, "");

	return EXIT_SUCCESS;
}

static int _cmnSysJsonUpdateRs232(EXT_RUNTIME_CFG *runCfg, cJSON *obj)
{
	/* configuration */
	CJSON_REPLACE_INTEGRE(obj, FIELD_RS232_BAUDRATE, runCfg->rs232Cfg.baudRate);
	CJSON_REPLACE_INTEGRE(obj, FIELD_RS232_DATABITS, runCfg->rs232Cfg.charLength);
	CJSON_REPLACE_STRING(obj, FIELD_RS232_PARITY, CMN_FIND_RS_PARITY(runCfg->rs232Cfg.parityType) );
	CJSON_REPLACE_INTEGRE(obj, FIELD_RS232_STOPBITS, runCfg->rs232Cfg.stopbits );

	/* data */
	CJSON_REPLACE_STRING(obj, FIELD_RS232_DATA,  "" );
	CJSON_REPLACE_INTEGRE(obj, FIELD_RS232_IS_FEEDBACK, 0);
	CJSON_REPLACE_INTEGRE(obj, FIELD_RS232_WAIT_TIME, 0);
	return EXIT_SUCCESS;
}

static int __cmnSysJsonUpdateOneSdp(cJSON *sdpObj, HttpClientReq *sdpUri)
{
	CJSON_REPLACE_STRING(sdpObj, FIELD_SDP_IP, cmnSysNetAddress(sdpUri->ip) );
	CJSON_REPLACE_INTEGRE(sdpObj, FIELD_SDP_PORT, sdpUri->port );
	CJSON_REPLACE_STRING(sdpObj, FIELD_SDP_URI, sdpUri->uri );
	
	return EXIT_SUCCESS;
}



cJSON *cmnSysJsonGetSdpObject(cJSON *dataArray, char *sdpName, HttpClientReq *sdpUri)
{
	cJSON *obj;
	int count, index;
	char *keyStr;
	
	count = cJSON_GetArraySize(dataArray);
	for(index = 0; index< count; index++)
	{
	
		obj = cJSON_GetArrayItem(dataArray, index);
		if(obj == NULL)
		{
			EXT_ERRORF("No SDP item is in index#%d", index);
			return NULL;
		}
		
		keyStr = cmnGetStrFromJsonObject(obj, FIELD_SDP_MEDIA);
		if(! IS_STRING_NULL_OR_ZERO(keyStr) )
		{
			if( IS_STRING_EQUAL(keyStr, sdpName) )
			{
				__cmnSysJsonUpdateOneSdp(obj, sdpUri);
				return obj;
			}
		}
		else
		{
			EXT_ERRORF("Index#%d Field '%s' is not found in %s item", index, FIELD_SDP_MEDIA, MUX_REST_URI_SDP);
			return NULL;
		}
	}

	return NULL;
}


static int _cmnSysJsonUpdateSdp(EXT_RUNTIME_CFG *runCfg, cJSON *objArray)
{
	if(cmnSysJsonGetSdpObject(objArray, MUX_REST_URI_VIDEO, &runCfg->sdpUriVideo) == NULL)
	{
		MUX_ERROR("SDP of '%s' update failed", MUX_REST_URI_VIDEO);
		return EXIT_FAILURE;
	}
	if(cmnSysJsonGetSdpObject(objArray, MUX_REST_URI_AUDIO, &runCfg->sdpUriAudio) == NULL)
	{
		MUX_ERROR("SDP of '%s' update failed", MUX_REST_URI_AUDIO);
		return EXIT_FAILURE;
	}
	
#if WITH_ANCILLIARY_STREAM
	if(cmnSysJsonGetSdpObject(objArray, MUX_REST_URI_ANC, &runCfg->sdpUriAnc) == NULL)
	{
		MUX_ERROR("SDP of '%s' update failed", MUX_REST_URI_ANC);
		return EXIT_FAILURE;
	}
#endif
	return EXIT_SUCCESS;
}

static int _cmnSysJsonUpdateAnc(EXT_RUNTIME_CFG *runCfg, cJSON *obj)
{
	/* protocol */
	CJSON_REPLACE_STRING(obj, FIELD_ANC_IP, cmnSysNetAddress(runCfg->dest.ancIp) );
	CJSON_REPLACE_INTEGRE(obj, FIELD_ANC_PORT, runCfg->dest.dport );
	CJSON_REPLACE_STRING(obj, FIELD_ANC_SDP, runCfg->sdpUriAnc.uri );
	CJSON_REPLACE_INTEGRE(obj, FIELD_SDP_PAYLOAD_TYPE, runCfg->runtime.vpid );
	CJSON_REPLACE_INTEGRE(obj, FIELD_ANC_VP_ID, runCfg->runtime.rtpTypeAnc );
	
	return EXIT_SUCCESS;
}

static int _cmnSysJsonUpdateAudio(EXT_RUNTIME_CFG *runCfg, cJSON *obj)
{
	/* protocol */
	CJSON_REPLACE_STRING(obj, FIELD_AUDIO_IP, cmnSysNetAddress(runCfg->dest.audioIp) );
	CJSON_REPLACE_INTEGRE(obj, FIELD_AUDIO_PORT, runCfg->dest.aport );
	CJSON_REPLACE_STRING(obj, FIELD_AUDIO_SDP, runCfg->sdpUriAudio.uri );

	/* media */
	CJSON_REPLACE_INTEGRE(obj, FIELD_AUDIO_CHANNELS, runCfg->runtime.aChannels);
	CJSON_REPLACE_INTEGRE(obj, FIELD_AUDIO_DEPTH, runCfg->runtime.aDepth);

	CJSON_REPLACE_STRING(obj, FIELD_AUDIO_SAMPLE, CMN_FIND_A_RATE(runCfg->runtime.aSampleRate) );
	CJSON_REPLACE_STRING(obj, FIELD_AUDIO_PKT_SIZE, CMN_FIND_A_PKTSIZE(runCfg->runtime.aPktSize) );

	CJSON_REPLACE_INTEGRE(obj, FIELD_ANC_VP_ID, runCfg->runtime.rtpTypeAudio);
	
	return EXIT_SUCCESS;
}


static int _cmnSysJsonUpdateVideo(EXT_RUNTIME_CFG *runCfg, cJSON *obj)
{
	EXT_INFOF("Update cfg:%p:%p", runCfg, obj);
	/* protocol */
	CJSON_REPLACE_STRING(obj, FIELD_VIDEO_IP, cmnSysNetAddress(runCfg->dest.ip) );
	CJSON_REPLACE_INTEGRE(obj, FIELD_VIDEO_PORT, runCfg->dest.vport );
	CJSON_REPLACE_STRING(obj, FIELD_VIDEO_SDP, runCfg->sdpUriVideo.uri );


	CJSON_REPLACE_INTEGRE(obj, FIELD_VIDEO_WIDTH, runCfg->runtime.vWidth);
	CJSON_REPLACE_INTEGRE(obj, FIELD_VIDEO_HEIGHT, runCfg->runtime.vHeight);

	/* all interface: Ip Cmd, REST, SDP all use the format of SDP */
#if 1
	CJSON_REPLACE_STRING(obj, FIELD_VIDEO_FRAME_RATE, CMN_FIND_V_FRAME_RATE(runCfg->runtime.vFrameRate) );	/* foR SDP format */
#else	
	CJSON_REPLACE_STRING(obj, FIELD_VIDEO_FRAME_RATE, CMN_FIND_V_FPS_4_REST(runCfg->runtime.vFrameRate) );		/* for REST API/IpCmd */
#endif
	CJSON_REPLACE_STRING(obj, FIELD_VIDEO_COLOR_SPACE, CMN_FIND_V_COLORSPACE(runCfg->runtime.vColorSpace) );

	CJSON_REPLACE_INTEGRE(obj, FIELD_VIDEO_DEPTH, CMN_INT_FIND_NAME_V_DEPTH(runCfg->runtime.vDepth));
	CJSON_REPLACE_INTEGRE(obj, FIELD_VIDEO_INTERLACE, runCfg->runtime.vIsInterlaced );

	EXT_INFOF("Update cfg:%p; %dx%d; fps:%d; cs:%d; depth:%d; intlc:%d", 
		runCfg, runCfg->runtime.vWidth, runCfg->runtime.vHeight, runCfg->runtime.vFrameRate, runCfg->runtime.vColorSpace, runCfg->runtime.vDepth, runCfg->runtime.vIsInterlaced);
	
	CJSON_REPLACE_INTEGRE(obj, FIELD_ANC_VP_ID, runCfg->runtime.rtpTypeVideo);
	return EXIT_SUCCESS;
}

static int _cmnSysJsonUpdateSystem(EXT_RUNTIME_CFG *runCfg, cJSON *systemObj)
{
	cJSON *newObj = NULL;
	char macAddress[128];
	char *ifName = MUX_ETH_DEVICE;

	/* use new info after ip address has been changed */
	runCfg->local.ip = cmnSysNetGetIp(ifName);
	runCfg->ipMask = cmnSysNetGetMask(ifName);
	runCfg->ipGateway = cmnSysNetGetDefaultGw(ifName);

	/* system name */
	CJSON_REPLACE_STRING(systemObj, FIELD_SYS_CFG_PRODUCT, runCfg->product);
	CJSON_REPLACE_STRING(systemObj, FIELD_SYS_CFG_NAME, runCfg->name);
	CJSON_REPLACE_STRING(systemObj, FIELD_SYS_CFG_MODEL, runCfg->model);
	snprintf(macAddress, sizeof(macAddress), "%.2X.%.2X-%.3X",  runCfg->version.major, runCfg->version.minor, runCfg->version.revision);
	CJSON_REPLACE_STRING(systemObj, FIELD_SYS_CFG_VERSION, macAddress);
	CJSON_REPLACE_STRING(systemObj, FIELD_SYS_CFG_BUILT_DATA, runCfg->muxMain->builtDate);
	CJSON_REPLACE_INTEGRE(systemObj, FIELD_SYS_CFG_IS_TX, (runCfg->isTx==0)?0:1 );

	CJSON_REPLACE_STRING(systemObj, FIELD_SYS_CFG_FPGA_VERSION, sysFgpaVersion() );
	CJSON_REPLACE_STRING(systemObj, FIELD_SYS_CFG_FPGA_BUILT, sysFgpaBuilt());

	/* network */
	newObj = cJSON_CreateString(cmnSysNetAddress(runCfg->local.ip) );
	cJSON_ReplaceItemInObject(systemObj, FIELD_SYS_CFG_ADDRESS, newObj);

	newObj = cJSON_CreateString(cmnSysNetAddress(runCfg->ipMask) );
	cJSON_ReplaceItemInObject(systemObj, FIELD_SYS_CFG_NETMASK, newObj);

	newObj = cJSON_CreateString(cmnSysNetAddress(runCfg->ipGateway) );
	cJSON_ReplaceItemInObject(systemObj, FIELD_SYS_CFG_GATEWAY, newObj);

	snprintf(macAddress, sizeof(macAddress), "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X" , 
		runCfg->local.mac.address[0], runCfg->local.mac.address[1], runCfg->local.mac.address[2], 
		runCfg->local.mac.address[3], runCfg->local.mac.address[4], runCfg->local.mac.address[5]);
	CJSON_REPLACE_STRING(systemObj, FIELD_SYS_CFG_MAC, macAddress);

	CJSON_REPLACE_INTEGRE(systemObj, FIELD_SYS_CFG_IS_DHCP, (runCfg->netMode==0)?0:1 );

	CJSON_REPLACE_INTEGRE(systemObj, FIELD_SYS_CFG_DIP, (runCfg->isDipOn==0)?0:1 );

	
	if(!EXT_IS_TX(runCfg))
	{
		CJSON_REPLACE_STRING(systemObj, FIELD_SYS_CFG_MEDIA_AUTO, CMN_FIND_MEDIA_MODE(runCfg->fpgaAuto) );
	}
	else
	{
		CJSON_REPLACE_STRING(systemObj, FIELD_SYS_CFG_MEDIA_AUTO, "None");
	}
	
	/* reboot, reset, blinkLed are not needed to update*/	
	CJSON_REPLACE_INTEGRE(systemObj, FIELD_SYS_CFG_RESET, (runCfg->runtime.reset==0)?0:1 );
	CJSON_REPLACE_INTEGRE(systemObj, FIELD_SYS_CFG_REBOOT, (runCfg->runtime.reboot==0)?0:1 );
	CJSON_REPLACE_INTEGRE(systemObj, FIELD_SYS_CFG_BLINK, (runCfg->runtime.blink==0)?0:1 );


	return EXIT_SUCCESS;
}

/* refresh params of RunCfg into JSON object */
int cmnSysJsonUpdate(MuxMain *muxMain)
{
	cJSON *itemObj = NULL;
	
	itemObj = cmnJsonSystemGetSubItem(muxMain->systemJson, MUX_REST_URI_SYSTEM, INVALIDATE_VALUE_U32);
	if(_cmnSysJsonUpdateSystem(&muxMain->runCfg, itemObj))
	{
		MUX_ERROR("Update '%s' JSON failed", MUX_REST_URI_SYSTEM);
	}

	sysFpgaRefresh();

	itemObj = cmnJsonSystemGetSubItem(muxMain->systemJson, MUX_REST_URI_VIDEO, 0);
	if(!itemObj || _cmnSysJsonUpdateVideo(&muxMain->runCfg, itemObj))
	{
		MUX_ERROR("Update '%s' JSON failed", MUX_REST_URI_VIDEO);
	}

	itemObj = cmnJsonSystemGetSubItem(muxMain->systemJson, MUX_REST_URI_AUDIO, 0);
	if(!itemObj ||_cmnSysJsonUpdateAudio(&muxMain->runCfg, itemObj))
	{
		MUX_ERROR("Update '%s' JSON failed", MUX_REST_URI_AUDIO);
	}

#if WITH_ANCILLIARY_STREAM
	itemObj = cmnJsonSystemGetSubItem(muxMain->systemJson, MUX_REST_URI_ANC, 0);
	if(!itemObj ||_cmnSysJsonUpdateAnc(&muxMain->runCfg, itemObj))
	{
		MUX_ERROR("Update '%s' JSON failed", MUX_REST_URI_ANC);
	}
#endif

	itemObj = cmnJsonSystemGetSubItem(muxMain->systemJson, MUX_REST_URI_SDP, INVALIDATE_VALUE_U32);
	if(!itemObj ||_cmnSysJsonUpdateSdp(&muxMain->runCfg, itemObj))
	{
		MUX_ERROR("Update '%s' JSON failed", MUX_REST_URI_SDP);
	}

	itemObj = cmnJsonSystemGetSubItem(muxMain->systemJson, MUX_REST_URI_RS232, INVALIDATE_VALUE_U32);
	if(!itemObj ||_cmnSysJsonUpdateRs232(&muxMain->runCfg, itemObj))
	{
		MUX_ERROR("Update '%s' JSON failed", MUX_REST_URI_RS232);
	}

	itemObj = cmnJsonSystemGetSubItem(muxMain->systemJson, MUX_REST_URI_SECURITY, INVALIDATE_VALUE_U32);
	if(!itemObj ||_cmnSysJsonUpdateSecurity(&muxMain->runCfg, itemObj))
	{
		MUX_ERROR("Update '%s' JSON failed", MUX_REST_URI_SECURITY);
	}

	itemObj = cmnJsonSystemGetSubItem(muxMain->systemJson, MUX_REST_URI_IR, INVALIDATE_VALUE_U32);
	if(!itemObj ||_cmnSysJsonUpdateIR(&muxMain->runCfg, itemObj))
	{
		MUX_ERROR("Update '%s' JSON failed", MUX_REST_URI_IR);
	}

	itemObj = cmnJsonSystemGetSubItem(muxMain->systemJson, MUX_REST_URI_OTHERS, INVALIDATE_VALUE_U32);
	if(!itemObj ||_cmnSysJsonUpdateOthers(&muxMain->runCfg, itemObj))
	{
		MUX_ERROR("Update '%s' JSON failed", MUX_REST_URI_OTHERS);
	}
	
	return EXIT_SUCCESS;
}

