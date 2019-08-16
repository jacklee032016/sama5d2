
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

static int _getRequestMethod(cJSON *reqObj)
{
	char *method = cmnGetStrFromJsonObject(reqObj, MUX_REST_METHOD);
	if( IS_STRING_NULL_OR_ZERO(method) )
	{
		return CMN_JSON_METHOD_INVALIDATE;
	}
	
	if(IS_STRING_EQUAL(method, CMN_MEHOD_STR_GET) )
	{
		return CMN_JSON_METHOD_GET;
	}
	else if(IS_STRING_EQUAL(method, CMN_MEHOD_STR_POST) )
	{
		return CMN_JSON_METHOD_POST;
	}
	else if(IS_STRING_EQUAL(method, CMN_MEHOD_STR_PUT) )
	{
		return CMN_JSON_METHOD_PUT;
	}
	else if(IS_STRING_EQUAL(method, CMN_MEHOD_STR_DELETE) )
	{
		return CMN_JSON_METHOD_DELETE;
	}
	else if(IS_STRING_EQUAL(method, CMN_MEHOD_STR_PATCH) )
	{
		return CMN_JSON_METHOD_PATCH;
	}

	return CMN_JSON_METHOD_INVALIDATE;
}


void cmnMuxDataConnClose(struct DATA_CONN *dataConn)
{
	if( dataConn->cmdObjs)
	{
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("cJSON_Delete cmdObjs:%p", dataConn->cmdObjs);	
#endif
		cJSON_Delete( dataConn->cmdObjs);
	}

	/* resultObj always refers to dataObj in cmdObj or object in SystemObj */
#if 0	
	if( dataConn->resultObject)
	{/* when is it been freed?? */
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("cJSON_Delete resultObject:%p", dataConn->resultObject);	
#endif
//		cJSON_Delete( dataConn->resultObject);
	}
#endif

	if(dataConn->sock > 0) //dataConn->ctrlConn->type != CTRL_LINK_UDP)
	{
		close(dataConn->sock);
	}
	
	close(dataConn->timeFd);
	cmn_mutex_destroy(dataConn->mutexLock);

#if __DEBUG_CONNECTION
	EXT_DEBUGF(MUX_DEBUG_BROKER, "free DATA CONN:%s:%p", dataConn->name, dataConn);	
#endif
	cmn_free(dataConn);
}



static int _dataConnOutput(struct DATA_CONN *dataConn, void *buf, int size)
{
	int len = 0;

#if 0//MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG("Reply %d bytes packet to %s", size, dataConn->name);
	CMN_HEX_DUMP((const uint8_t *)buf, size, "send out data in data connection" );
#endif

	if(dataConn->ctrlConn->type == CTRL_LINK_TCP || dataConn->ctrlConn->type == CTRL_LINK_UNIX)
	{
		len = write(dataConn->sock, buf, size );
		if (len < 0)
		{
			MUX_ERROR("ERROR writing to socket: '%s'", strerror(errno));
		}
		// ? in destroy handler
		//close(dataConn->sock);
	}
	else if(dataConn->ctrlConn->type == CTRL_LINK_UDP)
	{
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("Reply %d bytes packet to %s:%d, addrLen:%d:"EXT_NEW_LINE"'%.*s'", 
			size, inet_ntoa(dataConn->peerAddr.sin_addr), ntohs(dataConn->peerAddr.sin_port), dataConn->addrlen, size-8, buf+4 );
#endif
		len = sendto(dataConn->ctrlConn->sockCtrl, buf, size, 0, (struct sockaddr *)&dataConn->peerAddr, dataConn->addrlen); //  if suceeded the updated data is send back
	}

	if(len != size || len < 0 )
	{
		MUX_ERROR("ERROR writing to socket only %d bytes: '%s'", len, strerror(errno));
	}

	if(CONTROLLER_IS_DEBUG(dataConn) )
	{
		if(dataConn->ctrlConn->type == CTRL_LINK_UNIX)
		{
			MUX_INFO("Replied %d bytes to %s"EXT_NEW_LINE"\'%.*s'", len, dataConn->name, size, buf );
		}
		else
		{
			MUX_INFO("Replied %d bytes to %s"EXT_NEW_LINE"\'%.*s'", len, dataConn->name, size-8, buf+4);
		}
	}

	return len;
}



