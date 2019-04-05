#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdio.h>
#include <unistd.h>
#include "led.h"

static int _muxThLedInit(CmnThread *th, void *data)
{
	MuxMain *muxMain = (MuxMain *)data;
	th->data = muxMain;
	
	muxMain->ledCtrl.fd = open(MUX_DEV_BUTTON, O_RDWR);
	if(muxMain->ledCtrl.fd < 0 )
	{
		MUX_ERROR("can not open "MUX_DEV_BUTTON );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static void _muxThLedDestory(struct _CmnThread *th)
{
	MuxMain *muxMain = (MuxMain *)th->data;

	close(muxMain->ledCtrl.fd);
	muxMain->ledCtrl.fd = 0;
	return;
}


static int _muxThLedMain(CmnThread *th)
{
	int res = EXIT_SUCCESS;
	MuxMain *muxMain = (MuxMain *)th->data;

	int cmd;
	int i;
	char val;

	int currentMode = LED_MODE_ON;

	while(1)
	{
		if(muxMain->ledCtrl.ledMode != currentMode)
		{
			currentMode = muxMain->ledCtrl.ledMode;
		}
		else
		{
			if( currentMode == LED_MODE_OFF)
			{
				ioctl(muxMain->ledCtrl.fd, POWER_LED_OFF,0);
				continue;
			}
			else if( currentMode == LED_MODE_ON)
			{
				ioctl(muxMain->ledCtrl.fd, POWER_LED_ON,0);
				continue;
			}
			else if( currentMode == LED_MODE_BLINK)
			{
				ioctl(muxMain->ledCtrl.fd, POWER_LED_ON,0);
				usleep(300000);
				ioctl(muxMain->ledCtrl.fd, POWER_LED_OFF,0);
				usleep(100000);
				continue;
			}
			
			usleep(200000);			
		}			
	}

}


CmnThread  threadMuxLed =
{
	name		:	"Led",
	flags			:	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	init			:	_muxThLedInit,
	mainLoop		:	_muxThLedMain,
	eventHandler	:	NULL,
	destory		:	_muxThLedDestory,
	data			:	NULL,
};



static int _muxThWatchdogInit(CmnThread *th, void *data)
{
	MuxMain *muxMain = (MuxMain *)data;
	th->data = muxMain;
	
	int timeout;
	int value;
	muxMain->watchdogCtrl.fd = open(MUX_DEV_WATCHDOG, O_RDWR);/*when open watchdog , enable watchdog*/
	if(muxMain->watchdogCtrl.fd < 0) 
	{
		MUX_ERROR("watchdog open error: %s", strerror(errno) );
		return EXIT_FAILURE;
	}

	timeout = 5 ;
	if(ioctl(muxMain->watchdogCtrl.fd , WDIOC_SETTIMEOUT ,&timeout) != 0) 
	{
		printf("SET watchdog time out error: %s", strerror(errno) );
		
		close (muxMain->watchdogCtrl.fd);
		muxMain->watchdogCtrl.fd = -1;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
	
}


static void _muxThWatchdogDestory(struct _CmnThread *th)
{
	MuxMain *muxMain = (MuxMain *)th->data;

	/*
	value = WDIOS_DISABLECARD; //disable watchdog  //WDIOS_ENABLECARD
	ioctl(fd ,WDIOC_SETOPTIONS,&value);
	*/
	close(muxMain->watchdogCtrl.fd);
	muxMain->watchdogCtrl.fd = -1;
}


static int _muxThWatchdogMain(CmnThread *th)
{
	int res = EXIT_SUCCESS;
	MuxMain *muxMain = (MuxMain *)th->data;

	//printf("IF use Ctrl + c , after %d Sec , System will Reboot!\n",timeout);
	while(1) 
	{
		ioctl(muxMain->watchdogCtrl.fd, WDIOC_KEEPALIVE); /* Kick the watchdog */
		sleep(1);
	}

	return EXIT_SUCCESS;
}


CmnThread  threadMuxWatchdog =
{
	name		:	"Led",
	flags			:	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	init			:	_muxThWatchdogInit,
	mainLoop		:	_muxThWatchdogMain,
	eventHandler	:	NULL,
	destory		:	_muxThWatchdogDestory,
	data			:	NULL,
};

