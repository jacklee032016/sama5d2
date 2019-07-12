
#include "libCmn.h"
#include "libMux.h"

#include <sys/stat.h>

#include "_cmnMux.h"
#include "libCmnSys.h"

#include "_sdp.h"


/* first field for this media stream */
static int _sdpParsePort(char *data, uint32_t size, const char *mediaKey, uint16_t *port)
{
	char *p;
	
	p= strnstr(data, mediaKey, size);
	if(p == NULL)
	{
		return 0;
	}
	p += strlen(mediaKey);
	while(*p == ' ')
	{
		p++;
	}

	if(cmnUtilsParseInt16(p, port) == EXIT_FAILURE)
	{
		EXT_ERRORF("No port number for SDP %s stream", mediaKey);
		*port = -1;
		return p-data;
	}

	return p-data;
}

static int _sdpParseIp(char *data, uint32_t size, uint32_t *ip)
{
	int index = 0;
	char *p, *pnext;
	
	p = strnstr(data+index, SDP_ADDR_TYPE, size - index);
	if(p== NULL)
	{
		*ip = IPADDR_NONE;	
		EXT_ERRORF("Invalidate format '"SDP_ADDR_TYPE"' for SDP stream");
		return EXIT_FAILURE;
	}
	
	p += strlen(SDP_ADDR_TYPE);
	while(*p == ' ')
	{
		p++;
	}
	index= p -data;
	
	pnext = strnstr(data+index, "/", size - index);
	if(pnext== NULL)
	{
		*ip = IPADDR_NONE;	
		EXT_ERRORF("Invalidate format '"SDP_ADDR_TYPE"' for SDP stream");
		return EXIT_FAILURE;
	}
	*pnext = '\0';

	if(cmnUtilsParseIp(p, ip) == EXIT_FAILURE)
	{
		*ip = IPADDR_NONE;	
		EXT_ERRORF("No IP address for SDP stream");
//		return EXIT_FAILURE;
	}
	p = pnext+1;

	return p - data;
}


static int _sdpParseAudioStream(struct SDP_CLIENT *sdpClient, EXT_RUNTIME_CFG *rxCfg, char *data, uint32_t size)
{
	uint32_t  index = 0;
	uint16_t _shVal;
	char *p, *pnext;//, _chVal;
	
	p = strnstr(data+index, SDP_MEDIA_RTP_MAP, size - index);
	if(p== NULL)
	{
		EXT_ERRORF("Invalidate format '"SDP_ADDR_TYPE"' for SDP stream");
		return EXIT_FAILURE;
	}
	if(cmnUtilsParseInt8(p+ strlen(SDP_MEDIA_RTP_MAP), &rxCfg->runtime.rtpTypeAudio) == EXIT_FAILURE)
	{
		EXT_ERRORF("No rtpmap for SDP audio stream");
		rxCfg->runtime.rtpTypeAudio = SDP_P_MEDIA_FORMAT_AUDIO;
		return EXIT_FAILURE;
	}
//	EXT_DEBUGF(EXT_DBG_OFF, ("Audio RTP Payload : '%d'", rxCfg->runtime.rtpTypeAudio ));
	
	index= p -data;

	/* audio depth */
	p = strnstr(data+index, "L", size - index);
	if(p== NULL)
	{
		rxCfg->runtime.aDepth = INVALIDATE_VALUE_U8;
		EXT_ERRORF("Invalidate format '"SDP_ADDR_TYPE"' for SDP stream");
		return EXIT_FAILURE;
	}
	p++;
	
	index = p -data;
	pnext = strnstr(data+index, "/", size - index);
	if(pnext== NULL)
	{
		rxCfg->runtime.aDepth = INVALIDATE_VALUE_U8;
		EXT_ERRORF("Invalidate format '"SDP_ADDR_TYPE"' for SDP stream");
		return EXIT_FAILURE;
	}
	*pnext = '\0';

	if(cmnUtilsParseInt8(p, &rxCfg->runtime.aDepth) == EXIT_FAILURE)
	{
		EXT_ERRORF("No depth for SDP audio stream");
		rxCfg->runtime.aDepth = INVALIDATE_VALUE_U8;
		return EXIT_FAILURE;
	}

	/* sample freq */
	p = pnext+1;
	index = p -data;
	pnext = strnstr(data+index, "/", size - index);
	if(pnext== NULL)
	{
		rxCfg->runtime.aSampleRate = INVALIDATE_VALUE_U8;
		EXT_ERRORF("Invalidate format for SDP audio stream");
		return EXIT_FAILURE;
	}
	*pnext = '\0';

	if(cmnUtilsParseInt16(p, &_shVal) == EXIT_FAILURE)
	{
		EXT_ERRORF("No Sample Freq for SDP audio stream" );
		rxCfg->runtime.aSampleRate = EXT_A_RATE_48K;
		return p-data;
	}
	else
	{
		if(_shVal == 48000)
		{
			rxCfg->runtime.aSampleRate = EXT_A_RATE_48K;
		}
		else if(_shVal == 44100)
		{
			rxCfg->runtime.aSampleRate = EXT_A_RATE_44K;
		}
		else if(_shVal == 96000)
		{
			rxCfg->runtime.aSampleRate = EXT_A_RATE_96K;
		}
		else 
		{
			EXT_ERRORF("Not support Sample Freq for SDP audio stream '%d'", _shVal );
			rxCfg->runtime.aSampleRate = EXT_A_RATE_48K;
		}
	}

	/* Audio Channels */
	p = pnext+1;
	if(cmnUtilsParseInt8(p, &rxCfg->runtime.aChannels) == EXIT_FAILURE)
	{
		EXT_ERRORF("Invalidate channel number for SDP audio stream");
		rxCfg->runtime.aChannels = INVALIDATE_VALUE_U8;
		return p-data;
	}

	/* Audio packet size */
	index = p - data;
	pnext = strnstr(data+index, SDP_P_AUDIO_PACK_SIZE, size - index);
	if(pnext== NULL)
	{
		rxCfg->runtime.aSampleRate = INVALIDATE_VALUE_U8;
		EXT_ERRORF("No packet size in SDP audio stream: '%s'", data+index);
		return EXIT_FAILURE;
	}

	pnext += strlen(SDP_P_AUDIO_PACK_SIZE)+1;
	
	if(strnstr(pnext, "0.1", 3) )
	{/* page 21, AES67-2015 specs*/
		rxCfg->runtime.aPktSize = EXT_A_PKT_SIZE_125US;
	}
	else if( pnext[0] == '1' )
	{
		rxCfg->runtime.aPktSize = EXT_A_PKT_SIZE_1MS;
	}
	else
	{
		rxCfg->runtime.aPktSize = EXT_A_PKT_SIZE_1MS;
		EXT_ERRORF("Not support audio packet size '%s'", pnext);
	}

	if(EXT_DEBUG_HC_IS_ENABLE())
	{
		printf("\tParsed SDP Audio params:IP:%s; Port:%d; Depth:%d; Sample Freq:%d; Channels:%d; PktSize:%d"EXT_NEW_LINE,
			cmnSysNetAddress(rxCfg->dest.audioIp), rxCfg->dest.aport, rxCfg->runtime.aDepth, rxCfg->runtime.aSampleRate, rxCfg->runtime.aChannels, rxCfg->runtime.aPktSize );
	}
	
	return EXIT_SUCCESS;
}


