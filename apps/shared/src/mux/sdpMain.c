
#include <string.h>
#include <sys/stat.h>

#include "libCmnSys.h"
#include "_cmnMux.h"

#include "_sdp.h"

static struct SDP_CLIENT_CTX _sdpCtx;

int sdpcErrorMsg(struct SDP_CLIENT *sdpClient, const char *frmt,...)
{
	char		buf[2048*10]={0};
	int length = 0;

	va_list ap;
	va_start(ap, frmt);
	//snprintf(buf, sizeof(buf), frmt, ap);
//	vprintf( frmt, ap);
	vsnprintf(buf, sizeof(buf), frmt, ap);
	va_end(ap);

	length = strlen(buf);
	if( 1)//(length + sdpClient->msgLength) > sizeof(sdpClient->msg) )
	{
		length = snprintf(sdpClient->msg, sizeof(sdpClient->msg), "%s", buf);
		sdpClient->msgLength = length;
	}
	else
	{
		length = snprintf(sdpClient->msg+sdpClient->msgLength, sizeof(sdpClient->msg)-sdpClient->msgLength, "%s", buf);
		sdpClient->msgLength += length;
	}
	
	_SDP_DEBUG("finial msg='%s'", sdpClient->msg );

	return length;
}


static int	_sdpEventHandler(struct _CmnThread *th, void *_event)
{
	struct SDP_CLIENT *sdpClient;
	struct SDP_EVENT *sdpEvent = (struct SDP_EVENT *)_event;
	
	EXT_ASSERT((sdpEvent != NULL), "SDP Event can not be null");
	sdpClient = sdpEvent->client;
	EXT_ASSERT((sdpClient != NULL), "SDP Client can't be null");

	cmn_mutex_lock(sdpClient->lock);

	/* update with new URL */
//	memcpy(&sdpClient->url, sdpEvent->data, sizeof(HttpClientReq) );

TRACE();
	sdpClientFsmHandle(sdpClient, sdpEvent);
	cmn_mutex_unlock(sdpClient->lock);

//	if(sdpEvent->data)
	{/*HttpClientReq */
//		cmn_free(sdpEvent->data);
	}
	
	cmn_free(sdpEvent);

	/* semaphore can be post when timer add new task to minimize the chance SdpReceiver is waked */
	if(cmn_sem_post(sdpClient->sdpCtx->semaphore) < 0)
	{
		EXT_ERRORF("SDPC can't post semaphore, because %m");
		exit(1);
	}

#if 0	
	if (SYSTEM_IS_EXITING())
	{
		MUX_PLAY_WARN( "'%s' Thread recv EXIT signal\n", th->name );
		return -EXIT_FAILURE;
	}
#endif
	return EXIT_SUCCESS;
}

static struct SDP_CLIENT *_sdpFindClient(struct SDP_CLIENT_CTX *sdpCtx, char type)
{
	struct SDP_CLIENT *sdpClient = NULL;

	LIST_FOREACH(sdpClient, &sdpCtx->clients, list)
	{
		if(sdpClient->type == type)
		{
			return sdpClient;
		}
	}

	EXT_ERRORF("SDP type %d is not found", type);
	return NULL;
}

static int __initAddSdpClient(struct SDP_CLIENT_CTX *sdpCtx, char type)
{
	struct SDP_CLIENT *sdpClient;
	
	sdpClient = cmn_malloc(sizeof(struct SDP_CLIENT));
	EXT_ASSERT( (sdpClient != NULL), "SDP Client context can not be null");
	sdpClient->type = type;
	sdpClient->sdpCtx = sdpCtx;

	sdpClient->timerFd = timerfd_create(EXT_CLOCK_ID, 0);
	if(sdpClient->timerFd < 0)
	{
		SDPC_ERR_MSG(sdpClient, "Timer ID error: %s", strerror(errno));
//		return NULL;
		exit(1);
	}
	
	snprintf(sdpClient->name, sizeof(sdpClient->name), "Sdpc%s", (type==HC_REQ_SDP_VIDEO)?"Video":(type==HC_REQ_SDP_AUDIO)?"Audio":"Anc");
	sdpClient->fsm = _sdpStateMachine;
	sdpClient->state = SDPC_STATE_WAIT;
	sdpClient->event = EXT_EVENT_NONE;
	
	sdpClient->lock = cmn_mutex_init();
	LIST_INSERT_HEAD(&sdpCtx->clients, sdpClient, list);

	return EXIT_SUCCESS;
}


static int _initSdpThread(CmnThread *th, void *data)
{
	struct SDP_CLIENT_CTX *sdpCtx = &_sdpCtx;
	
	LIST_INIT(&sdpCtx->clients);
	sdpCtx->muxMain = (MuxMain *)data;
	EXT_ASSERT((sdpCtx->muxMain != NULL), "MuxMain in SDP Context can not be null" );

	__initAddSdpClient(sdpCtx, HC_REQ_SDP_VIDEO);
	__initAddSdpClient(sdpCtx, HC_REQ_SDP_AUDIO);
#if WITH_ANCILLIARY_STREAM
	__initAddSdpClient(sdpCtx, HC_REQ_SDP_ANC);
#endif

	sdpCtx->timer = cmn_add_timer(sdpCtx->muxMain->sdpPollTime*1000, cmnMuxSdpTimerCallback, sdpCtx, cmn_timer_type_reload, "sdpTimer");
	if(sdpCtx->timer == NULL)
	{
		EXT_ERRORF("SDPC: Timer is not created for SDP client in RX");
	}

	sdpCtx->lock = cmn_mutex_init();
	sdpCtx->semaphore = cmn_sem_init(1);
	
	cmnThreadMask(th->name);
	th->data = sdpCtx;

#if 0
	if(cmnThreadInit( &threadSdpReceiver, sdpCtx)== EXIT_FAILURE)
	{
		EXT_ERRORF("Thread %s can't be created, check system errors", threadSdpReceiver.name);
		return EXIT_FAILURE;
	}
#endif
	return EXIT_SUCCESS;
}