/* controller interface of sending back all JSON reply which is in format of buf. Only interface from controller called by other modules  */
static int _dataConnIpCmdOutput(struct DATA_CONN *dataConn, void *buf, int size)
{
	CMN_IP_COMMAND cmd;
	unsigned int crc32 =0;
	char *sendBuffer;
	int sendSize;
	
	cmd.tag = CMD_TAG_B;
	cmd.length =  htons(size + 4);

	memcpy( cmd.data, buf, size);
	crc32 =  htonl(cmnMuxCRC32b(buf, size));
	*(unsigned int *)(cmd.data+size) = crc32;

	sendBuffer = (char *)&cmd;
	sendSize = size + 8;
	return _dataConnOutput(dataConn, sendBuffer, sendSize);
}


/* Reply JSON object in any threads (context of controller and other threads) and 
* all cases: OK or error after DATA_CONN->cmdObjs has been parsed in _read_command 
*/
int	cmnMuxDataConnIpCmdOutput(struct DATA_CONN *dataConn)
{
	int len = 0;
	cJSON *dataArray = NULL;
	cJSON *responseObj = NULL;
	char *msg = NULL;

	if(! DATA_CONN_IS_IPCMD(dataConn) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_JSON_CORRUPTED, "Output interface of IP command can not be used by REST API");
	}

	if(dataConn->cmdObjs == NULL)
	{ /* error before JSON object is parsed*/
		responseObj = cJSON_CreateObject();
		cJSON_AddStringToObject(responseObj, IPCMD_NAME_KEYWORD_TARG, "FF:FF:FF:FF:FF:FF");
		cJSON_AddStringToObject(responseObj, IPCMD_NAME_KEYWORD_CMD, "Unknown");
		cJSON_AddStringToObject(responseObj, IPCMD_NAME_KEYWORD_LOGIN_ACK, "NOK");
		cJSON_AddStringToObject(responseObj, IPCMD_NAME_KEYWORD_PWD_MSG, MUX_JSON_ERROR_STR(dataConn->errCode) );
		cJSON_AddStringToObject(responseObj, IPCMD_NAME_KEYWORD_MSG_DETAIL, IS_STRING_NULL_OR_ZERO(dataConn->detailedMsg)?"Command is not validate format": dataConn->detailedMsg );

		goto sendout;
	}
	
	if(dataConn->errCode == IPCMD_ERR_NOERROR)
	{
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_LOGIN_ACK, cJSON_CreateString(EXT_IPCMD_STATUS_OK));
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_PWD_MSG, cJSON_CreateString(EXT_IPCMD_STATUS_OK));
		
		if( (dataConn->errCode == IPCMD_ERR_NOERROR ||dataConn->errCode == IPCMD_ERR_FTP_PARTLY_FAILED) && dataConn->resultObject != NULL )
		{
			/* if DATA exists, then remove it */
//			if(dataConn->dataObj && dataConn->dataObj != dataConn->resultObject )
			if(dataConn->dataObj != dataConn->resultObject )
			{/* if resultObject is not dataObj, replace dataObj in CmdObj with resultObj */
				cJSON_DeleteItemFromObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_DATA);
				
				dataArray = cJSON_Duplicate(dataConn->resultObject, 1);
				cJSON_AddItemToObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_DATA, dataArray );
			}
			/* default, resultObj is alias of dataObj in CmdObj */
		}

	}
	else
	{
		char *errMsg = MUX_JSON_ERROR_STR(dataConn->errCode);
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_LOGIN_ACK, cJSON_CreateString(EXT_IPCMD_STATUS_FAIL));
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_PWD_MSG, cJSON_CreateString(errMsg));
		cJSON_AddStringToObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_MSG_DETAIL, IS_STRING_NULL_OR_ZERO(dataConn->detailedMsg)?"Command is not validate format": dataConn->detailedMsg );

		if(dataConn->dataObj != NULL )
		{/* dataObj is data obj in CmdObj */
			cJSON_DeleteItemFromObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_DATA);
		}
	}
	
	responseObj = dataConn->cmdObjs;

sendout:
	EXT_ASSERT((responseObj), "DataConnect response object is null ");
	if(responseObj == NULL)
	{
		EXT_ERRORF("DataConnect response object is null ");
	}
