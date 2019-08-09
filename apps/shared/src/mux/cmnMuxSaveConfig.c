
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#include "libCmn.h"
#include "libMux.h"

static int _savePrompt(FILE *f)
{
	int res = EXIT_SUCCESS;
	struct timeval tv;
	struct tm *ptm;

	char date[32] = {0};
	gettimeofday(&tv, NULL);
	ptm = localtime(&tv.tv_sec);
	strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", ptm);

	res = fprintf(f, "## Configuration file created by program, don't modify it manually##\n");
	res = fprintf(f, "## Datetime created : %s\n##\n\n", date);

	return res;
}

static int _savePlayerGlobalConfig(FILE *f, MuxPlayerConfig *cfg)
{
	int res = 0;

	res = fprintf(f, "### Global configuration items of PLAYER## \n");

	res = fprintf(f, "## URLs ## \n");
	res = fprintf(f, "PlayURL\t\t%s\n", cfg->playUrl);
	res = fprintf(f, "URL_HTTP\t\t%s\n", cfg->urlHttp);
	res = fprintf(f, "URL_RTSP\t\t%s\n", cfg->urlRtsp);
	res = fprintf(f, "URL_WMSP\t\t%s\n", cfg->urlWmsp);
	res = fprintf(f, "URL_RTMP\t\t%s\n", cfg->urlRtmp);
	res = fprintf(f, "URL_RTP\t\t%s\n", cfg->urlRtp);
	res = fprintf(f, "URL_UDP\t\t%s\n", cfg->urlUdp);


	res = fprintf(f, "VideoFormat\t\t%s\n", cfg->displayFormat);

	res = fprintf(f, "AudioVolume\t\t%d\n", cfg->audioVolume );

	res = fprintf(f, "TimeoutLocal\t\t%d\n", cfg->playTimeoutLocal);
	res = fprintf(f, "TimeoutNetwork\t\t%d\n", cfg->playTimeoutNetwork);


	res = fprintf(f, "CaptureName\t\t%s\n", cfg->captureName);

	return res;
}



int cmnMuxSavePlayerConfig( MuxPlayerConfig *cfg)
{
	FILE *f;
	int res = EXIT_SUCCESS;
//	int i;

	f = fopen(cfg->configFileName, "w");
	if (!f)
	{
		perror(cfg->configFileName);
		return EXIT_FAILURE;
	}

	_savePrompt(f);

	res = _savePlayerGlobalConfig(f, cfg);

	fclose(f);

	return res;
}



#if 0
static int _savePluginConfig(int index, void *ele,  void *data)
{
	FILE *f = (FILE *)data;
	RECT_CONFIG *win = (RECT_CONFIG *)ele;
	int res = 0;
	char *type = "SUBTITLE";

	if(win->type == RECT_TYPE_MAIN)
		type = "MAIN";
	if(win->type == RECT_TYPE_PIP)
		type = "PIP";
	if(win->type == RECT_TYPE_SUBTITLE)
		type = "SUBTITLE";
	if(win->type == RECT_TYPE_ALERT)
		type = "ALERT";
	if(win->type == RECT_TYPE_LOGO)
		type = "LOGO";
	
	res = fprintf(f, "<Window  %s>\n", type);

	res = fprintf(f, "\tLeft\t\t%d\n", win->left);

	res = fprintf(f, "\tTop\t\t%d\n", win->top);

	res = fprintf(f, "\tWidth\t\t%d\n", win->width);

	res = fprintf(f, "\tHeight\t\t%d\n", win->height);

	res = fprintf(f, "\tFontSize\t\t%d\n", win->fontSize);
	res = fprintf(f, "\tFontColor\t\t0x%8x\n", win->fontColor);

	res = fprintf(f, "\tBackgroundColor\t\t0x%8x\n", win->backgroundColor);

	res = fprintf(f, "\tAlpha\t\t%d\n", win->alpha);

	res = fprintf(f, "\tEnable\t\t%s\n", STR_BOOL_VALUE(win->enable));

	res = fprintf(f, "\tROTATE_MODE\t\t%d\n", win->rotateType);

	res = fprintf(f, "\tURL\t\t%s\n", win->url);

	res = fprintf(f, "</Window>\n\n");
	
	return res;	
}
#endif

static int _saveGlobalConfig(FILE *f, MuxMain *muxMain)
{
	int res = 0;
//	char		*name;

	res = fprintf(f, "### Global configuration items## \n");
	res = fprintf(f, "MuxBoard\t\t%s\n\n", (muxMain->boardType==MUX_BOARD_TYPE_RX769)?"RX769":"RX762");
	res = fprintf(f, "# Controller configuration #\nUDPCtrlPort\t\t%d\n\n", muxMain->udpCtrlPort );
	res = fprintf(f, "TCPCtrlPort\t\t%d\n\n", muxMain->tcpCtrlPort );
	res = fprintf(f, "UNIXPort\t\t%s\n\n\n", muxMain->unixPort );

	/* log config */
	res = fprintf(f, "## Logging configuration ##\nCustomLog\t\t%s\n\n", muxMain->muxLog.logFileName);
	res = fprintf(f, "MaxLogSize\t\t%dK\n\n", muxMain->muxLog.maxSize/UNIT_OF_KILO);
	res = fprintf(f, "DebugLevel\t\t%d\n\n", muxMain->muxLog.llevel);
	res = fprintf(f, "Daemon\t\t%s\n\n\n", STR_BOOL_VALUE(muxMain->muxLog.isDaemonized) );
	
	return res;
}

static int _cmnSavePlugin(FILE *f, MuxPlugIn *plugin, int i, char *name)
{
	int res =0;
	
	res = fprintf(f, "\n# No.%d Plugin configuration. This one is first No.%d load to run #\n<PlugIn %s>\n", i, 4-i, name);
	res = fprintf(f, "\tLib\t\t%s\n\tEnable\t\t%s\n", plugin->path, STR_BOOL_VALUE(plugin->enable) );
	res = fprintf(f, "</PlugIn>\n# End of No.%d Plugin #\n\n", i);

	return res;
}

int cmnMuxSaveAllConfig(MuxMain *muxMain)
{
	FILE *f;
	int res = EXIT_SUCCESS;

	f = fopen(muxMain->configFileName, "w");
	if (!f)
	{
		perror(muxMain->configFileName);
		return EXIT_FAILURE;
	}

	res = _saveGlobalConfig(f, muxMain);
	
	res = fprintf(f, "\n### PLUGIN configuration items## \n");
	/* the first one, saved in file, is the last one load to run */
	res = _cmnSavePlugin(f, cmnMuxPluginFind(muxMain, MUX_PLUGIN_TYPE_RECORDER ), 0, "RECORDER");
	res = _cmnSavePlugin(f, cmnMuxPluginFind(muxMain, MUX_PLUGIN_TYPE_SERVER ), 1, "SERVER");
	res = _cmnSavePlugin(f, cmnMuxPluginFind(muxMain, MUX_PLUGIN_TYPE_WEB ), 2, "WEB");
	res = _cmnSavePlugin(f, cmnMuxPluginFind(muxMain, MUX_PLUGIN_TYPE_PLAYER ), 3, "PLAYER");

	fclose(f);

//	res = _saveCrontableForPlaylist(cfg);

#if 0
	res = cmnMuxSavePlayerConfig( &muxMain->muxPlayer->playerConfig);
#endif
	return res;

}

