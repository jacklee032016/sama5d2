
#include "libCmnSys.h"
#include "_cmnMux.h"


void cmnMuxClearConfig(EXT_RUNTIME_CFG *rxCfg)
{
	memset(rxCfg, 0, sizeof(EXT_RUNTIME_CFG));

	FIELD_INVALIDATE_U8(rxCfg->isMCast);
	FIELD_INVALIDATE_U8(rxCfg->netMode);
	FIELD_INVALIDATE_U8(rxCfg->fpgaAuto);

	FIELD_INVALIDATE_U8(rxCfg->sfpCfg);
	FIELD_INVALIDATE_U8(rxCfg->isConvert);

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


	FIELD_INVALIDATE_U8(rxCfg->runtime.rtpTypeVideo);
	FIELD_INVALIDATE_U8(rxCfg->runtime.rtpTypeAudio);
	FIELD_INVALIDATE_U8(rxCfg->runtime.rtpTypeAnc);
	
	rxCfg->sdpUriVideo.ip = INVALIDATE_VALUE_U32;
	FIELD_INVALIDATE_U16(rxCfg->sdpUriVideo.port);

	rxCfg->sdpUriAudio.ip = INVALIDATE_VALUE_U32;
	FIELD_INVALIDATE_U16(rxCfg->sdpUriAudio.port);

	rxCfg->sdpUriAnc.ip = INVALIDATE_VALUE_U32;
	FIELD_INVALIDATE_U16(rxCfg->sdpUriAnc.port);
	
	rxCfg->local.ip = INVALIDATE_VALUE_U32;

	rxCfg->ipGateway = INVALIDATE_VALUE_U32;
	rxCfg->ipMask = INVALIDATE_VALUE_U32;
	rxCfg->dest.ip = INVALIDATE_VALUE_U32;
	
	rxCfg->dest.audioIp = INVALIDATE_VALUE_U32;
	rxCfg->dest.ancIp = INVALIDATE_VALUE_U32;
#if EXT_FPGA_AUX_ON	
	rxCfg->dest.auxIp = INVALIDATE_VALUE_U32;
	FIELD_INVALIDATE_U16(rxCfg->dest.sport);
#endif

	FIELD_INVALIDATE_U16(rxCfg->dest.vport);
	FIELD_INVALIDATE_U16(rxCfg->dest.aport);
	FIELD_INVALIDATE_U16(rxCfg->dest.dport);

	FIELD_INVALIDATE_U8(rxCfg->setupData.isFeedBack);
	FIELD_INVALIDATE_U16(rxCfg->setupData.waitMs );

}


int	cmnMuxObjectParseSystem(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char *value;
	int	intValue;
	EXT_MAC_ADDRESS macAddress;
	int isFound = EXT_FALSE;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;

	value = cmnGetStrFromJsonObject(dataObj, FIELD_SYS_CFG_ADDRESS);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->local.ip = cmnSystemNetIp(value);
		if(rxCfg->local.ip == INVALIDATE_VALUE_U32)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_SYS_CFG_ADDRESS, MUX_REST_URI_SYSTEM);
			return EXIT_FAILURE;
		}
		isFound = EXT_TRUE;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_SYS_CFG_NETMASK);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->ipMask = cmnSystemNetIp(value);
		if(rxCfg->ipMask == INVALIDATE_VALUE_U32)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_SYS_CFG_NETMASK, MUX_REST_URI_SYSTEM);
			return EXIT_FAILURE;
		}
		isFound = EXT_TRUE;
	}
	
	value = cmnGetStrFromJsonObject(dataObj, FIELD_SYS_CFG_GATEWAY);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->ipGateway = cmnSystemNetIp(value);
		if(rxCfg->ipGateway == INVALIDATE_VALUE_U32)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_SYS_CFG_GATEWAY, MUX_REST_URI_SYSTEM);
			return EXIT_FAILURE;
		}
		isFound = EXT_TRUE;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_SYS_CFG_MAC);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		if(extMacAddressParse(&macAddress, value) == EXIT_FAILURE)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_SYS_CFG_MAC, MUX_REST_URI_SYSTEM);
			return EXIT_FAILURE;
		}
		memcpy(&rxCfg->local.mac, &macAddress, sizeof(EXT_MAC_ADDRESS));
		isFound = EXT_TRUE;
	}


	value = cmnGetStrFromJsonObject(dataObj, FIELD_SYS_CFG_NAME);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		snprintf(rxCfg->name, sizeof(rxCfg->name), "%s", value);
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_IS_DHCP);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->netMode = (intValue != 0)?1: 0;
		isFound = EXT_TRUE;
	}