#if __DEBUG_CONNECTION
	EXT_DEBUGF(MUX_DEBUG_BROKER, "CONN %s output obj#%p", dataConn->name, responseObj);
#endif

	msg = cJSON_PrintUnformatted(responseObj);
//	EXT_DEBUGF(MUX_DEBUG_BROKER, "CONN %s output msg:'%s'", dataConn->name, msg);
	if(msg == NULL)
	{
		msg = cmn_malloc(128);
		snprintf(msg, 128, "%s", "Unknown");
	}

	cmn_mutex_lock(dataConn->mutexLock);
	if(dataConn->status == DATA_CONN_STATUS_INIT || dataConn->status == DATA_CONN_STATUS_WAITING)
	{/* called by Broker and Manager */
		len = _dataConnIpCmdOutput(dataConn, msg, strlen(msg));
	}
	else
	{
		len = strlen(msg);
	}
	dataConn->status = DATA_CONN_STATUS_FINISHED;
	cmn_mutex_unlock(dataConn->mutexLock);
	
	
	cmn_free(msg);
	if(responseObj != dataConn->cmdObjs && responseObj != NULL)
	{
		cJSON_Delete(responseObj);
	}
	
	return len;
}

int cmnMuxDataConnRestOutput(struct DATA_CONN *dataConn)
{
	char *msg = NULL;
	int len = 0;

	if(DATA_CONN_IS_IPCMD(dataConn) )
	{/* ???, can not reply here */
		DATA_CONN_ERR(dataConn, IPCMD_ERR_JSON_CORRUPTED, "Output interface of REST API can not be used by IP Command");
		return -EXIT_FAILURE;
	}

	if(dataConn->errCode == IPCMD_ERR_NOERROR)
	{
//		msg = cJSON_PrintUnformatted(dataConn->cmdObjs);
		msg = cJSON_PrintUnformatted(dataConn->resultObject);
	}
	else
	{
		cJSON *resultObj = cJSON_CreateObject();
		char *errMsg = MUX_JSON_ERROR_STR(dataConn->errCode);

		JEVENT_ADD_INTEGER(resultObj, MUX_REST_STATUS_CODE, dataConn->errCode );
		JEVENT_ADD_STRING(resultObj, MUX_REST_STATUS_ERROR, errMsg);
		JEVENT_ADD_STRING(resultObj, MUX_REST_STATUS_DEBUG, dataConn->detailedMsg );
		
		msg = cJSON_PrintUnformatted(resultObj);

		cJSON_Delete(resultObj);
	}

	cmn_mutex_lock(dataConn->mutexLock);

	if(msg)
	{
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("REST output :'%s'", msg);
#endif
		if(dataConn->status == DATA_CONN_STATUS_INIT || dataConn->status == DATA_CONN_STATUS_WAITING)
		{/* called by Broker and Manager both */
			len = _dataConnOutput(dataConn, msg, strlen(msg));
		}
		else
		{
			len = strlen(msg);
		}
		cmn_free(msg);
	}
	

	dataConn->status = DATA_CONN_STATUS_FINISHED;
	cmn_mutex_unlock(dataConn->mutexLock);
	
	return len;
}

static cJSON *_dataConnInput(struct DATA_CONN *dataConn, void *buf, int size)
{
	cJSON *cmdObjs = cJSON_Parse((char *)buf );
	
	if( cmdObjs == NULL)
	{/*Wrong JSON format */
		DATA_CONN_ERR(dataConn, IPCMD_ERR_JSON_CORRUPTED,"JSON of command is wrong:received JSON: '%s'", (char *)buf) ;
		return NULL;
	}

	return cmdObjs;
}


