
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
	if( (dataConn)->cmdObjs)
	{
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("cJSON_Delete cmdObjs:%p", dataConn->cmdObjs);	
#endif
		cJSON_Delete( (dataConn)->cmdObjs);
	}
	
	if( (dataConn)->resultObject)
	{/* when is it been freed?? */
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("cJSON_Delete cmdObjs:%p", dataConn->resultObject);	
#endif
//		cJSON_Delete( (dataConn)->resultObject);
	}

	close((dataConn)->sock);
	cmn_mutex_destroy(dataConn->mutexLock);
	
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG("free dataConn:%p", dataConn);	
#endif
	cmn_free((dataConn));
}



static int _dataConnOutput(struct DATA_CONN *dataConn, void *buf, int size)
{
	int len = 0;

	TRACE();
#if 1//MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG("Reply %d bytes packet to %s:%d", size, 
		DATA_CONN_IS_IPCMD(dataConn)? inet_ntoa(dataConn->peerAddr.sin_addr):"Unix", DATA_CONN_IS_IPCMD(dataConn)?ntohs(dataConn->peerAddr.sin_port):0);
	CMN_HEX_DUMP((const uint8_t *)buf, size, "send out data in data connection" );
#endif
	TRACE();

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
		MUX_DEBUG("Reply %d bytes packet to %s:%d, addrLen:%d", size, inet_ntoa(dataConn->peerAddr.sin_addr), ntohs(dataConn->peerAddr.sin_port), dataConn->addrlen);
		len = sendto(dataConn->ctrlConn->sockCtrl, buf, size, 0, (struct sockaddr *)&dataConn->peerAddr, dataConn->addrlen); //  if suceeded the updated data is send back
	}

	if(len != size || len < 0 )
	{
		MUX_ERROR("ERROR writing to socket only %d bytes: '%s'", len, strerror(errno));
	}
	MUX_DEBUG("Replied %d bytes packet", len);

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
	cJSON *dataArray = NULL, *dataItem = NULL;
	cJSON *responseObj;
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
	

	dataArray = cJSON_GetObjectItem(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_DATA);
	if(!cJSON_IsArray(dataArray))
	{
		MUX_ERROR("'%s' is not array object", IPCMD_NAME_KEYWORD_DATA );
	}
	
#if 0
	dataItem = cJSON_GetArrayItem(dataArray, 0);
#else
	dataItem = dataConn->resultObject;
#endif
	if(dataItem == NULL)
	{
//		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No data item is in reply object");
	}
	
TRACE();
	if(dataConn->errCode == IPCMD_ERR_NOERROR)
	{
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_LOGIN_ACK, cJSON_CreateString(EXT_IPCMD_STATUS_OK));
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_PWD_MSG, cJSON_CreateString(EXT_IPCMD_STATUS_OK));
		
		if( (dataConn->errCode == IPCMD_ERR_NOERROR ||dataConn->errCode == IPCMD_ERR_FTP_PARTLY_FAILED) && dataConn->resultObject != NULL )
		{
			if(cJSON_GetArraySize(dataArray) > 0)
			{
				cJSON_ReplaceItemInArray(dataArray, 0, dataConn->resultObject);
			}
			else
			{	
				cJSON_AddItemToArray(dataArray, dataConn->resultObject);
			}
	//		cJSON_ReplaceItemInObjectCaseSensitive(dataItem, MEDIA_CTRL_OBJECTS, dataConn->resultObject);
		}
	}
	else
	{
		char *errMsg = MUX_JSON_ERROR_STR(dataConn->errCode);
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_LOGIN_ACK, cJSON_CreateString(EXT_IPCMD_STATUS_FAIL));
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_PWD_MSG, cJSON_CreateString(errMsg));
		cJSON_AddStringToObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_MSG_DETAIL, IS_STRING_NULL_OR_ZERO(dataConn->detailedMsg)?"Command is not validate format": dataConn->detailedMsg );

		/* remove raw data from response when something is wrong */
		if(dataArray)
		{
			cJSON_DeleteItemFromObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_DATA);
		}
	}
	
TRACE();

