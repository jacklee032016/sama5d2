#define _GNU_SOURCE
#include <time.h>
#include <unistd.h>
#include <sys/timex.h>
#include <stdio.h>

/*
* busybox: date -s 2010.02.17-19:14:32
*/

void printDateString(long int second)
{
	time_t nowtime;

	struct tm *nowtm;
	char tmbuf[64], buf[64];

	struct timeval tv;
	gettimeofday(&tv,NULL);

	nowtime = tv.tv_sec - second;

	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	printf("Dest timestamp: %s\n", tmbuf);
}

int readTime(void)
{
	struct timespec ts;

	if(clock_gettime(CLOCK_REALTIME, &ts)< 0)
	{
		printf("read failed:%m\n");
		return 1;
	}
	printf("CLOCK_REALTIME: %d\n", ts.tv_sec);

	if(clock_gettime(CLOCK_MONOTONIC, &ts)< 0)
	{
		printf("read failed:%m\n");
		return 1;
	}
	printf("CLOCK_MONOTONIC: %d\n", ts.tv_sec);

	return 0;	
}

int main(int argc, char *argv[])
{
	int r;
	struct timex tmx = {};
	tmx.modes = ADJ_STATUS | ADJ_NANO | ADJ_SETOFFSET;

	long int second = 25221;
	if(argc >= 2)
	{
		second = atoi(argv[1]);
	}

	printDateString(second);
	

//	while(second > 0)
	{
		tmx.time.tv_sec = -second;
		tmx.time.tv_usec = 12123; // 802308708

		printf("Setting second :%ld, nsec: %ld\n", tmx.time.tv_sec, tmx.time.tv_usec );
		r = clock_adjtime(CLOCK_REALTIME, &tmx);
		if (r < 0)
		{
			printf("\tFAILED: clock_adjtime when second %m: %ld(%ld), nsec: %ld; status:0x%x \n", second, tmx.time.tv_sec, tmx.time.tv_usec, tmx.status);
//			break;
		}
#if 1		
		else
		{
			printf("\tOK: second is:%ld(%ld), nsec: %ld; status:0x%x  \n", second, tmx.time.tv_sec, tmx.time.tv_usec, tmx.status );
//			break;
		}
#endif
		second++;
	}

	printf("Finally, second is:%ld(returned:%ld), nsec: %ld \n", second, tmx.time.tv_sec, tmx.time.tv_usec );

	readTime();
	
	return r;
}
	
