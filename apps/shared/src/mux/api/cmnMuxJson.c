

#include "libCmn.h"
#include "libMedia.h"
#include "libMux.h"

#include "_cmnMux.h"

char* cmnGetStrFromJsonObject(cJSON* json, const char * key)
{
	cJSON * obj = cJSON_GetObjectItem(json, key);

	if(cJSON_IsString(obj) )
		return obj->valuestring;
	else
		return "";
}

int cmnGetIntegerFromJsonObject(cJSON* json, const char * key)
{
	cJSON * obj = cJSON_GetObjectItem(json, key);
	
	if(cJSON_IsNumber(obj) )
		return obj->valueint;
	else
		return -1;
}


int	cmnMuxJsonHandle4GetParams(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	int res = EXIT_SUCCESS;

//	dataConn->resultObject = cJSON_CreateArray();
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG		*runCfg = &muxMain->runCfg;

	SYS_PLAYLIST_LOCK(muxMain);

//	cJSON *objects = cJSON_CreateObject();
	cJSON *objects = cmnMuxSystemJSon2Flat(muxMain->systemJson);


	SYS_PLAYLIST_UNLOCK(muxMain);
//	cJSON_AddItemToArray(dataConn->resultObject, objects);
	dataConn->resultObject = objects;

TRACE();
//MUX_DEBUG_JSON_OBJ(dataConn->resultObject);

TRACE();
	dataConn->errCode = IPCMD_ERR_NOERROR;
#if 0
	else
	{
		return CMN_CONTROLLER_REPLY_DATA_ERR(dataConn, "Data invalidate: action '%s' is not validate", action);
	}
#endif

	return res;
}


/* cmd is command cJSON from client */
char *cmnMuxCreateErrReply(int errCode, cJSON *cmd)
{
	cJSON *errReply = cJSON_CreateObject();
	cJSON *arrayItem = cJSON_CreateArray();
	cJSON *params = cJSON_CreateObject();
	char *errStr = MUX_JSON_ERROR_STR(errCode);
	char* reply = NULL;

	if(cmd == NULL)
	{
		cJSON_AddItemToObject(errReply, IPCMD_NAME_KEYWORD_TARG, cJSON_CreateString("FF:FF:FF:FF:FF:FF"));
		cJSON_AddItemToObject(errReply, IPCMD_NAME_KEYWORD_CMD, cJSON_CreateString("Unknown"));
	}
	else
	{
		cJSON_AddItemToObject(errReply, IPCMD_NAME_KEYWORD_TARG, cJSON_CreateString(cmnGetStrFromJsonObject(cmd, IPCMD_NAME_KEYWORD_TARG)));
		cJSON_AddItemToObject(errReply, IPCMD_NAME_KEYWORD_CMD, cJSON_CreateString(cmnGetStrFromJsonObject(cmd, IPCMD_NAME_KEYWORD_CMD)));
	}
	cJSON_AddItemToObject(errReply, IPCMD_NAME_KEYWORD_LOGIN_ACK, cJSON_CreateString("NOK"));
	cJSON_AddItemToObject(errReply, IPCMD_NAME_KEYWORD_PWD_MSG, cJSON_CreateString( (errStr!=NULL)?errStr:"Unknown Error" ));
	cJSON_AddItemToObject(errReply, IPCMD_NAME_KEYWORD_DATA, arrayItem);
	
	cJSON_AddItemToArray(arrayItem, params);
	cJSON_AddItemToObject(params, MEDIA_CTRL_STATUS, cJSON_CreateNumber((double) errCode));

	//reply = cJSON_Print(errReply);
	reply = cJSON_PrintUnformatted(errReply);

	cJSON_Delete(errReply);

	return reply;
}


struct json_handler
{
	char		*name;
	
	/* which plugins or main should process this JSON object */
	MUX_PLUGIN_TYPE		dest;

	/* handler only operate errorStats, msg, resultObjs of dataConn object, never execute send operation.
	* return value has no meaning 
	*/
	int		(*handler)(MUX_PLUGIN_TYPE dest, struct DATA_CONN *, cJSON *data);
};

#if 0
static int	_jsonHandle4GetParam(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	return -EXIT_SUCCESS;
}

static int	_jsonHandle4SetParam(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	return -EXIT_SUCCESS;
}

static int	_jsonHandle4SendDataRs232(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	return -EXIT_SUCCESS;
}

static int	_jsonHandle4SendDataIr(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	return -EXIT_SUCCESS;
}
#endif