static int _ipCmdIsLocal(struct DATA_CONN *dataConn)
{
	char *target;	
	EXT_MAC_ADDRESS macAddress;
	MuxMain *muxMain = SYS_MAIN(dataConn);

	target = cmnGetStrFromJsonObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_TARG);
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG("target: %s", target );
#endif

	if(extMacAddressParse(&macAddress, target) == EXIT_FAILURE)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' : '%s' is not a validate MAC address", IPCMD_NAME_KEYWORD_TARG, target);
		return EXIT_FAILURE;
	}

	if(MAC_ADDR_IS_BOARDCAST(&macAddress))
	{
		/* */
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' : '%s' is board address when command is not 'get_param'", IPCMD_NAME_KEYWORD_TARG, target);
		return EXIT_FAILURE;
	}

	if(!MAC_ADDR_IS_EQUAL(&macAddress, &muxMain->runCfg.local.mac) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' : '%s' is not my MAC address", IPCMD_NAME_KEYWORD_TARG, target);
		MUX_ERROR("My MAC address:%02x:%02x:%02x:%02x:%02x:%02x", 
			muxMain->runCfg.local.mac.address[0], muxMain->runCfg.local.mac.address[1], muxMain->runCfg.local.mac.address[2], 
			muxMain->runCfg.local.mac.address[3], muxMain->runCfg.local.mac.address[4], muxMain->runCfg.local.mac.address[5]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static int _dataConnIpCmdAdapt(struct DATA_CONN *dataConn, cJSON *cmdObj)
{
/* must support hier data to support video/audio/anc data configuration */
#define	_IP_CMD_NO_HIER		0	/* data in IP command is in flat, no leval */

//	cJSON *newCmdObj = NULL;
	cJSON *_dataArrayObj= NULL;
	char *uri = NULL;
	char *cmdName = NULL;
	int isGetParams = FALSE;
	MuxMain *muxMain = SYS_MAIN(dataConn);

	dataConn->method = CMN_JSON_METHOD_INVALIDATE;

	/* handler process data object directly without any array */
	_dataArrayObj = cJSON_GetObjectItem(cmdObj, IPCMD_NAME_KEYWORD_DATA);
	if(cJSON_IsArray(_dataArrayObj))
	{
		dataConn->dataObj = cJSON_GetArrayItem(_dataArrayObj, 0);
	}
	else
	{
		dataConn->dataObj = _dataArrayObj;
	}
	
	cmdName = cmnGetStrFromJsonObject(cmdObj, IPCMD_NAME_KEYWORD_CMD);
	if( IS_STRING_NULL_OR_ZERO(cmdName) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Invalidate IP Command, without '%s'", IPCMD_NAME_KEYWORD_CMD );
		return EXIT_FAILURE;
	}

	if(IS_STRING_EQUAL(cmdName, IPCMD_NAME_GET_PARAM) )
	{
#if _IP_CMD_NO_HIER
		uri = MUX_REST_URI_ROOT MUX_REST_URI_SYSTEM;
#else
		uri = MUX_REST_URI_PARAMS;
#endif
		dataConn->method = CMN_JSON_METHOD_GET;
		isGetParams = TRUE;
	}
	else if(IS_STRING_EQUAL(cmdName, IPCMD_NAME_SET_PARAM) )
	{
#if _IP_CMD_NO_HIER
		uri = MUX_REST_URI_ROOT MUX_REST_URI_SYSTEM;
#else
		uri = MUX_REST_URI_PARAMS;
#endif
		dataConn->method = CMN_JSON_METHOD_POST;
	}
	else if(IS_STRING_EQUAL(cmdName, IPCMD_NAME_SEND_RS232) )
	{
		uri = MUX_REST_URI_ROOT MUX_REST_URI_RS232;
		dataConn->method = CMN_JSON_METHOD_POST;
	}
	else if(IS_STRING_EQUAL(cmdName, IPCMD_NAME_SECURITY_CHECK) )
	{
		uri = MUX_REST_URI_ROOT MUX_REST_URI_SECURITY;
		dataConn->method = CMN_JSON_METHOD_POST;
	}
	else if(IS_STRING_EQUAL(cmdName, IPCMD_NAME_SEND_IR) )
	{
		uri = MUX_REST_URI_ROOT MUX_REST_URI_IR;
		dataConn->method = CMN_JSON_METHOD_POST;
	}
	else if(IS_STRING_EQUAL(cmdName, IPCMD_NAME_SYS_ADMIN) )
	{
		uri = MUX_REST_URI_ROOT MUX_REST_URI_OTHERS;
		if(dataConn->dataObj)
		{
			dataConn->method = CMN_JSON_METHOD_POST;
		}
		else
		{
			dataConn->method = CMN_JSON_METHOD_GET;
			isGetParams = TRUE;
		}
	}
	else
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Unrecognized IP Command '%s'", cmdName );
		return EXIT_FAILURE;
	}

	snprintf(dataConn->cmd, sizeof(dataConn->cmd), "%s", uri);

	if( IS_STRING_NULL_OR_ZERO(dataConn->cmd) || dataConn->method == CMN_JSON_METHOD_INVALIDATE )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No validate cmd in IP Command, without '%s'", cmdName );
		return EXIT_FAILURE;
	}


	/* check even if get_params command is used */
	if(isGetParams == FALSE && muxMain->isAuthen)
	{
		if( _ipCmdIsLocal(dataConn)== EXIT_FAILURE)
		{
			return EXIT_FAILURE;
		}

		if(dataConn->dataObj == NULL && isGetParams == FALSE )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No validate data for IP Command '%s'", cmdName );
			return EXIT_FAILURE;
		}
	}
	
	return EXIT_SUCCESS;
}


