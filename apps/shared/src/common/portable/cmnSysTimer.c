
/* for timer fd */
#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "libCmn.h"

int timerfd_create(int clockid, int flags)
{
	return syscall(__NR_timerfd_create, clockid, flags);
}

int timerfd_settime(int ufc, int flags, const struct itimerspec *utmr, struct itimerspec *otmr)
{
	return syscall(__NR_timerfd_settime, ufc, flags, utmr, otmr);
}

int timerfd_gettime(int ufc, struct itimerspec *otmr)
{
	return syscall(__NR_timerfd_gettime, ufc, otmr);
}

void set_timespec(struct timespec *tmr, unsigned long long ustime)
{
	tmr->tv_sec = (time_t) (ustime / 1000000ULL);
	tmr->tv_nsec = (long) (1000ULL * (ustime % 1000000ULL));
}

int timerfd_set_time(int fd, long mstime)
{
	struct itimerspec tmr;
	
	set_timespec(&tmr.it_value, mstime*1000); /* ustime*/
	set_timespec(&tmr.it_interval, 0);

	if (timerfd_settime(fd, 0, &tmr, NULL))
	{
		MUX_ERROR("timerfd_settime to %ld ms on fd %d: %m", mstime, fd);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
	
#include <string.h>
#if 0
char *strnstr(const char *haystack, const char *needle, size_t len)
{
        int i;
        size_t needle_len;

        if (0 == (needle_len = strnlen(needle, len)))
                return (char *)haystack;

        for (i=0; i<=(int)(len-needle_len); i++)
        {
                if ((haystack[0] == needle[0]) &&
                        (0 == strncmp(haystack, needle, needle_len)))
                        return (char *)haystack;

                haystack++;
        }
        return NULL;
}
#else
char	*strnstr(const char* buffer, const char* token, size_t n)
{
	const char* p;
	size_t tokenlen = strlen(token);
	if (tokenlen == 0)
	{
		return (char *)buffer;
	}
	
	for (p = buffer; *p && (p + tokenlen <= buffer + n); p++)
	{
//		if ((*p == *token) && (qqstrncmp(p, token, tokenlen) == 0))
		if ((*p == *token) && (strncasecmp(p, token, tokenlen) == 0))
		{
			return (char *)p;
		}
	}
	return NULL;
}

#endif


