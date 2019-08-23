/*
* Client of socket server, eg. UDP/TCP/Unix socket client, send JSON request and receive jSON response.. 
* used in webAdmin (CGI)
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>

#include "libCmnSys.h"
#include "_cmnMux.h"


void cmnMuxClientConnDestroy(struct CLIENT_CONN *clientConn)
{
	close(clientConn->socket);
	cmn_free(clientConn);
}

struct CLIENT_CONN *cmnMuxClientConnCreate(CTRL_LINK_TYPE type, int port, char *address, int timeoutSecond)
{
	int res = EXIT_SUCCESS;
	struct	CLIENT_CONN *clientConn = NULL;
	struct sockaddr_in addr;
	int sock = 0;

	if(type == CTRL_LINK_UDP)
	{
		sock = socket(AF_INET, SOCK_DGRAM, 0);
	}
	else if(type == CTRL_LINK_TCP)
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);
	}
	else if(type == CTRL_LINK_UNIX)
	{
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
	}
	else
	{
		MUX_ERROR("Ctrl Connection type:'%d' is not support", type);
		exit(1);
	}
	
	if (sock < 0)
	{
		MUX_ERROR("Socket Error:%s", strerror(errno));
		return NULL;
	}

	if(CLIENT_TYPE_IS_IPCMD(type) )
	{
		struct timeval timeout;      
		timeout.tv_sec = timeoutSecond;
		timeout.tv_usec = 0;

		if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		{
			MUX_ERROR("Set socket REV timeout error:%s", strerror(errno));
		}

		if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		{
			MUX_ERROR("Set socket SEND timeout error:%s", strerror(errno));
		}

		//char broadcast = '1';
		int broadcast = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast) ) < 0)
		{
			MUX_ERROR("Set socket BOARDCAST error:%s", strerror(errno));
		}

		struct hostent *he;
		bzero(&addr, sizeof(struct sockaddr_in));
		he = gethostbyname(address);
		if (!he)
		{
			MUX_ERROR("Invalid host or ip address '%s'", address);
			return NULL;
		}
		else
		{
			/* Only take the first */
//			addr.s_addr = ntohl(((struct in_addr *) he->h_addr_list[0])->s_addr);
//			bcopy((char *)he->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
			memcpy(&addr.sin_addr, he->h_addr_list[0], sizeof(struct in_addr));
		}

		MUX_DEBUG("parse address %s into %s", address, cmnSysNetAddress((uint32_t)addr.sin_addr.s_addr));

		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
//		addr.sin_addr.s_addr = INADDR_BROADCAST;

		if((type) == CTRL_LINK_TCP )
		{
			res = connect(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
		}

	}
	else
	{/* unix socket */

		int len;
		struct sockaddr_un server_sockaddr; 
		struct sockaddr_un client_sockaddr; 	

		memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
		memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));

		client_sockaddr.sun_family = AF_UNIX;
		strcpy(client_sockaddr.sun_path, UNIX_SOCKET_CLIENT_PATH);
		len = sizeof(client_sockaddr);

		unlink(UNIX_SOCKET_CLIENT_PATH);
		res = bind(sock, (struct sockaddr *) &client_sockaddr, len);
		if (res == -1)
		{
			MUX_ERROR("BIND ERROR in client Unix socket: %s\n", strerror(errno));
			close(sock);
			return NULL;
		}
		
		char *socket_path = UNIX_SOCKET_SVR_PATH;
		server_sockaddr.sun_family = AF_UNIX;

		if ( 0)//*socket_path == '\0')
		{
			*server_sockaddr.sun_path = '\0';
			strncpy(server_sockaddr.sun_path+1, socket_path+1, sizeof(server_sockaddr.sun_path)-2);
		}
		else
		{
			strncpy(server_sockaddr.sun_path, socket_path, sizeof(server_sockaddr.sun_path)-1);
		}

		res = connect(sock, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr));
		if(res < 0)
		{
			MUX_ERROR("Bind Error:%s", strerror(errno));
			close(sock);
			return NULL;
		}
	}

	if(res < 0)
	{
		MUX_ERROR("Bind Error:%s", strerror(errno));
		close(sock);
		return NULL;
	}

	clientConn = cmn_malloc(sizeof(struct CLIENT_CONN));
	clientConn->socket = sock;
	clientConn->type = type;
	clientConn->port = port;
	clientConn->errCode = IPCMD_ERR_NOERROR;
	
	if(CLIENT_TYPE_IS_IPCMD(type) )
	{
		clientConn->addrlen = sizeof(struct sockaddr);
		memcpy(&clientConn->peerAddr, &addr, clientConn->addrlen);
	}

	return clientConn;
}