struct json_handler jsonActionHandlers[] =
{
	{
		.name 	= IPCMD_NAME_GET_PARAM,
		.dest	= MUX_PLUGIN_TYPE_MAIN,
		.handler	= cmnMuxJsonHandle4GetParams
	},
#if 0
	{
		.name 	= IPCMD_NAME_SET_PARAM,
		.dest	= MUX_PLUGIN_TYPE_UNKNOWN,
		.handler	= _jsonHandle4SetParam
	},
	{
		.name 	= IPCMD_NAME_SEND_RS232,
		.dest	= MUX_PLUGIN_TYPE_UNKNOWN,
		.handler	= _jsonHandle4SendDataRs232
	},
	{
		.name 	= IPCMD_NAME_SEND_IR,
		.dest	= MUX_PLUGIN_TYPE_UNKNOWN,
		.handler	= _jsonHandle4SendDataIr
	},
#endif	
	{
		.name 	= IPCMD_NAME_MEDIA_GET,
		.dest	= MUX_PLUGIN_TYPE_MAIN,
		.handler	= cmnMuxJsonHandle4GetMedia
	},
	{
		.name 	= IPCMD_NAME_MEDIA_SET,
		.dest	= MUX_PLUGIN_TYPE_MAIN,
		.handler	= cmnMuxJsonHandle4SetMedia
	},
	{
		.name 	= IPCMD_NAME_MEDIA_PLAY,
		.dest	= MUX_PLUGIN_TYPE_PLAYER,
		.handler	= cmnMuxJsonHandle4Plugin
	},
	{
		.name 	= IPCMD_NAME_MEDIA_RECORDER,
		.dest	= MUX_PLUGIN_TYPE_RECORDER,
		.handler	= cmnMuxJsonHandle4Plugin
	},
	{
		.name 	= IPCMD_NAME_MEDIA_SERVER,
		.dest	= MUX_PLUGIN_TYPE_SERVER,
		.handler	= cmnMuxJsonHandle4Plugin
	},
	{
		.name 	= IPCMD_NAME_MEDIA_WEB,
		.dest	= MUX_PLUGIN_TYPE_WEB,
		.handler	= cmnMuxJsonHandle4Plugin
	},

	{
		.name 	= IPCMD_NAME_SYS_ADMIN,
		.dest	= MUX_PLUGIN_TYPE_MAIN,
		.handler	= cmnMuxJsonHandle4SysAdmin
	},

	{
		.name 	= NULL,
		.dest	= MUX_PLUGIN_TYPE_UNKNOWN,
		.handler	= NULL
	}
};


cJSON *cmnMuxJsonLoadConfiguration(char *cfgFileName)
{
	cJSON *cfgHandlers = NULL;

	char *jsonStr = cmn_read_file(cfgFileName);
	if ((jsonStr == NULL) )
	{
		MUX_ERROR("IP Command configuration file '%s' reading failed", cfgFileName);
		return NULL;
	}
	
	cfgHandlers = cJSON_Parse(jsonStr);
	if (cfgHandlers== NULL)
	{
		cmn_free(jsonStr);
		MUX_ERROR("IP Command configuration file '%s' Parsing failed", cfgFileName);
		return NULL;
	}

	cmn_free(jsonStr);

	return cfgHandlers;
}



/* Reply JSON object in any threads (context of controller and other threads) and 
* all cases: OK or error after DATA_CONN->cmdObjs has been parsed in _read_command 
*/
static int	_cmnMuxJsonReply4All(struct DATA_CONN *dataConn)
{
	int res = EXIT_SUCCESS;
	cJSON *dataArray, *dataItem;

	if(dataConn->errCode == IPCMD_ERR_NOERROR)
	{
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_LOGIN_ACK, cJSON_CreateString("OK"));
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_PWD_MSG, cJSON_CreateString("OK"));
	}
	else
	{
		char *errMsg = MUX_JSON_ERROR_STR(dataConn->errCode);
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_LOGIN_ACK, cJSON_CreateString("NOK"));
		cJSON_ReplaceItemInObject(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_PWD_MSG, cJSON_CreateString(errMsg));
	}
	
	dataArray = cJSON_GetObjectItem(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_DATA);
	if(!cJSON_IsArray(dataArray))
	{
	}
	
	if( (dataConn->errCode == IPCMD_ERR_NOERROR ||dataConn->errCode == IPCMD_ERR_FTP_PARTLY_FAILED) && dataConn->resultObject != NULL )
	{
	TRACE();
		cJSON_ReplaceItemInArray(dataArray, 0, dataConn->resultObject);
//		cJSON_ReplaceItemInObjectCaseSensitive(dataItem, MEDIA_CTRL_OBJECTS, dataConn->resultObject);
	}
	
	TRACE();
	dataItem = cJSON_GetArrayItem(dataArray, 0);
	if(dataItem == NULL)
	{
		MUX_ERROR("No item is array object");
		return EXIT_FAILURE;
	}

	cJSON_AddItemToObject(dataItem, MEDIA_CTRL_STATUS, cJSON_CreateNumber((double) dataConn->errCode));
	if(!IS_STRING_NULL(dataConn->detailedMsg) )
	{
		cJSON_AddItemToObject(dataItem, MEDIA_CTRL_STATUS_MSG, cJSON_CreateString( dataConn->detailedMsg) );
	}

	char *msg = cJSON_PrintUnformatted(dataConn->cmdObjs);