#if EXT_DIP_SWITCH_ON
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_DIP);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->isDipOn = (intValue != 0)?1: 0;
		isFound = EXT_TRUE;
	}
#endif

	/* system parameters for FPGA */
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_SFP_CFG);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		if(intValue < EXT_SFP_CFG_FIRST ||intValue > EXT_SFP_CFG_SPLIT )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "value '%d' Field '%s' in %s item is wrong", intValue, FIELD_SYS_CFG_SFP_CFG, MUX_REST_URI_SYSTEM);
			return EXIT_FAILURE;
		}

		rxCfg->sfpCfg = intValue;
		isFound = EXT_TRUE;
	}
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_IS_CONVERT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->isConvert = (intValue != 0)?1: 0;
		isFound = EXT_TRUE;
	}



	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_RESET);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.reset = (intValue != 0)?1: 0;
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_REBOOT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.reboot = (intValue != 0)?1: 0;
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_BLINK);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.blink = (intValue != 0)?1: 0;
		isFound = EXT_TRUE;
	}

TRACE();
	if(!EXT_IS_TX(&muxMain->runCfg) )
	{
		value = cmnGetStrFromJsonObject(dataObj, FIELD_SYS_CFG_MEDIA_AUTO);
		if(!IS_STRING_NULL_OR_ZERO(value) )
		{
			rxCfg->fpgaAuto = CMN_FIND_STR_MEDIA_MODE(value);
			if(rxCfg->fpgaAuto == -1 )
			{
				DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_SYS_CFG_MEDIA_AUTO, MUX_REST_URI_SYSTEM);
				return EXIT_FAILURE;
			}
			isFound = EXT_TRUE;
		}
	}

	if(isFound == EXT_FALSE )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No validated field is found in '%s' packet", MUX_REST_URI_SYSTEM );
		return EXT_TRUE;
	}

	MUX_DEBUG("'%s' command OK!", MUX_REST_URI_SYSTEM);
	return EXIT_SUCCESS;
}


int	cmnMuxObjectParseVideo(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char *value;
	int	intValue;
	int isFound = EXT_FALSE;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;

	value = cmnGetStrFromJsonObject(dataObj, FIELD_VIDEO_IP);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->dest.ip = cmnSystemNetIp(value);
		if(rxCfg->dest.ip == INVALIDATE_VALUE_U32)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_VIDEO_IP, MUX_REST_URI_VIDEO);
			return EXIT_FAILURE;
		}
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_PORT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		if(PORT_IS_INVALIDATE(intValue) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong:%d(1024~65535)", FIELD_ANC_PORT, MUX_REST_URI_VIDEO, intValue );
			return EXIT_FAILURE;
		}
		rxCfg->dest.vport = (unsigned short) intValue;
		isFound = EXT_TRUE;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_VIDEO_SDP);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		snprintf(rxCfg->sdpUriVideo.uri, sizeof(rxCfg->sdpUriVideo.uri), "%s", value);
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_WIDTH);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.vWidth = (unsigned short)intValue;
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_HEIGHT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.vHeight = (unsigned short)intValue;
		isFound = EXT_TRUE;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_VIDEO_FRAME_RATE);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		intValue = CMN_FIND_STR_V_FPS_4_REST(value);
		if(FIELD_IS_INVLIDATE_U32(intValue) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong: '%s'", FIELD_VIDEO_FRAME_RATE, MUX_REST_URI_VIDEO, value);
			return EXIT_FAILURE;
		}
		rxCfg->runtime.vFrameRate = (unsigned char)intValue;
		isFound = EXT_TRUE;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_VIDEO_COLOR_SPACE);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		intValue = CMN_FIND_STR_V_COLORSPACE(value);
		if(FIELD_IS_INVLIDATE_U32(intValue) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong: '%s'", FIELD_VIDEO_COLOR_SPACE, MUX_REST_URI_VIDEO, value);
			return EXIT_FAILURE;
		}
		rxCfg->runtime.vColorSpace = (unsigned char)intValue;
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_DEPTH);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
#if 1	
		rxCfg->runtime.vDepth = (unsigned char)intValue;
