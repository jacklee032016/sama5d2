
#include "libCmn.h"
#include "libMedia.h"
#include "libMux.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mux7xx.h"

static int _muxThBtnInit(CmnThread *th, void *data)
{
	int res = EXIT_SUCCESS;
	MuxMain *muxMain = (MuxMain *)data;
	th->data = muxMain;
	
	muxMain->btnCtrl.fd = open(MUX_DEV_BUTTON, O_RDWR);
	if(muxMain->btnCtrl.fd < 0 )
	{
		MUX_ERROR("can not open "MUX_DEV_BUTTON );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static void _muxThBtnDestory(struct _CmnThread *th)
{
	MuxMain *muxMain = (MuxMain *)th->data;

	close(muxMain->btnCtrl.fd);
	muxMain->btnCtrl.fd = 0;
	return 0;
}

static int _muxThBtnMain(CmnThread *th)
{
	int res = EXIT_SUCCESS;
	MuxMain *muxMain = (MuxMain *)th->data;

	int fd;
	unsigned int status, status_prev;
	int cnt, cnt_start, cnt_end;	
	
	cnt = 0;
	status = 1;
	status_prev = 1;
	cnt_start = 0;
	cnt_end = 0;
	
	while(1)
	{
		if(ioctl(fd, READ_BUTTON, &status) == -1)	
		{
			MUX_ERROR("Read button status error!");
		}
		else
		{
			if((status_prev == 1) &&(status == 0))
			{
				MUX_DEBUG("Button Pressed");
				cnt_start = 1;				
				cnt = 0;
			}
			
			if((status_prev == 0) &&(status == 1))
			{
				MUX_DEBUG("Button Released");
				cnt_end = 1;				
			}
			
			if(cnt_start == 1)
			{
				if(status == 0)
					cnt ++;	
				
#ifndef BUTTON_FOR_FPGA_INIT
				if(cnt > 10)
				{
					cnt = 0;
					cnt_start = 0;

					muxReset(muxMain);
				}
#endif
			}	
			
			if(cnt_end == 1)
			{
#ifdef BUTTON_FOR_FPGA_INIT
				fpga_set_ip();
#else
				if((cnt < 10) & (cnt !=0))
				{
					muxReboot(muxMain);
					
					cnt = 0;
					cnt_end = 0;
				}
				else if(cnt > 10)
				{
					cnt = 0;
					cnt_end = 0;
					muxReset(muxMain);
				}				
				else
#endif
				{					
					cnt = 0;
					cnt_end = 0;
				}
			}			
			status_prev = status;
		}
		
		usleep(300000);
	}
}


CmnThread  threadMuxButton =
{
	name		:	"Button",
	flags			:	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	init			:	_muxThBtnInit,
	mainLoop		:	_muxThBtnMain,
	eventHandler	:	NULL,
	destory		:	_muxThBtnDestory,
	data			:	NULL,
};

