
#include <string.h>
#include <sys/stat.h>

#include "libCmnSys.h"
#include "_cmnMux.h"

#include "_sdp.h"

#define	_SFM_DEBUG(sdpClient)	\
	EXT_DEBUGF(SDP_CLIENT_DEBUG, "%s#%d process %s event in state of %s'", (sdpClient)->name,  (sdpClient)->reqs, CMN_FIND_SDPC_EVENT( (sdpClient)->event), CMN_FIND_SDPC_STATE( (sdpClient)->state) )


static int _sdpcClearRequest(struct SDP_CLIENT *sdpClient)
{
	if(sdpClient->socket > 0)
	{
		close(sdpClient->socket);
	}
	
	sdpClient->length = 0;
	sdpClient->data = NULL;
	sdpClient->responseType = HC_REQ_UNKNOWN;
	sdpClient->contentLength = 0;
	
	return EXIT_SUCCESS;
}

static int _sdpcConnect(struct SDP_CLIENT *sdpClient, int timeoutSeconds)
{
	struct sockaddr_in servaddr;
	struct timeval timeout;
	int sock;
	int index = 0;
	uint32_t len = 0;
	
	if(IS_STRING_NULL_OR_ZERO(sdpClient->url.uri) )
	{
		SDPC_MSG(sdpClient, "SDPC request URI is not defined");
		return IPCMD_ERR_FTP_SERVER_ERROR;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock <0 )
	{ 
		SDPC_MSG(sdpClient, "socket creation failed: %m" );
		sdpClient->fails++;
		return EXIT_FAILURE;
	}

	timeout.tv_sec = timeoutSeconds;
	timeout.tv_usec = 0;
	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
	{
		MUX_ERROR("Set socket REV timeout error:%s", strerror(errno));
	}

	if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
	{
		MUX_ERROR("Set socket SEND timeout error:%s", strerror(errno));
	}

	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT 
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = sdpClient->url.ip;//  inet_addr("127.0.0.1");
	servaddr.sin_port = htons(sdpClient->url.port);

	if(connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		SDPC_MSG(sdpClient, "connection with the server %s:%d failed...",  cmnSysNetAddress(sdpClient->url.ip), sdpClient->url.port);
		sdpClient->fails++;
		close(sock);
		return EXIT_FAILURE;
	} 

	index += snprintf(sdpClient->buffer, sizeof(sdpClient->buffer), "GET /%s HTTP/1.0"EXT_NEW_LINE EXT_NEW_LINE, sdpClient->url.uri);
	len = write(sock, (unsigned char *)sdpClient->buffer, index);
	if (len < 0)
	{
		SDPC_MSG(sdpClient, "writing to socket: '%s'", strerror(errno) );
		sdpClient->fails++;
		close(sock);
		return EXIT_FAILURE;
	}

	if(SDP_IS_DEBUG(sdpClient->sdpCtx) )
	{
		SDPC_INFO_MSG(sdpClient, "send requests to http://%s:%d/%s"EXT_NEW_LINE"'%.*s'", 
			cmnSysNetAddress(sdpClient->url.ip), sdpClient->url.port, sdpClient->url.uri, index, sdpClient->buffer );
	}

	sdpClient->socket = sock;
	/* initial receiving buffer */
	sdpClient->length = 0;
	sdpClient->data = NULL;
	sdpClient->contentLength = 0;
	sdpClient->headerLength = 0;
	sdpClient->responseType = HC_REQ_UNKNOWN;
	sdpClient->msgLength = 0;

	SDPC_DEBUG_MSG(sdpClient, "ready on socket=%d", sdpClient->socket );
	return EXIT_SUCCESS;
}


static int _sdpcEventNewReq(void *arg, EVENT *event)
{
	struct SDP_CLIENT *sdpClient = (struct SDP_CLIENT *)arg;
	struct SDP_EVENT *sdpEvent = (struct SDP_EVENT *)event;
	EXT_ASSERT((sdpClient != NULL), "SDP Client can't be null");
	EXT_ASSERT((sdpEvent != NULL), "SDP Event can't be null");
	int timeoutSeconds = 3; /* Poll in 15 seconds or so */
	HttpClientReq *reqUri;
	
	reqUri = (HttpClientReq *)sdpEvent->data;

	if(! SDPC_CHECK_STATE(sdpClient, SDPC_STATE_WAIT) )
	{
		SDPC_MSG(sdpClient, "is in state of '%s', new request is ignored", CMN_FIND_SDPC_STATE(sdpClient->state));
		cmn_free(reqUri);
TRACE();
		return EXT_STATE_CONTINUE;
	}

	memcpy(&sdpClient->url, reqUri, sizeof(HttpClientReq));
	sdpClient->reqs++;
	cmn_free(reqUri);

	_SFM_DEBUG(sdpClient);
	
	if(_sdpcConnect(sdpClient, timeoutSeconds) == EXIT_FAILURE)
	{
		return EXT_STATE_CONTINUE;
	}
	
	return SDPC_STATE_CONNECTED;
}

