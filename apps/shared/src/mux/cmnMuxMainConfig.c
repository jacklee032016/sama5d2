
#include <ctype.h>


#include "libCmn.h"
#include "libMux.h"

static int _parseGlobalConfig(const char **p, MuxMain *muxMain, int linenum)
{
	char cmd[64];
	char arg[1024];
	int val;
	int ret = EXIT_SUCCESS;
	
	cmnParseGetArg(cmd, sizeof(cmd), p);
	
	/***** configuration for log *****/
	if (!strcasecmp(cmd, "MuxBoard"))
	{
		cmnParseGetArg(arg, sizeof(arg), p);

		if (!strcasecmp(arg, "RX744"))
		{
			muxMain->boardType = MUX_BOARD_TYPE_774;
		}
		else
		{
			muxMain->boardType = MUX_BOARD_TYPE_UNKNOWN;
		}	
#if DEBUG_CONFIG_FILE
		MUX_INFO("MuxBoard: %d", arg);
#endif
	}
	else if (!strcasecmp(cmd, "AUTHEN"))
	{
		muxMain->isAuthen = cmnParseGetBoolValue(p);
#if DEBUG_CONFIG_FILE
		MUX_INFO("AUTHEN: %s", STR_BOOL_VALUE(muxMain->isAuthen));
#endif
	}
	else if (!strcasecmp(cmd, "Daemon"))
	{
		muxMain->muxLog.isDaemonized = cmnParseGetBoolValue(p);
#if DEBUG_CONFIG_FILE
		if(muxMain->muxLog.isDaemonized )
		{
			MUX_INFO("running as daemon");
		}	
		else
		{
			MUX_INFO("running as front-end");
		}
#endif
	}
	else if (!strcasecmp(cmd, "CustomLog"))
	{
		cmnParseGetArg(muxMain->muxLog.logFileName, 256, p);
		if (!strcmp(muxMain->muxLog.logFileName, "-"))
		{
			muxMain->muxLog.lstyle = USE_CONSOLE;
		}
	}
	else if (!strcasecmp(cmd, "MaxLogSize"))
	{
		muxMain->muxLog.maxSize = 2048*UNIT_OF_KILO;//cmnParseGetIntValue(p);
	}
	else if (!strcasecmp(cmd, "DebugLevel"))
	{
		val = cmnParseGetIntValue(p);
		if(val < CMN_LOG_EMERG || val > CMN_LOG_DEBUG)
		{
			val = CMN_LOG_NOTICE;
		}
		muxMain->muxLog.llevel = val;
	}

	else if (!strcasecmp(cmd, "UDPCtrlPort"))
	{
		muxMain->udpCtrlPort = cmnParseGetIntValue(p);
#if DEBUG_CONFIG_FILE
		MUX_INFO("Controller UDP Port: %d", muxMain->udpCtrlPort );
#endif
	}
	else if (!strcasecmp(cmd, "TCPCtrlPort"))
	{
		muxMain->tcpCtrlPort = cmnParseGetIntValue(p);
#if DEBUG_CONFIG_FILE
		MUX_INFO("Controller TCP Port: %d", muxMain->tcpCtrlPort );
#endif
	}
	else if (!strcasecmp(cmd, "UNIXPort"))
	{
		cmnParseGetArg(muxMain->unixPort, CMN_NAME_LENGTH, p);
#if DEBUG_CONFIG_FILE
		MUX_INFO("Controller UNIX Port: %s", muxMain->unixPort);
#endif
	}

	else
	{
		MUX_ERROR("Invalidate configuration item: %s on line %d", cmd, linenum);
		ret = EXIT_FAILURE;
	}


	return ret;
}


static int _parsePlugin(MuxPlugIn *plugin, const char **p, int linenum)
{
	char cmd[64];
	int	ret = EXIT_SUCCESS;

	if(plugin==NULL)
		return EXIT_FAILURE;

	cmnParseGetArg(cmd, sizeof(cmd), p);

	if (!strcasecmp(cmd, "Enable"))
	{
		plugin->enable = cmnParseGetBoolValue(p);
#if DEBUG_CONFIG_FILE
		MUX_INFO("\tPlugin Enable: %s", (plugin->enable==FALSE)?"NO":"YES");
#endif
	}
	else if (!strcasecmp(cmd, "Lib"))
	{
		cmnParseGetArg(plugin->path, sizeof(plugin->path),  p);
#if DEBUG_CONFIG_FILE
		MUX_INFO("\tPlugin Library: %s", plugin->path);
#endif
	}
	else
	{
		MUX_ERROR("Invalidate configuration item: %s on line %d", cmd, linenum);
		ret = EXIT_FAILURE;
	}
	
	return ret;
}


