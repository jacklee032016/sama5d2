
#include "libCmn.h"
#include "libMux.h"

#include "_cmnMux.h"

#include "libCmnSys.h"

void cmnMuxClearConfig(EXT_RUNTIME_CFG *rxCfg)
{
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
	}


	value = cmnGetStrFromJsonObject(dataObj, FIELD_SYS_CFG_NAME);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		snprintf(rxCfg->name, sizeof(rxCfg->name), "%s", value);
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_IS_DHCP);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->netMode = (intValue != 0)?1: 0;
	}

#if EXT_DIP_SWITCH_ON
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_DIP);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->isDipOn = (intValue != 0)?1: 0;
	}
#endif

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_RESET);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.reset = (intValue != 0)?1: 0;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_REBOOT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.reboot = (intValue != 0)?1: 0;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SYS_CFG_BLINK);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.blink = (intValue != 0)?1: 0;
	}

	return EXIT_SUCCESS;
}


int	cmnMuxObjectParseVideo(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char *value;
	int	intValue;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;

TRACE();
	value = cmnGetStrFromJsonObject(dataObj, FIELD_VIDEO_IP);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->dest.ip = cmnSystemNetIp(value);
		if(rxCfg->dest.ip == INVALIDATE_VALUE_U32)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_VIDEO_IP, MUX_REST_URI_VIDEO);
			return EXIT_FAILURE;
		}
	}

TRACE();
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_PORT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
TRACE();
		rxCfg->dest.vport = intValue;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_VIDEO_SDP);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		snprintf(rxCfg->sdpUriVideo.uri, sizeof(rxCfg->sdpUriVideo.uri), "%s", value);
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_WIDTH);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.vWidth = intValue;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_HEIGHT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.vHeight = intValue;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_VIDEO_FRAME_RATE);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->runtime.vFrameRate = CMN_FIND_STR_V_FPS_4_REST(value);
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_VIDEO_COLOR_SPACE);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->runtime.vColorSpace = CMN_FIND_STR_V_COLORSPACE(value);
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_DEPTH);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.vDepth = CMN_INT_FIND_TYPE_V_DEPTH(intValue);;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_VIDEO_INTERLACE);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.vIsInterlaced = intValue;
	}

	return EXIT_SUCCESS;
}



int	cmnMuxObjectParseAudio(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char *value;
	int	intValue;
	
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
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_AUDIO_PORT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->dest.aport = intValue;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_AUDIO_SDP);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		snprintf(rxCfg->sdpUriAudio.uri, sizeof(rxCfg->sdpUriAudio.uri), "%s", value);
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_AUDIO_SAMPLE);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->runtime.aSampleRate = CMN_FIND_STR_A_RATE(value );
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_AUDIO_CHANNELS);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.aChannels = intValue;
	}
	
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_AUDIO_DEPTH);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->runtime.aDepth = intValue;
	}
	
	value = cmnGetStrFromJsonObject(dataObj, FIELD_AUDIO_PKT_SIZE);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		rxCfg->runtime.aPktSize = CMN_FIND_STR_A_PKTSIZE(value);
	}

	return EXIT_SUCCESS;
}



int	cmnMuxObjectParseAnc(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char *value;
	int	intValue;
	
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
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_ANC_PORT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->dest.dport = intValue;
	}

	value = cmnGetStrFromJsonObject(dataObj, FIELD_ANC_SDP);
	if(!IS_STRING_NULL_OR_ZERO(value) )
	{
		snprintf(rxCfg->sdpUriAnc.uri, sizeof(rxCfg->sdpUriAnc.uri), "%s", value);
	}

	return EXIT_SUCCESS;
}

static int	_objectParseOneSdp(struct DATA_CONN *dataConn, cJSON *dataObj, HttpClientReq *sdpUrl)
{
	char *value;
	int	intValue;
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
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SDP_PORT);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		sdpUrl->port = intValue;
	}

	if(cmnJsonGetStrIntoString(dataObj, FIELD_SDP_URI, sdpUrl->uri, sizeof(sdpUrl->uri)) == EXIT_FAILURE)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is wrong", FIELD_SDP_URI, MUX_REST_URI_SDP);
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

