/*
* Monitoring socket and timer
*/

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <poll.h>
#include <sys/stat.h>


#include "libCmnSys.h"
#include "_cmnMux.h"

#include "_sdp.h"

#define	_SDP_RV_ERROR			-1
#define	_SDP_RV_CONTINUE		0
#define	_SDP_RV_OK				1

static int _findResponseType(char *response, uint32_t size)
{
	if(strnstr(response, HTTP_HDR_SDP, size) != NULL )
	{
		return HC_REQ_SDP_VIDEO;
	}
	else if(strnstr(response, HTTP_HDR_SDP_EXT, size) != NULL )
	{
		return HC_REQ_SDP_VIDEO;
	}
	else if(strnstr(response, HTTP_HDR_JSON, size) != NULL )
	{
		return HC_REQ_JSON;
	}

	return HC_REQ_ERROR;
}


/* return < 0, error; 0 : on this header; others, content length */
uint32_t	cmnHttpParseHeaderContentLength(char *headers, uint32_t headerLength)
{
	char *scontent_len_end, *scontent_len;
	char *content_len_num;
	int contentLen;

		/* search for "Content-Length: " */
#define HTTP_HDR_CONTENT_LEN                "Content-Length: "
#define HTTP_HDR_CONTENT_LEN_LEN            16
#define HTTP_HDR_CONTENT_LEN_DIGIT_MAX_LEN  10

	scontent_len = strnstr(headers, HTTP_HDR_CONTENT_LEN, headerLength);
	if (scontent_len == NULL)
	{
		return _SDP_RV_CONTINUE;
	}
	
	scontent_len_end = strnstr(scontent_len + HTTP_HDR_CONTENT_LEN_LEN, EXT_HTTP_CRLF, HTTP_HDR_CONTENT_LEN_DIGIT_MAX_LEN);
	if (scontent_len_end == NULL)
	{
		EXT_ERRORF("Error when parsing number in Content-Length: '%s'",scontent_len+HTTP_HDR_CONTENT_LEN_LEN );
		return _SDP_RV_ERROR;
	}

	content_len_num = scontent_len + HTTP_HDR_CONTENT_LEN_LEN;
	contentLen = atoi(content_len_num);
			
	if (contentLen == 0)
	{
		/* if atoi returns 0 on error, fix this */
		if ((content_len_num[0] != '0') || (content_len_num[1] != '\r'))
		{
			contentLen = -1;
		}
	}
			
	if (contentLen < 0)
	{
		EXT_ERRORF("POST received invalid Content-Length: %s", content_len_num);
//		cmnHttpRestError(ehc, WEB_RES_BAD_REQUEST, "Content-Length is wrong");
		return _SDP_RV_ERROR;
	}

	_SDP_DEBUG( "SDPC content length:%d", contentLen);
	
	return contentLen;
}