static int _sdpcEventTimeout(void *arg, EVENT *event)
{
	struct SDP_CLIENT *sdpClient = (struct SDP_CLIENT *)arg;
	EXT_ASSERT((sdpClient != NULL), "SDP Client can't be null");
	
	_SFM_DEBUG(sdpClient);
	_sdpcClearRequest(sdpClient);

	return SDPC_STATE_WAIT;
}

/* tcp err callback or err in TCP recv callback */
static int _sdpcEventError(void *arg, EVENT *event)
{
	struct SDP_CLIENT *sdpClient = (struct SDP_CLIENT *)arg;
	EXT_ASSERT((sdpClient != NULL), "SDP Client can't be null");
	
	_SFM_DEBUG(sdpClient);
	_sdpcClearRequest(sdpClient);

	return SDPC_STATE_WAIT;
}


static int _sdpcEventRecv(void *arg, EVENT *event)
{
	struct SDP_CLIENT *sdpClient = (struct SDP_CLIENT *)arg;
	struct SDP_EVENT *sdpEvent = (struct SDP_EVENT *)event;
	int _ret = EXIT_SUCCESS;
	
	EXT_ASSERT((sdpClient != NULL), "SDP Client can't be null");
	EXT_ASSERT((sdpEvent != NULL), "SDP Event can't be null");

	_SFM_DEBUG(sdpClient);
	if(sdpClient->contentLength > 0)
	{/* contentLength has been parsed, media description may be in the same packet or the next packet */
		if(sdpClient->responseType == HC_REQ_SDP_VIDEO)
		{
			_ret = sdpResponseParse(sdpClient);
		}
		else if(sdpClient->responseType == HC_REQ_JSON )
		{
//			_ret = cmnHttpParseRestJson(rxCfg, hc->data, hc->contentLength);
			SDPC_INFO_MSG(sdpClient, "JSON response for SDP is not support");
		}
		else
		{
		
		}
		
		if(_ret == EXIT_FAILURE)
		{
//			EXT_ERRORF(("Response is not supported type of JSON or SDP"));
			
			sdpClient->dataErrors++;
			SDPC_MSG(sdpClient, "Data error for %s: Error when parsing data", sdpClient->url.uri);
		}
		else
		{
			
//			rxCfg->fpgaAuto = FPGA_CFG_SDP;	/* SDP parameter write to flash, and configure into registers, so it is manual mode for FPGA. Jan.2nd, 2019 JL */
			cmnMuxSystemConfig(&sdpClient->sdpCtx->muxMain->runCfg, &sdpClient->sdpCtx->rxCfg);
			
			SDPC_DEBUG_MSG(sdpClient, "OK in state of %s for request '%s'", CMN_FIND_SDPC_STATE(sdpClient->state), sdpClient->url.uri );
			snprintf(sdpClient->msg, sizeof(sdpClient->msg), "OK");
		}
	}
	
	sdpClient->statusCode = 200;//WEB_RES_REQUEST_OK;
	
	return SDPC_STATE_WAIT;
}

/* in RECV callback, when pbuf is NULL: means closed by peer */
static int _sdpcEventClose(void *arg, EVENT *event)
{
	struct SDP_CLIENT *sdpClient = (struct SDP_CLIENT *)arg;
	EXT_ASSERT((sdpClient != NULL), "SDP Client can't be null");
	
	_SFM_DEBUG(sdpClient);
	_sdpcClearRequest(sdpClient);
	return SDPC_STATE_WAIT;
}


/* 
*  Enter handlers of every state
*/
static void _sdpcEnterStateWait(void *arg)
{
	struct SDP_CLIENT *sdpClient = (struct SDP_CLIENT *)arg;
	EXT_ASSERT((sdpClient != NULL), "SDP Client can't be null");
	
	_sdpcClearRequest(sdpClient);
	
	SDPC_DEBUG_MSG(sdpClient, "enter WAIT state" );
}


/* wait for request */
const transition_t	_sdpcStateWait[] =
{
	{
		.event = SDPC_EVENT_NEW,
		.name = "New",	
		.handle = _sdpcEventNewReq,
	},
	{
		.event = SDPC_EVENT_CLOSE,
		.name = "Close",	
		.handle = _sdpcEventClose,
	}
};