#else
		rxCfg->runtime.vDepth = CMN_INT_FIND_TYPE_V_DEPTH(intValue);;
		if(FIELD_IS_INVLIDATE_U32(rxCfg->runtime.vDepth) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong: '%s'", FIELD_VIDEO_DEPTH, MUX_REST_URI_VIDEO, value);
			return EXIT_FAILURE;
		}
#endif		
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_INTERLACE);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.vIsInterlaced = (unsigned char)intValue;
		isFound = EXT_TRUE;
	}

	if(isFound == EXT_FALSE )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No validated field is found in '%s' packet", MUX_REST_URI_VIDEO );
		return EXT_TRUE;
	}

	MUX_DEBUG("'%s' command OK!", MUX_REST_URI_VIDEO);
	return EXIT_SUCCESS;
}



int	cmnMuxObjectParseAudio(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char *value;
	int	intValue;
	int isFound = EXT_FALSE;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;

	value = cmnGetStrFromJsonObject(dataObj, FIELD_AUDIO_IP);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->dest.audioIp = cmnSystemNetIp(value);
		if(rxCfg->dest.audioIp == INVALIDATE_VALUE_U32)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_AUDIO_IP, MUX_REST_URI_AUDIO);
			return EXIT_FAILURE;
		}
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_AUDIO_PORT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		if(PORT_IS_INVALIDATE(intValue) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong:%d(1024~65535)", FIELD_ANC_PORT, MUX_REST_URI_AUDIO, intValue );
			return EXIT_FAILURE;
		}
		rxCfg->dest.aport = (unsigned short)intValue;
		isFound = EXT_TRUE;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_AUDIO_SDP);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		snprintf(rxCfg->sdpUriAudio.uri, sizeof(rxCfg->sdpUriAudio.uri), "%s", value);
		isFound = EXT_TRUE;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_AUDIO_SAMPLE);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		intValue = CMN_FIND_STR_A_RATE(value );
		if(FIELD_IS_INVLIDATE_U32(intValue) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong:'%s'", FIELD_AUDIO_SAMPLE, MUX_REST_URI_AUDIO, value);
			return EXIT_FAILURE;
		}
		rxCfg->runtime.aSampleRate = (unsigned char )intValue;
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_AUDIO_CHANNELS);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.aChannels = (unsigned char)intValue;
		isFound = EXT_TRUE;
	}
	
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_AUDIO_DEPTH);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.aDepth = (unsigned char)intValue;
		isFound = EXT_TRUE;
	}
	
	value = cmnGetStrFromJsonObject(dataObj, FIELD_AUDIO_PKT_SIZE);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		intValue = CMN_FIND_STR_A_RATE(value );
		if(FIELD_IS_INVLIDATE_U32(intValue) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong:'%s'", FIELD_AUDIO_PKT_SIZE, MUX_REST_URI_AUDIO, value);
			return EXIT_FAILURE;
		}
		rxCfg->runtime.aPktSize = (unsigned char)intValue;
		isFound = EXT_TRUE;
	}

	if(isFound == EXT_FALSE )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No validated field is found in '%s' packet", MUX_REST_URI_AUDIO );
		return EXT_TRUE;
	}

	MUX_DEBUG("'%s' command OK!", MUX_REST_URI_AUDIO);
	return EXIT_SUCCESS;
}



