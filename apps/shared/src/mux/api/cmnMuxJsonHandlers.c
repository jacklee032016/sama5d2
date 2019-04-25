

#include "libCmn.h"
#include "libMedia.h"
#include "libMux.h"

#include "_cmnMux.h"

struct json_handler
{
	char		*name;
	
	/* which plugins or main should process this JSON object */
	MUX_PLUGIN_TYPE		dest;

	/* handler only operate errorStats, msg, resultObjs of dataConn object, never execute send operation.
	* return value has no meaning 
	*/
	int		(*handleGet)(MUX_PLUGIN_TYPE dest, struct DATA_CONN *, cJSON *data);
	int		(*handlePost)(MUX_PLUGIN_TYPE dest, struct DATA_CONN *, cJSON *data);
};


static int	_defaultGet(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	int res = EXIT_SUCCESS;
	cJSON *resultObj = NULL;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
//	EXT_RUNTIME_CFG		*runCfg = &muxMain->runCfg;

	SYS_PLAYLIST_LOCK(muxMain);

//	DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Invalidate IP Command, without '%s'", IPCMD_NAME_KEYWORD_CMD );
//	dataConn->resultObject = cJSON_CreateArray();
	resultObj = cJSON_CreateObject();
	cJSON_AddStringToObject(resultObj, "OK", "To be implemented in future");

	dataConn->resultObject = resultObj;

	SYS_PLAYLIST_UNLOCK(muxMain);

//MUX_DEBUG_JSON_OBJ(dataConn->resultObject);

	dataConn->errCode = IPCMD_ERR_NOERROR;
#if 0
	else
	{
		return DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Data invalidate: action '%s' is not validate", action);
	}
#endif

	return res;
}

static int	_defaultPost(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	int res = EXIT_SUCCESS;
	cJSON *resultObj = NULL;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
//	EXT_RUNTIME_CFG		*runCfg = &muxMain->runCfg;

	SYS_PLAYLIST_LOCK(muxMain);

	if(data == NULL)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "No 'data' item in JSON data");
		SYS_PLAYLIST_UNLOCK(muxMain);
		return EXIT_FAILURE;
	}
	
	MUX_DEBUG_JSON_OBJ(data);
	
//	DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Invalidate IP Command, without '%s'", IPCMD_NAME_KEYWORD_CMD );
//	dataConn->resultObject = cJSON_CreateArray();
	resultObj = cJSON_CreateObject();
	cJSON_AddStringToObject(resultObj, "OK", "To be implemented in future");

	dataConn->resultObject = resultObj;

	SYS_PLAYLIST_UNLOCK(muxMain);

	dataConn->errCode = IPCMD_ERR_NOERROR;
#if 0
	else
	{
		return DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Data invalidate: action '%s' is not validate", action);
	}
#endif

	return res;
}