TRACE();
	if(!cJSON_IsArray(dataObj) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "SDP item %s must be JSON array", MUX_REST_URI_SDP);
		return EXIT_FAILURE;
	}
	
	count = cJSON_GetArraySize(dataObj);
	MUX_DEBUG("Count : %d", count);
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

TRACE();
	return EXIT_SUCCESS;
}


int	cmnMuxObjectParseRs232(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char *value;
	int	intValue;
	
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
	}


	if(cmnJsonGetStrIntoString(dataObj, FIELD_RS232_DATA, rxCfg->setupData.hexData, sizeof(rxCfg->setupData.hexData)) == EXIT_FAILURE)
	{
//		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is not found", FIELD_RS232_DATA, MUX_REST_URI_RS232);
//		return EXIT_FAILURE;
	}
	
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_RS232_IS_FEEDBACK);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->setupData.isFeedBack = (intValue)?1:0;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_RS232_WAIT_TIME);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		if(intValue < 0)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is wrong", FIELD_RS232_WAIT_TIME, intValue, MUX_REST_URI_RS232);
			return EXIT_FAILURE;
		}
		rxCfg->setupData.waitMs = intValue;
	}

	return EXIT_SUCCESS;
}

int	cmnMuxObjectParseIR(struct DATA_CONN *dataConn, cJSON *dataObj)
{
//	char *value;
	int	intValue;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;

	if(cmnJsonGetStrIntoString(dataObj, FIELD_RS232_DATA, rxCfg->setupData.hexData, sizeof(rxCfg->setupData.hexData)) == EXIT_FAILURE)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' in %s item is not found", FIELD_RS232_DATA, MUX_REST_URI_IR);
		return EXIT_FAILURE;
	}
	
	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_RS232_IS_FEEDBACK);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		rxCfg->setupData.isFeedBack = (intValue)?1:0;
	}

	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_RS232_WAIT_TIME);
	if( FIELD_IS_CHANGED_U32(intValue) )
	{
		if(intValue < 0)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s': %d in %s item is wrong", FIELD_RS232_WAIT_TIME, intValue, MUX_REST_URI_IR);
			return EXIT_FAILURE;
		}
		rxCfg->setupData.waitMs = intValue;
	}

	return EXIT_SUCCESS;
}

static int _securityCheckId(struct DATA_CONN *dataConn)
{
//	int index = 0;
//	char *data = NULL;
//	int size = 0;
	int i;
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;

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
			extBcd2Ascii(rxCfg->sc.romId[i], rxCfg->setupData.scID+i*2);
		}	
	}

#if 0
	parser->status = JSON_STATUS_OK;

	extIpCmdResponseHeaderPrint( parser);
	data = parser->outBuffer+IPCMD_HEADER_LENGTH + parser->outIndex;
	size = parser->outSize - IPCMD_HEADER_LENGTH - parser->outIndex;
	
	index += snprintf(data+index, size-index, ",\""EXT_IPCMD_DATA_ARRAY"\":[{");
	index += snprintf(data+index, size-index, "\""EXT_IPCMD_SC_GET_ID"\":\"%s\"", parser->setupData.scID );
	index += snprintf(data+index, size-index, "}]" );

	parser->outIndex += index;

	extIpCmdResponseTailCalculate(parser, EXT_FALSE);
#endif

	return EXIT_SUCCESS;
}


