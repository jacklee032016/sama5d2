
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <poll.h>
#include <sys/stat.h>

#include "libCmnSys.h"
#include "_cmnMux.h"

#define	BTN_TIMER_TIMEOUT		500 /* ms */

#define		_BTN_STATUS_NONE		0
#define		_BTN_STATUS_PRESSED	1
#define		_BTN_STATUS_RELEASE	2

struct ButtonCtrl
{
	int		event;
	int		status;

	int		timerFd;
	int		timeout;		/*seconds */
	void 	*data;
};

struct ButtonCtrl _button;

static int _muxThBtnInit(CmnThread *th, void *data)
{
	MuxMain *muxMain = (MuxMain *)data;
	struct ButtonCtrl *btn = &_button;

	th->data = btn;

	btn->timeout = muxMain->resetTime;
	btn->data = muxMain;
	btn->event = cmnSysSW1CheckEvent();
	btn->status = _BTN_STATUS_NONE;
	
	if(btn->event == BTN_EVENT_UNKNOWN )
	{
		MUX_ERROR("can not open button driver");
		return EXIT_FAILURE;
	}

	btn->timerFd = timerfd_create(EXT_CLOCK_ID_MONO, 0);
	if(btn->timerFd < 0)
	{
		MUX_ERROR("Timer ID error: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static void _muxThBtnDestory(struct _CmnThread *th)
{
	struct ButtonCtrl *btn = (struct ButtonCtrl *)th->data;

	if(btn->timerFd > 0)
	{
		close(btn->timerFd);
	}
}


static int _btnDelay(struct ButtonCtrl *btn)
{
	int res = EXIT_SUCCESS;
	struct pollfd 	pollfd;

	timerfd_set_time(btn->timerFd, BTN_TIMER_TIMEOUT*2);
	pollfd.fd = btn->timerFd;
	pollfd.events = POLLIN;
	
	res = poll(&pollfd, 1, BTN_TIMER_TIMEOUT/*-1*/); /* in milliseconds */
	if (res == -1 )
	{
		MUX_ERROR("POLL for Button polling failed: '%s'", strerror(errno));
		/* what is the appropriate thing to do here on EBADF */
		if (errno == EINTR)
		{
			return EXIT_SUCCESS;
		}
		else if (errno != EBADF)
		{
			MUX_ERROR("POLL for Button polling failed: '%s'", strerror(errno));
			return EXIT_FAILURE;
		}
	}
#if 0
	else if(res == 0)/* timeout */
	{
		MUX_DEBUG("Timeout of Button polling");
		return EXIT_SUCCESS;
	}
#endif

	return EXIT_SUCCESS;
}

static int _muxThBtnMain(CmnThread *th)
{
	struct ButtonCtrl *btn = (struct ButtonCtrl *)th->data;
	MuxMain *muxMain = (MuxMain *)btn->data;
	int count = 0;

	while(1)
	{
		_btnDelay(btn);
		
		btn->event = cmnSysSW1CheckEvent();

		if(btn->event == BTN_EVENT_NONE )
		{
			count++;
			continue;
		}

		if(btn->event == BTN_EVENT_PRESSED && btn->status == _BTN_STATUS_NONE )
		{
			MUX_DEBUG("Button pressed now" );
			btn->status = _BTN_STATUS_PRESSED;
			count = 0;
		}
		else if(btn->event == BTN_EVENT_RELEASED && btn->status == _BTN_STATUS_PRESSED )
		{
			MUX_DEBUG("Button released after about %d seconds", count/2);

			if(count/2 >= btn->timeout )
			{
				MUX_INFO("Button pressed for about %d seconds, so reset", count/2);
				cmnSysCtrlDelayReboot(SYS_REBOOT_DELAY_MS_4_BTN, &muxMain->runCfg.runtime);
			}

			/* call function or set event to manager thread */
			btn->status = _BTN_STATUS_NONE;
			count = 0;
		}
		
		count++;
		
	}


	return EXIT_SUCCESS;
}


CmnThread  threadButton =
{
	name		:	CMN_THREAD_NAME_BUTTON,
	flags			:	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	init			:	_muxThBtnInit,
	mainLoop		:	_muxThBtnMain,
	eventHandler	:	NULL,
	destory		:	_muxThBtnDestory,
	data			:	NULL,
};

