
#include "libCmnSys.h"

int testSysTimerResolutions(int clockid, char *name)
{
	int rc;
	struct timespec res;

	rc = clock_getres(clockid, &res);
	if (!rc)
	{
		EXT_INFOF("Clock %s: %ldns", name, res.tv_nsec);
		return EXIT_SUCCESS;
	}
	else
	{
		EXT_ERRORF("CLock %s res get failed:%m", name);
	}

	return EXIT_FAILURE;
}

/* 
* only CLOCK_REALTIME, CLOCK_MONOTONIC and CLOCK_PROCESS_CPUTIME_ID
*/
int	cmnSysNanoSleep(int	clockId, struct timespec *request, char *name)
{
	struct timespec _start;
	struct timespec _curr;
//	int secs, nsecs;	
	
//	uint64_t _startTime, _endTime, _delayed, _diffs;

//	_startTime = cmnGetTimeUs();
	if (clock_gettime(clockId, &_start) == -1)
	{
            EXT_ERRORF("clock_gettime");
	}
	if(clock_nanosleep( clockId, 0, request, NULL) )
	{
		EXT_ERRORF("Clock %s nanosleep %ldsecond, %ldnanosec failed: %m", name, request->tv_sec, request->tv_nsec);
		return EXIT_FAILURE;
	}	
//	_endTime = cmnGetTimeUs();
	if (clock_gettime(clockId, &_curr) == -1)
	{
		EXT_ERRORF("clock_gettime2");
	}

	EXT_DEBUGF(EXT_DBG_ON, "%s Delay  %ldsecond, %ldnanosec; delayed:%ld", name, request->tv_sec, request->tv_nsec, /*_endTime-_startTime*/ _curr.tv_nsec - _start.tv_nsec);
	
	return EXIT_SUCCESS;
}


int testTimers(void)
{
#define	_GET_TIMER_RESOLUTION(id)	\
	testSysTimerResolutions((id), #id )


#define	_CMN_DELAY_MICRO_SECOND(id, microSecond)	\
	cmnSysNanoSleep((id), (microSecond), #id )


	_GET_TIMER_RESOLUTION(CLOCK_MONOTONIC);
	_GET_TIMER_RESOLUTION(CLOCK_MONOTONIC_COARSE);

	_GET_TIMER_RESOLUTION(CLOCK_REALTIME);
	_GET_TIMER_RESOLUTION(CLOCK_REALTIME_COARSE );
	
	_GET_TIMER_RESOLUTION(CLOCK_PROCESS_CPUTIME_ID);
	_GET_TIMER_RESOLUTION(CLOCK_THREAD_CPUTIME_ID);

	_GET_TIMER_RESOLUTION(CLOCK_BOOTTIME );
	_GET_TIMER_RESOLUTION(CLOCK_REALTIME_ALARM );
	_GET_TIMER_RESOLUTION(CLOCK_BOOTTIME_ALARM );

	uint64_t _startTime, _endTime, _delayed, _diffs;
	int count = 100;
	uint64_t durationMs = 500;

	struct timespec request;
	
	long value = durationMs*UNIT_K_HZ;
	
	request.tv_sec = (value)/UNIT_B_HZ;
	request.tv_nsec = (value)%UNIT_B_HZ;  

	_startTime = cmnGetTimeUs();

	for(int i=0; i< count; i++)
	{
//		_CMN_DELAY_MICRO_SECOND(EXT_CLOCK_ID, durationMs);
//		_CMN_DELAY_MICRO_SECOND(CLOCK_PROCESS_CPUTIME_ID, durationMs);
		_CMN_DELAY_MICRO_SECOND(CLOCK_MONOTONIC, &request);
	}
	_endTime = cmnGetTimeUs();
	_delayed =  _endTime-_startTime;
	_diffs = _delayed-count*durationMs;

	EXT_INFOF("Count %d delay of %lld microsecond: %lld(%lld - %lld), Diffs:%lld", count, durationMs, _delayed, _startTime, _endTime, _delayed-count*durationMs);

	EXT_INFOF("Total %lld ms, should be:%lld ms", _delayed, count*durationMs);
	
	EXT_INFOF("Diffs: %lld ms; Average diff: %f ms", _diffs, (double)_diffs/(double)count);

	return EXIT_SUCCESS;

}


