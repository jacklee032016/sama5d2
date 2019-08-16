/*
* $Id$
*/

#include "mux.h"

#include "ms_version.h"


static	MuxMain		_muxMain;

/* init terminal so that we can grab keys */
static struct termios _oldtty;

volatile sig_atomic_t recvSigalTerminal = FALSE;

#define	DISABLE_SEGMENT_FAULT	0

#if DISABLE_SEGMENT_FAULT
void sig_seg(void)
{
	printf("sig_seg\n");
	fflush(stdout);
	while(1)
		;
}
#endif

static void doExit(MuxMain *muxMain)
{
	fprintf(stderr, "doExit" );
}

static void termExit(void)
{
	MUX_INFO("termExit" );
	
//	if(_muxMain.muxLog.isDaemonized)
	{
		tcsetattr (0, TCSANOW, &_oldtty);
	}

	exit(1);
}

static void sigTermHandler(int sig)
{
//	SYSTEM_SIGNAL_EXIT();
	
	MuxPlugIn *plugin = _muxMain.plugins;

	recvSigalTerminal = TRUE;
	while(plugin)
	{
		if(plugin->enable)
		{
			MUX_INFO("plugin '%s' exiting", plugin->name );
			plugin->signalExit(plugin, 1);
		}
		
		plugin = plugin->next;
	}

	termExit();
}

static void termInit(MuxMain	*muxMain)
{
	struct termios tty;

	tcgetattr (0, &tty);
	_oldtty = tty;

	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP |INLCR|IGNCR|ICRNL|IXON); /* input modes */
	tty.c_oflag |= OPOST;/* output modes */
	tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN); /* local modes */
	tty.c_cflag &= ~(CSIZE|PARENB);  /* control modes */
	tty.c_cflag |= CS8; 
	tty.c_cc[VMIN] = 1; /* special characters */
	tty.c_cc[VTIME] = 0;

	if(muxMain->muxLog.isDaemonized)
	{/* change occurs immediately*/
		if(tcsetattr (0, TCSANOW, &tty) )
		{
			fprintf(stderr, "Set Terminal failed:%s", strerror(errno) );
		}
	}

	signal(SIGINT , sigTermHandler); /* Interrupt (ANSI).  */
	signal(SIGQUIT, sigTermHandler); /* Quit (POSIX).  */
	signal(SIGTERM, sigTermHandler); /* Termination (ANSI).  */
	
	/*  register a function to be called at normal program termination */
	atexit(termExit);
	
#ifdef CONFIG_BEOS_NETSERVER
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
#endif
}


static int _muxMainAddPlugIn(MuxPlugIn *plugin, MuxMain *muxMain)
{
	char					name[CMN_NAME_LENGTH];
	int ( *callBack)(MuxPlugIn *plugin);
	int ret;
	
	void *foo;
	void* handle = dlopen (plugin->path, RTLD_NOW);
	if(handle==NULL)
	{
		MUX_ERROR("Dynamic Load %s(%s) Failed : %s", plugin->name, plugin->path, dlerror() );
		exit(1);
	}	

	foo = dlsym (handle, "init");
	if( foo ==NULL)
	{
		MUX_ERROR("Symbol not found in Dynamic Load library %s(%s) : %s",plugin->name, plugin->path, dlerror() );
		exit(1);
	}
	callBack = (int (*)(MuxPlugIn *))foo;

	memcpy(name, plugin->name, sizeof(plugin->name) );
	MUX_DEBUG("Plugin '%s' ..... ", name );
	ret = callBack(plugin );
	MUX_INFO("Plugin '%s' (%s %s) ended!\n", name, plugin->name, plugin->version );

	return ret;
}

static int _muxMainReportEvent(MuxMain *muxMain, MUX_PLUGIN_TYPE pluginType, CMN_PLAY_JSON_EVENT *jEvent)
{
	MuxPlugIn	*plugin = NULL;

	plugin = muxMain->plugins;
	while(plugin)
	{
		if(plugin->type == pluginType)
		{
			if(!plugin->reportEvent)
			{
				MUX_WARN("No report event interface in plugin '%s'", plugin->name);
			}
			else
			{
				plugin->reportEvent(plugin, jEvent);

				return EXIT_SUCCESS;
			}
		}
		plugin = plugin->next;
	}
	
	MUX_WARN("Event of Plugin Type %d is not handled", pluginType);
	return EXIT_FAILURE;
}

/* return thread ID or -1 when error */
static int _muxMainStartThreadEvent(MuxMain *muxMain, CmnThread *thread, void *data)
{
	int res = EXIT_FAILURE;
	
	MUX_DEBUG("Start thread '%s' .....", thread->name );
	res = cmnThreadInit( thread, data);
	if(res == EXIT_SUCCESS)
	{
		MuxThread *muxThread = cmn_malloc(sizeof(MuxThread));
		if(muxThread == NULL)
		{
			abort();
		}
		muxThread->thread = thread;

		ADD_ELEMENT(muxMain->threads, muxThread);
//		return (int )thread->id;
		return 1;
	}

	return -EXIT_FAILURE;
}