int	cmnMuxObjectParseAnc(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char *value;
	int	intValue;
	int isFound = EXT_FALSE;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;

	value = cmnGetStrFromJsonObject(dataObj, FIELD_ANC_IP);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->dest.ancIp = cmnSystemNetIp(value);
		if(rxCfg->dest.ancIp == INVALIDATE_VALUE_U32)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_ANC_IP, MUX_REST_URI_ANC);
			return EXIT_FAILURE;
		}
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_ANC_PORT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
//		MUX_DEBUG("port: %d", intValue);
		if(PORT_IS_INVALIDATE(intValue) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong:%d(1024~65535)", FIELD_ANC_PORT, MUX_REST_URI_ANC, intValue );
			return EXIT_FAILURE;
		}
		rxCfg->dest.dport = (unsigned short)intValue;
		isFound = EXT_TRUE;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_ANC_SDP);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		snprintf(rxCfg->sdpUriAnc.uri, sizeof(rxCfg->sdpUriAnc.uri), "%s", value);
		isFound = EXT_TRUE;
	}

	if(isFound == EXT_FALSE )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No validated field is found in '%s' packet", MUX_REST_URI_ANC );
		return EXT_TRUE;
	}

	MUX_DEBUG("'%s' command OK!", MUX_REST_URI_ANC);
	return EXIT_SUCCESS;
}

static int	_objectParseOneSdp(struct DATA_CONN *dataConn, cJSON *dataObj, HttpClientReq *sdpUrl)
{
	char *value;
	int	intValue;
	int isFound = EXT_FALSE;
//	MuxMain *muxMain = SYS_MAIN(dataConn);

	value = cmnGetStrFromJsonObject(dataObj, FIELD_SDP_IP);
	if(! IS_STRING_NULL_OR_ZERO(value) )
	{
		sdpUrl->ip = cmnSystemNetIp(value);
		if(sdpUrl->ip == INVALIDATE_VALUE_U32)
		{/* when IP address is in param, and it's invalidate format */
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_SDP_IP, MUX_REST_URI_SDP);
			return EXIT_FAILURE;
		}
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SDP_PORT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		if(PORT_IS_INVALIDATE_SERVER(intValue) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong:%d(1024~65535)", FIELD_ANC_PORT, MUX_REST_URI_VIDEO, intValue );
			return EXIT_FAILURE;
		}
		sdpUrl->port = (unsigned short)intValue;
		isFound = EXT_TRUE;
	}

	if(cmnJsonGetStrIntoString(dataObj, FIELD_SDP_URI, sdpUrl->uri, sizeof(sdpUrl->uri)) == EXIT_SUCCESS)
	{
		isFound = EXT_TRUE;
	}

	if(isFound == EXT_FALSE)
	{
//		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_SDP_URI, MUX_REST_URI_SDP);
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No validated field is found in '%s' packet", MUX_REST_URI_SDP );
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}



int	cmnMuxObjectParseSdp(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	cJSON *obj = NULL;
	char *value;
	int index, count;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;

	if(!cJSON_IsArray(dataObj) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "SDP item %s must be JSON array", MUX_REST_URI_SDP);
		return EXIT_FAILURE;
	}
	
	count = cJSON_GetArraySize(dataObj);
