
/*
* Controller: 
*   1. communicate with controller 811; 
*   2. finish the commands (FTP download commands) 
*   3. or send to commands to correspoding threads, such as Play/Stop
*/

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <poll.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "libCmn.h"
#include "libMux.h"

#include "_cmnMux.h"


/*#define	BROKER_ADD_CTRL_CONN(broker, ctrlConn) \
//	{ if((ctrlConn)==NULL){exit(1);} (ctrlConn)->broker =(broker); if((broker)->ctrlConns==NULL) {(broker)->ctrlConns=(ctrlConn);} \
//	else{(ctrlConn)->next=(broker)->ctrlConns; (broker)->ctrlConns = (ctrlConn);} }
*/

#define	BROKER_ADD_CTRL_CONN(broker, ctrlConn) \
		{if(!(ctrlConn)) return NULL; LIST_INSERT_HEAD(&broker->ctrlConns, ctrlConn, list);broker->connCount++; (ctrlConn)->broker =(broker);}

#define	DATA_CONN_ADDR(ctrlConn, dataConn)	\
		{	(ctrlConn)->connCount++; (ctrlConn)->total++; \
			LIST_INSERT_HEAD(&(ctrlConn)->dataConns, (dataConn), list );}
	
#define	DATA_CONN_REMOVE(dataConn)	\
		{ 		LIST_REMOVE((dataConn), list); \
				(dataConn)->ctrlConn->connCount--; }


/* read command and parse cmdObj for every DATA_CONN */
static struct DATA_CONN *_createDataConnection(struct CTRL_CONN *ctrlConn)
{
	int dataSocket = 0, timerFd;
	struct DATA_CONN *dataConn = NULL;
	struct sockaddr_in		peerAddr;
//	struct sockaddr 		peerAddr;
	socklen_t				addrlen;
	int len = 0;
	char					name[CMN_NAME_LENGTH];
//	unsigned char *objBuffer;
	
	memset(&ctrlConn->buffer, 0, sizeof(ctrlConn->buffer) );

	dataConn = cmn_malloc(sizeof(struct DATA_CONN));

	addrlen = sizeof(struct sockaddr);
	if(ctrlConn->type == CTRL_LINK_TCP || ctrlConn->type == CTRL_LINK_UNIX)
	{
		dataSocket = accept(ctrlConn->sockCtrl, (struct sockaddr *) &peerAddr, &addrlen);
		if(dataSocket < 0)
		{
			MUX_ERROR("accept error:%s; number of current connections:%d", strerror(errno), ctrlConn->connCount );
			goto err;
		}
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("%s connection accepted", ((ctrlConn->type == CTRL_LINK_TCP))?"TCP":"UNIX");
#endif
		len = read(dataSocket, (uint8_t *)dataConn->buffer, sizeof(dataConn->buffer));

		if(ctrlConn->type == CTRL_LINK_TCP)
		{
			snprintf(name, sizeof(name), "TCP#%d-%s:%d", ctrlConn->total, inet_ntoa(peerAddr.sin_addr), ntohs(peerAddr.sin_port));
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
			MUX_DEBUG("TCP Received %d bytes packet from %s", len, name );
#endif
		}
		else
		{
			snprintf(name, sizeof(name), "Unix#%d", ctrlConn->total);
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
			MUX_DEBUG("UNIX Received %d bytes packet", len);
#endif
		}
	}
	else
	{
		len = recvfrom(ctrlConn->sockCtrl, (uint8_t *)dataConn->buffer, sizeof(dataConn->buffer), 0, (struct sockaddr *) &peerAddr, &addrlen);
		dataSocket = -1; /* UDP, this socket is not used */
		snprintf(name, sizeof(name), "UDP#%d-%s:%d", ctrlConn->total, inet_ntoa(peerAddr.sin_addr), ntohs(peerAddr.sin_port));
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("UDP Received %d bytes packet from %s, addrLen:%d", len, name, addrlen );
#endif
	}

	if(len == 0)
	{
		MUX_INFO("Connection is broken by client");
		goto err;
	}
	if(len<0)
	{
		MUX_ERROR("Receive data error: %s", strerror(errno));
//		errCode = IPCMD_ERR_COMMUNICATION;
		goto err;
	}