static void _destorySdpThread(struct _CmnThread *th)
{
	struct SDP_CLIENT_CTX *sdpCtx = (struct SDP_CLIENT_CTX *)th->data;

	cmn_free(sdpCtx);
}


CmnThread  threadSdpClient =
{
	name		:	CMN_THREAD_NAME_SDP_MANAGER,
	flags			:	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	init			:	_initSdpThread,
	mainLoop		:	NULL,
	eventHandler	:	_sdpEventHandler,
	destory		:	_destorySdpThread,
	data			:	NULL,
};

int cmnMuxSdpAddEvent(struct SDP_CLIENT *sdpClient, int eventType, void *data)
{
	struct SDP_EVENT *event;
	
	/* send to FSM, no matter whether is there request current running */
	event =  (struct SDP_EVENT *)cmn_malloc(sizeof(struct SDP_EVENT) );
	if(event == NULL)
	{
		EXT_ERRORF("No memory available for SDPC %s#%d request, try later", sdpClient->name, sdpClient->reqs );
		return EXIT_FAILURE;
	}
	
	event->event = eventType;
	event->data = data;
	event->client = sdpClient;
	
	return cmnThreadAddEvent(&threadSdpClient, event);
}


static int _cmnMuxSdpAddRequest(HttpClientReq *sdpUri, struct SDP_CLIENT_CTX *sdpCtx)
{
	HttpClientReq *uri;

	_SDP_DEBUG("add SDP request='%s', %s:%d; type:%d", sdpUri->uri,  cmnSysNetAddress(sdpUri->ip), sdpUri->port, sdpUri->type );

	struct SDP_CLIENT *sdpClient = _sdpFindClient(sdpCtx, sdpUri->type);
	if(sdpClient == NULL)
	{
		EXT_ERRORF("Can't found SDP Client with type %d for %s", sdpUri->type, sdpUri->uri );
		return EXIT_FAILURE;
	}

	uri =  (HttpClientReq *)cmn_malloc(sizeof(HttpClientReq) );
	if(uri == NULL)
	{
		SDPC_ERR_MSG(sdpClient, "No memory available for SDP %s request, try later", sdpUri->uri );
		return EXIT_FAILURE;
	}

	memcpy(uri, sdpUri, sizeof(HttpClientReq));
	
	sdpCtx->totalReqs++;

	if(SDP_IS_DEBUG(sdpCtx) )
	{
		SDPC_INFO_MSG(sdpClient, "SDPC #.%d requests http://%s:%d/%s", sdpCtx->totalReqs, cmnSysNetAddress(uri->ip), uri->port, uri->uri);
	}

	return cmnMuxSdpAddEvent(sdpClient, SDPC_EVENT_NEW, uri);
}


int cmnMuxSdpTimerCallback(int interval, void *param)
{
	struct SDP_CLIENT_CTX *sdpCtx = (struct SDP_CLIENT_CTX *)param;
	EXT_ASSERT((sdpCtx != NULL), "Params fault in SDP timer callback");
	EXT_ASSERT((sdpCtx->muxMain != NULL), "Params fault in SDP timer callback");

	_cmnMuxSdpAddRequest(&sdpCtx->muxMain->runCfg.sdpUriVideo, sdpCtx);
	_cmnMuxSdpAddRequest(&sdpCtx->muxMain->runCfg.sdpUriAudio, sdpCtx);
#if WITH_ANCILLIARY_STREAM
	_cmnMuxSdpAddRequest(&sdpCtx->muxMain->runCfg.sdpUriAnc, sdpCtx);
#endif

	return EXIT_SUCCESS;
}


static cJSON *_cmnMuxSdpOneClient(struct SDP_CLIENT *sdpClient)
{
//	cJSON *res = cJSON_CreateString(sdpClient->url.uri);
	cJSON *res = cJSON_CreateObject();

	cJSON_AddStringToObject(res, "uri", (const char * const )sdpClient->url.uri );
	cJSON_AddNumberToObject(res, "reqs", (const double )sdpClient->reqs);
	cJSON_AddNumberToObject(res, "pkts", (const double )sdpClient->pkts);
	cJSON_AddNumberToObject(res, "fails", (const double )sdpClient->fails);
	cJSON_AddNumberToObject(res, "headerErrs", (const double )sdpClient->headerErrors);
	cJSON_AddNumberToObject(res, "dataErrs", (const double )sdpClient->dataErrors);

	cJSON_AddStringToObject(res, "msg", (const char * const )sdpClient->msg);

	return res;
}

cJSON *cmnMuxSdpStatus(int isTx)
{
	cJSON *resultObj = NULL;
	struct SDP_CLIENT_CTX *sdpCtx = &_sdpCtx;
	struct SDP_CLIENT *sdpClient;

	resultObj = cJSON_CreateArray();
	//cJSON_CreateObject();
	cmn_mutex_lock(sdpCtx->lock);

	if(isTx)
	{
		cJSON_AddItemToArray(resultObj, cJSON_CreateString("SDP Client is not available on TX"));
	}
	else
	{
		LIST_FOREACH(sdpClient, &sdpCtx->clients, list)
		{
			cJSON_AddItemToArray(resultObj, _cmnMuxSdpOneClient(sdpClient));
		}
	}

	cmn_mutex_unlock(sdpCtx->lock);

	return resultObj;
}