#if 0	
	cJSON_AddItemToObject(dataItem, MEDIA_CTRL_STATUS, cJSON_CreateNumber((double) dataConn->errCode));
	if(!IS_STRING_NULL_OR_ZERO(dataConn->detailedMsg) )
	{
		cJSON_AddItemToObject(dataItem, MEDIA_CTRL_STATUS_MSG, cJSON_CreateString( dataConn->detailedMsg) );
	}
#endif
TRACE();
	responseObj = dataConn->cmdObjs;

sendout:

	msg = cJSON_PrintUnformatted(responseObj);
	if(msg == NULL)
	{
		msg = "Unknown";
	}

	len = _dataConnIpCmdOutput(dataConn, msg, strlen(msg));

	cmn_free(msg);

	dataConn->isFinished = TRUE;
	
	return len;
}

int cmnMuxDataConnRestOutput(struct DATA_CONN *dataConn)
{
	char *msg = NULL;
	int len = 0;

	if(DATA_CONN_IS_IPCMD(dataConn) )
	{
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

		cJSON_AddItemToObjectCS(resultObj, MUX_REST_STATUS_CODE, cJSON_CreateNumber(dataConn->errCode) );
		cJSON_AddItemToObjectCS(resultObj, MUX_REST_STATUS_ERROR, cJSON_CreateString(errMsg));
		cJSON_AddItemToObjectCS(resultObj, MUX_REST_STATUS_DEBUG, cJSON_CreateString(dataConn->detailedMsg) );
		
		msg = cJSON_PrintUnformatted(resultObj);
	}

	if(msg)
	{
		MUX_DEBUG("REST output :'%s'", msg);
		len = _dataConnOutput(dataConn, msg, strlen(msg));
		cmn_free(msg);
	}

	dataConn->isFinished = TRUE;

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

	if(extMacAddressParse(&macAddress, target) == EXIT_FAILURE)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' : '%s' is not a validate MAC address", IPCMD_NAME_KEYWORD_TARG, target);
		return EXIT_FAILURE;
	}

	if(!MAC_ADDR_IS_EQUAL(&macAddress, &muxMain->runCfg.local.mac) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Field '%s' : '%s' is not my MAC address", IPCMD_NAME_KEYWORD_TARG, target);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static int _dataConnIpCmdAdapt(struct DATA_CONN *dataConn, cJSON *cmdObj)
{
//	cJSON *newCmdObj = NULL;
	cJSON *_dataArrayObj= NULL;
	char *uri = NULL;
	char *cmdName = NULL;
	int isGetParams = FALSE;

	dataConn->method = CMN_JSON_METHOD_INVALIDATE;
	
	cmdName = cmnGetStrFromJsonObject(cmdObj, IPCMD_NAME_KEYWORD_CMD);
	if( IS_STRING_NULL_OR_ZERO(cmdName) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Invalidate IP Command, without '%s'", IPCMD_NAME_KEYWORD_CMD );
		return EXIT_FAILURE;
	}

	if(IS_STRING_EQUAL(cmdName, IPCMD_NAME_GET_PARAM) )
	{
		uri = MUX_REST_URI_PARAMS;
		dataConn->method = CMN_JSON_METHOD_GET;
		isGetParams = TRUE;
	}
	else if(IS_STRING_EQUAL(cmdName, IPCMD_NAME_SET_PARAM) )
	{
		uri = MUX_REST_URI_PARAMS;
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
	
	if(isGetParams == FALSE )
	{
		if( _ipCmdIsLocal(dataConn)== EXIT_FAILURE)
		{
			return EXIT_FAILURE;
		}

		if(dataConn->dataObj == NULL)
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No validate data for IP Command '%s'", cmdName );
			return EXIT_FAILURE;
		}
	}
	
//	cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string);

	return EXIT_SUCCESS;
}


int cmnMuxDataConnIpCmdInput(struct DATA_CONN *dataConn)
{
	unsigned int crcDecoded, crcReceived;
	cJSON *reqObj = NULL;
	char *username = NULL, *pwd = NULL;
	MuxMain *muxMain = SYS_MAIN(dataConn);
	
	if(! CONN_IS_IPCMD(dataConn->ctrlConn))
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_WRONG_PROTOCOL, "TCP/UDP socket can only be used in IP Command interface");
		return EXIT_FAILURE;
	}
	
	dataConn->ctrlConn->cmdBuffer = (CMN_IP_COMMAND *)dataConn->ctrlConn->buffer;

	dataConn->ctrlConn->cmdBuffer->length = ntohs(dataConn->ctrlConn->cmdBuffer->length);
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG("received length: %d, length field: %d", dataConn->ctrlConn->length, dataConn->ctrlConn->cmdBuffer->length);
#endif

	if(dataConn->ctrlConn->cmdBuffer->length+4 != dataConn->ctrlConn->length )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_WRONG_PROTOCOL,"length field:%d, received length:%d", dataConn->ctrlConn->cmdBuffer->length, dataConn->ctrlConn->length) ;
		return EXIT_FAILURE;
	}


	if (CMD_TAG_A != dataConn->ctrlConn->cmdBuffer->tag)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_WRONG_PROTOCOL, "Tag of command is wrong:0x%x", dataConn->ctrlConn->cmdBuffer->tag);
		return EXIT_FAILURE;
	}

	crcReceived = *(unsigned int *)(dataConn->ctrlConn->buffer + dataConn->ctrlConn->cmdBuffer->length /*-4*/);	/* the first 4 bytes are header of command, so here is the position of CRC */
	crcDecoded = htonl(cmnMuxCRC32b(dataConn->ctrlConn->buffer+4, dataConn->ctrlConn->cmdBuffer->length-4 /*-4*/));
	if (crcReceived != crcDecoded)
	{//Wrong CRC
		//sendto(sockCtrl, msg_reply, n /*+ 8*/, 0, (struct sockaddr *) &addr, addrlen); //  if suceeded the updated data is send back
		DATA_CONN_ERR(dataConn, IPCMD_ERR_CRC_FAIL,"CRC of command is wrong:received CRC: 0x%x, Decoded CRC:0x%x", crcReceived, crcDecoded) ;
		return EXIT_FAILURE;
	}
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG( "received CRC: 0x%x, Decoded CRC:0x%x", crcReceived, crcDecoded);
#endif

	reqObj =  _dataConnInput(dataConn, dataConn->ctrlConn->buffer+4,  dataConn->ctrlConn->cmdBuffer->length);
	if(reqObj == NULL)
	{/* error before JSON object is parsed */
		return EXIT_FAILURE;
	}

	dataConn->cmdObjs = reqObj;
	
	if(muxMain->isAuthen)
	{
		username = cmnGetStrFromJsonObject(reqObj, IPCMD_NAME_KEYWORD_LOGIN_ACK);
		pwd = cmnGetStrFromJsonObject(reqObj, IPCMD_NAME_KEYWORD_PWD_MSG);
		if( IS_STRING_NULL_OR_ZERO(username)  || IS_STRING_NULL_OR_ZERO(pwd) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_UNAUTHORIZED, "No '%s' or '%s' in request", IPCMD_NAME_KEYWORD_LOGIN_ACK, IPCMD_NAME_KEYWORD_PWD_MSG);
			return EXIT_FAILURE;
		}

		if(dataConn->handleAuthen(dataConn, username, pwd) == EXIT_FAILURE) 
		{
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

	reqObj = _dataConnInput(dataConn, dataConn->ctrlConn->buffer, dataConn->ctrlConn->length);
	if(reqObj == NULL)
	{
		return EXIT_FAILURE;
	}

	if(muxMain->isAuthen)
	{
		username = cmnGetStrFromJsonObject(reqObj, MUX_REST_USER_NAME);
		pwd = cmnGetStrFromJsonObject(reqObj, MUX_REST_USER_PWD);
		if( IS_STRING_NULL_OR_ZERO(username) || IS_STRING_NULL_OR_ZERO(pwd) )
		{
			DATA_CONN_ERR(dataConn, IPCMD_ERR_UNAUTHORIZED, "No '%s' or '%s' is found in request", MUX_REST_USER_NAME, MUX_REST_USER_PWD);
			return EXIT_FAILURE;
		}

		if(dataConn->handleAuthen(dataConn, username, pwd) == EXIT_FAILURE)  
		{
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
	dataConn->cmdObjs = reqObj;
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