static int	_ipCmdHandle4GetParams(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	cJSON *obj = NULL;
//	dataConn->resultObject = cJSON_CreateArray();
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
	EXT_RUNTIME_CFG		*runCfg = &muxMain->runCfg;

	SYS_PLAYLIST_LOCK(muxMain);

//	obj = cJSON_CreateObject();
#if 0
	obj = cmnMuxSystemJSon2Flat(muxMain->systemJson);
#else
	obj = muxMain->systemJson;
#endif

	SYS_PLAYLIST_UNLOCK(muxMain);
//	cJSON_AddItemToArray(dataConn->resultObject, objects);
	dataConn->resultObject = obj;

//MUX_DEBUG_JSON_OBJ(dataConn->resultObject);

	dataConn->errCode = IPCMD_ERR_NOERROR;
#if 0
	else
	{
		return DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Data invalidate: action '%s' is not validate", action);
	}
#endif

	return EXIT_SUCCESS;
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


static int	_restHandle4GetRoot(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data)
{
	cJSON *resultObj = NULL;
	int count, i;
	
	MuxMain *muxMain = SYS_MAIN(dataConn);
//	EXT_RUNTIME_CFG		*runCfg = &muxMain->runCfg;

	SYS_PLAYLIST_LOCK(muxMain);

//	DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Invalidate IP Command, without '%s'", IPCMD_NAME_KEYWORD_CMD );
//	dataConn->resultObject = cJSON_CreateArray();
	resultObj = cJSON_CreateArray();
#if 0
	cJSON_AddItemToArray(resultObj, cJSON_CreateString(MUX_REST_URI_SYSTEM) );
	cJSON_AddItemToArray(resultObj, cJSON_CreateString(MUX_REST_URI_VIDEO) );
	cJSON_AddItemToArray(resultObj, cJSON_CreateString(MUX_REST_URI_AUDIO) );
	cJSON_AddItemToArray(resultObj, cJSON_CreateString(MUX_REST_URI_ANC) );
	cJSON_AddItemToArray(resultObj, cJSON_CreateString(MUX_REST_URI_RS232) );
//	cJSON_AddArrayToObject(resultObj, MUX_REST_URI_OTHERS);
	cJSON_AddItemToArray(resultObj, cJSON_CreateString(MUX_REST_URI_OTHERS) );
#else

	count = cJSON_GetArraySize(muxMain->systemJson);
	for(i=0; i < count; i++ )
	{
		cJSON *item = cJSON_GetArrayItem(muxMain->systemJson, i);
		cJSON_AddItemToArray(resultObj, cJSON_CreateString(item->string) );
	}

#endif
	dataConn->resultObject = resultObj;

	SYS_PLAYLIST_UNLOCK(muxMain);

	MUX_DEBUG_JSON_OBJ(dataConn->resultObject);

	dataConn->errCode = IPCMD_ERR_NOERROR;

	return EXIT_SUCCESS;
}


static int	_restDefaultGet(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data)
{
	MuxMain *muxMain = SYS_MAIN(dataConn);
//	EXT_RUNTIME_CFG		*runCfg = &muxMain->runCfg;

	SYS_PLAYLIST_LOCK(muxMain);

	dataConn->resultObject = extSystemFindObject(muxMain, dataConn->cmd+1);

	SYS_PLAYLIST_UNLOCK(muxMain);

	if(dataConn->resultObject == NULL)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_SERVER_INTERNEL_ERROR, "No data is found for URI: '%s'", dataConn->cmd );
	}
	else
	{
		MUX_DEBUG_JSON_OBJ(dataConn->resultObject);
		dataConn->errCode = IPCMD_ERR_NOERROR;
	}

	return EXIT_SUCCESS;
}


static int	_restHandle4GetSystem(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data)
{
#if 0
	MuxMain *muxMain = SYS_MAIN(dataConn);
//	EXT_RUNTIME_CFG		*runCfg = &muxMain->runCfg;

	SYS_PLAYLIST_LOCK(muxMain);

	dataConn->resultObject = extSystemFindObject(muxMain, dataConn->cmd+1);

	SYS_PLAYLIST_UNLOCK(muxMain);

	if(dataConn->resultObject == NULL)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_SERVER_INTERNEL_ERROR, "No data is found");
	}
	else
	{
		MUX_DEBUG_JSON_OBJ(dataConn->resultObject);
		dataConn->errCode = IPCMD_ERR_NOERROR;
	}
#else
	return _restDefaultGet(dest, dataConn, data);
#endif
}

static int	_restHandle4GetVideo(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data)
{
	return _restDefaultGet(dest, dataConn, data);
}

static int	_restHandle4GetAudio(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data)
{
	return _restDefaultGet(dest, dataConn, data);
}

static int	_restHandle4GetAnc(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data)
{
	return _restDefaultGet(dest, dataConn, data);
}

static int	_restHandle4GetSdp(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data)
{
	return _restDefaultGet(dest, dataConn, data);
}

static int	_restHandle4GetRs232(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data)
{
	return _restDefaultGet(dest, dataConn, data);
}

static int	_restHandle4GetSecurity(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data)
{
	return _restDefaultGet(dest, dataConn, data);
}

static int	_restHandle4GetOthers(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data)
{
	return _restDefaultGet(dest, dataConn, data);
}


