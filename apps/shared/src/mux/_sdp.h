
#ifndef	___SDP_H__
#define	___SDP_H__

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

typedef	enum
{
	SDPC_EVENT_NEW = EXT_EVENT_NONE +1,		/* new req */
	SDPC_EVENT_CONNECTED,	/* TCP callback: connected */
	SDPC_EVENT_RECV,		/* TCP callback: recv */
	SDPC_EVENT_POLL,			/* TCP callback: POLL */
	SDPC_EVENT_SENT,			/* TCP callback: SENT */

	SDPC_EVENT_CLOSE,
	SDPC_EVENT_TIMEOUT,

	SDPC_EVENT_ERROR,
	
	SDPC_EVENT_MAX,
}SDPC_EVENT_T;


typedef enum
{
	SDPC_STATE_WAIT = EXT_STATE_CONTINUE +1,
	SDPC_STATE_INIT,	
	SDPC_STATE_CONN,
	SDPC_STATE_DATA, 
	SDPC_STATE_ERROR,		/* error event from TCP stack implementation  */
	SDPC_STATE_MAX
}SDPC_STATE_T;



#define	SDP_BUFFER					2048
#define	SDP_CLIENT_NUMBER			3


#define	SDP_RECV_TIMER_TIMEOUT		500 /* ms, must be less than POLL_TIMEOUT */


struct	SDP_REQ;
struct	SDP_CLIENT_CTX;

struct SDP_REQ
{
	LIST_ENTRY(_SDP_REQ)		list;

	char							name[CMN_NAME_LENGTH];
	char							type;
	
	HttpClientReq					url;

	ext_fsm_t					fsm;
	

	cmn_mutex_t					*lock;

	int							event;
	int							state;

	int							socket;
	int							timerFd;

	char							buffer[SDP_BUFFER];
	int							length;

	int							contentLength;

	int							statusCode;
	char							msg[128];

	/* statistics*/
	int							reqs;
	int							fails;

	struct	SDP_CLIENT_CTX		*sdpCtx;

	void 						*priv;
};


struct	SDP_CLIENT_CTX
{
//	SDP_REQ			sdpClients[SDP_CLIENT_NUMBER];

//	SDP_REQ			*currentReq;
	LIST_HEAD(D_SDP_REQ, _SDP_REQ)		reqs;
	
	int				socket;
	char				buffer[2048];
	int				length;

	int				totalReqs;
	int				totalSuccess;
	int				totalFailed;

	cmn_sem_t		*semaphore;
	cmn_mutex_t 		*lock;
	void				*timer;
	MuxMain			*muxMain;

	char				msg[1024];
	int				msgLength;

	void				*priv;	/*save MuxPlayerConfig **/
};


struct SDP_EVENT
{
	int					event;
	struct	SDP_REQ		*req;

	void					*data;	/* can be different for different events */
};

#define	SDP_ERR_MSG(sdpCtx, frmat, message...)		\
	{EXT_ERRORF( frmat, ##message); \
	 _sdpErrorMsg(sdpCtx, frmat, ##message); }

#define	EXT_DEBUG_HC_IS_ENABLE()			1		


#endif

