/*
* Media IP command in json format are handled in this file
*/

#include "libCmn.h"
#include "libMux.h"

#include "_cmnMux.h"

#define	MEDIA_CTRL_PLAYLIST_REPEAT	"repeat"
#define	MEDIA_CTRL_PLAYLIST_STATUS	"playlistStatus"
#define	MEDIA_CTRL_PLAYLIST_HOUR		"hour"
#define	MEDIA_CTRL_PLAYLIST_MINUTE	"minute"
#define	MEDIA_CTRL_PLAYLIST_DATE		"date"


char	*_checkDataValidate(cJSON *data)
{
	char *action = cmnGetStrFromJsonObject(data, MEDIA_CTRL_ACTION);
	cJSON *objects = cJSON_GetObjectItem(data, MEDIA_CTRL_OBJECTS);

	if( IS_STRING_NULL_OR_ZERO(action) )
	{
		MUX_ERROR( "Data invalidate: 'action' can not be found");
		return NULL;
	}

	if( (objects == NULL) || (!cJSON_IsArray(objects)) )
	{
		MUX_ERROR( "Data invalidate: 'objects' is not an array");
		return NULL;
	}

	return action;
}

static int _cmnMuxAddJSonEvent(MUX_PLUGIN_TYPE pluginType, char *action, cJSON *data, void *_dataConn)
{
	CMN_PLAY_JSON_EVENT *jsonEvent =(CMN_PLAY_JSON_EVENT *)cmn_malloc(sizeof(CMN_PLAY_JSON_EVENT));
	int res = EXIT_SUCCESS;

	struct DATA_CONN *dataConn = (struct DATA_CONN *)_dataConn;
	MuxMain *muxMain = SYS_MAIN(dataConn);
	
	if(muxMain == NULL )
	{
		MUX_ERROR("No MuxMain is found in DataConn");
		exit(1);
		return EXIT_FAILURE;
	}
	/* event->event set as 0, that is OK  */
	snprintf(jsonEvent->action, JSON_ACTION_LENGTH, "%s", action);
	jsonEvent->object = data;
	jsonEvent->priv = _dataConn;
	
	res = muxMain->reportEvent(muxMain, pluginType, jsonEvent);
	if(res  == EXIT_FAILURE)
	{/* jsonEvent not sent out, free it */
		cmn_free(jsonEvent);
		dataConn->errCode = IPCMD_ERR_PLUGIN_NOT_EXISTS;
	}
	else
	{/* delay for thread/FSM process */
		dataConn->errCode = IPCMD_ERR_IN_PROCESSING;
	}
	
	return EXIT_SUCCESS;
}


/* IP commands handled by plugin which external to main component */
int	cmnMuxJsonHandle4Plugin(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	int res = EXIT_SUCCESS;

	char *action = _checkDataValidate(data);
	if(!action)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Data invalidate: action '%s' is not supportted in PLAY_MEDIA now", action);
		return res;
	}
	
	cJSON *objects = cJSON_GetObjectItem(data, MEDIA_CTRL_OBJECTS);
	cJSON *content = cJSON_GetArrayItem(objects, 0);

	res = _cmnMuxAddJSonEvent(dest, action, content, dataConn);

	return res;
}


/* system administration of this program */
int	cmnMuxJsonHandle4SysAdmin(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	int res = EXIT_SUCCESS;

//	cJSON *objects = cJSON_GetObjectItem(data, MEDIA_CTRL_OBJECTS);
	MuxMain *muxMain = SYS_MAIN(dataConn);
//	cmn_list_t *_playlists = SYS_PLAYLISTS(muxMain);
	int i = 0;

	char *action = _checkDataValidate(data);
	if(!action)
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Data invalidate: no 'action' is defined in this sys_admin command");
		return res;
	}

	if( strcasecmp(action, IPCMD_SYS_ADMIN_THREADS) && 
		strcasecmp(action, IPCMD_SYS_ADMIN_VER_INFO) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Data invalidate: action '%s' is not supportted in sys_admin now", action);
		return res;
	}

	if(!strcasecmp(action, IPCMD_SYS_ADMIN_THREADS))
	{
		cmn_list_t  *threads = cmnThreadLockList();//&_threadList;
		cJSON *objectsArrary = cJSON_CreateArray();

		for(i=0; i< cmn_list_size(threads); i++)
		{
			CmnThread *thread = (CmnThread *)cmn_list_get(threads, i);
			cJSON *obj = NULL;

#if MUX_OPTIONS_DEBUG_IP_COMMAND			
			MUX_DEBUG( "No. %d thread :'%s' is added", i+1, thread->name);
#endif

			obj = cJSON_CreateObject();
				
			cJSON_AddItemToObject(obj, "name", cJSON_CreateString(thread->name) );
			cJSON_AddItemToObject(obj, "PID", cJSON_CreateNumber((int)thread->pId) );

			cJSON_AddItemToArray(objectsArrary, obj);

		}

		dataConn->resultObject = objectsArrary;

		cmnThreadReleaseList();
		return EXIT_SUCCESS;
	}

	if(!strcasecmp(action, IPCMD_SYS_ADMIN_VER_INFO))
	{
		MuxPlugIn	*plugin = NULL;
		cJSON *obj = NULL;

		cJSON *objectsArrary = cJSON_CreateArray();
		
		obj = cJSON_CreateObject();
		cJSON_AddItemToObject(obj, "name", cJSON_CreateString(CMN_MODULE_MAIN_NAME) );
		cJSON_AddItemToObject(obj, "version", cJSON_CreateString(CMN_VERSION_INFO(CMN_MODULE_MAIN_NAME)) );
		cJSON_AddItemToArray(objectsArrary, obj);

		plugin = muxMain->plugins;
		while(plugin)
		{
			if(plugin->enable )
			{
				obj = cJSON_CreateObject();
				cJSON_AddItemToObject(obj, "name", cJSON_CreateString(plugin->name) );
				cJSON_AddItemToObject(obj, "version", cJSON_CreateString(plugin->version) );
				cJSON_AddItemToArray(objectsArrary, obj);
			}

 			plugin = plugin->next;
		}

		dataConn->resultObject = objectsArrary;

		cmnThreadReleaseList();

		return EXIT_SUCCESS;
	}

	
	return res;
}