struct json_handler jsonActionHandlers[] =
{
	{
		.name 		= MUX_REST_URI_PARAMS,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= _ipCmdHandle4GetParams,
		.handlePost	= _defaultPost
	},
	{
		.name 		= MUX_REST_URI_ROOT,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= _restHandle4GetRoot,
		.handlePost	= _defaultPost
	},
	{
		.name 		= MUX_REST_URI_ROOT MUX_REST_URI_SYSTEM,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= _restHandle4GetSystem,
		.handlePost	= _defaultPost
	},
	{
		.name 		= MUX_REST_URI_ROOT MUX_REST_URI_VIDEO,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= _restHandle4GetVideo,
		.handlePost	= _defaultPost
	},
	{
		.name 		= MUX_REST_URI_ROOT MUX_REST_URI_AUDIO,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= _restHandle4GetAudio,
		.handlePost	= _defaultPost
	},
	{
		.name 		= MUX_REST_URI_ROOT MUX_REST_URI_ANC,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= _restHandle4GetAnc,
		.handlePost	= _defaultPost
	},
	{
		.name 		= MUX_REST_URI_ROOT MUX_REST_URI_SDP,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= _restHandle4GetSdp,
		.handlePost	= _defaultPost
	},
	{
		.name 		= MUX_REST_URI_ROOT MUX_REST_URI_RS232,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= _restHandle4GetRs232,
		.handlePost	= _defaultPost
	},
	{
		.name 		= MUX_REST_URI_ROOT MUX_REST_URI_SECURITY,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= _restHandle4GetSecurity,
		.handlePost	= _defaultPost
	},
	{
		.name 		= MUX_REST_URI_ROOT MUX_REST_URI_OTHERS,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= _restHandle4GetOthers,
		.handlePost	= _defaultPost
	},
#if 0
	{
		.name 	= IPCMD_NAME_MEDIA_WEB,
		.dest	= MUX_PLUGIN_TYPE_WEB,
		.handler	= cmnMuxJsonHandle4Plugin
		.handleGet	= _restHandle4GetRoot,
		.handlePost	= _defaultPost
	},
#endif	

	{
		.name 		= IPCMD_NAME_SYS_ADMIN,
		.dest		= MUX_PLUGIN_TYPE_MAIN,
		.handleGet	= cmnMuxJsonHandle4SysAdmin,
		.handlePost	= _defaultPost
	},

	{
		.name 		= NULL,
		.dest		= MUX_PLUGIN_TYPE_UNKNOWN,
		.handleGet	= NULL,
		.handlePost	= NULL
	}
};



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
		dataConn->handleOutput(dataConn);
		
		cmn_mutex_unlock(dataConn->mutexLock);

		dataConn->handleDestroy(dataConn);
		dataConn = NULL;

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

	while(_handle->handleGet )
	{
		if(!strcasecmp(_handle->name, dataConn->cmd))
		{

			if(_handle->dest != MUX_PLUGIN_TYPE_MAIN)
			{
				MuxPlugIn *plugin;
				plugin = cmnMuxPluginFind( SYS_MAIN(dataConn), _handle->dest);
				if(!plugin)
				{
					DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Function '%s' is not loaded", CMN_MUX_FIND_PLUGIN_NAME(_handle->dest) );
					goto _ret;
				}
			}
			
#if 1// MUX_OPTIONS_DEBUG_IP_COMMAND			
			MUX_DEBUG("URI '%s' '%s' is processing.....", dataConn->cmd, CMN_MUX_FIND_METHOD_NAME(dataConn->method) );
#endif

			if((!dataConn->dataObj || !cJSON_IsArray(dataConn->dataObj)) && (dataConn->method !=  CMN_JSON_METHOD_GET) )
			{
//				MUX_ERROR( "Data invalidate: Data Item is not an array JSON object");
				DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR,  "Data invalidate: Data Item is not an array JSON object");
				goto _ret;
			}

			cJSON *data = cJSON_GetArrayItem( dataConn->dataObj, 0);
			if(!data &&  dataConn->method != CMN_JSON_METHOD_GET )
			{
//				MUX_ERROR("Data invalidate: No data Item is not an data array object");
				DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Data invalidate: No data Item is not an data array object");
				goto _ret;
			}

			// dataConn->dataObj = data;
			if(dataConn->method == CMN_JSON_METHOD_POST )
			{
				res = _handle->handlePost(_handle->dest, dataConn, data);
			}
			else
			{
				res = _handle->handleGet(_handle->dest, dataConn, data);
			}
			if(res == EXIT_FAILURE)
			{
//				MUX_ERROR("Internal error ");
				cmnMuxJsonControllerReply(dataConn, IPCMD_ERR_SERVER_INTERNEL_ERROR, "Internal error ");
			}

			
			if(dataConn->errCode != IPCMD_ERR_IN_PROCESSING)
			{
				dataConn->handleOutput(dataConn);
			}

			return res;
		}

		_handle++;
	}

	/* make controller reply 'command is not defined' in controller's receiving */
	DATA_CONN_ERR(dataConn, IPCMD_ERR_NOT_SUPPORT_COMMND, "'%s' '%s' is not found", dataConn->cmd, CMN_MUX_FIND_METHOD_NAME(dataConn->method));

_ret:

	dataConn->handleOutput(dataConn);

	return res;
}



