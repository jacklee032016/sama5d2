
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

static int	_ctrlEventHandler(struct _CmnThread *th, void *_event)
{
//	MuxMain *muxMain = (MuxMain *)th->data;

	CMN_JSON_EVENT *jsonEvent = (CMN_JSON_EVENT *)_event;
	int res = EXIT_SUCCESS;

	res = cmnMuxCtrlDataHandle(jsonEvent->dataConn);
#if 0	
	if (SYSTEM_IS_EXITING())
	{
		MUX_PLAY_WARN( "'%s' Thread recv EXIT signal\n", th->name );
		return -EXIT_FAILURE;
	}
#endif
	return res;
}


static int _initCtrlThread(CmnThread *th, void *data)
{
	int res = EXIT_SUCCESS;
//	MuxMain *muxMain = (MuxMain *)data;

	cmnThreadMask(th->name);
	th->data = data;

	return res;
}

static void _destoryCtrlThread(struct _CmnThread *th)
{

}


CmnThread  threadController =
{
	name		:	"Manager",
	flags			:	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	init			:	_initCtrlThread,
	mainLoop		:	NULL,
	eventHandler	:	_ctrlEventHandler,
	destory		:	_destoryCtrlThread,
	data			:	NULL,
};



int cmnMuxControllerAddEvent(char *cmd, 	int method, void *dataConn)
{
	CMN_JSON_EVENT *jsonEvent =(CMN_JSON_EVENT *)cmn_malloc(sizeof(CMN_JSON_EVENT));
	if(jsonEvent == NULL )
	{
		MUX_ERROR("No memory for JSON Event to Controller");
		return EXIT_FAILURE;
	}

	snprintf(jsonEvent->cmd, sizeof(jsonEvent->cmd), "%s", cmd);
	jsonEvent->method = method;
	
	jsonEvent->dataConn = (struct DATA_CONN *)dataConn;

	return cmnThreadAddEvent(&threadController, jsonEvent);
}


