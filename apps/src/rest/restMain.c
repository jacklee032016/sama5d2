/*
* 
*/

#include "restMux.h"

static int __status_init(MUX_SERVER_INFO *svrInfo)
{
	memset(svrInfo, 0, sizeof(MUX_SERVER_INFO));
	
	return cgiMuxInit( &svrInfo->cgiMux);
}


int main(int argc, char *argv[])
{
	MUX_SERVER_INFO	_svrInfo;
	MUX_SERVER_INFO	*svrInfo = &_svrInfo;
	char 	*cmd=NULL;
//	int res = 0;

	if(__status_init(svrInfo) == EXIT_FAILURE )
	{
//		cgi_error_page(NULL, gettext("REST Status Operation Failed"), gettext( CGI_STR_SOME_ERROR ) );
		svrInfo->cgiMux.response = cJSON_CreateString("Internal error");
//		return 1;
	}
	else
	{
		svrInfo->cgiMux.response = muxApiGetParams();
	}

	char *printedStr =cJSON_Print((svrInfo->cgiMux.response));
	printf("%s", printedStr);
	free(printedStr);

#if 0
	cmd = GET_CGI_OP(&svrInfo->cgiMux.cgiVariables);
	if( !cmd || !strlen(cmd) || !strcasecmp(cmd, CGI_MUX_SERVER_OP_CONFIG ) )
	{
		return	muxServerUrlConfig(svrInfo);
	}
	else if( !strcasecmp(cmd, CGI_MUX_SERVER_OP_SERVER) )
	{
		return muxCgiSvrRuntime(svrInfo);
	}
	else
	{
		cgi_invalidate_page();
		return 0;
	}
#endif

	return 0;	
}