//	MUX_DEBUG("Count : %d", count);
	for(index = 0; index< count; index++)
	{
		HttpClientReq *sdpUrl = NULL;
		obj = cJSON_GetArrayItem(dataObj, index);
		if(obj == NULL)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_SERVER_INTERNEL_ERROR, "No SDP item is in index#%d", index);
			return EXIT_FAILURE;
		}
		
		value = cmnGetStrFromJsonObject(obj, FIELD_SDP_MEDIA);

		if(! IS_STRING_NULL_OR_ZERO(value) )
		{
			if( IS_STRING_EQUAL(value, MUX_REST_URI_VIDEO) )
			{
				sdpUrl = &rxCfg->sdpUriVideo;
			}
			else if( IS_STRING_EQUAL(value, MUX_REST_URI_AUDIO) )
			{
				sdpUrl = &rxCfg->sdpUriAudio;
			}
			else if( IS_STRING_EQUAL(value, MUX_REST_URI_ANC) )
			{
				sdpUrl = &rxCfg->sdpUriAnc;
			}

			if(sdpUrl)
			{
				if(_objectParseOneSdp(dataConn, obj, sdpUrl) == EXIT_FAILURE)
				{
					return EXIT_FAILURE;
				}
			}
			else
			{
				DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Index#%d Field '%s':%s in %s item is wrong", index, FIELD_SDP_MEDIA, value, MUX_REST_URI_SDP);
				return EXIT_FAILURE;
			}
			
		}
		else
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Index#%d Field '%s' is not found in %s item", index, FIELD_SDP_MEDIA, MUX_REST_URI_SDP);
			return EXIT_FAILURE;
		}
TRACE();
		
	}

	MUX_DEBUG("'%s' command OK!", MUX_REST_URI_SDP);
	return EXIT_SUCCESS;
}


int	cmnMuxObjectParseRs232(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char *value;
	int	intValue;
	int	isFound = 0;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_RS232_BAUDRATE);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		if( CHECK_BAUDRATE(intValue) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is wrong", FIELD_RS232_BAUDRATE, intValue, MUX_REST_URI_RS232);
			return EXIT_FAILURE;
		}
		else
		{
			rxCfg->rs232Cfg.baudRate = intValue;
		}
		isFound = 1;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_RS232_DATABITS);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		if( CHECK_DATABITS(intValue) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is wrong", FIELD_RS232_DATABITS, intValue, MUX_REST_URI_RS232);
			return EXIT_FAILURE;
		}
		else
		{
			rxCfg->rs232Cfg.charLength = intValue;
		}
		isFound = 1;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_RS232_PARITY);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		intValue = CMN_FIND_STR_RS_PARITY(value );
		if(intValue == INVALIDATE_VALUE_U32)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s':%s in %s item is wrong", FIELD_RS232_PARITY, value, MUX_REST_URI_RS232);
			return EXIT_FAILURE;
		}
		
		rxCfg->rs232Cfg.parityType = (unsigned char )intValue;
		isFound = 1;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_RS232_STOPBITS);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		if(intValue == 1 || intValue == 2 )
		{
			rxCfg->rs232Cfg.stopbits = intValue;
		}
		else 
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is wrong", FIELD_RS232_STOPBITS, intValue, MUX_REST_URI_RS232);
			return EXIT_FAILURE;
		}
		isFound = 1;
	}


	if(cmnMuxObjectParseHexaData(dataConn, dataObj, MUX_REST_URI_RS232, (isFound==0)?EXT_TRUE:EXT_FALSE) == EXIT_SUCCESS)
	{
		cmnMuxSendRsData(dataConn, dataObj);
	}

	/* go on for RS232 configuration */
	
	return EXIT_SUCCESS;
}

int	cmnMuxObjectParseIR(struct DATA_CONN *dataConn, cJSON *dataObj)
{
//	MuxMain *muxMain = SYS_MAIN(dataConn);

	if(cmnMuxObjectParseHexaData(dataConn, dataObj, MUX_REST_URI_IR, EXT_TRUE) == EXIT_SUCCESS)
	{
		cmnMuxSendIRData(dataConn, dataObj);
		return EXIT_SUCCESS;
	}
		
	return EXIT_FAILURE;
}