int cmnMuxClientConnSendRequest(struct CLIENT_CONN *clientConn, void *buf, int size)
{
	int len = 0;
	CMN_IP_COMMAND cmd;
	unsigned int crc32 =0;

	void *sendBuf = buf;
	int sendSize = size;

	if(CLIENT_TYPE_IS_IPCMD(clientConn->type) )
	{
		cmd.tag = CMD_TAG_A;
		cmd.length = htons(size + 4);

		memcpy( cmd.data, buf, size);
		crc32 =  htonl(cmnMuxCRC32b(buf, size));
		*((unsigned int *)(cmd.data+size)) = crc32;
		
		sendBuf = &cmd;
		sendSize = size+8;
		
#if MUX_OPTIONS_DEBUG_IPC_CLIENT			
		MUX_DEBUG( "CtrlClient send %d bytes packet to %s:%d, CRC=0x%x", sendSize, inet_ntoa(clientConn->peerAddr.sin_addr), ntohs(clientConn->peerAddr.sin_port), crc32);
		CMN_HEX_DUMP((const uint8_t *)sendBuf, sendSize, "Client send out data" );
#endif
	}

	if(clientConn->type == CTRL_LINK_TCP || clientConn->type == CTRL_LINK_UNIX)
	{
		len = write(clientConn->socket, sendBuf, sendSize );
		if (len < 0)
		{
			MUX_ERROR("ERROR writing to socket: '%s'", strerror(errno));
		}
		//close(clientConn->socket);
	}
	else if(clientConn->type == CTRL_LINK_UDP)
	{
		len = sendto(clientConn->socket, sendBuf, sendSize, 0, (struct sockaddr *)&clientConn->peerAddr, clientConn->addrlen); 
	}

	return len;
}


cJSON *cmnMuxClientConnReadReponse(struct CLIENT_CONN *clientConn)
{
	unsigned int crcDecoded, crcReceived;
	cJSON	*responseObjs = NULL;
	int len = 0;

	CMN_IP_COMMAND responseBuf;
	char *recvBuf = (char *)&responseBuf;
	
	memset(&responseBuf, 0, sizeof(CMN_IP_COMMAND));

	if(clientConn->type == CTRL_LINK_TCP || clientConn->type == CTRL_LINK_UNIX)
	{
#if 1	
		len = read(clientConn->socket, (uint8_t *)&responseBuf, sizeof(CMN_IP_COMMAND));
#else
		len = recv(clientConn->socket, (uint8_t *)&responseBuf, sizeof(CMN_IP_COMMAND), 0);
#endif

#if MUX_OPTIONS_DEBUG_IPC_CLIENT			
		MUX_DEBUG("TCP/UNIX Received %d bytes packet from %s:%d", len, inet_ntoa(clientConn->peerAddr.sin_addr), ntohs(clientConn->peerAddr.sin_port));
#endif
	}
	else
	{
		len = recvfrom(clientConn->socket, (uint8_t *)&responseBuf, sizeof(CMN_IP_COMMAND), 0, (struct sockaddr *) &clientConn->peerAddr, &clientConn->addrlen);
#if MUX_OPTIONS_DEBUG_IPC_CLIENT			
		MUX_DEBUG("UDP Received %d bytes packet from %s:%d", len, inet_ntoa(clientConn->peerAddr.sin_addr), ntohs(clientConn->peerAddr.sin_port));
#endif
	}
	
	if(len == -1 && (errno == EAGAIN || errno== ESPIPE ||errno== EINPROGRESS||errno== EWOULDBLOCK))
	{ // timed out before any data read : EINTR
		MUX_WARN("Timeout for this command from %s:%d", inet_ntoa(clientConn->peerAddr.sin_addr), ntohs(clientConn->peerAddr.sin_port));
		return NULL;
	}
	else if(len == 0)
	{
		MUX_WARN("Connection is shutdown by peer" );
//		errCode = IPCMD_ERR_COMMUNICATION;
		return NULL;
	}
	else if(len < 0)
	{
		MUX_ERROR("Receive data error: %s(%d)", strerror(errno), errno);
		return NULL;
	}


	if(CLIENT_TYPE_IS_IPCMD(clientConn->type) )
	{
		MUX_DEBUG("Received %d bytes packet from %s:%d", len, inet_ntoa(clientConn->peerAddr.sin_addr), ntohs(clientConn->peerAddr.sin_port));
#if MUX_OPTIONS_DEBUG_IPC_CLIENT			
		CMN_HEX_DUMP( (uint8_t *)&responseBuf, len, "client received data");
#endif

		responseBuf.length = ntohs(responseBuf.length);

		if (CMD_TAG_B != responseBuf.tag)
		{
			MUX_ERROR("Tag of command is wrong:0x%x", responseBuf.tag);
			clientConn->errCode = IPCMD_ERR_WRONG_PROTOCOL;
			return NULL;
		}

		if(responseBuf.length+4 != len )
		{
			MUX_ERROR("length field:%d, received length:%d", responseBuf.length, len);
			clientConn->errCode = IPCMD_ERR_CRC_FAIL;
			return NULL;
		}
		
		crcReceived = *((unsigned int *)(responseBuf.data+responseBuf.length-4));	/* the first 4 bytes are header of command, so here is the position of CRC */
		crcDecoded = htonl(cmnMuxCRC32b(responseBuf.data, responseBuf.length-4));
		if (crcReceived != crcDecoded)
		{//Wrong CRC
			//sendto(sockCtrl, msg_reply, n /*+ 8*/, 0, (struct sockaddr *) &addr, addrlen); //  if suceeded the updated data is send back
			MUX_ERROR("CRC of command is wrong:received CRC: 0x%x, Decoded CRC:0x%x", crcReceived, crcDecoded);
			clientConn->errCode = IPCMD_ERR_CRC_FAIL;
			return NULL;
		}
#if MUX_OPTIONS_DEBUG_IPC_CLIENT			
		MUX_DEBUG("received CRC: 0x%x, Decoded CRC:0x%x", crcReceived, crcDecoded);
#endif
		recvBuf = responseBuf.data;
	}
	else
	{
	}

	responseObjs = cJSON_Parse((char *)recvBuf );
	if( responseObjs == NULL)
	{/*Wrong JSON format */
		MUX_ERROR("JSON of command is wrong:received JSON: '%s'", (char *)recvBuf );
		clientConn->errCode = IPCMD_ERR_JSON_CORRUPTED;
	}

	return responseObjs;
}

