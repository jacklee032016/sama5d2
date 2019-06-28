
#include "libCmn.h"
#include "libMux.h"

#include <sys/stat.h>

#include "_cmnMux.h"

struct	CMN_SDP_CONTROLLER
{
	char			localPath[CMN_NAME_LENGTH];
	int			freeSpace;	/* used to extended */

	CMN_FTP_CLIENT	*currentReq;

	int				downloadSize;	/* how many bytes have been downloaded for all requests */
	int				totalReqs;
	int				totalSuccess;
	int				totalFailed;

	void				*timer;
	MuxMain			*muxMain;

	void				*priv;	/*save MuxPlayerConfig **/
};

typedef	struct _SDP_REQ
{
	HttpClientReq		req;
	int				index;
	
	void 		*priv;
}CMN_FTP_CLIENT;


static int _cmnFtpClientConnect(CMN_FTP_CLIENT *client, struct CMN_FTP_CONTROLLER *ftpAgent)
{
	if (!FtpConnect(client->host,&client->ctrlConn))
	{
		MUX_ERROR("Unable to connect to node %s", client->host);
		return IPCMD_ERR_FTP_SERVER_ERROR;
	}
	
	if (!FtpLogin(client->username,client->password, client->ctrlConn))
	{
		MUX_ERROR("Login failure: %s", FtpLastResponse(client->ctrlConn));
		return IPCMD_ERR_FTP_USER_PASSWORD;
	}
	
	FtpClearCallback(client->ctrlConn);

	return EXIT_SUCCESS;
}

static int _cmnFtpClientGetFiles(CMN_FTP_CLIENT *client, struct CMN_FTP_CONTROLLER *ftpAgent)
{
	char		localFile[CMN_NAME_LENGTH];
	char		remoteFile[CMN_NAME_LENGTH];
	fsz_t		fsz;
	char		mode = 'I';
	fsz_t		returnSz = 0;
	int i;
	
	for(i=0; i< cmn_list_size(client->downloadFileList); i++)
	{
		char *file = cmn_list_get(client->downloadFileList, i);
		
		snprintf(remoteFile,CMN_NAME_LENGTH, "%s/%s", client->remotePath, file);
		snprintf(localFile, CMN_NAME_LENGTH, "%s/%s", ftpAgent->localPath, file);

#if 0
		if(cmnMediaCheckSDCardFile(localFile) )
		{
			cmnMediaRemountSDCard(TRUE);
		}
#endif

		
#if defined(__UINT64_MAX)
		if (!FtpSizeLong(remoteFile, &fsz, mode, client->ctrlConn))
#else
		if (!FtpSize(remoteFile, &fsz, mode, client->ctrlConn) )
#endif
		{	
			MUX_ERROR("Retrieving length of remote file '%s' failed", remoteFile);
			fsz = 0;	
		}

		client->fsz = fsz;
#if 0		
		fsz /= 100;
		if (fsz > 100000)
			fsz = 100000;
		if ( fsz == 0 )
			fsz = 32768;

		if (ftplib_debug)
		{
			FtpCallbackOptions opt;
			opt.cbFunc = log_progress;
			opt.cbArg = f;
			opt.idleTime = 1000;
			opt.bytesXferred = fsz;
			FtpSetCallback(&opt,conn);
		}
#endif

		returnSz = FtpGet(localFile, remoteFile, mode, client->ctrlConn);
		if(!returnSz)
		{
			MUX_ERROR("Get No. %d remote file '%s', local file '%s' failed", i, remoteFile, localFile);
			return IPCMD_ERR_FTP_DATA_ERROR;
		}
		
		ftpAgent->totalSuccess++;
		ftpAgent->downloadSize += fsz;

#if 0
		if(cmnMediaCheckSDCardFile(localFile) )
		{
			cmnMediaRemountSDCard(FALSE);
		}
#endif

		cmn_list_append(&client->downloadResults, strdup(localFile));
		MUX_DEBUG("%d bytes have been transferred sucessfully for '%s'", fsz, localFile);
	}
	
	return EXIT_SUCCESS;
}

static void _cmnFtpClientClose(CMN_FTP_CLIENT *client)
{
	cmn_list_ofchar_free(&client->downloadResults, FALSE);
	cmn_list_ofchar_free(client->downloadFileList, TRUE);
	
	if (client->ctrlConn)
	{
		FtpClose(client->ctrlConn);
		client->ctrlConn = NULL;
	}
	
	cmn_free(client);
}