static int _securityCheckId(struct DATA_CONN *dataConn)
{
	int i;
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;
	SC_CTRL		*sc = &muxMain->scf->sc;
	cJSON *resultObj;


	/* get ID here */
	if(0)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "sub-command '%s' EXEC failed", FIELD_SECURITY_GET_ID);
		return EXIT_FAILURE;
	}
	else
	{
		memset(rxCfg->setupData.scID, 0, sizeof(rxCfg->setupData.scID));
		for(i=0; i< SC_ROM_ID_SIZE; i++)
		{
//			extBcd2Ascii(rxCfg->sc.romId[i], rxCfg->setupData.scID+i*2);
			extBcd2Ascii(sc->romId[i], rxCfg->setupData.scID+i*2);
		}	
	}

	resultObj = cJSON_CreateObject();
	JEVENT_ADD_STRING(resultObj, FIELD_SECURITY_GET_ID, rxCfg->setupData.scID );
	REPLY_DATACONN_OK(dataConn, resultObj);

	return EXIT_SUCCESS;
}


int cmnMuxObjectParseSecurity(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	int ret = EXIT_FAILURE;
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;
//	cJSON *resultObj;

	if(! IS_SECURITY_CHIP_EXIST( muxMain) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_SERVER_INTERNEL_ERROR, "No Security Chip is found in this unit");
		return EXIT_FAILURE;
	}

#if 0
	value = cmnGetStrFromJsonObject(dataObj, FIELD_SECURITY_SET_KEY);
	if(! IS_STRING_NULL_OR_ZERO(value) )
#else
	if(cmnJsonGetStrIntoString(dataObj, FIELD_SECURITY_SET_KEY, rxCfg->setupData.scKey, sizeof(rxCfg->setupData.scKey)) == EXIT_SUCCESS)
#endif		
	{ /* for set_key */

		int size = 0;
		unsigned char secret[SC_PAGE_SIZE];
		
		memset(secret, 0xFF, SC_SECRET_SIZE);
//		snprintf(rxCfg->setupData.scKey, sizeof(rxCfg->setupData.scKey), "%s", value);
		
		if(strlen(rxCfg->setupData.scKey)%2 != 0)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "'%s' error: Key size '%d' is not even number", FIELD_SECURITY_SET_KEY, strlen(rxCfg->setupData.scKey) );
			return EXIT_FAILURE;
		}
		else
		{
			unsigned int i;
			for(i=0; i< strlen(rxCfg->setupData.scKey)/2; i++)
			{
				ret = extSysAtoInt8(rxCfg->setupData.scKey+i*2, secret+i);
				if(ret ==  EXIT_FAILURE)
				{
					DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "'%s' error: #%d letters '%.*s' is not an integer", FIELD_SECURITY_SET_KEY, i*2, 2, rxCfg->setupData.scKey+i*2);
					return EXIT_FAILURE;
				}
				
				size++;

#if 0
				if(parser->runCfg->sc->readMac[i] != parser->runCfg->sc->secret[i] )
				{
					EXT_ERRORF("Key #%d %02x != %02x (board)", i, parser->runCfg->sc->readMac[i], parser->runCfg->sc->secret[i]);
					snprintf(parser->msg, sizeof(parser->msg), "'%s' error: key is not correct", EXT_IPCMD_SC_GET_STATUS);
				}
#endif				
			}

			if(size != SC_PAGE_SIZE)
			{
				DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "'%s' error: Size of parsed secret must be %d: %d", FIELD_SECURITY_SET_KEY, SC_PAGE_SIZE, size );
				return EXIT_FAILURE;
			}
		}

		if( ret == EXIT_SUCCESS)
		{
			ret = cmnSysScWriteKey(muxMain->scf, secret, size);
		}

		if( ret == EXIT_SUCCESS)
		{
			REPLAY_OK_WITHOUT_PARAM_IN_IP_CMD(dataConn);
		}
		else
		{
//			snprintf(parser->msg, sizeof(parser->msg), "'%s' error on hardware", EXT_IPCMD_SC_GET_STATUS);
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "sub-command '%s' failed: %s", FIELD_SECURITY_SET_KEY, (size==SC_SECRET_SIZE)?"Hardware error":"secret is not correct");
		}
		
		MUX_DEBUG("'%s' command OK!", FIELD_SECURITY_SET_KEY);
		return EXIT_SUCCESS;
	}


	/* check read-only field, no matter the type is string or integer */
	if(cJSON_HasObjectItem(dataObj, FIELD_SECURITY_GET_STATUS) )	
	{/* for get_status */
		
		if( cmnSysScValidate(muxMain->scf ) == EXIT_SUCCESS )
		{
			REPLAY_OK_WITHOUT_PARAM_IN_IP_CMD(dataConn);
			MUX_DEBUG("'%s' command OK!", FIELD_SECURITY_GET_STATUS);
		}
		else
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "sub-command '%s' EXEC failed", FIELD_SECURITY_GET_STATUS);
		}

		return EXIT_SUCCESS;
	}

	if(! cJSON_HasObjectItem(dataObj, FIELD_SECURITY_GET_ID) )	
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No validate sub-command is found for '%s'", MUX_REST_URI_SECURITY);
		return EXIT_FAILURE;
	}

	/* default, get_id */
	if( _securityCheckId(dataConn) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	MUX_DEBUG("'%s' command OK!", FIELD_SECURITY_GET_ID);
	return EXIT_SUCCESS;
	
}


