
#ifndef	___SDP_H__
#define	___SDP_H__

#include "cmnFsm.h"

/* section 5 of RFC4566*/
#define	SDP_VERSION		"0"

#define	SDP_USER_NAME		"-"	/*host which not support user ID */

#define	SDP_NET_TYPE		"IN"
#define	SDP_ADDR_TYPE		"IP4"

/* start/stop are 0, session is permanent */
#define	SDP_TIME_START		"0"
#define	SDP_TIME_STOP		"0"

#define	SDP_MEDIA_VIDEO							"m=video"
#define	SDP_MEDIA_AUDIO							"m=audio"

#define	SDP_MEDIA_RTP_MAP							"a=rtpmap:"

#define	SDP_MEDIA_FORMAP_PARAMS					"a=fmtp:"


#define	SDP_MEDIA_TEXT							"text"
#define	SDP_MEDIA_APPLICATION						"application"
#define	SDP_MEDIA_MESSAGE						"message"

#define	SDP_MEDIA_PROTOCOL_AVP					"RTP/AVP"

/* 2110 definitions */
#define	SDP_2110_VIDEO_FORMAT					"raw"
#define	SDP_2110_PM_GENERAL						"2110GPM"	/* general packing mode */
#define	SDP_2110_PM_BLOCK						"2110BPM"	/* general packing mode */

#define	SDP_2110_VIDEO_INTERLACE					"interlace"
#define	SDP_2110_VIDEO_SEGMENTED					"segmented"

#define	SDP_2110_VKEY_SAMPLING					"sampling"
#define	SDP_2110_VKEY_WIDTH						"width"
#define	SDP_2110_VKEY_HEIGHT						"height"
#define	SDP_2110_VKEY_FRAME_RATE					"exactframerate"
#define	SDP_2110_VKEY_DEPTH						"depth"
#define	SDP_2110_VKEY_TCS							"TCS"		/* Transfer Characteritic System */
#define	SDP_2110_VKEY_COLORIMETRY				"colorimetry"

#define	SDP_2110_VKEY_PM							"PM"				/* Packing Mode */
#define	SDP_2110_VKEY_SSN							"SSN"			/* SMPTE Standard Number  */
#define	SDP_2110_VKEY_TP							"TP"  			/*  TP=2110TPN; is not defined in specs */

#define	SDP_2110_ANC_FORMAT						"smpte291"

#define	SDP_VIDEO_DEFAULT_RATE					"90000"

#define	SDP_2110_ANC_VPID_CODE					"VPID_Code"

/* private definitions of SDP */
#define	SDP_P_SESSION_VERSION						1

#define	SDP_P_TTL									64

/* p.21 of AES67-2015 */
#define	SDP_P_AUDIO_DEPTH							"L24"
#define	SDP_P_AUDIO_SAMPLE_RATE					48000
#define	SDP_P_AUDIO_CHANNELS						16

#define	SDP_P_AUDIO_PACK_SIZE						"ptime"

#define	SDP_P_AUDIO_PACK_TIME_125US				"0.125"
#define	SDP_P_AUDIO_PACK_TIME_1MS				"1"


#define		WITH_STATIC_RTP_PAYLOAD_TYPE		0

#define	_CHAR_SEPERATE			"&"
#define	_CHAR_EQUAL				"="


#define	SDP_BUFFER					2048
#define	SDP_CLIENT_NUMBER			3


#define	SDP_RECV_TIMER_TIMEOUT		500 /* ms, must be less than SDPC_POLL_TIMEOUT */
#define	SDPC_POLL_TIMEOUT				1000 /* ms */


struct	SDP_CLIENT;
struct	SDP_CLIENT_CTX;


struct SDP_CLIENT
{
	LIST_ENTRY(SDP_CLIENT)		list;

	char							name[CMN_NAME_LENGTH];
	char							type;
	
	HttpClientReq					url;

	int							event;
	int							state;
	const statemachine_t			*fsm;
	

	cmn_mutex_t					*lock;

	int							socket;
	int							timerFd;