int cmnMuxObjectParseSecurity(struct DATA_CONN *dataConn, cJSON *dataObj)
{
	char ret;
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG *rxCfg = &muxMain->rxCfg;
//	int intValue;
//	char *value;

#if 0
	extIpCmdSendMediaData(parser, EXT_FALSE);
#endif

	if(! IS_SECURITY_CHIP_EXIST( &muxMain->runCfg.sc) )
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
		
		memset(rxCfg->sc.readMac, 0xFF, SC_SECRET_SIZE);
//		snprintf(rxCfg->setupData.scKey, sizeof(rxCfg->setupData.scKey), "%s", value);
		
		if(strlen(rxCfg->setupData.scKey)%2 != 0)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "'%s' error: Key size '%d' is not even number", FIELD_SECURITY_GET_STATUS, strlen(rxCfg->setupData.scKey) );
			return EXIT_FAILURE;
		}
		else
		{
			unsigned int i;
//			for(i=0; i< SC_SECRET_SIZE; i++)
			for(i=0; i< strlen(rxCfg->setupData.scKey)/2; i++)
			{
				ret = extSysAtoInt8(rxCfg->setupData.scKey+i*2, muxMain->runCfg.sc.readMac+i);
				if(ret ==  EXIT_FAILURE)
				{
					DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "'%s' error: #%d letters '%.*s' is not an integer", FIELD_SECURITY_GET_STATUS, i*2, 2, rxCfg->setupData.scKey+i*2);
					return EXIT_FAILURE;
				}

#if 0
				if(parser->runCfg->sc->readMac[i] != parser->runCfg->sc->secret[i] )
				{
					EXT_ERRORF("Key #%d %02x != %02x (board)", i, parser->runCfg->sc->readMac[i], parser->runCfg->sc->secret[i]);
					snprintf(parser->msg, sizeof(parser->msg), "'%s' error: key is not correct", EXT_IPCMD_SC_GET_STATUS);
				}
#endif				
			}
		}

#if 0
		if( ret == EXIT_SUCCESS)
		{
			ret = bspScWriteKey(muxMain->runCfg->sc, muxMain->runCfg->sc->readMac);
		}

		if( ret == EXIT_SUCCESS)
		{
			muxMain->status = JSON_STATUS_OK;
		}
		else
		{
			muxMain->status = JSON_STATUS_CMD_EXEC_ERROR;
//			snprintf(parser->msg, sizeof(parser->msg), "'%s' error on hardware", EXT_IPCMD_SC_GET_STATUS);
		}
#endif
		
		MUX_DEBUG("'%s' command OK!", FIELD_SECURITY_SET_KEY);
//		return extIpCmdResponseReply(muxMain);
		return EXIT_SUCCESS;
	}


#if 1
//	intValue = cmnGetIntegerFromJsonObject(dataObj, FIELD_SECURITY_GET_STATUS);
//	if( FIELD_IS_CHANGED_U32(intValue) )

	/* check read-only field, no matter the type is string or integer */
	if(cJSON_HasObjectItem(dataObj, FIELD_SECURITY_GET_STATUS) )	
#else
	if(cmnGetStrIntoString(dataObj, FIELD_SECURITY_GET_STATUS, rxCfg->setupData.scKey, sizeof(rxCfg->setupData.scKey) )  == EXIT_SUCCESS )
#endif		
	{/* for get_status */
		
#if 0
		if( bspScCheckMAC(muxMain->runCfg->sc ) == EXIT_SUCCESS )
		{
			parser->status = JSON_STATUS_OK;
		}
		else
		{
			parser->status = JSON_STATUS_CMD_EXEC_ERROR;
			snprintf(parser->msg, sizeof(parser->msg), "'%s' error on hardware", EXT_IPCMD_SC_GET_STATUS);
		}
#endif

		MUX_DEBUG("'%s' command OK!", FIELD_SECURITY_GET_STATUS);
//		return extIpCmdResponseReply(parser);
		return EXIT_SUCCESS;
	}

TRACE();
//	if(cmnGetStrIntoString(dataObj, FIELD_SECURITY_GET_ID, rxCfg->setupData.scKey, sizeof(rxCfg->setupData.scKey) ) == EXIT_FAILURE)
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

	MUX_DEBUG("'%s' command OK!", MUX_REST_URI_SYSTEM);
	return EXIT_SUCCESS;
	
}


int cmnMuxObjectParseOthers(struct DATA_CONN *dataConn, cJSON *dataObj)
{
#if 0
	char *value;
	int	intValue;
	
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
	}
#endif

	MUX_DEBUG("'%s' command OK!", MUX_REST_URI_OTHERS);
	return EXIT_SUCCESS;
	
}