	timerFd = timerfd_create(EXT_CLOCK_ID, 0);
	if(timerFd < 0)
	{
		MUX_ERROR("Timer ID error: %s", strerror(errno));
//		return NULL;
		goto err;
	}
	
	dataConn->length = len;



	dataConn->errCode = IPCMD_ERR_NOERROR;
	dataConn->sock = dataSocket;
	dataConn->timeFd = timerFd;
	dataConn->port =  ntohs(peerAddr.sin_port);
	memcpy(&dataConn->peerAddr, &peerAddr, addrlen);
	dataConn->addrlen = addrlen;
	dataConn->ctrlConn = ctrlConn;
	dataConn->mutexLock = cmn_mutex_init();

	if(CONTROLLER_IS_DEBUG(dataConn) )
	{
		if(ctrlConn->type == CTRL_LINK_UNIX)
		{
			MUX_INFO("Received %d bytes from %s:"EXT_NEW_LINE"'%.*s'", len,  name,len, dataConn->buffer );
		}
		else
		{
			MUX_INFO("Received %d bytes data from %s:"EXT_NEW_LINE"'%.*s'", len-8, name, len-8, dataConn->buffer+4 );
		}
#if 0//MUX_OPTIONS_DEBUG_IP_COMMAND
		CMN_HEX_DUMP( (uint8_t *)dataConn->buffer, dataConn->length, "Received data from socket" );
#endif
	}
	
	snprintf(dataConn->name, sizeof(dataConn->name), "%s", name);
	if(CONN_IS_IPCMD(dataConn->ctrlConn))
	{
		dataConn->handleInput = cmnMuxDataConnIpCmdInput;
		dataConn->handleOutput = cmnMuxDataConnIpCmdOutput;
	}
	else
	{
		dataConn->handleInput = cmnMuxDataConnRestInput;
		dataConn->handleOutput = cmnMuxDataConnRestOutput;
	}
	
	dataConn->handleAuthen = cmnMuxDataConnAuthen;
	dataConn->handleDestroy = cmnMuxDataConnClose;

	DATA_CONN_ADDR(ctrlConn, dataConn);
#if __DEBUG_CONNECTION
	EXT_DEBUGF(MUX_DEBUG_BROKER, "CTRL CONN %s: current %d, total %d", ctrlConn->name, ctrlConn->connCount, ctrlConn->total);
#endif
	return dataConn;

err:
	if(dataSocket > 0)
	{
		close(dataSocket);
	}
	
	cmn_free(dataConn);
	
	return NULL;
}

static struct CTRL_CONN *_createCtrlConnection(CTRL_LINK_TYPE type, MuxMain *muxMain)
{
	int res = EXIT_SUCCESS;
	struct	CTRL_CONN *ctrlConn = NULL;
	char *socket_path = NULL; 
	int port = 0;
	
	int sockCtrl = 0;

	if(type == CTRL_LINK_UDP)
	{
		sockCtrl = socket(AF_INET, SOCK_DGRAM, 0);
	}
	else if(type == CTRL_LINK_TCP)
	{
		sockCtrl = socket(AF_INET, SOCK_STREAM, 0);
	}
	else if(type == CTRL_LINK_UNIX)
	{
		sockCtrl = socket(AF_UNIX, SOCK_STREAM, 0);
	}
	else
	{
		MUX_ERROR("Ctrl Connection type:'%d' is not support", type);
		exit(1);
	}
	
	if (sockCtrl < 0)
	{
		MUX_ERROR("Socket Error:%s", strerror(errno));
		return NULL;
	}


