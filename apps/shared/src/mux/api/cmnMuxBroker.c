
/*
* Controller: 
*   1. communicate with controller 811; 
*   2. finish the commands (FTP download commands) 
*   3. or send to commands to correspoding threads, such as Play/Stop
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "libCmn.h"
#include "libMedia.h"
#include "libMux.h"

#include "_cmnMux.h"

/* read command and parse cmdObj for every DATA_CONN */
static struct DATA_CONN *_createDataConnection(struct CTRL_CONN *ctrlConn)
{
	int dataSocket = 0;
	struct DATA_CONN *dataConn = NULL;
	struct sockaddr_in		peerAddr;
//	struct sockaddr 		peerAddr;
	socklen_t				addrlen;
	int len = 0;
	unsigned char *objBuffer;
	
	memset(&ctrlConn->buffer, 0, sizeof(ctrlConn->buffer) );

	addrlen = sizeof(struct sockaddr);
	if(ctrlConn->type == CTRL_LINK_TCP || ctrlConn->type == CTRL_LINK_UNIX)
	{
		dataSocket = accept(ctrlConn->sockCtrl, (struct sockaddr *) &peerAddr, &addrlen);
		if(dataSocket < 0)
		{
			MUX_ERROR("accept Error:%s", strerror(errno));
			return NULL;
		}
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("%s connection accepted", ((ctrlConn->type == CTRL_LINK_TCP))?"TCP":"UNIX");
#endif
		len = read(dataSocket, (uint8_t *)ctrlConn->buffer, sizeof(ctrlConn->buffer));
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
		len = recvfrom(ctrlConn->sockCtrl, (uint8_t *)ctrlConn->buffer, sizeof(ctrlConn->buffer), 0, (struct sockaddr *) &peerAddr, &addrlen);
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("UDP Received %d bytes packet from %s:%d, addrLen:%d", len, inet_ntoa(peerAddr.sin_addr), ntohs(peerAddr.sin_port), addrlen );
#endif
	}

	if(len == 0)
	{
		MUX_INFO("Connection is broken by client");
		return NULL;
	}
	if(len<0)
	{
		MUX_ERROR("Receive data error: %s", strerror(errno));
//		errCode = IPCMD_ERR_COMMUNICATION;
		return NULL;
	}
	
	ctrlConn->length = len;
	ctrlConn->cmdBuffer = (CMN_IP_COMMAND *)ctrlConn->buffer;

#if 1//MUX_OPTIONS_DEBUG_IP_COMMAND
	CMN_HEX_DUMP( (uint8_t *)ctrlConn->cmdBuffer, ctrlConn->length, "Received data from socket" );
#endif

	dataConn = cmn_malloc(sizeof(struct DATA_CONN));

	dataConn->errCode = IPCMD_ERR_NOERROR;
	dataConn->sock = dataSocket;
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

	return dataConn;
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
		socket_path = IS_STRING_NULL(muxMain->unixPort)? UNIX_SOCKET_SVR_PATH:muxMain->unixPort;
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



#define	BROKER_ADD_CTRL_CONN(broker, ctrlConn) \
	{ if((ctrlConn)==NULL){exit(1);} (ctrlConn)->broker =(broker); if((broker)->ctrlConns==NULL) {(broker)->ctrlConns=(ctrlConn);} \
	else{(ctrlConn)->next=(broker)->ctrlConns; (broker)->ctrlConns = (ctrlConn);} }
	

static CMN_MUX_BROKER *_cmnMuxBrokerInit(MuxMain *muxMain)
{
	struct CTRL_CONN *ctrlConn = NULL;
	CMN_MUX_BROKER *broker = NULL;


	broker = cmn_malloc(sizeof(CMN_MUX_BROKER));

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
	BROKER_ADD_CTRL_CONN(broker, ctrlConn);

	broker->muxMain = muxMain;
//	broker->cfg = muxMain-> cfg;

	return broker;
}


static void _cmnMuxBrokerDestroy(CMN_MUX_BROKER *broker)
{
	struct CTRL_CONN *ctrlConn = broker->ctrlConns;

	while(ctrlConn)
	{
		struct CTRL_CONN *tmp = ctrlConn;
		ctrlConn = tmp->next;
		
		close(tmp->sockCtrl);
		cmn_free(tmp);
	}

	cmn_free(broker);
}



static int _cmnMuxBrokerReceive(CMN_MUX_BROKER *broker)
{
	struct CTRL_CONN *ctrlConn = broker->ctrlConns;
	fd_set				readFdSet;
	fd_set				writeFdSet;
	int					maxFd = 0;
	int	res = EXIT_SUCCESS;
	struct timeval req_timeout; /* timeval for select */

	FD_ZERO(&readFdSet);
	FD_ZERO(&writeFdSet);
	
	while(ctrlConn)
	{
		FD_SET(ctrlConn->sockCtrl, &readFdSet);
		if(ctrlConn->sockCtrl > maxFd)
			maxFd = ctrlConn->sockCtrl;
		
		ctrlConn = ctrlConn->next;
	}

	req_timeout.tv_sec = 5;
	req_timeout.tv_usec = 0l; /* reset timeout */

	res = select(maxFd + 1, &readFdSet, &writeFdSet, NULL, &req_timeout);
	if (res == -1 )
	{
		/* what is the appropriate thing to do here on EBADF */
		if (errno == EINTR)
			return EXIT_SUCCESS;
		else if (errno != EBADF)
		{
			MUX_ERROR("select system call failed: '%s'", strerror(errno));
			return EXIT_FAILURE;
		}
	}
	else if(res == 0)/* timeout */
	{
//		MUX_DEBUG("Timeout of socket");
		return EXIT_SUCCESS;
	}

	
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


