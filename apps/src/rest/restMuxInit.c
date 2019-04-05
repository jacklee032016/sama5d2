
#include "restMux.h"

int cgiMuxInit(MUX_WEB_CMN *muxWeb)
{
//	CGI_HTML_HEADE();

	CGI_JSON_HEADE();

	memset(muxWeb, 0, sizeof(MUX_WEB_CMN));
	read_cgi_input(&muxWeb->cgiVariables, NULL, NULL);

#if 0
	if( cmnMuxMainParse(MUX_MAIN_CONFIG_FILE, &muxWeb->cgiMain ) )
	{
		cgi_error_page(NULL, gettext(CGI_STR_ERROR), gettext(CGI_STR_WITHOUT_CONF_FILE) );
		return -1;
	}

	if( cmnMuxPlayerParseConfig(MUX_PLAYER_CONFIG_FILE, &muxWeb->cgiCfgPlay) )
	{
		cgi_error_page(NULL, gettext(CGI_STR_ERROR), gettext(CGI_STR_WITHOUT_CONF_FILE) );
		return -1;
	}

	cmnMuxConfigParseWeb(MUX_WEB_CONFIG_FILE, &muxWeb->cgiCfgWeb);

	if ( cmnMuxConfigParseRecord(MUX_RECORDER_CONFIG_FILE, &muxWeb->cgiCfgRecord ) < 0)
	{
		cgi_error_page(NULL, gettext(CGI_STR_ERROR), gettext(CGI_STR_WITHOUT_CONF_FILE) );
		return EXIT_FAILURE;
	}
#endif	

#if 0
	/* web server redirect all stderr of CGI to its log file, so CGI needs no real log, just output to its stderr */
	sprintf(muxWeb->cfg.serverLog.name, "%s", "WebAdmin");
	sprintf(muxWeb->cfg.serverLog.logFileName, "%s", CMN_LOG_FILE_WEBADMIN);

	muxWeb->cfg.serverLog.isDaemonized = 0;	/* no matter what configuration of player is, webAdmin never runs as daemon */
	if(cmn_log_init(&muxWeb->cfg.serverLog)<0)
	{
		printf("%s Log Init Failed.\n", muxWeb->cfg.serverLog.name );
	}
#endif

//	cmnMediaInit(&muxWeb->cgiMain.mediaCaptureConfig);

	return CLIENT_INIT_REST();

}


