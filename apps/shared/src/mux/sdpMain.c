
#include "libCmn.h"
#include "libMux.h"

#include <sys/stat.h>

#include "_cmnMux.h"
#include "libCmnSys.h"

#include <string.h>

#include "_sdp.h"

static struct SDP_CLIENT_CTX _sdpCtx;

static int _sdpErrorMsg(struct SDP_CLIENT_CTX *sdpCtx, const char *format, ...)
{
	char		msg[256];
	int length = 0;

	va_list ap;
	va_start(ap, format);
	snprintf(msg, sizeof(msg), format, ap);
	va_end(ap);

	length = strlen(msg);

	if(length + sdpCtx->msgLength > sizeof(sdpCtx->msg) )
	{
		length = snprintf(sdpCtx->msg, sizeof(sdpCtx->msg), "%s", msg);
		sdpCtx->msgLength = length;
	}
	else
	{
		length = snprintf(sdpCtx->msg+sdpCtx->msgLength, sizeof(sdpCtx->msg)-sdpCtx->msgLength, "%s", msg);
		sdpCtx->msgLength += length;
	}

	return length;
}

// 



static int _sdpClientConnect(struct SDP_CLIENT_CTX *sdpCtx, int timeoutSeconds)
{
	struct sockaddr_in servaddr;
	struct timeval timeout;
	int sock;
	
	if(sdpCtx->currentReq == NULL)
	{
		MUX_ERROR("SDPC request URI is not defined");
		return IPCMD_ERR_FTP_SERVER_ERROR;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock <0 )
	{ 
		MUX_ERROR("SDPC socket creation failed: %m");
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
	servaddr.sin_addr.s_addr = sdpCtx->currentReq->req.ip;//  inet_addr("127.0.0.1");
	servaddr.sin_port = htons(sdpCtx->currentReq->req.port);

	if(connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		MUX_ERROR("SDPC connection with the server %s:%d failed...", cmnSysNetAddress(sdpCtx->currentReq->req.ip), sdpCtx->currentReq->req.port);
		close(sock);
		return EXIT_FAILURE;
	} 

	sdpCtx->socket = sock;

	return EXIT_SUCCESS;
}

static int _sdpClientGetResponse(struct SDP_CLIENT_CTX *sdpCtx)
{
	int index = 0, len;

	index += snprintf(sdpCtx->buffer, sizeof(sdpCtx->buffer), "GET /%s HTTP/1.0"EXT_NEW_LINE EXT_NEW_LINE, sdpCtx->currentReq->req.uri);
//	CMN_SN_PRINTF(sdpCtx->buffer, sizeof(sdpCtx->buffer), index, "GET /%s HTTP/1.0"EXT_NEW_LINE EXT_NEW_LINE, sdpCtx->currentReq->req.uri);
	
	len = write(sdpCtx->socket, (unsigned char *)sdpCtx->buffer, index);
	if (len < 0)
	{
		MUX_ERROR("SDPC writing to socket: '%s'", strerror(errno));
		goto _return;
	}

	len = read(sdpCtx->socket, (unsigned char *)sdpCtx->buffer, sizeof(sdpCtx->buffer) );
	if (len < 0)
	{
		MUX_ERROR("SDPC read from socket: '%s'", strerror(errno));
		sdpCtx->length = 0;
	}
	else
	{
		EXT_DEBUGF(EXT_DBG_ON, "SDPC received %d bytes data:'%.*s'", len, len, (char *)sdpCtx->buffer);
		sdpCtx->length = len;
	}

_return:
	close(sdpCtx->socket);
	
	return len;
}


static int	_sdpEventHandler(struct _CmnThread *th, void *_event)
{
	struct	SDP_CLIENT_CTX *sdpCtx = (struct	SDP_CLIENT_CTX *)th->data;
	int timeoutSecond = 3; /* Poll in 15 seconds or so */
	int res = EXIT_SUCCESS;

	SDP_REQ *req = (SDP_REQ *)_event;
	EXT_ASSERT((req != NULL), "SDP Event can not be null");

	sdpCtx->currentReq = req;

	if(_sdpClientConnect(sdpCtx, timeoutSecond) == EXIT_FAILURE)
	{
		SDP_ERR_MSG(sdpCtx, "SDPC Connect to %s:%d failed", cmnSysNetAddress(req->req.ip), req->req.port );
	}
	else
	{
		if(_sdpClientGetResponse(sdpCtx) == EXIT_FAILURE) 
		{
			SDP_ERR_MSG(sdpCtx, "SDPC Request %s failed", req->req.uri);
		}
		else
		{
			if(_sdpClientParse(sdpCtx) == EXIT_FAILURE)
			{
				SDP_ERR_MSG(sdpCtx, "SDPC parse repsonse from %s failed", req->req.uri);
			}
			else
			{
				EXT_DEBUGF(EXT_DBG_ON, "SDPC submit %s to Manager...", req->req.uri );
			}
		}
	}

	cmn_free(req);
	sdpCtx->currentReq = NULL;

#if 0	
	if (SYSTEM_IS_EXITING())
	{
		MUX_PLAY_WARN( "'%s' Thread recv EXIT signal\n", th->name );
		return -EXIT_FAILURE;
	}
#endif
	return res;
}

struct SDP_REQ *sdpFind(char type)
{
	struct SDP_CLIENT_CTX *sdpCtx = _sdpCtx;
	struct SDP_REQ *req = NULL;

	LIST_FOREACH(req, &sdpCtx->reqs, list)
	{
		if(req->type == type)
		{
			return req;
		}
	}

	
	EXT_ERRORF("SDP type %d is not found", type);
	
	return NULL;
}

static int __initAddSdpClient(struct SDP_CLIENT_CTX *sdpCtx, char type)
{
	struct SDP_REQ *req;
	
	req = cmn_malloc(sizeof(struct SDP_REQ));
	EXT_ASSERT( (req != NULL), "SDPC context can not be null");
	req->type = type;
	req->sdpCtx = sdpCtx;

	req->timerFd = timerfd_create(EXT_CLOCK_ID, 0);
	if(req->timerFd < 0)
	{
		MUX_ERROR("Timer ID error: %s", strerror(errno));
//		return NULL;
		exit(1);
	}
	
	req->fsm.states = _sdpStateMachine;
	req->fsm.arg = req;
	
	req->lock = cmn_mutex_init();
	LIST_INSERT_HEAD(sdpCtx->reqs, req, list);

	return EXIT_SUCCESS;
}


static int _initSdpThread(CmnThread *th, void *data)
{
	struct SDP_CLIENT_CTX *sdpCtx = _sdpCtx;
	
	LIST_INIT(sdpCtx->reqs);
	sdpCtx->muxMain = (MuxMain *)data;
	EXT_ASSERT((sdpCtx->muxMain != NULL), "MuxMain in SDP Context can not be null" );

	__initAddSdpClient(sdpCtx, HC_REQ_SDP_VIDEO);
	__initAddSdpClient(sdpCtx, HC_REQ_SDP_AUDIO);
	__initAddSdpClient(sdpCtx, HC_REQ_SDP_ANC);
	
	sdpCtx->timer = cmn_add_timer(sdpCtx->muxMain->pollTime*1000, cmnMuxSdpTimerCallback, sdpCtx, cmn_timer_type_reload, "sdpTimer");
	if(sdpCtx->timer == NULL)
	{
		EXT_ERRORF("SDPC: Timer is not created for SDP client in RX");
	}

	sdpCtx->lock = cmn_mutex_init();
	sdpCtx->semaphore = cmn_sem_init();
	
	cmnThreadMask(th->name);
	th->data = sdpCtx;

	return EXIT_SUCCESS;
}

static void _destorySdpThread(struct _CmnThread *th)
{
	struct SDP_CLIENT_CTX *sdpCtx = (struct SDP_CLIENT_CTX *)th->data;

	cmn_free(sdpCtx);
}


CmnThread  threadSdpClient =
{
	name		:	"sdpManager",
	flags			:	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	init			:	_initSdpThread,
	mainLoop		:	NULL,
	eventHandler	:	_sdpEventHandler,
	destory		:	_destorySdpThread,
	data			:	NULL,
};




int cmnMuxSdpAddEvent(HttpClientReq *sdpUri, struct SDP_CLIENT_CTX *sdpCtx)
{
	struct	SDP_REQ *req = sdpFind(sdpUri->type);
	struct SDP_EVENT *event;
	HttpClientReq *uri;

	if(req == NULL)
	{
		EXT_ERRORF("No memory available for SDP %s request", sdpUri->uri );
		return EXIT_FAILURE;
	}

	/* send to FSM, no matter whether is there request current running */
	event =  (struct SDP_EVENT *)cmn_malloc(sizeof(struct SDP_EVENT) );
	if(event == NULL)
	{
		EXT_ERRORF("No memory available for SDP %s request, try later", sdpUri->uri );
		return EXIT_FAILURE;
	}

	uri =  (HttpClientReq *)cmn_malloc(sizeof(HttpClientReq) );
	if(event == NULL)
	{
		EXT_ERRORF("No memory available for SDP %s request, try later", sdpUri->uri );
		cmn_free(event);
		return EXIT_FAILURE;
	}

	memcpy(uri, sdpUri, sizeof(HttpClientReq));
	
	sdpCtx->totalReqs++;
	event->event = SDPC_EVENT_NEW;
	event->data = uri;
	event->req = req;
	
	EXT_INFOF("SDPC #.%d requests http://%s:%d/%s", sdpCtx->totalReqs, cmnSysNetAddress(uri->ip), uri->port, uri->uri);
	
	return cmnThreadAddEvent(&threadSdpClient, event);
}


int cmnMuxSdpTimerCallback(int interval, void *param)
{
	struct SDP_CLIENT_CTX *sdpCtx = (struct SDP_CLIENT_CTX *)param;
	EXT_ASSERT((sdpCtx != NULL), "Params fault in SDP timer callback");
	EXT_ASSERT((sdpCtx->muxMain != NULL), "Params fault in SDP timer callback");

	cmnMuxSdpAddEvent(&sdpCtx->muxMain->runCfg.sdpUriVideo, sdpCtx);
	cmnMuxSdpAddEvent(&sdpCtx->muxMain->runCfg.sdpUriAudio, sdpCtx);
	cmnMuxSdpAddEvent(&sdpCtx->muxMain->runCfg.sdpUriAnc, sdpCtx);

	return EXIT_SUCCESS;
}