	char							buffer[SDP_BUFFER];	/* contains all package for header+data */
	int							length;	/* total length of data received */
	char							*data;	/* pointer to the data part of response buffer */
	int							contentLength;	/* data length, parsed from header */
	int							headerLength;

	int							responseType;							

	int							statusCode;
	char							msg[1024];
	int							msgLength;

	/* statistics*/
	int							reqs;
	int							fails;			/* wrong before http response */
	int							headerErrors;	/* HTTP header wrong */
	int							dataErrors;		/* data in HTTP wrong */
	int							pkts;	/* number of independent paavkets */

	struct	SDP_CLIENT_CTX		*sdpCtx;

	void 						*priv;
};


struct	SDP_CLIENT_CTX
{
	LIST_HEAD(D_SDP_REQ, SDP_CLIENT)		clients;
	
	int										socket;
	char										buffer[2048];
	int										length;

	int										totalReqs;
	int										totalSuccess;
	int										totalFailed;

	EXT_RUNTIME_CFG							rxCfg;

	cmn_sem_t								*semaphore;
	cmn_mutex_t 								*lock;
	void										*timer;
	MuxMain									*muxMain;

	char										msg[1024];
	int										msgLength;

	void										*priv;	/*save MuxPlayerConfig **/
};


#define	SDP_IS_DEBUG(sdpCtx)	\
			MUX_MAIN_IS_DEBUG_SDP( (sdpCtx)->muxMain)



struct SDP_EVENT
{
	int						event;
	struct	SDP_CLIENT		*client;

	void						*data;	/* can be different for different events */
};



#define	SDPC_MSG(sdpClient, frmat, ...)		\
	{MUX_ERROR( "%s#%d "frmat, sdpClient->name, sdpClient->reqs, ##__VA_ARGS__); \
		sdpcErrorMsg(sdpClient, frmat, ##__VA_ARGS__);  }

//		 sdpcErrorMsg(sdpClient, frmat, __VA_ARGS__); }

//##__VA_ARGS__
//

//	 sdpcErrorMsg(sdpClient, "%s#%d " frmat, sdpClient->name, sdpClient->reqs, ##message); }

#define	SDPC_ERR_MSG(sdpClient, frmat, message...)		\
	MUX_ERROR( "%s#%d "frmat, sdpClient->name, sdpClient->reqs, ##message)


#define	SDPC_INFO_MSG(sdpClient, frmat, message...)		\
			MUX_INFO("%s#%d "frmat, sdpClient->name, sdpClient->reqs, ##message)


#if SDP_CLIENT_DEBUG
#define	_SDP_DEBUG(frmat, message...)	\
			EXT_DEBUGF(SDP_CLIENT_DEBUG, ""frmat, ##message )
			
#else
#define	_SDP_DEBUG(frmat, message...)						{}

#endif

#define	SDPC_DEBUG_MSG(sdpClient, frmat, message...)		\
			_SDP_DEBUG( "%s#%d "frmat, sdpClient->name, sdpClient->reqs, ##message)

#define	_SFM_DEBUG(sdpClient)	\
			SDPC_DEBUG_MSG(sdpClient, "process %s event in state of %s'", CMN_FIND_SDPC_EVENT( (sdpClient)->event), CMN_FIND_SDPC_STATE( (sdpClient)->state) )



#define	EXT_HTTP_CRLF							"\r\n"


#define	SDPC_CHECK_STATE(sdpClient, _state)	\
	( (sdpClient)->state == (_state) )


#define	SDPC_SEND_EVENT(sdpClient, event)	\
	{ 	close(sdpClient->socket); sdpClient->socket = -1; \
		cmnMuxSdpAddEvent((sdpClient), (event), NULL ); }


extern	const statemachine_t	_sdpStateMachine[];

void	sdpClientFsmHandle(struct SDP_CLIENT *sdpClient, struct SDP_EVENT *sdpEvent);

int cmnMuxSdpAddEvent(struct SDP_CLIENT *sdpClient, int eventType, void *data);

int cmnMuxSdpTimerCallback(int interval, void *param);

int sdpResponseParse(struct SDP_CLIENT *req);

int sdpcErrorMsg(struct SDP_CLIENT *sdpClient, const char *format, ...);

#endif

