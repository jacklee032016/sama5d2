
/*
* Controller: 
*   1. communicate with controller 811; 
*   2. finish the commands (FTP download commands) 
*   3. or send to commands to correspoding threads, such as Play/Stop
*/

/* for timer fd */
#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

/* Definitions from include/linux/timerfd.h */
#define TFD_TIMER_ABSTIME	(1 << 0)

//#define	EXT_CLOCK_ID				CLOCK_MONOTONIC
#define	EXT_CLOCK_ID				CLOCK_REALTIME



int timerfd_create(int clockid, int flags)
{
	return syscall(__NR_timerfd_create, clockid, flags);
}

int timerfd_settime(int ufc, int flags, const struct itimerspec *utmr, struct itimerspec *otmr)
{
	return syscall(__NR_timerfd_settime, ufc, flags, utmr, otmr);
}

int timerfd_gettime(int ufc, struct itimerspec *otmr)
{
	return syscall(__NR_timerfd_gettime, ufc, otmr);
}

void set_timespec(struct timespec *tmr, unsigned long long ustime)
{
	tmr->tv_sec = (time_t) (ustime / 1000000ULL);
	tmr->tv_nsec = (long) (1000ULL * (ustime % 1000000ULL));
}


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

int extSetTimer(int fd, long mstime)
{
	struct itimerspec tmr;
	
	set_timespec(&tmr.it_value, mstime*1000); /* ustime*/
	set_timespec(&tmr.it_interval, 0);

	if (timerfd_settime(fd, 0, &tmr, NULL))
	{
		EXT_ERRORF("timerfd_settime to %ld ms on fd %d: %m", mstime, fd);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
	

/* read command and parse cmdObj for every DATA_CONN */
static struct DATA_CONN *_createDataConnection(struct CTRL_CONN *ctrlConn)
{
	int dataSocket = 0, timerFd;
	struct DATA_CONN *dataConn = NULL;
	struct sockaddr_in		peerAddr;
//	struct sockaddr 		peerAddr;
	socklen_t				addrlen;
	int len = 0;
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
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		if(ctrlConn->type == CTRL_LINK_TCP)
		{
			MUX_DEBUG("TCP Received %d bytes packet from %s:%d", len, inet_ntoa(peerAddr.sin_addr), ntohs(peerAddr.sin_port));
		}
		else
		{
			MUX_DEBUG("UNIX Received %d bytes packet", len);
		}
#endif
	}
	else
	{
		len = recvfrom(ctrlConn->sockCtrl, (uint8_t *)dataConn->buffer, sizeof(dataConn->buffer), 0, (struct sockaddr *) &peerAddr, &addrlen);
		dataSocket = -1; /* UDP, this socket is not used */
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("UDP Received %d bytes packet from %s:%d, addrLen:%d", len, inet_ntoa(peerAddr.sin_addr), ntohs(peerAddr.sin_port), addrlen );
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

#if 1//MUX_OPTIONS_DEBUG_IP_COMMAND
	CMN_HEX_DUMP( (uint8_t *)dataConn->buffer, dataConn->length, "Received data from socket" );
#endif


	dataConn->errCode = IPCMD_ERR_NOERROR;
	dataConn->sock = dataSocket;
	dataConn->timeFd = timerFd;
	dataConn->port =  ntohs(peerAddr.sin_port);
	memcpy(&dataConn->peerAddr, &peerAddr, addrlen);
	dataConn->addrlen = addrlen;
	dataConn->ctrlConn = ctrlConn;
	dataConn->mutexLock = cmn_mutex_init();

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

	ctrlConn->connCount++;
	LIST_INSERT_HEAD(&ctrlConn->dataConns, dataConn, list );

	TRACE();
	return dataConn;

err:
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
		MUX_ERROR("Bind Error:%s", strerror(errno));
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
		MUX_DEBUG("Controller's Unix port %s", socket_path);
	}
	else
	{
		MUX_DEBUG("Controller's %s port %d", (type == CTRL_LINK_TCP)?"TCP":"UDP", port);
	}
	
	return ctrlConn;
}



/*#define	BROKER_ADD_CTRL_CONN(broker, ctrlConn) \
//	{ if((ctrlConn)==NULL){exit(1);} (ctrlConn)->broker =(broker); if((broker)->ctrlConns==NULL) {(broker)->ctrlConns=(ctrlConn);} \
//	else{(ctrlConn)->next=(broker)->ctrlConns; (broker)->ctrlConns = (ctrlConn);} }
*/

#define	BROKER_ADD_CTRL_CONN(broker, ctrlConn) \
		{ LIST_INSERT_HEAD(&broker->ctrlConns, ctrlConn, list);broker->connCount++; (ctrlConn)->broker =(broker);}
	

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

#define	POLL_TIMEOUT			500 /* ms */

#define	TIMER_FD_TIMEOUT		250 /* ms, must be less than POLL_TIMEOUT */


static int _cmnMuxBrokerReceive(CMN_MUX_BROKER *broker)
{
	struct CTRL_CONN *ctrlConn = NULL;// broker->ctrlConns;
	struct DATA_CONN *dataConn;
	struct pollfd 	*pollfd, *pfd;
	int 	fdCount = 0;
	int index;
	
	fd_set				readFdSet;
	fd_set				writeFdSet;
	int					maxFd = 0;
	int	res = EXIT_SUCCESS;
	struct timeval req_timeout; /* timeval for select */

//	FD_ZERO(&readFdSet);
//	FD_ZERO(&writeFdSet);
	
//	while(ctrlConn)
	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{
//		FD_SET(ctrlConn->sockCtrl, &readFdSet);
//		if(ctrlConn->sockCtrl > maxFd)
//			maxFd = ctrlConn->sockCtrl;
		
		fdCount += 1 + ctrlConn->connCount*2; /* 2 fd for every data connection */
//		ctrlConn = ctrlConn->next;
	}


	/* Need to allocate one whole extra block of fds for UDS. */
	pollfd = cmn_malloc( fdCount*sizeof(struct pollfd));
	if (!pollfd)
	{
		return -1;
	}

	pfd = pollfd;
	index = 0;
	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{
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

	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{
		index = 0;
		LIST_FOREACH(dataConn, &ctrlConn->dataConns, list)
		{
			if(dataConn->ctrlConn->type !=  CTRL_LINK_UDP)
			{
				/* socket */
				pfd->fd = dataConn->sock;
				pfd->events = POLLIN|POLLPRI| POLLRDHUP|POLLERR;
				pfd++;

				/* timer */
				extSetTimer(dataConn->timeFd, TIMER_FD_TIMEOUT);
				pfd->fd = dataConn->timeFd;
				pfd->events = POLLIN;
				pfd++;
				
				EXT_DEBUGF(MUX_DEBUG_BROKER, "DATA CONN#%d.%p", index, dataConn);
				index++;
			}
		}
		
		if(index != ctrlConn->connCount)
		{
			EXT_ERRORF("Number of Data Connection in Ctrl Connection %s not correct: %d != %d", 
				(ctrlConn->type == CTRL_LINK_TCP)?"TCP":(ctrlConn->type == CTRL_LINK_UDP)?"UDP":"UNIX", index, ctrlConn->connCount );
		}
	}

	req_timeout.tv_sec = 5;
	req_timeout.tv_usec = 0l; /* reset timeout */

//	res = select(maxFd + 1, &readFdSet, &writeFdSet, NULL, &req_timeout);
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
	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{
		if( pfd->revents & (POLLIN|POLLPRI))
		{
			int isClose = TRUE;
			dataConn = ctrlConn->handleCreateData(ctrlConn);
			TRACE();

#if 1			
			if(dataConn == NULL)
			{
				continue;
			}
			
			/* socket */
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
			MUX_DEBUG( "Read data from peer port %d", dataConn->port );
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
				if( cmnMuxControllerAddEvent(dataConn->cmd, dataConn->method, dataConn) == EXIT_SUCCESS)
				{
					isClose = FALSE;
				}
				else
				{
					DATA_CONN_ERR(dataConn, IPCMD_ERR_NOT_SUPPORT_COMMND,  "Command '%s' is not supportted", dataConn->cmd);
					dataConn->isFinished = TRUE;
				}
			}

#if 1
			cmn_mutex_unlock(dataConn->mutexLock);

			if(isClose)
			{
				MUX_DEBUG( "Broker reply DATA_CONN %p directly", dataConn);
				dataConn->handleOutput(dataConn);
					
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
				MUX_DEBUG( "Broker finish this DATA_CONN %p", dataConn);
#endif
				dataConn->handleDestroy(dataConn);
				dataConn = NULL;
			}
#else			
			//if(dataConn->isFinished)
			if(dataConn->errCode != IPCMD_ERR_IN_PROCESSING)
			{/* not finished, ie. this dataConn is end to other process */
				cmn_mutex_unlock(dataConn->mutexLock);
				
				dataConn->handleDestroy(dataConn);
				dataConn = NULL;
			}
			else 
			{/* otherwise, send data to other thread to process it */
				cmn_mutex_unlock(dataConn->mutexLock);
			}
#endif				

#endif
		}
		else if(pfd->revents & (POLLRDHUP|POLLERR))
		{
			EXT_ERRORF("Ctrl Connection is broken");
			exit(1);
		}

		pfd++;
	}

	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{
		LIST_FOREACH(dataConn, &ctrlConn->dataConns, list)
		{
			int isClose = TRUE;
			
			/* socket */
			if( pfd->revents & (POLLIN|POLLPRI))
			{
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
				MUX_DEBUG( "Read data from peer port %d", dataConn->port );
#endif
				dataConn->handleInput(dataConn);

				cmn_mutex_lock(dataConn->mutexLock);
				if(dataConn->errCode == IPCMD_ERR_NOERROR )
				{
					if( cmnMuxControllerAddEvent(dataConn->cmd, dataConn->method, dataConn) == EXIT_SUCCESS)
					{
						isClose = FALSE;
					}
					else
					{
						DATA_CONN_ERR(dataConn, IPCMD_ERR_NOT_SUPPORT_COMMND,  "Command '%s' is not supportted", dataConn->cmd);
						dataConn->isFinished = TRUE;
					}
				}

			TRACE();
#if 1
				cmn_mutex_unlock(dataConn->mutexLock);

				if(isClose)
				{
					MUX_DEBUG( "Broker reply DATA_CONN %p directly", dataConn);
					dataConn->handleOutput(dataConn);
					
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
					MUX_DEBUG( "Broker finish this DATA_CONN %p", dataConn);
#endif
					dataConn->handleDestroy(dataConn);
					dataConn = NULL;
				}
#else			
				//if(dataConn->isFinished)
				if(dataConn->errCode != IPCMD_ERR_IN_PROCESSING)
				{/* not finished, ie. this dataConn is end to other process */
					cmn_mutex_unlock(dataConn->mutexLock);
					
					dataConn->handleDestroy(dataConn);
					dataConn = NULL;
				}
				else 
				{/* otherwise, send data to other thread to process it */
					cmn_mutex_unlock(dataConn->mutexLock);
				}
#endif
			}
			else if(pfd->revents & (POLLRDHUP|POLLERR))
			{
				EXT_ERRORF("Data Connection broken")
			}
			pfd++;

			/* timer */
			if( pfd->revents & POLLIN )
			{
				EXT_ERRORF("Data Connection timeout")
			}
			
			index++;
		}
		
	}

	cmn_free(pollfd);

#if 0	
	ctrlConn = broker->ctrlConns;
	while(ctrlConn)
	{
		struct DATA_CONN *dataConn = NULL;
		if ( FD_ISSET(ctrlConn->sockCtrl, &readFdSet) )
		{
			int isClose = TRUE;
			dataConn = ctrlConn->handleCreateData(ctrlConn);
			if(dataConn == NULL)
			{
				return EXIT_SUCCESS;
			}
			
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
			MUX_DEBUG( "Read data from peer port %d", dataConn->port );
#endif
			dataConn->handleInput(dataConn);

			cmn_mutex_lock(dataConn->mutexLock);
			if(dataConn->errCode == IPCMD_ERR_NOERROR )
			{
				if( cmnMuxControllerAddEvent(dataConn->cmd, dataConn->method, dataConn) == EXIT_SUCCESS)
				{
					isClose = FALSE;
				}
				else
				{
					DATA_CONN_ERR(dataConn, IPCMD_ERR_NOT_SUPPORT_COMMND,  "Command '%s' is not supportted", dataConn->cmd);
					dataConn->isFinished = TRUE;
				}
			}

#if 1
			cmn_mutex_unlock(dataConn->mutexLock);

			if(isClose)
			{
				MUX_DEBUG( "Broker reply DATA_CONN %p directly", dataConn);
				dataConn->handleOutput(dataConn);
				
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
				MUX_DEBUG( "Broker finish this DATA_CONN %p", dataConn);
#endif
				dataConn->handleDestroy(dataConn);
				dataConn = NULL;
			}
#else			
			//if(dataConn->isFinished)
			if(dataConn->errCode != IPCMD_ERR_IN_PROCESSING)
			{/* not finished, ie. this dataConn is end to other process */
				cmn_mutex_unlock(dataConn->mutexLock);
				
				dataConn->handleDestroy(dataConn);
				dataConn = NULL;
			}
			else 
			{/* otherwise, send data to other thread to process it */
				cmn_mutex_unlock(dataConn->mutexLock);
			}
#endif

			
		}
		
		ctrlConn = ctrlConn->next;
	}
#endif

	LIST_FOREACH(ctrlConn, &broker->ctrlConns, list)
	{
		struct DATA_CONN *tmp;
		TRACE();
		LIST_FOREACH_SAFE(dataConn, &ctrlConn->dataConns, list, tmp)
		{
			if(dataConn->isFinished ==  EXT_TRUE)
			{
		TRACE();
				LIST_REMOVE(dataConn, list);
		TRACE();
				cmnMuxDataConnClose( dataConn);
		TRACE();
			}
		}
		
	}


	return EXIT_SUCCESS;
}

static int _muxBrokerMainLoop(CmnThread *th)
{
	int res = EXIT_SUCCESS;
	CMN_MUX_BROKER *broker =(CMN_MUX_BROKER *)th->data;

#if 0//ARCH_ARM
	if (SYSTEM_IS_EXITING())
	{
		MUX_WARN("Thread '%s' recv EXIT signal\n", th->name);
		return -EXIT_FAILURE;
	}
#endif

	res = _cmnMuxBrokerReceive(broker);
	
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
	name		:	"Broker",
	init			:	_initBrokerThread,
	mainLoop		:	_muxBrokerMainLoop,
	eventHandler	:	NULL,
	destory		:	_destoryBrokerThread,
	
	data			:	NULL,
};


