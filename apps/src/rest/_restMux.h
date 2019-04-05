/*
*/

#ifndef __REST_MUX_H__
#define __REST_MUX_H__

#include "libCgi.h"
#include "libCmn.h"

#define	CGI_OPTIONS_WINDOW_LIST		1


typedef	struct 
{
	char		lastFile[2048];
	char		currentFile[2048];
	
	char		status[128];
	
	int		totalTime;

	int		totalBytes;
	int		packets;
}_RECORDER_STATUS;

typedef	struct
{
	int		startTime;
	int		duration; /* ms */
	int		videoFormat;	
	int		capLevel;
	int		frameRate;

	int		audioFormat;
}RECORDER_CONFIG;

typedef	struct
{
	MUX_WEB_CMN		cgiMux;

	llist					cfgList;/* for the URLs which are only accessed by web */

	RECORDER_CONFIG		recordCfg;
	_RECORDER_STATUS 	recordStatus;

}MUX_SERVER_INFO;


#define 	MUX_SERVER_CONFIG_FILE	"/etc/muxMedia.conf"

int	muxCgiSvrRuntime(MUX_SERVER_INFO *svrInfo);

#endif