int	cmnMuxObjectParsePtp(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char *value;
	int	intValue;
	int	isFound = 0;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;
	MuxPtpRuntime *ptpRuntime = &rxCfg->ptpRuntime;

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_PTP_IS_ENABLE);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		ptpRuntime->isEnable = (intValue==0)?0:1;
		isFound = 1;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_PTP_IS_SLAVE_ONLY);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		ptpRuntime->isSlaveOnly = (intValue==0)?0:1;
		isFound = 1;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_PTP_DOMAIN_NUM);
	if( FIELD_IS_CHANGED_U32(intValue)  )
	{
		if( intValue< 0 || intValue > 127 )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is out of range (0~127)", FIELD_PTP_DOMAIN_NUM, intValue, MUX_REST_URI_PTP);
			return EXIT_FAILURE;
		}
		ptpRuntime->domainCfg =(unsigned char ) intValue;
		isFound = 1;
	}
	
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_PTP_PRIORITY_1);
	if( FIELD_IS_CHANGED_U32(intValue)  )
	{
		if( intValue< 0 || intValue > 255 )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is out of range (0~127)", FIELD_PTP_PRIORITY_1, intValue, MUX_REST_URI_PTP);
			return EXIT_FAILURE;
		}
		ptpRuntime->priority1 = (unsigned char )intValue;
		isFound = 1;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_PTP_PRIORITY_2);
	if( FIELD_IS_CHANGED_U32(intValue)  )
	{
		if( intValue< 0 || intValue > 255 )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is out of range (0~127)", FIELD_PTP_PRIORITY_2, intValue, MUX_REST_URI_PTP);
			return EXIT_FAILURE;
		}
		ptpRuntime->priority2 = (unsigned char )intValue;
		isFound = 1;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_PTP_CLOCK_CLASS);
	if( FIELD_IS_CHANGED_U32(intValue)  )
	{
		if( intValue< 0 || intValue > 255 )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is out of range (0~127)", FIELD_PTP_CLOCK_CLASS, intValue, MUX_REST_URI_PTP);
			return EXIT_FAILURE;
		}
		ptpRuntime->clockClass= (unsigned char )intValue;
		isFound = 1;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_PTP_CLOCK_ACCURACY);
	if( FIELD_IS_CHANGED_U32(intValue)  )
	{
		if( intValue< 0 || intValue > 255 )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is out of range (0~127)", FIELD_PTP_CLOCK_ACCURACY, intValue, MUX_REST_URI_PTP);
			return EXIT_FAILURE;
		}
		ptpRuntime->clockAccuracy= (unsigned char )intValue;
		isFound = 1;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_PTP_OFFSET_LOG);
	if( FIELD_IS_CHANGED_U32(intValue)  )
	{
		if( intValue< 0 || intValue > 65535 )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is out of range (0~127)", FIELD_PTP_OFFSET_LOG, intValue, MUX_REST_URI_PTP);
			return EXIT_FAILURE;
		}
		ptpRuntime->offsetScaledLogVariance = (unsigned short )intValue;
		isFound = 1;
	}

	return EXIT_SUCCESS;
}


