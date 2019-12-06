/*
* Monitor button to reboot/reset
*/

#include <termios.h>
#include <signal.h>
#include "libCmnSys.h"

#define	CMN_BNT_CTL_LOG				"/var/log/btnCtrl.log"
#define CMN_MODULE_BTN_CTRL			"btnCtrl"


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


int main(int argc, char *argv[])
{
	MuxMain		__muxMain;
	MuxMain		*muxMain = &__muxMain;
	int res;

	printf(CMN_VERSION_INFO(CMN_MODULE_BTN_CTRL) EXT_NEW_LINE );

	memset(muxMain, 0 , sizeof(MuxMain) );
	muxMain->runCfg.muxMain = muxMain;

	if( cmnMuxMainParse(MUX_BTN_CONFIG_FILE, muxMain ) )
	{
		fprintf(stderr, "Incorrect config file : %s - exiting.\n", MUX_BTN_CONFIG_FILE);
		exit(1);
	}

	if (!strcmp(muxMain->muxLog.logFileName, "-"))
	{
		muxMain->muxLog.lstyle = USE_CONSOLE;
	}

//	cmnSysCfgFromFactory(runCfg);

	res = cmnSysCfgRead(&muxMain->runCfg, EXT_CFG_MAIN);
	if(res == EXIT_FAILURE)
	{/* TX/RX must be get before default configuration*/

		MUX_INFO("Please reboot to make it effective after it finished"EXT_NEW_LINE );
		return EXIT_FAILURE;	/* quit from running */
	}

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

	MUX_WARN("Timer initialize...");
	res = cmn_timer_init(CMN_THREAD_NAME_TIMER);
	if(res != EXIT_SUCCESS)
	{
		MUX_ERROR("timer initialization failed");
		exit(1);
	}

	extSysNetInit(muxMain);


	MUX_WARN("Start thread '%s' .....", threadButton.name );
	res = cmnThreadInit( &threadButton, muxMain);
	if(res == EXIT_SUCCESS)
	{
	}

	cmnThreadJoin( &threadButton);

	EXT_INFOF(EXT_NEW_LINE"OK");
	return 0;
}