int cmnMuxMainParse(const char *filename, MuxMain *muxMain)
{
#define	PARSE_GLOBAL		0
#define	PARSE_PLUGIN		1
#define	PARSE_PLAYLIST		2

	FILE *f;
	char line[1024];
	char cmd[64];
	const char *p;
	int line_num = 0;
	int parseState = PARSE_GLOBAL;
	int ret = EXIT_SUCCESS;

	MuxPlugIn	*plugin = NULL;
	
	f = fopen(filename, "r");
	if (!f)
	{
		perror(filename);
		return EXIT_FAILURE;
	}

	/** it must be initialized as all 0s before enter into this function */
//	memset(cfg, 0, sizeof(MuxPlayerConfig));	

	muxMain->muxLog.lfacility = CMN_LOG_7;
	muxMain->muxLog.lstyle = USE_FILE;
	muxMain->muxLog.llevel  = CMN_LOG_DEBUG;
	muxMain->muxLog.isDaemonized = TRUE;

	cmn_list_init(SYS_PLAYLISTS(muxMain) );
	muxMain->playlistLock = cmn_mutex_init();

	snprintf(muxMain->configFileName, CMN_NAME_LENGTH, "%s", filename );

	for(;;)
	{
		if (fgets(line, sizeof(line), f) == NULL)
			break;
		
		line_num++;
		p = line;
		while (isspace(*p)) 
			p++;
		
		if (*p == '\0' || *p == '#')
			continue;


		if ( strstr(line, "<PlugIn") )
		{
			char *q;

			cmnParseGetArg(cmd, sizeof(cmd), &p);
			if (plugin )
			{
				MUX_ERROR("%s:%d: Already in a tag for <plugin>", filename, line_num);
				ret = EXIT_FAILURE;
				break;
			}
			else
			{
				plugin = cmn_malloc(sizeof(MuxPlugIn));
				plugin->muxMain = muxMain;

				
				cmnParseGetArg(plugin->name, sizeof(plugin->name), &p);
				q = strrchr(plugin->name, '>');
				if (*q)
					*q = '\0';
				
				if (!strcasecmp(plugin->name, "PLAYER"))
				{
					plugin->type = MUX_PLUGIN_TYPE_PLAYER;
				}
				else if (!strcasecmp(plugin->name, "SERVER"))
				{
					plugin->type = MUX_PLUGIN_TYPE_SERVER;
				}
				else if (!strcasecmp(plugin->name, "WEB"))
				{
					plugin->type = MUX_PLUGIN_TYPE_WEB;
				}
				else if (!strcasecmp(plugin->name, "RECORDER"))
				{
					plugin->type = MUX_PLUGIN_TYPE_RECORDER;
				}
				else
				{
					plugin->type = MUX_PLUGIN_TYPE_UNKNOWN;
					MUX_ERROR("Unknow plugin type '%s'", plugin->name);
				}
#if DEBUG_CONFIG_FILE
				MUX_INFO("PlugIn: %s", plugin->name );
#endif
			}

			parseState = PARSE_PLUGIN;

			continue;
		}
		else if ( strstr(line, "</PlugIn>"))
		{
			if (!plugin)
			{
				MUX_ERROR("%s:%d: No corresponding <PlugIn> for </PlugIn>", filename, line_num);
				ret = EXIT_FAILURE;
				break;
			}

			ADD_ELEMENT(muxMain->plugins, plugin);

			plugin = NULL;
			parseState = PARSE_GLOBAL;
			continue;
		}
		
		if(parseState == PARSE_GLOBAL)
		{
			ret = _parseGlobalConfig( &p, muxMain, line_num);
		}
		else
		{
			ret = _parsePlugin(plugin, &p, line_num);
		}
		
		if(ret == -EXIT_FAILURE )
		{
			MUX_ERROR("%s:%d: Incorrect keyword: '%s'", filename, line_num, cmd);
			break;
		}
		
	}

	fclose(f);

	return ret;
}