/* data from HTTP form for SDP client */
static char __sdpVideoParams(EXT_RUNTIME_CFG *rxCfg, char *key, char *value)
{
//	uint8_t colorspace;
	/* string change into integer */
	uint8_t _chVal;

	if( strnstr(key, SDP_2110_VKEY_SAMPLING, strlen(key)))
	{
		rxCfg->runtime.vColorSpace = CMN_FIND_STR_V_COLORSPACE(value);
		if(rxCfg->runtime.vColorSpace == INVALIDATE_VALUE_U8)
		{
			EXT_ERRORF("Not support colorspace '%s' for SDP video stream", value);
			return EXIT_FAILURE;
		}
	}
	else if( strnstr(key, SDP_2110_VKEY_FRAME_RATE, strlen(key)))
	{
		rxCfg->runtime.vFrameRate = CMN_FIND_STR_V_FRAME_RATE(value);
		if(rxCfg->runtime.vFrameRate == INVALIDATE_VALUE_U8)
		{
			EXT_ERRORF("Not support frame rate '%s' for SDP video stream", value);
			return EXIT_FAILURE;
		}
	}

	/* integer directly */
	else if( strnstr(key, SDP_2110_VKEY_WIDTH, strlen(key)))
	{
		if(cmnUtilsParseInt16(value, &rxCfg->runtime.vWidth) == EXIT_FAILURE)
		{
			EXT_ERRORF("No validate width for SDP video stream");
			return EXIT_FAILURE;
		}
	}
	else if(strnstr(key, SDP_2110_VKEY_HEIGHT, strlen(key)))
	{
		if(cmnUtilsParseInt16(value, &rxCfg->runtime.vHeight) == EXIT_FAILURE)
		{
			EXT_ERRORF("No validate height for SDP video stream");
			return EXIT_FAILURE;
		}
	}
	else if( strnstr(key, SDP_2110_VKEY_DEPTH, strlen(key)))
	{
		if(cmnUtilsParseInt8(value, &_chVal) == EXIT_FAILURE)
		{
			EXT_ERRORF("No validate depth for SDP video stream");
			return EXIT_FAILURE;
		}
		rxCfg->runtime.vDepth = CMN_INT_FIND_TYPE_V_DEPTH(_chVal);
	}
	else
	{
//		EXT_DEBUGF(EXT_DBG_ON, ("Key '%s' and value '%s' is not support now", key,  value));
	}

	return EXIT_SUCCESS;
}


