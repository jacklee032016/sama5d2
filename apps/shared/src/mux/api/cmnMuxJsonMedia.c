/*
* Media IP command in json format are handled in this file
*/

#include "libCmn.h"
#include "libMedia.h"
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


int	cmnMuxJsonHandle4DownloadMedia(MUX_PLUGIN_TYPE dest, struct DATA_CONN *dataConn, cJSON *data) /* data is first item in data array */
{
	int res = EXIT_SUCCESS;
	int i;
	cmn_list_t *filelist = NULL;

	char *server = cmnGetStrFromJsonObject(data, MEDIA_CTRL_FTP_SERVER);
	char *username = cmnGetStrFromJsonObject(data, MEDIA_CTRL_FTP_USERNAME);
	char *password = cmnGetStrFromJsonObject(data, MEDIA_CTRL_FTP_PASSWORD);
	char *path = cmnGetStrFromJsonObject(data, MEDIA_CTRL_FTP_PATH);
//	cJSON *files = cJSON_GetObjectItem(data, MEDIA_CTRL_FTP_FILES);
	cJSON *files = cJSON_GetObjectItem(data, MEDIA_CTRL_OBJECTS);

	MUX_DEBUG("server:'%s'; username:'%s'; password:'%s'; path:'%s'", server, username, password, path);

	if(!cJSON_IsArray(files))
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Data invalidate: mediafiles is not an array");
		return res;
	}

	if( IS_STRING_NULL_OR_ZERO(server) || IS_STRING_NULL_OR_ZERO(username) || IS_STRING_NULL_OR_ZERO(password) ||IS_STRING_NULL_OR_ZERO(path) )
	{
		DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Data invalidate: info is uncomplete");
		return res;
	}

	filelist = (cmn_list_t *)cmn_malloc(sizeof(cmn_list_t));
	
	for(i=0; i < cJSON_GetArraySize(files); i++)
	{
		cJSON *item = cJSON_GetArrayItem(files, i);
		if(!cJSON_IsString(item))
		{
			cmn_list_ofchar_free(filelist, TRUE);
			DATA_CONN_ERR(dataConn, IPCMD_ERR_DATA_ERROR, "Data invalidate: info is uncomplete");
			return res;
		}
		
		int length =  strlen(path)+strlen(item->valuestring) + 2;
		length = (length< CMN_NAME_LENGTH)?CMN_NAME_LENGTH:length;
		
		char *filename = cmn_malloc(length);
		//snprintf(filename, length, "%s/%s", path, item->valuestring);
		snprintf(filename, length, "%s", item->valuestring);
		cmn_list_append( filelist, filename);
		
		MUX_DEBUG("No.%d Media File: '%s'", i, filename);
	}

	res = cmnMuxFtpAddEvent(server, username, password, path, filelist, dataConn);

	if(res  == EXIT_SUCCESS)
	{/* delay for thread/FSM process */
		dataConn->errCode = IPCMD_ERR_IN_PROCESSING;
	}
	else
	{/* jsonEvent not sent out, free it */
		dataConn->errCode = IPCMD_ERR_SERVER_INTERNEL_ERROR;
	}
	
	return res;
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