int cmnMuxDataConnIpCmdInput(struct DATA_CONN *dataConn)
{
	unsigned int crcDecoded, crcReceived;
	cJSON *reqObj = NULL;
	char *username = NULL, *pwd = NULL;
	CMN_IP_COMMAND				*cmdBuffer;
	MuxMain *muxMain = SYS_MAIN(dataConn);

	if(! CONN_IS_IPCMD(dataConn->ctrlConn))
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_WRONG_PROTOCOL, "TCP/UDP socket can only be used in IP Command interface");
		return EXIT_FAILURE;
	}
	
	cmdBuffer = (CMN_IP_COMMAND *)dataConn->buffer;

	cmdBuffer->length = ntohs(cmdBuffer->length);
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG("received length: %d, length field: %d", dataConn->length, cmdBuffer->length);
#endif

	if(cmdBuffer->length+4 != dataConn->length )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_WRONG_PROTOCOL,"length field:%d, received length:%d", cmdBuffer->length, dataConn->length) ;
		return EXIT_FAILURE;
	}

	if (CMD_TAG_A != cmdBuffer->tag)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_WRONG_PROTOCOL, "Tag of command is wrong:0x%x", cmdBuffer->tag);
		return EXIT_FAILURE;
	}

	crcReceived = *(unsigned int *)(dataConn->buffer + cmdBuffer->length /*-4*/);	/* the first 4 bytes are header of command, so here is the position of CRC */
	crcDecoded = htonl(cmnMuxCRC32b(dataConn->buffer+4, cmdBuffer->length-4 /*-4*/));
	if (crcReceived != crcDecoded)
	{//Wrong CRC
		//sendto(sockCtrl, msg_reply, n /*+ 8*/, 0, (struct sockaddr *) &addr, addrlen); //  if suceeded the updated data is send back
		DATA_CONN_ERR(dataConn, IPCMD_ERR_CRC_FAIL,"CRC of command is wrong:received CRC: 0x%x, Decoded CRC:0x%x", crcReceived, crcDecoded) ;
		return EXIT_FAILURE;
	}
	