static int _cmnFtpHandlerDownload(void *priv, struct DATA_CONN *dataConn, CMN_PLAY_JSON_EVENT *jEvent)
{
	struct	CMN_FTP_CONTROLLER *ftpAgent = (struct	CMN_FTP_CONTROLLER *)priv;
	int res = EXIT_SUCCESS;
	CMN_FTP_CLIENT *client = (CMN_FTP_CLIENT *)jEvent->object;
	cJSON *fileObjs = NULL;
	int	i;

#if 0//ARCH_ARM
	if (SYSTEM_IS_EXITING())
	{
		MUX_WARN(CMN_LOG_WARNING, "Thread '%s' recv EXIT signal\n", th->name);
		return -EXIT_FAILURE;
	}
#endif

	dataConn->errCode = IPCMD_ERR_NOERROR;
	ftpAgent->currentReq = client;
	res = _cmnFtpClientConnect(client, ftpAgent);
	if( res == EXIT_SUCCESS)
	{
		res = _cmnFtpClientGetFiles(client, ftpAgent);
	}

	if(res != EXIT_SUCCESS)
	{
		dataConn->errCode = res;
		return EXIT_SUCCESS;
	}

	if( (cmn_list_size(&client->downloadResults) /*!= cmn_list_size(&client->downloadFileList)*/ )
		&& dataConn->errCode != IPCMD_ERR_NOERROR)
	{
		MUX_DEBUG( "DATA_CONN error code: '%d' ", dataConn->errCode);
		dataConn->errCode = IPCMD_ERR_FTP_PARTLY_FAILED;
	}
	
	fileObjs = cJSON_CreateArray();
	for(i=0; i< cmn_list_size(&client->downloadResults); i++)
	{
		char	*filename = (char *)cmn_list_get(&client->downloadResults, i);
		MUX_DEBUG( "No.%d: '%s' have been saved sucessfully", i+1, filename);
		cJSON_AddItemToArray(fileObjs, cJSON_CreateString(filename));
	}
	dataConn->resultObject = fileObjs;
	
	_cmnFtpClientClose(client);

	return EXIT_SUCCESS;
}


static PluginJSonHandler _jsonFtpHandlers[] =
{
	{
		.type	= CMD_TYPE_SET_MEDIA,
		.name 	= "download",
		handler	: _cmnFtpHandlerDownload
	},
	{
		.type	= CMD_TYPE_UNKNOWN,
		.name	= NULL,
		.handler	= NULL
	}
};

static int	_sdpEventHandler(struct _CmnThread *th, void *_event)
{
	struct	CMN_FTP_CONTROLLER *ftpAgent = (struct	CMN_FTP_CONTROLLER *)th->data;

	CMN_PLAY_JSON_EVENT *jsonEvent = (CMN_PLAY_JSON_EVENT *)_event;
	int res = EXIT_SUCCESS;

	res = cmnMuxJSonPluginHandle(ftpAgent, _jsonFtpHandlers, jsonEvent);

#if 0	
	if (SYSTEM_IS_EXITING())
	{
		MUX_PLAY_WARN( "'%s' Thread recv EXIT signal\n", th->name );
		return -EXIT_FAILURE;
	}
#endif
	return res;
}


int cmnMuxSdpAddEvent(HttpClientReq *sdpUri)
{
	CMN_FTP_CLIENT *client =(CMN_FTP_CLIENT *)cmn_malloc(sizeof(CMN_FTP_CLIENT));

	CMN_PLAY_JSON_EVENT *jsonEvent =(CMN_PLAY_JSON_EVENT *)cmn_malloc(sizeof(CMN_PLAY_JSON_EVENT));
	if(jsonEvent == NULL)
	{
		EXT_ERRORF("No memory available for SDP %s request", sdpUri->uri );
		return EXIT_FAILURE;
	}

	snprintf(jsonEvent->action, JSON_ACTION_LENGTH, "%s", "download");
	jsonEvent->object = (cJSON *)client;
	jsonEvent->priv = dataConn;

	return cmnThreadAddEvent(&threadSdpClient, jsonEvent);
}

int cmnMuxSdpTimerCallback(int interval, void *param)
{
	CMN_SDP_CONTROLLER *sdpCtx = (CMN_SDP_CONTROLLER *)param;
	EXT_ASSERT((sdpCtx != NULL), "Params fault in SDP timer callback");

	sdpCtx->totalReqs++;
	
	EXT_INFOF("SDP Timer is called, total %d requests", sdpCtx->totalReqs);
}

static int _initSdpThread(CmnThread *th, void *data)
{
	int res = EXIT_SUCCESS;

	CMN_SDP_CONTROLLER *sdpCtx = cmn_malloc(sizeof(CMN_SDP_CONTROLLER));
	EXT_ASSERT( (sdpCtx != NULL), "SDP context can not be null");
	
	sdpCtx->muxMain = (MuxMain *)data;
	EXT_ASSERT((sdpCtx->muxMain != NULL), ("MuxMain in SDP Context can not be null"));
	
	sdpCtx->timer = cmn_add_timer(sdpCtx->muxMain->pollTime, cmnMuxSdpTimerCallback, sdpCtx, cmn_timer_type_reload, "sdpTimer");
	if(sdpCtx->timer == NULL)
	{
		EXT_ERRORF("Timer is not created for SDP client in RX");
	}

	cmnThreadMask(th->name);
	th->data = sdpCtx;

	return EXIT_SUCCESS;
}

static void _destorySdpThread(struct _CmnThread *th)
{
	struct	CMN_FTP_CONTROLLER *ftpAgent = (struct	CMN_FTP_CONTROLLER *)th->data;

	cmn_free(ftpAgent);
}



CmnThread  threadSdpClient =
{
	name		:	"SDP",
	flags			:	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	init			:	_initSdpThread,
	mainLoop		:	NULL,
	eventHandler	:	_sdpEventHandler,
	destory		:	_destorySdpThread,
	data			:	NULL,
};

