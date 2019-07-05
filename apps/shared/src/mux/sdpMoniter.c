/*
* Monitoring socket and timer
*/


#include "libCmn.h"
#include "libMux.h"

#include <sys/stat.h>

#include "_cmnMux.h"
#include "libCmnSys.h"

#include <string.h>

#include "_sdp.h"

static int _sdpMonitorMainLoop(CmnThread *th)
{
	int res = EXIT_SUCCESS;
	struct SDP_CLIENT_CTX *sdpCtx =(struct SDP_CLIENT_CTX *)th->data;
	struct SDP_REQ *req;
	struct pollfd 	*pollfd, pfds[SDP_CLIENT_NUMBER*2];
	int 	fdCount = 0, count= 0;
	int index;
	uint32_t len;
	

#if 0//ARCH_ARM
	if (SYSTEM_IS_EXITING())
	{
		MUX_WARN("Thread '%s' recv EXIT signal\n", th->name);
		return -EXIT_FAILURE;
	}
#endif

	int	res = EXIT_SUCCESS;

	if(fdCount != 3)
	{
		EXT_INFOF("fdCount:%d", fdCount);
	}

	pollfd = pfds;
	index = 0;
	LIST_FOREACH(req, &sdpCtx->reqs, list)
	{/* no timeout for Ctrl Connection */
	
		cmn_mutex_lock(req->lock);
		if(req->state !=  SDPC_STATE_DATA )
		{
			cmn_mutex_unlock(req->lock);
			continue;
		}

		if(req->socket <= 0)
		{
			cmn_mutex_unlock(req->lock);
			EXT_ERRORF("req is in data state, but socket is not open");
			continue;
		}
		
		pollfd->fd = req->sock;
		pollfd->events = POLLIN|POLLPRI| POLLRDHUP|POLLERR; /* poll read and error event*/

//		EXT_DEBUGF(MUX_DEBUG_BROKER, "Ctrl CONN#%d.%p", index, ctrlConn);
		pollfd++;
		index ++;

		extSetTimer(req->timerFd, SDP_RECV_TIMER_TIMEOUT);
		pollfd->fd = req->timerFd;
		pollfd->events = POLLIN;

		cmn_mutex_unlock(req->lock);
		
		pollfd++;
		count++;
		
		index++;
	}

	if(index == 0)
	{
		goto wait;
	}

	EXT_ASSERT((index >= SDP_CLIENT_NUMBER*2), "Number of fd monitored is too big: %d", index);
	count = index;

	res = poll(pollfd, index, POLL_TIMEOUT/*-1*/); /* in milliseconds */
	if (res == -1 )
	{
		/* what is the appropriate thing to do here on EBADF */
		if (errno == EINTR)
			return EXIT_SUCCESS;
		else if (errno != EBADF)
		{
			MUX_ERROR("POLL for SDP receiving failed: '%s'", strerror(errno));
			return EXIT_FAILURE;
		}
	}
	else if(res == 0)/* timeout */
	{
//		MUX_DEBUG("Timeout of socket");
		return EXIT_SUCCESS;
	}


	pollfd = pfds;
	index = 0;
	LIST_FOREACH(req, &sdpCtx->ctrlConns, list)
	{/* loop 3 ctrl connections */

		cmn_mutex_lock(req->lock);
		
		if( pollfd->fd != req->socket)
		{/* first pollfd is always for the 3 control connections orderly */
			cmn_mutex_unlock(req->lock);
			continue;
		}
		
		if( pollfd->revents & (POLLIN|POLLPRI) )
		{
			int isClose = FALSE;

			len = read(req->socket, (uint8_t *)req->buffer+req->length, sizeof(req->buffer)-req->length );
			if(len == 0)
			{
				isClose = TRUE;
			}

			EXT_DEBUGF("Read %d data from '%.*s'", len, len, req->buffer+req->length );
			req->length += len;

			if(isClose)
			{/* send reply, and then it will be freed */
				MUX_DEBUG( "Broker reply DATA_CONN %s %p directly", dataConn->name, dataConn);
				dataConn->handleOutput(dataConn);
			}
		}
		else if(pollfd->revents & (POLLRDHUP|POLLERR))
		{
			EXT_ERRORF("Ctrl Connection %s is broken", ctrlConn->name );
			exit(1);
		}

		/* timer for this req */
		pollfd++;
		index++;
		if( pollfd->revents & (POLLIN|POLLPRI) )
		{
			EXT_DEBUGF(EXT_DBG_ON, "Timeout for SDP request %s", req )
		}

		cmn_mutex_unlock(req->lock);

		pollfd++;
		index++;
	}

	EXT_ASSERT((count == index), "Number of fd checked is not correct: %d != %d", count, index);

	return res;

wait:
	cmn_sem_wait(sdpCtx->semaphore);
	
	return EXIT_SUCCESS;
	
}

static int _initSdpMonitorThread(CmnThread *th, void *data)
{
	int res = EXIT_SUCCESS;

	cmnThreadMask(th->name);
	th->data = data;

	return res;
}

static void _destorySdpMonitorThread(struct _CmnThread *th)
{
	CMN_MUX_BROKER *broker = (CMN_MUX_BROKER *)th->data;

	_cmnMuxBrokerDestroy(broker);
}


CmnThread  threadSdpMonitor =
{
	.name		=	"sdpMonitor",
	.flags		=	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	.init			=	_initSdpMonitorThread,
	.mainLoop	=	_sdpMonitorMainLoop,
	.eventHandler	= 	NULL,
	.destory		=	_destorySdpThread,
	.data		=	NULL,
};


