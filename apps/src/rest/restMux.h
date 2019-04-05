
#ifndef	__LIB_CGI_MUX_H__
#define	__LIB_CGI_MUX_H__

#include "libCmn.h"
#include "libMedia.h"
#include "libMux.h"

#include "cmnMuxApi.h"

#include "libCgi.h"

#define	CMN_LOG_FILE_WEBADMIN	"/tmp/cgi.log"

/*
 Common data structure for all CGI program
*/
typedef	struct
{
	llist						cgiVariables;

#if 0
	MuxMain					cgiMain;
	MuxPlayerConfig			cgiCfgPlay;

	CmnMuxRecordConfig		cgiCfgRecord;
	CmnMuxWebConfig			cgiCfgWeb;
#endif

//	CMG_MQ_T			mqCtx;
	cJSON				*response;
}MUX_WEB_CMN;

#include "_restMux.h"

int cgiMuxInit(MUX_WEB_CMN *muxWeb);


#endif