int cmnMuxObjectParseOthers(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	int	intValue;
	int isFound = EXT_FALSE;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_OTHERS_AUTHEN);
	if( FIELD_IS_CHANGED_U32(intValue) && intValue != muxMain->isAuthen )
	{
		muxMain->isAuthen = intValue;
		MUX_MAIN_INFO(muxMain, "'%s' changed as %s", FIELD_OTHERS_AUTHEN, (muxMain->isAuthen)?"YES":"NO" );
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_OTHERS_DEBUG_REST);
	if( FIELD_IS_CHANGED_U32(intValue) && intValue != muxMain->debugRest)
	{
		muxMain->debugRest = intValue;
		MUX_MAIN_INFO(muxMain, "'%s' changed as %s", FIELD_OTHERS_DEBUG_REST, (muxMain->debugRest)?"YES":"NO" );
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_OTHERS_DEBUG_IP_CMD);
	if( FIELD_IS_CHANGED_U32(intValue) && intValue != muxMain->debugCmd )
	{
		muxMain->debugCmd = intValue;
		MUX_MAIN_INFO(muxMain, "'%s' changed as %s", FIELD_OTHERS_DEBUG_IP_CMD, (muxMain->debugCmd)?"YES":"NO" );
		isFound = EXT_TRUE;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_OTHERS_DEBUG_SDP);
	if( FIELD_IS_CHANGED_U32(intValue) && intValue != muxMain->debugSdp )
	{
		muxMain->debugSdp = intValue;
		MUX_MAIN_INFO(muxMain, "'%s' changed as %s", FIELD_OTHERS_DEBUG_SDP, (muxMain->debugSdp)?"YES":"NO" );
		isFound = EXT_TRUE;
	}

	if(isFound == EXT_FALSE )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No validated field is found in '%s' packet", MUX_REST_URI_OTHERS );
		isFound = EXT_TRUE;
	}

	MUX_DEBUG("'%s' command OK!", MUX_REST_URI_OTHERS);
	return EXIT_SUCCESS;	
}




static cJSON *_addOneThread(char *name, int pId)
{
	cJSON *obj = cJSON_CreateObject();
				
	JEVENT_ADD_STRING(obj, "name", name);
	JEVENT_ADD_INTEGER(obj, "PID", pId );

	return obj;
}

cJSON *cmnMuxThreadsInfo(MuxMain *muxMain)
{
	MuxThread *muxThread = muxMain->threads;
	cJSON *resultObj = cJSON_CreateArray();

	/* threads not start yet */
	if(!muxThread )
	{
		return resultObj;
	}
#if 0	
	cJSON_AddItemToArray(resultObj, cJSON_CreateString(CMN_THREAD_NAME_MAIN) );
	cJSON_AddItemToArray(resultObj, cJSON_CreateString(CMN_THREAD_NAME_TIMER) );
	while(muxThread)
	{
//		MUX_DEBUG("No.%d thread '%s' is joined", ++i, muxThread->thread->name );
		cJSON_AddItemToArray(resultObj, cJSON_CreateString(muxThread->thread->name) );
		muxThread = muxThread->next;
	}
#else
	cJSON_AddItemToArray(resultObj, _addOneThread(CMN_THREAD_NAME_MAIN, cmnSysGetPidByName(CMN_THREAD_NAME_MAIN) ) );
	cJSON_AddItemToArray(resultObj, _addOneThread(CMN_THREAD_NAME_TIMER, cmn_timer_id() ));
	while(muxThread)
	{
//		MUX_DEBUG("No.%d thread '%s' is joined", ++i, muxThread->thread->name );
		cJSON_AddItemToArray(resultObj, _addOneThread(muxThread->thread->name, muxThread->thread->pId) );
		muxThread = muxThread->next;
	}

#endif

	return resultObj;
}