static int __sdpParseHttpProtocol(struct SDP_CLIENT *sdpClient)
{
	int ret;
	char *data;
		
	data = strnstr(sdpClient->buffer, EXT_HTTP_CRLF EXT_HTTP_CRLF, sdpClient->length);
	
	/* step 2: parse response type(SDP/REST), and content length */
	if(sdpClient->contentLength == 0 )
	{/* this is first packet replied from server*/
		if( data == NULL )
		{/* when end of http header is not found, don't parse header, and continue to receive */
			return _SDP_RV_CONTINUE;
		}

		sdpClient->responseType = _findResponseType(sdpClient->buffer, (uint32_t)sdpClient->length);
		if(sdpClient->responseType != HC_REQ_SDP_VIDEO)
		{
			sdpClient->headerErrors++;
			SDPC_MSG(sdpClient, "Data error for %s: parsing invalidate Content-Type:"EXT_NEW_LINE"'%.*s'", sdpClient->url.uri, sdpClient->length, sdpClient->buffer );
			return _SDP_RV_ERROR;
		}

		ret = cmnHttpParseHeaderContentLength(sdpClient->buffer, (uint32_t)sdpClient->length);
		if(ret <= _SDP_RV_CONTINUE)
		{/* content_length found, but parsing wrong */
			sdpClient->headerErrors++;
			SDPC_MSG(sdpClient, "Data error for %s: parsing number in Content-Length:"EXT_NEW_LINE"'%.*s'", sdpClient->url.uri, sdpClient->length, sdpClient->buffer );
			return _SDP_RV_ERROR;
		}
		else
		{/* otherwise return content length in HTTP header */
			uint32_t _dataLen = 0;
			
			sdpClient->headerLength = (data - sdpClient->buffer ) + __HTTP_CRLF_SIZE;
			sdpClient->contentLength = ret;
		
			_dataLen = sdpClient->length - sdpClient->headerLength;
			if(_dataLen == sdpClient->contentLength )
			{
				sdpClient->contentLength = ret;
//				sdpClient->data = sdpClient->buffer + (sdpClient->length - sdpClient->contentLength);
				sdpClient->data = sdpClient->buffer + sdpClient->headerLength;

				SDPC_DEBUG_MSG(sdpClient, "recv %d bytes, header length %d, content length %d, data "EXT_NEW_LINE"'%.*s'", 
					sdpClient->length, sdpClient->headerLength, sdpClient->contentLength, sdpClient->contentLength, sdpClient->data);

				SDPC_DEBUG_MSG(sdpClient, "Recv SDP response in one packet OK" );
				return _SDP_RV_OK;
			}
			else if(_dataLen >= 0)
			{/* */
				return _SDP_RV_CONTINUE;
			}
			else
			{/* error */
				sdpClient->dataErrors++;
				SDPC_MSG(sdpClient, "Data error for %s: Content Length %"FOR_U32" is not same as data length:%"FOR_U32, sdpClient->url.uri, ret, _dataLen);
				return _SDP_RV_ERROR;
			}
		}
	}
	else
	{
		SDPC_DEBUG_MSG(sdpClient, "Recv late packet, total length %d, Content Length %d, header length %d ", sdpClient->length, sdpClient->contentLength, sdpClient->headerLength );
		if(sdpClient->contentLength + sdpClient->headerLength == sdpClient->length)
		{/* after parsed content_length and new line in the end of http header, and then received new packet only for data */
			SDPC_DEBUG_MSG(sdpClient, "Recv SDP response in multiple packets OK" );
			sdpClient->data = sdpClient->buffer + sdpClient->headerLength;
			return _SDP_RV_OK;
		}
		else if(sdpClient->contentLength + sdpClient->headerLength > sdpClient->length)
		{/* wait another packet for http data  */
			return _SDP_RV_CONTINUE;
		}
		else
		{
			sdpClient->dataErrors++;
			SDPC_MSG(sdpClient, "Data error for %s: Recv second packet, length %d, Content Length %d, data length %d", 
				sdpClient->url.uri, sdpClient->length, sdpClient->contentLength, sdpClient->headerLength );
			return _SDP_RV_ERROR;
		}
	}

#if 0
	/* don't use content type to check now. 07.08, 2019 */
	if(sdpClient->reqType == HC_REQ_ERROR)
	{
		sdpClient->dataErrors++;
		snprintf(sdpClient->msg, sizeof(sdpClient->msg), "Response is not supported type of JSON or SDP");
		EXT_ERRORF("Data error for %s: Response is not supported type of JSON or SDP", sdpClient->url.uri );
		return _SDP_RV_ERROR;
	}
#endif

	return _SDP_RV_CONTINUE;
}

static struct SDP_CLIENT *_findSdpcClient(struct SDP_CLIENT_CTX *sdpCtx, int *isTimer, int fd)
{
	struct SDP_CLIENT *sdpClient = NULL;

	LIST_FOREACH(sdpClient, &sdpCtx->clients, list)
	{
		if(sdpClient->socket == fd && sdpClient->socket >0 )
		{
			*isTimer = FALSE;
			return sdpClient;
		}
		
		if(sdpClient->timerFd == fd)
		{
			*isTimer = TRUE;
			return sdpClient;
		}
	}

