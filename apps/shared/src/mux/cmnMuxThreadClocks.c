/*
* thread: control VCOX of clock generator
* thread: test test pin with timer
*/

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <poll.h>
#include <sys/stat.h>
#include <time.h>

#include "libCmnSys.h"
#include "_cmnMux.h"

/* 01.13, 2020, added for test FPGA */
#define	LED_TEST				LED_HOME"/""test/brightness"

#define	TEST_PIN_DELAY_MS			200

typedef	struct
{
	int						delayMs;	/* micro second */
	
	int						clockId;
	int						fdPin;
	char						currentStatus;

	int						total;
	struct timespec			startTime;
	struct timespec			currentTime;

	void						*data;
}TEST_DATA;

static TEST_DATA		_testData;

static int _muxThTestPinInit(CmnThread *th, void *data)
{
//	MuxMain *muxMain = (MuxMain *)data;
	TEST_DATA	*testData = &_testData;

	memset(testData,0 , sizeof(TEST_DATA));
	
	testData->clockId = EXT_CLOCK_ID;
	testData->delayMs = TEST_PIN_DELAY_MS;
	testData->currentStatus = '0';
	
	testData->fdPin = open(LED_TEST, O_WRONLY);
	if( testData->fdPin < 0)
	{
		MUX_ERROR("Open '%s' failed: %m", LED_TEST);
		return EXIT_FAILURE;
	}

	th->data = data;

	return EXIT_SUCCESS;
}

static void _muxThTestPinDestory(struct _CmnThread *th)
{
	TEST_DATA	*testData = &_testData;

	if(testData->fdPin > 0 )
	{
		close(testData->fdPin);
	}

	MUX_DEBUG("Total %d, delay %d ms: cost %ld sec + %ld nanosecond", testData->total, testData->delayMs, 
		testData->currentTime.tv_sec - testData->startTime.tv_sec, testData->currentTime.tv_nsec - testData->startTime.tv_nsec);

}

static int _muxThTestPinMain(CmnThread *th)
{
#define	_DEBUG_DELAY		0

	TEST_DATA	*testData = &_testData;
//	MuxMain *muxMain = (MuxMain *)th->data;

	struct timespec request;
	long value = testData->delayMs*UNIT_K_HZ;

#if _DEBUG_DELAY
	struct timespec _start;
	struct timespec _curr;
#endif
	
	request.tv_sec = (value)/UNIT_B_HZ;
	request.tv_nsec = (value)%UNIT_B_HZ;  
	
	if (clock_gettime(testData->clockId, &testData->startTime) == -1)
	{
            MUX_ERROR("PinCtrl: get start timestamp failed: %m");
	}

	while(1)
	{
#if _DEBUG_DELAY
		if (clock_gettime(clockId, &_start) == -1)
		{
	            EXT_ERRORF("clock_gettime");
		}
#endif		
		if(clock_nanosleep(testData->clockId, 0, &request, NULL) )
		{
			MUX_ERROR("PinCtrl: nanosleep %ldsecond, %ldnanosec failed: %m", request.tv_sec, request.tv_nsec);
			break;
		}	
#if _DEBUG_DELAY
		if (clock_gettime(clockId, &_curr) == -1)
		{
			EXT_ERRORF("clock_gettime2");
		}
		MUX_DEBUG(" Delay  %ldsecond, %ldnanosec; delayed:%ld", request.tv_sec, request.tv_nsec, /*_endTime-_startTime*/ _curr.tv_nsec - _start.tv_nsec);
#endif
		if(write(testData->fdPin, &testData->currentStatus, 1) < 0)
		{
			MUX_ERROR("PinCtrl: write pin failed");
			break;
		}
		
		testData->currentStatus = (testData->currentStatus == '0')? '1':'0';
		testData->total++;
	}


	if (clock_gettime(testData->clockId, &testData->currentTime) == -1)
	{
		MUX_ERROR("PinCtrl: get ending timestamp failed: %m");
	}

	return EXIT_SUCCESS;
}


/* test pin FPGA */
CmnThread  threadTestPin =
{
	name		:	CMN_THREAD_NAME_TEST_PIN,
	flags			:	SET_BIT(1, CMN_THREAD_FLAG_WAIT),
	
	init			:	_muxThTestPinInit,
	mainLoop		:	_muxThTestPinMain,
	eventHandler	:	NULL,
	destory		:	_muxThTestPinDestory,
	data			:	NULL,
};