static uint32_t __parseSdpKeyValue(EXT_RUNTIME_CFG *rxCfg, char *data, uint32_t size)
{
#define	_SDP_CHAR_SEPERATE			"; "

	char *key, *value, *nextKey;
	int i = 0;
	uint32_t left = size;

	data[left] = 0;
	key = data;
	while(key)
	{
		value = strnstr(key, _CHAR_EQUAL, left );
		if(value == NULL)
		{/* interlace and segmented is not in the format of 'key'='value' */
//			EXT_DEBUGF(EXT_DBG_OFF, ("Invalidate key/value (%s) for SDP stream, on position %d", key, size-left));
			return size - left;
		}
		key[value-key] = 0;
		value++;
		
		nextKey = strnstr(value, _SDP_CHAR_SEPERATE,  left - (value-key)) ;
		if(nextKey)
		{
			value[nextKey-value] = 0;
		}
		else
		{
			value[left-(value-key)] = 0;
		}
		
		if(EXT_DEBUG_HC_IS_ENABLE())
		{
			printf("\t\tNo#%d: '%s' = '%s'" EXT_NEW_LINE, ++i, key, value );
		}

		if(__sdpVideoParams(rxCfg, key, value) == EXIT_FAILURE)
		{
			EXT_ERRORF("Invalidate key/value '%s/%s' for SDP video stream", key, value);
			return size - left;
		}
			
		if(nextKey)
		{
			nextKey++;
			nextKey++;
			left = left - (nextKey-key);
		}
		
		key = nextKey;
	}


	return size - left;
}

static int _sdpParseVideoStream(struct SDP_CLIENT *sdpClient, EXT_RUNTIME_CFG	*rxCfg, char *data, uint32_t size)
{
	uint16_t  index = 0;
	uint16_t length = 0, left;
	char *p, *pnext;
	
	p = strnstr(data+index, SDP_MEDIA_FORMAP_PARAMS, size - index);
	if(p== NULL)
	{
		SDPC_MSG(sdpClient, "Invalidate format for SDP stream");
		return EXIT_FAILURE;
	}

	p += strlen(SDP_MEDIA_FORMAP_PARAMS);
	
	if(cmnUtilsParseInt8(p, &rxCfg->runtime.rtpTypeVideo) == EXIT_FAILURE)
	{
		SDPC_MSG(sdpClient, "No rtpmap for SDP video stream: '%s'", p);
		rxCfg->runtime.rtpTypeVideo = SDP_P_MEDIA_FORMAT_VIDEO;
		return EXIT_FAILURE;
	}
//	EXT_DEBUGF(EXT_DBG_OFF, ("Video RTP Payload : '%d'", rxCfg->runtime.rtpTypeVideo));

	while(*p != ' ')/* for rtpmap constants */
	{
		p++;
	}
	while(*p == ' ')/* spaces before first key */
	{
		p++;
	}
	
	index= p -data;
	
	pnext = strnstr(data+index, "\r", size - index);
	length = pnext - p + 1;

	left = __parseSdpKeyValue(rxCfg, data+index, length);
#if 1	
//	printf("left:%d\r\n", left);
	if(left > 0)
	{
		p = strnstr(data+index+left, SDP_2110_VIDEO_INTERLACE, (length-left) );
		if( p )
		{
			rxCfg->runtime.vIsInterlaced = EXT_VIDEO_INTLC_INTERLACED;
			p = strnstr(data+index+left, SDP_2110_VIDEO_SEGMENTED, (length-left) );
			if(p )
			{
				rxCfg->runtime.vIsSegmented = EXT_VIDEO_INTLC_B_PROGRESSIVE;
			}
		}
	}
#endif

	if(rxCfg->runtime.vIsInterlaced == EXT_VIDEO_INTLC_INTERLACED && rxCfg->runtime.vIsSegmented == EXT_VIDEO_INTLC_B_PROGRESSIVE)
	{
		rxCfg->runtime.vIsInterlaced = EXT_VIDEO_INTLC_B_PROGRESSIVE;
	}
	else if(rxCfg->runtime.vIsInterlaced == INVALIDATE_VALUE_U8 )
	{
		rxCfg->runtime.vIsInterlaced = EXT_VIDEO_INTLC_A_PROGRESSIVE;
	}


	if(EXT_DEBUG_HC_IS_ENABLE())
	{
		printf("\tParsed SDP Video params:IP:%s; Port:%d; ColorSpace:%s; width:%d; height:%d; framerate:%s; depth:%d; isInterlace:%d;"EXT_NEW_LINE,
			cmnSysNetAddress(rxCfg->dest.ip), rxCfg->dest.vport, CMN_FIND_V_COLORSPACE(rxCfg->runtime.vColorSpace),  
			rxCfg->runtime.vWidth, rxCfg->runtime.vHeight, CMN_FIND_V_FRAME_RATE(rxCfg->runtime.vFrameRate), rxCfg->runtime.vDepth, rxCfg->runtime.vIsInterlaced);
	}
	
	return EXIT_SUCCESS;
}