int main(int argc, char **argv)
{
//	struct sigaction sigact;
	MuxMain		*muxMain = &_muxMain;
	MuxPlugIn	*plugin = NULL;
	MuxThread 	*muxThread = NULL;
	
	int res = EXIT_SUCCESS;

	CMN_SHARED_INIT();

	memset(muxMain, 0 , sizeof(MuxMain) );
	muxMain->runCfg.muxMain = muxMain;
	
	muxMain->version.major = EXT_VERSION_MAJOR;
	muxMain->version.minor = EXT_VERSION_MINOR;
	muxMain->version.revision = EXT_VERSION_REVISION;
	snprintf(muxMain->builtDate, sizeof(muxMain->builtDate), "%s %s", __DATE__ , __TIME__ );
#if 0	
	MUX_INFO(CMN_VERSION_INFO(CMN_MODULE_MAIN_NAME));
	fprintf(stderr, CMN_MODULE_MAIN_NAME" " CMN_VERSION_INFO(CMN_MODULE_MAIN_NAME)"\n");
#else
	fprintf(stderr, CMN_MODULE_MAIN_NAME", %02d.%d-%02d, %s\n", muxMain->version.major, muxMain->version.minor, muxMain->version.revision, muxMain->builtDate );
#endif

	cmnThreadSetName(CMN_THREAD_NAME_MAIN);
//	putenv("http_proxy");
	unsetenv("http_proxy");/* Kill the http_proxy */

	srandom(cmnGetTimeMs() + (getpid() << 16));

	if( cmnMuxMainParse(MUX_MAIN_CONFIG_FILE, muxMain ) )
	{
		fprintf(stderr, "Incorrect config file : %s - exiting.\n", MUX_MAIN_CONFIG_FILE);
		exit(1);
	}
	
	snprintf(muxMain->muxLog.name, sizeof(muxMain->muxLog.name), "%s", CMN_MODULE_MAIN_NAME);
	
	muxMain->reportEvent = _muxMainReportEvent;
	muxMain->initThread = _muxMainStartThreadEvent;

	/* signal init */
	signal(SIGPIPE, SIG_IGN);

#if DISABLE_SEGMENT_FAULT
	if(signal(SIGSEGV,(void *)sig_seg)==SIG_ERR)
	{
		MUX_WARN("signal SEG error\n");
	}
#endif

	/* every thread must be created after set terminal attributes */
	termInit(muxMain);

	fprintf(stderr, "Start logging of '%s' .....\n", muxMain->muxLog.logFileName );
	if(cmn_log_init(&muxMain->muxLog)<0)
	{
		printf("%s Log Init Failed.\n", muxMain->muxLog.name );
		return 0;
	}

	muxMain->scf = cmnSysScInit();
	if( muxMain->scf == NULL )
	{
		MUX_ERROR("No Security Chip is found on board!");
//		return 0;
	}
	
	if( cmnSysScValidate(muxMain->scf) == EXIT_FAILURE )
	{/* enable run to configure key into security chip */
		MUX_ERROR("Security validation failed, no authority to access!");
//		return 0;
	}

	if(extSystemInit(muxMain) == EXIT_FAILURE)
	{
		return 0;
	}


#if CMN_TIMER_START_DELAY
#if CMN_TIMER_DEBUG
	fprintf(stderr,"Timer inited");
#endif
	res = cmn_timer_init(CMN_THREAD_NAME_TIMER);
	if(res != EXIT_SUCCESS)
	{
		MUX_ERROR("timer initialization failed");
		exit(1);
	}
#endif

#if 0
	av_register_all();
#endif

#if 0
	cmnMediaInit(&muxMain->mediaCaptureConfig);
#endif

	plugin = muxMain->plugins;
	while(plugin)
	{
		plugin->muxMain = muxMain;
		
		if(plugin->enable && !IS_STRING_NULL_OR_ZERO(plugin->path))
		{
			MUX_DEBUG("Loading plugin '%s' .....", plugin->name );
			res = _muxMainAddPlugIn(plugin, muxMain);
			if( res != EXIT_SUCCESS)
			{
				MUX_ERROR("Loading plugin '%s' failed!",plugin->name );
				exit(1);
			}
		}
		plugin = plugin->next;
	}
	

//	usleep(10*1000);

	if(muxMain->isClientPolling && muxMain->pollTime >0 && !EXT_IS_TX(&muxMain->runCfg) )
	{
		res = muxMain->initThread(muxMain, &threadSdpClient, muxMain);
		if(res < 0 )
		{
			MUX_ERROR("failed when '%s' initializing", threadSdpClient.name);
			exit(1);
		}

		if(muxMain->initThread(muxMain, &threadSdpReceiver, threadSdpClient.data) < 0 )
		{
			MUX_ERROR("Thread %s can't be created, check system errors", threadSdpReceiver.name);
			return EXIT_FAILURE;
		}

	}

	res =  muxMain->initThread(muxMain, &threadBroker, muxMain);
	if(res < 0)
	{
		MUX_ERROR("failed when '%s' initializing", threadBroker.name );
		exit(1);
	}

	res =  muxMain->initThread(muxMain, &threadManager, muxMain);
	if(res < 0)
	{
		MUX_ERROR("failed when '%s' initializing", threadManager.name );
		exit(1);
	}

#if 1
	res =  muxMain->initThread(muxMain, &threadButton, muxMain);
	if(res < 0)
	{
		MUX_ERROR("failed when '%s' initializing", threadButton.name );
		exit(1);
	}
#endif

	/* add polling timer */
	if(muxMain->isClientPolling)
	{
	
	}

	muxThread = muxMain->threads;
	while(muxThread)
	{
//		MUX_DEBUG("No.%d thread '%s' is joined", ++i, muxThread->thread->name );
		cmnThreadJoin( muxThread->thread);
		muxThread = muxThread->next;
	}
	
	MUX_INFO("Threads exited, now free resources!" );

	plugin = muxMain->plugins;
	while(plugin)
	{
		if(plugin->enable && plugin->destory!=NULL )
		{
			plugin->destory(plugin);
		}
		plugin = plugin->next;
	}

	cmnLibDestroy();

	doExit(muxMain);

	MUX_INFO( CMN_MODULE_MAIN_NAME " Exit now!" );
	
	exit(0); /* not all OS-es handle main() return value */
	
	return 0;
}