#if 0//MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG( "received CRC: 0x%x, Decoded CRC:0x%x", crcReceived, crcDecoded);
#endif

	reqObj =  _dataConnInput(dataConn, dataConn->buffer+4, cmdBuffer->length);
	if(reqObj == NULL)
	{/* error before JSON object is parsed */
		DATA_CONN_ERR(dataConn, IPCMD_ERR_BAD_REQUEST, "'%.*s' is not validate JSON data", cmdBuffer->length, dataConn->buffer+4);
		return EXIT_FAILURE;
	}

	dataConn->cmdObjs = reqObj;
	
	if((muxMain->isAuthen) && (dataConn->ctrlConn->type != CTRL_LINK_UNIX) )
	{
		username = cmnGetStrFromJsonObject(reqObj, IPCMD_NAME_KEYWORD_LOGIN_ACK);
		pwd = cmnGetStrFromJsonObject(reqObj, IPCMD_NAME_KEYWORD_PWD_MSG);
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("received username:'%s'; password:'%s'", username, pwd);
#endif
		if( IS_STRING_NULL_OR_ZERO(username)  || IS_STRING_NULL_OR_ZERO(pwd) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_UNAUTHORIZED, "No '%s' or '%s' in request", IPCMD_NAME_KEYWORD_LOGIN_ACK, IPCMD_NAME_KEYWORD_PWD_MSG);
			return EXIT_FAILURE;
		}

		if(dataConn->handleAuthen(dataConn, username, pwd) == EXIT_FAILURE) 
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_UNAUTHORIZED, "'%s/%s' authenticatio failed", username, pwd);
			return EXIT_FAILURE;
		}
	}

	if(_dataConnIpCmdAdapt(dataConn, reqObj) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


int cmnMuxDataConnRestInput(struct DATA_CONN *dataConn)
{
	cJSON *reqObj = NULL, *_dataArrayObj = NULL;
	char *username = NULL, *pwd = NULL;
	char *uri = NULL;
	MuxMain *muxMain = SYS_MAIN(dataConn);

	if(CONN_IS_IPCMD(dataConn->ctrlConn))
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_WRONG_PROTOCOL, "Unix socket can only be used in REST interface");
		return EXIT_FAILURE;
	}

	reqObj = _dataConnInput(dataConn, dataConn->buffer, dataConn->length);
	if(reqObj == NULL)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_BAD_REQUEST, "'%.*s' is not validate JSON data", dataConn->length, dataConn->buffer);
		return EXIT_FAILURE;
	}
	dataConn->cmdObjs = reqObj;

	if( (muxMain->isAuthen ) && (dataConn->ctrlConn->type != CTRL_LINK_UNIX) )
	{/* unix socket is for python service, so no authen is needed */
		username = cmnGetStrFromJsonObject(reqObj, MUX_REST_USER_NAME);
		pwd = cmnGetStrFromJsonObject(reqObj, MUX_REST_USER_PWD);
		if( IS_STRING_NULL_OR_ZERO(username) || IS_STRING_NULL_OR_ZERO(pwd) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_UNAUTHORIZED, "No '%s' or '%s' is found in request", MUX_REST_USER_NAME, MUX_REST_USER_PWD);
			return EXIT_FAILURE;
		}

		if(dataConn->handleAuthen(dataConn, username, pwd) == EXIT_FAILURE)  
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_UNAUTHORIZED, "'%s/%s' authenticatio failed", username, pwd);
			return EXIT_FAILURE;
		}
	}

	dataConn->method = _getRequestMethod(reqObj);
	if(dataConn->method == CMN_JSON_METHOD_INVALIDATE)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_METHOD_NOT_ALLOWED, "Request method is not allowed");
		return EXIT_FAILURE;
	}

	uri = cmnGetStrFromJsonObject(reqObj, MUX_REST_URL);
	if(IS_STRING_NULL_OR_ZERO(uri) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No field of '%s' is found in packet", MUX_REST_URL );
		return EXIT_FAILURE;
	}
	
	snprintf(dataConn->cmd, sizeof(dataConn->cmd), "%s", uri);

	_dataArrayObj = cJSON_GetObjectItem(reqObj, IPCMD_NAME_KEYWORD_DATA);
	if(cJSON_IsArray(_dataArrayObj) )
	{
		/* for POST of SDP object, it is an array with more than one item */
		if(cJSON_GetArraySize(_dataArrayObj) == 1)
		{
			dataConn->dataObj = cJSON_GetArrayItem(_dataArrayObj, 0);
		}
		else
		{
			dataConn->dataObj = _dataArrayObj;
		}
	}
	else
	{
		dataConn->dataObj = _dataArrayObj;
	}
	
	return EXIT_SUCCESS;
}


int cmnMuxDataConnAuthen(struct DATA_CONN *dataConn, char *user, char *pwd)
{
	if(IS_STRING_EQUAL(user, MUX_AUTH_USER) && IS_STRING_EQUAL(pwd, MUX_AUTH_PASSWORD) )
	{
		return EXIT_SUCCESS;
	}

	DATA_CONN_ERR(dataConn, IPCMD_ERR_UNAUTHORIZED, "User/password: '%s/%s' authentication failed", user, pwd);

	return EXIT_FAILURE;
}


/* reply error or OK message in context of controller and other threads */
int cmnMuxJsonControllerReply(struct DATA_CONN *dataConn, int status, const char *fmt, ... )
{
	va_list vargs;
	va_start(vargs, fmt);

	dataConn->errCode = status;

	vsnprintf(dataConn->detailedMsg, CMN_NAME_LENGTH,  fmt, vargs);
	
//	MUX_ERROR("JSON handler error: %s", dataConn->detailedMsg);

	va_end(vargs);

	return EXIT_SUCCESS;
}