#if 0
static cJSON *findJSonObject(cJSON *array, char *key, char *valuestring)
{
	int i;

	if(!cJSON_IsArray(array))
	{
		MUX_ERROR("cJSON is not an array object");
		return NULL;
	}

	for (i = 0; i < cJSON_GetArraySize(array); i++)
	{

		cJSON *subitem = cJSON_GetArrayItem(array, i);
		if(!subitem)
		{
			MUX_ERROR("IP Command configuration initialization error" );
			return NULL;
		}
		
		cJSON *temp = cJSON_GetObjectItem(subitem, key);
		if( !strcasecmp(valuestring, temp->valuestring) )
		{
			return subitem;
		}
	}

	return NULL;
}
#endif


CLIENT_CTRL _clientCtrl;

int cmnMuxClientInit(int port, CTRL_LINK_TYPE type, char *serverAddress, int timeoutSecond)
{
	CLIENT_CTRL	*clientCtrl = &_clientCtrl;
	struct CLIENT_CONN *clientConn;

	memset(clientCtrl, 0, sizeof(CLIENT_CTRL));

	clientConn = cmnMuxClientConnCreate(type, port, serverAddress, timeoutSecond);
	if(clientConn == NULL)
	{
		MUX_ERROR("Connect to '%s:%d' failed", serverAddress, port);
		cmnMuxClientConnDestroy(clientConn);
		return EXIT_FAILURE;
	}

	clientCtrl->conn = clientConn;
	clientCtrl->inited = TRUE;
	
	return EXIT_SUCCESS;
}


void cmnMuxClientDestroy(void)
{
	CLIENT_CTRL	*clientCtrl = &_clientCtrl;

	cmnMuxClientConnDestroy(clientCtrl->conn);
}

/*
* replace the 'action' object in xxxxAction.json, and send to next step
*/
cJSON *cmnMuxClientRequest(cJSON *ipCmd)
{
	CLIENT_CTRL *clientCtrl = &_clientCtrl;
	int res = EXIT_SUCCESS;
	cJSON *response = NULL;//, *dataArray;

	char *msg = cJSON_PrintUnformatted(ipCmd);

	/* output to debug in format */
	char *printed_json = cJSON_Print(ipCmd);
//	MUX_DEBUG("Client sendout ipCmd:\n'%s'\n", printed_json);
	printf("Client sendout ipCmd:\n'%s'\n", printed_json);
	cmn_free(printed_json);

	res = cmnMuxClientConnSendRequest(clientCtrl->conn, msg, strlen(msg));
	cmn_free(msg);

	if(res == -1 && errno == EINTR)
	{ // timed out before any data read
		MUX_ERROR("Timeout when send out command to %s:%d. Please check your parameters", inet_ntoa(clientCtrl->conn->peerAddr.sin_addr), ntohs(clientCtrl->conn->peerAddr.sin_port));
		return NULL;
	}
	if(res <= 0)
	{
		MUX_ERROR("IP command send failed, maybe the API server is not running!");
		return NULL;
	}

	response = cmnMuxClientConnReadReponse(clientCtrl->conn);

#if 0
	{
		if(response )
		{
			char *printed_json = cJSON_Print(response);
			MUX_DEBUG("Client received reply:\n'%s'\n", printed_json);
			cmn_free(printed_json);
		}
	}
#endif

	return response;
}