static int _sdpParseAncStream(struct SDP_CLIENT *sdpClient, EXT_RUNTIME_CFG	*rxCfg, char *data, uint32_t size)
{
	char *p;
	
	p = strnstr(data, SDP_2110_ANC_VPID_CODE, size );
	if(p== NULL)
	{
		SDPC_MSG(sdpClient, "Invalidate format '"SDP_2110_ANC_VPID_CODE"' for SDP ANC stream");
		return EXIT_FAILURE;
	}
	if(cmnUtilsParseInt8(p+ strlen(SDP_2110_ANC_VPID_CODE)+1, &rxCfg->runtime.vpid) == EXIT_FAILURE)
	{
		SDPC_MSG(sdpClient, "No VPID_CODE for SDP audio stream");
		rxCfg->runtime.vpid = SDP_P_MEDIA_VP_ID;
		return EXIT_FAILURE;
	}
//	EXT_DEBUGF(EXT_DBG_OFF, ("Audio RTP Payload : '%d'", rxCfg->runtime.rtpTypeAudio ));
	
	if(EXT_DEBUG_HC_IS_ENABLE())
	{
		printf("\tParsed SDP ANC params:IP:%s; Port:%d; PDID:%d"EXT_NEW_LINE,
			cmnSysNetAddress(rxCfg->dest.ancIp), rxCfg->dest.dport, rxCfg->runtime.vpid);
	}
	
	return EXIT_SUCCESS;
}


int sdpResponseParse(struct SDP_CLIENT *sdpClient)
{
#define	_SDP_TYPE_UNKNOWN		0
#define	_SDP_TYPE_VIDEO			1
#define	_SDP_TYPE_AUDIO			2
#define	_SDP_TYPE_ANC				3

	char *data = sdpClient->data;
	uint32_t size = sdpClient->contentLength;
//	struct SDP_CLIENT_CTX *sdpCtx = sdpClient->sdpCtx;
	EXT_RUNTIME_CFG	*rxCfg = &sdpClient->sdpCtx->rxCfg;

	int index = 0;
	uint16_t 	port;
	uint32_t 	ip;
	char type = _SDP_TYPE_UNKNOWN;
	int err = EXIT_SUCCESS;
	char* p;

	SDPC_DEBUG_MSG(sdpClient, "parsing %d byte SDP data "EXT_NEW_LINE"'%.*s'", size, size, data);
	
	extSysClearConfig(rxCfg);
	
	index = _sdpParsePort(data, size, SDP_MEDIA_VIDEO, &port);
	if(index == 0 || port == -1)
	{
		index = _sdpParsePort(data, size, SDP_MEDIA_AUDIO, &port);
		if(index != 0 && port != -1)
		{
			type = _SDP_TYPE_AUDIO;
		}
	}
	else
	{
		type = _SDP_TYPE_VIDEO;
		p= strnstr(data, SDP_2110_ANC_FORMAT, size);
		if(p != NULL)
		{
			type = _SDP_TYPE_ANC;
		}
	}
	
	if(type == _SDP_TYPE_UNKNOWN )
	{
		SDPC_MSG(sdpClient, "SDPC %s#%d: No media stream is found", (sdpClient)->name,  (sdpClient)->reqs);
		return EXIT_FAILURE;
	}

	index += _sdpParseIp(data+index, size-index, &ip);
	if(ip == IPADDR_NONE)
	{
		SDPC_MSG(sdpClient, "SDPC %s#%d: No IP address for SDP %s stream", (sdpClient)->name,  (sdpClient)->reqs, (type==_SDP_TYPE_AUDIO)?"audio":"video");
		return EXIT_FAILURE;
	}

	if(type == _SDP_TYPE_AUDIO)
	{
		rxCfg->dest.audioIp = ip;
		rxCfg->dest.aport = port;
		err = _sdpParseAudioStream(sdpClient, rxCfg, data+index, size-index);
	}
	else if(type == _SDP_TYPE_VIDEO)
	{
		rxCfg->dest.ip = ip;
		rxCfg->dest.vport = port;
		err = _sdpParseVideoStream(sdpClient, rxCfg, data+index, size-index);
	}
	else
	{
		rxCfg->dest.ancIp = ip;
		rxCfg->dest.dport = port;
		err = _sdpParseAncStream(sdpClient, rxCfg, data+index, size-index);
	}

	return err;
}