	if(type == CTRL_LINK_TCP || type == CTRL_LINK_UDP)
	{
		struct sockaddr_in addr;
		int yes;
		
		port = (type==CTRL_LINK_TCP)?muxMain->tcpCtrlPort:muxMain->udpCtrlPort;
		if (setsockopt(sockCtrl, SOL_SOCKET, SO_REUSEADDR, (char *) &yes, sizeof(yes)) < 0)
		{
			MUX_ERROR("Reusing ADDR failed");
			close(sockCtrl);
			return NULL;
		}

		int broadcast = 1;
		//     This option is needed on the socket in order to be able to receive broadcast messages
		//   If not set the receiver will not receive broadcast messages in the local network.
		if(setsockopt(sockCtrl, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
		{
			MUX_ERROR("Error in setting Broadcast option");
			close(sockCtrl);
			return 0;
		}		

		bzero(&addr, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);
		res = bind(sockCtrl, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
	}
	else
	{/* unix socket */
		struct sockaddr_un unaddr;
		memset(&unaddr, 0, sizeof(unaddr));

#if 0		
		socket_path = IS_STRING_NULL_OR_ZERO(muxMain->unixPort)? UNIX_SOCKET_SVR_PATH:muxMain->unixPort;
#else
		socket_path = UNIX_SOCKET_SVR_PATH;
#endif		
		unaddr.sun_family = AF_UNIX;

		if ( 0)//*socket_path == '\0')
		{
			*unaddr.sun_path = '\0';
			strncpy(unaddr.sun_path+1, socket_path+1, sizeof(unaddr.sun_path)-2);
		}
		else
		{
			strncpy(unaddr.sun_path, socket_path, sizeof(unaddr.sun_path)-1);
			unlink(socket_path);
		}

		res = bind(sockCtrl, (struct sockaddr*)&unaddr, sizeof(unaddr));
	}

	if(res < 0)
	{
		MUX_ERROR("Bind %s socket error: %s", (type == CTRL_LINK_TCP)?"TCP":(type == CTRL_LINK_UDP)?"UDP":"Unix", strerror(errno));
		close(sockCtrl);
		return NULL;
	}

#if 0
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(DEFAULT_MC_ADDRESS);         
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);         
	if (setsockopt(udpConn->sockCtrl, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
	{
		perror("setsockopt mreq");
		exit(1);
	}         
#endif

	if(type == CTRL_LINK_TCP || type == CTRL_LINK_UNIX)
	{
		if(listen(sockCtrl, 5) == -1)
		{
			MUX_ERROR("Bind Error:%s", strerror(errno));
			close(sockCtrl);
			return NULL;
		}
	}

	ctrlConn = cmn_malloc(sizeof(struct CTRL_CONN));
	ctrlConn->sockCtrl = sockCtrl;
	ctrlConn->type = type;
	ctrlConn->port = port;
	ctrlConn->handleCreateData = _createDataConnection;

	LIST_INIT(&ctrlConn->dataConns);
	ctrlConn->connCount = 0;

	if(type == CTRL_LINK_UNIX)
	{
		snprintf(ctrlConn->name, sizeof(ctrlConn->name), "%s", socket_path);
	}
	else
	{
		snprintf(ctrlConn->name, sizeof(ctrlConn->name), "%s:%d", (type == CTRL_LINK_TCP)?"TCP":"UDP", port);
	}

#if MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG("Controller's %s", ctrlConn->name );
#endif	
	return ctrlConn;
}


static CMN_MUX_BROKER *_cmnMuxBrokerInit(MuxMain *muxMain)
{
	struct CTRL_CONN *ctrlConn = NULL;
	CMN_MUX_BROKER *broker = NULL;

	broker = cmn_malloc(sizeof(CMN_MUX_BROKER));
	LIST_INIT(&broker->ctrlConns);

	if(muxMain->udpCtrlPort != 0)
	{
		ctrlConn = _createCtrlConnection(CTRL_LINK_UDP, muxMain);
		BROKER_ADD_CTRL_CONN(broker, ctrlConn);
	}

	if(muxMain->tcpCtrlPort != 0)
	{
		ctrlConn = _createCtrlConnection(CTRL_LINK_TCP, muxMain);
		BROKER_ADD_CTRL_CONN(broker, ctrlConn);
	}

	ctrlConn = _createCtrlConnection(CTRL_LINK_UNIX, muxMain);
//	LIST_INSERT_HEAD(&broker->ctrlConns, ctrlConn, list);
	BROKER_ADD_CTRL_CONN(broker, ctrlConn);

	broker->muxMain = muxMain;
//	broker->cfg = muxMain-> cfg;

	return broker;
}


static void _cmnMuxBrokerDestroy(CMN_MUX_BROKER *broker)
{
	struct CTRL_CONN *ctrlConn, *tmp;// = broker->ctrlConns;

	LIST_FOREACH_SAFE(ctrlConn, &broker->ctrlConns, list, tmp)
//	while(ctrlConn)
	{
#if 0	
		struct CTRL_CONN *tmp = ctrlConn;
		ctrlConn = tmp->next;
		
		close(tmp->sockCtrl);
		cmn_free(tmp);
#else		
		close(ctrlConn->sockCtrl);
		cmn_free(ctrlConn);
#endif		
	}

	cmn_free(broker);
}


/* read more from DATA_CONN */
static int _dataConnReadMore(struct DATA_CONN *dataConn)
{
//	struct sockaddr_in		peerAddr;
//	socklen_t				addrlen;
	int len = 0;
	
	if(dataConn->ctrlConn->type == CTRL_LINK_TCP || dataConn->ctrlConn->type == CTRL_LINK_UNIX)
	{
		len = read(dataConn->sock, (uint8_t *)dataConn->buffer, sizeof(dataConn->buffer));
	}
	else
	{/* it can't happens */
//		len = recvfrom(dataConn->ctrlConn->sockCtrl, (uint8_t *)dataConn->buffer, sizeof(dataConn->buffer), 0, (struct sockaddr *) &peerAddr, &addrlen);
	}
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG("DATA CONN %s received %d bytes more", dataConn->name, len);
#endif

	if(len > 0)
	{
		dataConn->length = len;
#if MUX_OPTIONS_DEBUG_IP_COMMAND
		CMN_HEX_DUMP( (uint8_t *)dataConn->buffer, dataConn->length, "Received more data from socket" );
#endif
	}

	return len;
}

static struct DATA_CONN *_findDataConn(CMN_MUX_BROKER *broker, int *isTimer, int fd)
{
	struct CTRL_CONN *ctrlConn = NULL;// broker->ctrlConns;
	struct DATA_CONN *dataConn;

	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{
		LIST_FOREACH(dataConn, &ctrlConn->dataConns, list)
		{
			if(dataConn->sock == fd)
			{
				*isTimer = FALSE;
				return dataConn;
			}
			
			if(dataConn->timeFd == fd)
			{
				*isTimer = TRUE;
				return dataConn;
			}
		}
	}

	return NULL;
}

#define	POLL_TIMEOUT			500 /* ms */

#define	TIMER_FD_TIMEOUT		250 /* ms, must be less than POLL_TIMEOUT */


static int _cmnMuxBrokerReceive(CMN_MUX_BROKER *broker)
{
	struct CTRL_CONN *ctrlConn = NULL;
	struct DATA_CONN *dataConn;
	struct pollfd 	*pollfd, *pfd;
	int 	fdCount = 0, count= 0;
	int index;
	
	int	res = EXIT_SUCCESS;

	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{
		if(ctrlConn->type != CTRL_LINK_UDP)
		{/* new data connection is monitored for UDP */
			fdCount += 1 + ctrlConn->connCount*2; /* 2 fd for every data connection */
		}
		else
		{
			fdCount += 1 + ctrlConn->connCount; /* only one for UDP control connection, and one for every data connection, timerfd */
		}
	}

#if 0
	if(fdCount != 3)
	{
		EXT_INFOF("fdCount:%d", fdCount);
	}
#endif

	/* Need to allocate one whole extra block of fds for UDS. */
	pollfd = cmn_malloc( fdCount*sizeof(struct pollfd));
	if (!pollfd)
	{
		return -1;
	}

	pfd = pollfd;
	index = 0;
	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{/* no timeout for Ctrl Connection */
		pfd->fd = ctrlConn->sockCtrl;
		pfd->events = POLLIN|POLLPRI| POLLRDHUP|POLLERR; /* poll read and error event*/

//		EXT_DEBUGF(MUX_DEBUG_BROKER, "Ctrl CONN#%d.%p", index, ctrlConn);
		pfd++;
		index ++;
	}
	
	if(index != broker->connCount)
	{
		EXT_ERRORF("Number of Ctrl Connection is not correct: %d != %d", index, broker->connCount )
	}

	/* set poll for data connection */
	count = 3;
	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{
		index = 0;
		
		LIST_FOREACH(dataConn, &ctrlConn->dataConns, list)
		{
#if __DEBUG_CONNECTION
			EXT_DEBUGF(MUX_DEBUG_BROKER, "DATA CONN#%s adding to poll...", dataConn->name);
#endif
			if( dataConn->ctrlConn->type !=  CTRL_LINK_UDP)
			{/* no matter fd of UDP connection is 0, it is also to be used to simplify the process */
				/* socket */
				pfd->fd = dataConn->sock;
				pfd->events = POLLIN|POLLPRI| POLLRDHUP|POLLERR;
				pfd++;
				count++;
			}
			
			/* timer */
			timerfd_set_time(dataConn->timeFd, TIMER_FD_TIMEOUT);
			pfd->fd = dataConn->timeFd;
			pfd->events = POLLIN;
			pfd++;
			count++;
			
			index++;
		}
		
		if(index != ctrlConn->connCount)
		{
			EXT_ERRORF("Number of Data Connection in Ctrl Connection %s not correct: %d != %d", ctrlConn->name, index, ctrlConn->connCount );
		}
	}

	EXT_ASSERT((count == fdCount), "Number of fd polled is not correct: %d != %d", count, fdCount);

	res = poll(pollfd, fdCount, POLL_TIMEOUT/*-1*/); /* in milliseconds */
	if (res == -1 )
	{
		/* what is the appropriate thing to do here on EBADF */
		if (errno == EINTR)
			return EXIT_SUCCESS;
		else if (errno != EBADF)
		{
			MUX_ERROR("POLL system call failed: '%s'", strerror(errno));
			return EXIT_FAILURE;
		}
	}
	else if(res == 0)/* timeout */
	{
//		MUX_DEBUG("Timeout of socket");
		return EXIT_SUCCESS;
	}


	pfd = pollfd;
	index = 0;
	count = 0;
	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{/* loop 3 ctrl connections */
		count++;
		
		if( pfd->fd != ctrlConn->sockCtrl)
		{/* first pollfd is always for the 3 control connections orderly */
			EXT_ERRORF("CtrlConn %s is not for this pollfd %d, real fd is %d", ctrlConn->name, pfd->fd, ctrlConn->sockCtrl);
			pfd++;
			continue;
		}
		
		if( pfd->revents & (POLLIN|POLLPRI))
		{
			int isClose = TRUE;
			dataConn = ctrlConn->handleCreateData(ctrlConn);

			if(dataConn == NULL)
			{
				pfd++;
				continue;
			}
			
			/* socket */
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
			MUX_DEBUG( "Read data from CONN %s", dataConn->name );
#endif
			EXT_ASSERT(dataConn->handleInput, "InputHandler can't be null");
			if(dataConn->handleInput==NULL)
			{
				EXT_ERRORF("InputHandler can't be null");
			}
			dataConn->handleInput(dataConn);

			cmn_mutex_lock(dataConn->mutexLock);
			if(dataConn->errCode == IPCMD_ERR_NOERROR )
			{
				if( cmnMuxManagerAddEvent(dataConn->cmd, dataConn->method, dataConn) == EXIT_SUCCESS)
				{
#if MUX_OPTIONS_DEBUG_IP_COMMAND
					EXT_DEBUGF(MUX_DEBUG_BROKER, "DATA CONN#%s submit to Mgr thread", dataConn->name);
#endif
					dataConn->status = DATA_CONN_STATUS_WAITING;
					isClose = FALSE;
				}
				else
				{
					DATA_CONN_ERR(dataConn, IPCMD_ERR_NOT_SUPPORT_COMMND,  "Command '%s' is not supportted", dataConn->cmd);
				}
			}

			cmn_mutex_unlock(dataConn->mutexLock);

			if(isClose)
			{/* send reply, and then it will be freed */
#if MUX_OPTIONS_DEBUG_IP_COMMAND
				MUX_DEBUG( "Broker reply DATA_CONN %s %p directly", dataConn->name, dataConn);
#endif
				dataConn->handleOutput(dataConn);
			}
		}
		else if(pfd->revents & (POLLRDHUP|POLLERR))
		{
			EXT_ERRORF("Ctrl Connection %s is broken", ctrlConn->name );
			exit(1);
		}

		pfd++;
	}

	/* loop all data connections */
	for(index=3; index < fdCount; index++)
	{/* data connection list may includes new data connection just created, and not be polled */
		int isTimer = 0;

		count++;
		dataConn = _findDataConn(broker, &isTimer, pfd->fd);

		if(dataConn == NULL )
		{
			EXT_ERRORF("Data Connection with fd of %d is not found", pfd->fd );
			pfd++;
			continue;
		}


		if(isTimer)
		{/* timer fd, check */
			if( pfd->revents & POLLIN )
			{
#if __DEBUG_CONNECTION
				EXT_ERRORF("Data Connection %s timeout", dataConn->name )
#endif				
				cmn_mutex_lock(dataConn->mutexLock);
				if(dataConn->status != DATA_CONN_STATUS_FINISHED)
				{
					dataConn->status = DATA_CONN_STATUS_CLOSING;
				}
				cmn_mutex_unlock(dataConn->mutexLock);
			}
		}
		else
		{
			if( pfd->revents & (POLLIN|POLLPRI))
			{
				int _readLen;
				
#if __DEBUG_CONNECTION
				EXT_ERRORF("Data Connection %s received more data, ignored!!", dataConn->name )
#endif
				if(dataConn->ctrlConn->type == CTRL_LINK_UDP )
				{
					EXT_ERRORF("UDP CONN %s can not be timeout", dataConn->name);
					exit(1);
				}
				
				cmn_mutex_lock(dataConn->mutexLock);
				_readLen = _dataConnReadMore(dataConn);
				cmn_mutex_unlock(dataConn->mutexLock);

				if(_readLen == 0)
				{
#if __DEBUG_CONNECTION
					EXT_DEBUGF(MUX_DEBUG_BROKER, "Connection is broken by peer");
#endif
				}
				else if(_readLen<0)
				{
					MUX_ERROR("Receive data error: %s", strerror(errno));
				}
			}
			else if(pfd->revents & (POLLRDHUP|POLLERR))
			{
				EXT_ERRORF("Data Connection %s broken", dataConn->name )
				cmn_mutex_lock(dataConn->mutexLock);
				if(dataConn->status != DATA_CONN_STATUS_FINISHED)
				{
					dataConn->status = DATA_CONN_STATUS_CLOSING;
				}
				cmn_mutex_unlock(dataConn->mutexLock);
			}
		}

		pfd++;
	}
	
	EXT_ASSERT((count == fdCount), "Number of fd checked is not correct: %d != %d", count, fdCount);

	cmn_free(pollfd);

	return EXIT_SUCCESS;
}

static int _muxBrokerMainLoop(CmnThread *th)
{
	int res = EXIT_SUCCESS;
	CMN_MUX_BROKER *broker =(CMN_MUX_BROKER *)th->data;
	struct CTRL_CONN *ctrlConn;
	struct DATA_CONN *dataConn;

#if 0//ARCH_ARM
	if (SYSTEM_IS_EXITING())
	{
		MUX_WARN("Thread '%s' recv EXIT signal\n", th->name);
		return -EXIT_FAILURE;
	}
#endif

	res = _cmnMuxBrokerReceive(broker);

	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{
		struct DATA_CONN *tmp;
		
		LIST_FOREACH_SAFE(dataConn, &ctrlConn->dataConns, list, tmp)
		{
			cmn_mutex_lock(dataConn->mutexLock);
			if(dataConn->status ==  DATA_CONN_STATUS_FINISHED )
			{
				DATA_CONN_REMOVE(dataConn);
				cmn_mutex_unlock(dataConn->mutexLock);
				
				cmnMuxDataConnClose( dataConn);
			}
			else
			{
				cmn_mutex_unlock(dataConn->mutexLock);
			}
		}
	}

	return res;
}

static int _initBrokerThread(CmnThread *th, void *data)
{
	int res = EXIT_SUCCESS;
	MuxMain *muxMain = (MuxMain *)data;

	CMN_MUX_BROKER *broker = _cmnMuxBrokerInit(muxMain);
	if(broker == NULL)
		return -EXIT_FAILURE;

	cmnThreadMask(th->name);
	th->data = broker;

	return res;
}

static void _destoryBrokerThread(struct _CmnThread *th)
{
	CMN_MUX_BROKER *broker = (CMN_MUX_BROKER *)th->data;

	_cmnMuxBrokerDestroy(broker);
}


CmnThread  threadBroker =
{
	name		:	CMN_THREAD_NAME_COMM,
	init			:	_initBrokerThread,
	mainLoop		:	_muxBrokerMainLoop,
	eventHandler	:	NULL,
	destory		:	_destoryBrokerThread,
	
	data			:	NULL,
};