const transition_t	_sdpcStateConnected[] =
{
	{
		.event = SDPC_EVENT_NEW,
		.name = "New",	
		.handle = _sdpcEventNewReq,
	},
	{
		.event = SDPC_EVENT_RECV,	/* parse http OK, begin to SDP parse */
		.name = "Recv",	
		.handle = _sdpcEventRecv,
	},
	
	{
		.event = SDPC_EVENT_TIMEOUT,
		.name = "Timeout",	
		.handle = _sdpcEventTimeout,
	},
	{
		.event = SDPC_EVENT_CLOSE,	/* closed by peer */
		.name = "Close",	
		.handle = _sdpcEventClose,
	},
	{
		.event = SDPC_EVENT_ERROR,	/* parse error, or protocol error */
		.name = "Error",	
		.handle = _sdpcEventError,
	}
};



const statemachine_t	_sdpStateMachine[] =
{
	{
		.state = SDPC_STATE_WAIT,
		.name = "WAIT",	
		.size = sizeof(_sdpcStateWait)/sizeof(transition_t),
		.eventHandlers = (transition_t	*)_sdpcStateWait,
		.enter_handle = _sdpcEnterStateWait
	},
	
	{
		.state = SDPC_STATE_CONNECTED,
		.name = "CONNECTED",	
		.size = sizeof(_sdpcStateConnected)/sizeof(transition_t),
		.eventHandlers = (transition_t	*)_sdpcStateConnected,
		.enter_handle = NULL
	},

	{
		.state = EXT_STATE_CONTINUE,
		.name = NULL,	
		.size = 0,
		.eventHandlers = NULL,
		.enter_handle = NULL
	}
};


static const statemachine_t *_sdpcFsmFindState(const statemachine_t *fsm, int state)
{
	const statemachine_t *_states = fsm;
	
	while(_states->state != EXT_STATE_CONTINUE )
	{
		if( _states->state == state )
		{
			return _states;
		}
		_states++;
	}

	EXT_ASSERT(0, "State %d is not found", state);

	return NULL;

}


void	sdpClientFsmHandle(struct SDP_CLIENT *sdpClient, struct SDP_EVENT *sdpEvent)
{
	unsigned char	i;
	unsigned int newState = EXT_STATE_CONTINUE;

	const statemachine_t *_fsm;
	const statemachine_t *_state = NULL;

	_fsm = sdpClient->fsm;
	EXT_ASSERT((_fsm != NULL), "SDP FSM can't be null in event handler");

	if(sdpEvent->event == EXT_EVENT_NONE || _fsm == NULL)
	{
		SDPC_ERR_MSG(sdpClient, "Invalidate params in state Machine");
		return;
	}

	_state = _sdpcFsmFindState(_fsm, sdpClient->state);

	const transition_t *handle = _state->eventHandlers;
	for(i=0; i < _state->size; i++)
	{
		if(sdpEvent->event == handle->event )
		{
#if SDP_CLIENT_DEBUG		
			if(EXT_DEBUG_HC_IS_ENABLE())
			{
				SDPC_INFO_MSG(sdpClient, " handle event '%s' in state '%s'", 
					CMN_FIND_SDPC_EVENT(sdpEvent->event), CMN_FIND_SDPC_STATE(sdpClient->state));
			}
#endif			

			sdpClient->event = sdpEvent->event;
			newState = (handle->handle)(sdpClient, (EVENT *)sdpEvent );
			//fsm->currentEvent = EXT_EVENT_NONE;

			if(newState!= EXT_STATE_CONTINUE && newState != sdpClient->state )
			{
#if SDP_CLIENT_DEBUG		
				if(EXT_DEBUG_HC_IS_ENABLE())
				{
					SDPC_INFO_MSG(sdpClient, "request from state '%s' enter into state '%s'",
						CMN_FIND_SDPC_STATE(sdpClient->state), CMN_FIND_SDPC_STATE(newState));
				}
#endif

				_state = _sdpcFsmFindState(_fsm, newState);
				if(_state->enter_handle )
				{
					(_state->enter_handle)(sdpClient);
				}

				sdpClient->state = newState;
			}

			return;
		}

		handle++;
	}
	
#if EXT_HTTPC_DEBUG
	SDPC_INFO_MSG(sdpClient, "State Machine no handle for event %s in state %s", CMN_FIND_SDPC_EVENT(sdpEvent->event), CMN_FIND_SDPC_STATE(sdpClient->state));
#endif
	return;
}