//	char *msg = cJSON_PrintUnformatted(dataConn->resultObject);

	res = cmnMuxCtrlResponse(dataConn, msg, strlen(msg));
	cmn_free(msg);

	dataConn->isFinished = TRUE;
	return res;
}

/* reply error or OK message in context of controller and other threads */
int cmnMuxJsonControllerReply(struct DATA_CONN *dataConn, int status, const char *fmt, ... )
{
	va_list vargs;
	va_start(vargs, fmt);

	dataConn->errCode = status;

	vsnprintf(dataConn->detailedMsg, CMN_NAME_LENGTH,  fmt, vargs);
	
	MUX_ERROR("JSON handler error: %s", dataConn->detailedMsg);

	va_end(vargs);

	return EXIT_SUCCESS;
}

/* reply with JEVENT in context of FSM and plugin */
int cmnMuxJEventReply(CMN_PLAY_JSON_EVENT *jsonEvent, int errCode, const char *fmt, ...)
{
	va_list vargs;

	struct DATA_CONN *dataConn = (struct DATA_CONN *) jsonEvent->priv;
	if(!dataConn)
	{
		CMN_ABORT("DATA_CONN is null");
	}
	dataConn->errCode = errCode;

	va_start(vargs, fmt);
	vsnprintf(dataConn->detailedMsg, CMN_NAME_LENGTH,  fmt, vargs);
	MUX_ERROR("JEVENT handler error: %s", dataConn->detailedMsg);
	va_end(vargs);

	return cmnMuxJsonPluginReplay(dataConn, jsonEvent);
}


/*
 * reply the JSON message to peer for sync plugin or async FSM, eg. in context of threads other than controller  
*/
int	cmnMuxJsonPluginReplay(struct DATA_CONN *dataConn, CMN_PLAY_JSON_EVENT *jsonEvent)
{
	cmn_mutex_lock(dataConn->mutexLock);

	if(dataConn->errCode != IPCMD_ERR_IN_PROCESSING)
	{/* otherwise, wait the end of processing in FSM */
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("API engine is replying '%s' action", jsonEvent->action );
#endif		
		_cmnMuxJsonReply4All(dataConn);
		
		cmn_mutex_unlock(dataConn->mutexLock);

		cmnMuxDataConnClose(dataConn);

#if MUX_OPTIONS_DEBUG_IP_COMMAND			
		MUX_DEBUG("free jsonEvent:%p", jsonEvent);	
#endif
		cmn_free(jsonEvent);
	}
	else
	{
		cmn_mutex_unlock(dataConn->mutexLock);
	}

	return EXIT_SUCCESS;
}




/* 
* Loop of json handler for plugins (other threads except main component), for synchronized commands which is not delayed for processing 
*/
int cmnMuxJSonPluginHandle(void *priv, PluginJSonHandler *firstHandler, CMN_PLAY_JSON_EVENT *jsonEvent)
{
	int res = EXIT_SUCCESS;
	PluginJSonHandler *handler = firstHandler;
	struct DATA_CONN *dataConn =(struct DATA_CONN *)jsonEvent->priv;
	if(!dataConn)
	{
		MUX_ERROR("DataConn is null for JSON reply");
		exit(1);
	}

	cmn_mutex_lock(dataConn->mutexLock);
	dataConn->errCode = IPCMD_ERR_NOT_SUPPORT_COMMND;

	while( handler->handler)
	{
		if(!strcasecmp( handler->name, jsonEvent->action))
		{
			MUX_DEBUG("API engine is processing '%s' action", handler->name );
			jsonEvent->event = handler->type;
			
			res = handler->handler(priv, dataConn, jsonEvent);

			break;
		}
		
		handler++;
	}
	cmn_mutex_unlock(dataConn->mutexLock);

#if MUX_OPTIONS_DEBUG_IP_COMMAND			
	MUX_DEBUG("API engine is replying '%s' action", handler->name );
#endif
	cmnMuxJsonPluginReplay(dataConn, jsonEvent);

	return res;
}