	return NULL;
}



static int _sdpReceiverMainLoop(CmnThread *th)
{
	int res = EXIT_SUCCESS;
	struct SDP_CLIENT_CTX *sdpCtx =(struct SDP_CLIENT_CTX *)th->data;
	struct SDP_CLIENT *sdpClient;
	struct pollfd 	*pollfd;
	struct pollfd	pfds[SDP_CLIENT_NUMBER*2];
	int 	i = 0, count= 0;
	int index;
	uint32_t len;
	

#if 0//ARCH_ARM
	if (SYSTEM_IS_EXITING())
	{
		MUX_WARN("Thread '%s' recv EXIT signal\n", th->name);
		return -EXIT_FAILURE;
	}
#endif

	pollfd = pfds;
	index = 0;
	LIST_FOREACH(sdpClient, &sdpCtx->clients, list)
	{/* no timeout for Ctrl Connection */
	
		cmn_mutex_lock(sdpClient->lock);
		if(! SDPC_CHECK_STATE(sdpClient, SDPC_STATE_CONNECTED) )
		{
			cmn_mutex_unlock(sdpClient->lock);
			continue;
		}

		/* sdp client will not been polled after sdp data has been received after http header */
		if(sdpClient->socket <= 0 || sdpClient->data != NULL )
		{/* sdpClient->data == NULL, data part of http packet is not received now */
			cmn_mutex_unlock(sdpClient->lock);
//			SDPC_INFO_MSG(sdpClient, "data has been received and socket close, wait to be parsed" );
			continue;
		}
		
		SDPC_DEBUG_MSG(sdpClient, "is polling on socket %d", sdpClient->socket );
		pollfd->fd = sdpClient->socket;
		pollfd->events = POLLIN|POLLPRI| POLLRDHUP|POLLERR; /* poll read and error event*/

		pollfd++;
		count ++;

		SDPC_DEBUG_MSG(sdpClient, "is polling on timer %d", sdpClient->timerFd);
		timerfd_set_time(sdpClient->timerFd, SDP_RECV_TIMER_TIMEOUT);
		pollfd->fd = sdpClient->timerFd;
		pollfd->events = POLLIN;

		cmn_mutex_unlock(sdpClient->lock);
		
		pollfd++;
		count++;

		EXT_ASSERT((count <= SDP_CLIENT_NUMBER*2), "Number of fd polled is too big: %d", count);
	}

	if(count == 0)
	{
		goto wait;
	}

	_SDP_DEBUG( "Total %d fds are polling now...", count );
	res = poll(pfds, count, SDPC_POLL_TIMEOUT/*-1*/); /* in milliseconds */
	if (res == -1 )
	{
		MUX_ERROR("POLL for SDP receiving failed: '%s'", strerror(errno));
		/* what is the appropriate thing to do here on EBADF */
		if (errno == EINTR)
		{
			return EXIT_SUCCESS;
		}
		else if (errno != EBADF)
		{
			MUX_ERROR("POLL for SDP receiving failed: '%s'", strerror(errno));
			return EXIT_FAILURE;
		}
	}
	else if(res == 0)/* timeout */
	{
		MUX_DEBUG("Timeout of SDP polling");
		return EXIT_SUCCESS;
	}


	pollfd = pfds;
	index = 0;
	
	for(i=0; i < count; i++)
	{
		int isTimer = 0;
		
		_SDP_DEBUG( "#%d: fd=%d; events=0x%x; revents:0x%x", i, pollfd->fd, pollfd->events, pollfd->revents );
		sdpClient = _findSdpcClient(sdpCtx, &isTimer, pollfd->fd);
		if(sdpClient)
		{
			cmn_mutex_lock(sdpClient->lock);
			if(isTimer )
			{
				SDPC_DEBUG_MSG(sdpClient, "#%d: %s#%d polling timer %d=%d", i, sdpClient->name, sdpClient->reqs, sdpClient->timerFd, pollfd->fd );
				/* timer for this req */
				if( pollfd->revents & (POLLIN|POLLPRI) )
				{
					SDPC_MSG(sdpClient, "Timeout for SDPC ");
					SDPC_SEND_EVENT(sdpClient, SDPC_EVENT_TIMEOUT);				
				}
			}
			else
			{
				SDPC_DEBUG_MSG(sdpClient, "#%d: polling socket %d=%d", i, sdpClient->socket, pollfd->fd );
				if( pollfd->revents & (POLLIN|POLLPRI) )
				{
					len = read(sdpClient->socket, (uint8_t *)sdpClient->buffer+sdpClient->length, sizeof(sdpClient->buffer)-sdpClient->length );
					if(len < 0)
					{
						SDPC_ERR_MSG(sdpClient, "Connection read error: %m");
						SDPC_SEND_EVENT(sdpClient, SDPC_EVENT_CLOSE);
					}
					else if(len == 0)
					{
						SDPC_MSG(sdpClient, "Connection is broken by peer");
						if(sdpClient->contentLength> 0 && sdpClient->data )
						{
							SDPC_SEND_EVENT(sdpClient, SDPC_EVENT_RECV);				
						}
						else
						{
							SDPC_SEND_EVENT(sdpClient, SDPC_EVENT_CLOSE);
						}
					}
					else
					{
						if(SDP_IS_DEBUG(sdpCtx) )
						{
							SDPC_INFO_MSG(sdpClient, "read %d data from "EXT_NEW_LINE"'%.*s'", len, len, sdpClient->buffer+ sdpClient->length );
						}
						
//						cmnHexDump((const uint8_t *)sdpClient->buffer+ sdpClient->length, len );
						sdpClient->length += len;
						sdpClient->pkts++;

						res = __sdpParseHttpProtocol(sdpClient);
						if(res == _SDP_RV_OK)
						{
							SDPC_SEND_EVENT(sdpClient, SDPC_EVENT_RECV);				
						}
						else if(res == _SDP_RV_ERROR)
						{
							SDPC_SEND_EVENT(sdpClient, SDPC_EVENT_ERROR);				
						}
						
						/* others, continue to receive more packets */
					}
				}
				else if(pollfd->revents & (POLLRDHUP|POLLERR))
				{
					SDPC_MSG(sdpClient, "Connection error");
					SDPC_SEND_EVENT(sdpClient, SDPC_EVENT_ERROR);				
					exit(1);
				}
				else if(pollfd->revents & (POLLNVAL /*0x0020*/ ))
				{
					SDPC_MSG(sdpClient, "Connection error on POLLNVAL");
					SDPC_SEND_EVENT(sdpClient, SDPC_EVENT_ERROR);				
					exit(1);
				}
				
			}
			cmn_mutex_unlock(sdpClient->lock);

			index++;
		}
		else
		{
			EXT_ERRORF("SDPC poll fd %d has not found SdpClient", pollfd->fd);
		}
		
		pollfd++;
	}

	EXT_ASSERT((count == index), "Number of fd checked is not correct: count:%d != polled:%d", count, index);

	return EXIT_SUCCESS;

wait:
	_SDP_DEBUG( "waiting to be waked by SdpManager" );
	if(cmn_sem_wait(sdpCtx->semaphore) < 0)
	{
		EXT_ERRORF("SDPC can't wait, because %m");
		exit(1);
	}
	
	return EXIT_SUCCESS;
	
}

static int _initSdpReceiverThread(CmnThread *th, void *data)
{
	int res = EXIT_SUCCESS;

	cmnThreadMask(th->name);
	th->data = data;

	return res;
}

static void _destorySdpReceiverThread(struct _CmnThread *th)
{
}


CmnThread  threadSdpReceiver =
{
	.name		=	CMN_THREAD_NAME_SDP_RECEIVER,
	.flags		=	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	.init			=	_initSdpReceiverThread,
	.mainLoop	=	_sdpReceiverMainLoop,
	.eventHandler	= 	NULL,
	.destory		=	_destorySdpReceiverThread,
	.data		=	NULL,
};