/*
 * Loop of json handler in context of controller
*/
int cmnMuxCtrlDataHandle( struct DATA_CONN *dataConn )
{
	int res = EXIT_SUCCESS;
	struct json_handler *_handle = jsonActionHandlers;

	TRACE();

	cJSON *cmdObj = cJSON_GetObjectItem(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_CMD);
	if(cmdObj == NULL)
	{
		CMN_CONTROLLER_REPLY_DATA_ERR(dataConn, "No field of '%s' is found in packet", IPCMD_NAME_KEYWORD_CMD );
		goto _ret;
	}

	while(_handle->handler )
	{
		if(!strcasecmp(_handle->name, cmdObj->valuestring))
		{

			if(_handle->dest != MUX_PLUGIN_TYPE_MAIN)
			{
				MuxPlugIn *plugin;
				plugin = cmnMuxPluginFind( SYS_MAIN(dataConn), _handle->dest);
				if(!plugin)
				{
					CMN_CONTROLLER_REPLY_DATA_ERR(dataConn, "Function '%s' is not loaded", CMN_MUX_FIND_PLUGIN_NAME(_handle->dest) );
					goto _ret;
				}
			}
			
#if MUX_OPTIONS_DEBUG_IP_COMMAND			
			MUX_DEBUG("IP Command '%s' is processing.....", cmdObj->valuestring);
#endif
			cJSON *dataArray = cJSON_GetObjectItem(dataConn->cmdObjs, IPCMD_NAME_KEYWORD_DATA);

			if(!dataArray || !cJSON_IsArray(dataArray))
			{
//				MUX_ERROR( "Data invalidate: Data Item is not an array JSON object");
				CMN_CONTROLLER_REPLY_DATA_ERR(dataConn, "Data invalidate: Data Item is not an array JSON object");
				goto _ret;
			}

			cJSON *data = cJSON_GetArrayItem( dataArray, 0);
			if(!data &&  !IS_STRING_EQUAL(cmdObj->valuestring, IPCMD_NAME_GET_PARAM) )
			{
//				MUX_ERROR("Data invalidate: No data Item is not an data array object");
				CMN_CONTROLLER_REPLY_DATA_ERR(dataConn, "Data invalidate: No data Item is not an data array object");
				goto _ret;
			}

			dataConn->dataObj = data;
			res = _handle->handler(_handle->dest, dataConn, data);
			if(res == EXIT_FAILURE)
			{
//				MUX_ERROR("Internal error ");
				cmnMuxJsonControllerReply(dataConn, IPCMD_ERR_SERVER_INTERNEL_ERROR, "Internal error ");
			}

			
			if(dataConn->errCode != IPCMD_ERR_IN_PROCESSING)
			{
				_cmnMuxJsonReply4All(dataConn);
			}

			goto _ret;
		}

		_handle++;
	}

	/* make controller reply 'command is not defined' in controller's receiving */
	dataConn->errCode = IPCMD_ERR_NOT_SUPPORT_COMMND;

_ret:

	cmnMuxJsonReplyErrorMsg(dataConn);

	return res;
}


cJSON *cmnMuxSystemJSon2Flat(cJSON *systemJson)
{
	int count, i;
	cJSON *flatObj = NULL;

//	flatObj =cJSON_CreateArray();
	flatObj =cJSON_CreateObject();
	
	count = cJSON_GetArraySize(systemJson);
	MUX_DEBUG("Total %d items", count );
	
	for(i=0; i < count; i++ )
	{
		cJSON *item = cJSON_GetArrayItem(systemJson, i);
		cJSON *obj = cJSON_GetArrayItem(item, 0);
		MUX_DEBUG("#%d: %s", i, obj->string );
		if(obj)
		{
			cJSON *child = obj->child;
			while(child)
			{
				MUX_DEBUG("Add '%s':'%s'", child->string, (child->type == cJSON_String)? child->valuestring:"Othertype");

				cJSON_AddItemToArray(flatObj, cJSON_Duplicate(child, 1));
				
				child = child->next;
			}

		}
	}

	MUX_DEBUG_JSON_OBJ(flatObj);

	return flatObj;
}

