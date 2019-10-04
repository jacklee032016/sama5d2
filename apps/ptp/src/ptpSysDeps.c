/**
 *
 */
 
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/ptp_clock.h>

#include "ptpCompact.h"

/*
 * On 32 bit platforms, the PHC driver's maximum adjustment (type
 * 'int' in units of ppb) can overflow the timex.freq field (type
 * 'long'). So in this case we clamp the maximum to the largest
 * possible adjustment that fits into a 32 bit long.
 */
#define BITS_PER_LONG	(sizeof(long)*8)
#define MAX_PPB_32	32767999	/* 2^31 - 1 / 65.536 */

static int phc_get_caps(clockid_t clkid, struct ptp_clock_caps *caps);

/* open phc device, such as /dev/ptp1 */
clockid_t phc_open(char *phc)
{
	clockid_t clkid;
	struct ptp_clock_caps caps;
	int fd = open(phc, O_RDWR);

	if (fd < 0)
		return CLOCK_INVALID;

	clkid = FD_TO_CLOCKID(fd);
	/* check if clkid is valid */
	if (phc_get_caps(clkid, &caps))
	{
		close(fd);
		return CLOCK_INVALID;
	}

	return clkid;
}

void phc_close(clockid_t clkid)
{
	if (clkid == CLOCK_INVALID)
		return;

	close(CLOCKID_TO_FD(clkid));
}

static int phc_get_caps(clockid_t clkid, struct ptp_clock_caps *caps)
{
	int fd = CLOCKID_TO_FD(clkid), err;

	err = ioctl(fd, PTP_CLOCK_GETCAPS, caps);
	if (err)
		perror("PTP_CLOCK_GETCAPS");
	return err;
}

int phc_max_adj(clockid_t clkid)
{
	int max;
	struct ptp_clock_caps caps;

	if (phc_get_caps(clkid, &caps))
		return 0;

	max = caps.max_adj;

	if (BITS_PER_LONG == 32 && max > MAX_PPB_32)
		max = MAX_PPB_32;

	return max;
}

int phc_has_pps(clockid_t clkid)
{
	struct ptp_clock_caps caps;

	if (phc_get_caps(clkid, &caps))
		return 0;
	return caps.pps;
}

#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/ptp_clock.h>


#ifdef PTP_SYS_OFFSET

static int64_t pctns(struct ptp_clock_time *t)
{
	return t->sec * NS_PER_SEC + t->nsec;
}

static struct {
	int64_t interval;
	int64_t offset;
	uint64_t timestamp;
} samples[PTP_MAX_SAMPLES];

static void insertion_sort(int length, int64_t interval, int64_t offset, uint64_t ts)
{
	int i = length - 1;
	while (i >= 0) {
		if (samples[i].interval < interval)
			break;
		samples[i+1] = samples[i];
		i--;
	}
	samples[i+1].interval = interval;
	samples[i+1].offset = offset;
	samples[i+1].timestamp = ts;
}

static int64_t sysoff_estimate(struct ptp_clock_time *pct, int n_samples,
			       uint64_t *ts, int64_t *delay)
{
	int64_t t1, t2, tp;
	int64_t interval, offset;
	int i;

	for (i = 0; i < n_samples; i++) {
		t1 = pctns(&pct[2*i]);
		tp = pctns(&pct[2*i+1]);
		t2 = pctns(&pct[2*i+2]);
		interval = t2 - t1;
		offset = (t2 + t1) / 2 - tp;
		insertion_sort(i, interval, offset, (t2 + t1) / 2);
	}
	*ts = samples[0].timestamp;
	*delay = samples[0].interval;
	return samples[0].offset;
}


int sysoff_measure(int fd, int n_samples, int64_t *result, uint64_t *ts, int64_t *delay)
{
	struct ptp_sys_offset pso;
	pso.n_samples = n_samples;
	
	if (ioctl(fd, PTP_SYS_OFFSET, &pso))
	{
		perror("ioctl PTP_SYS_OFFSET");
		return SYSOFF_RUN_TIME_MISSING;
	}
	
	*result = sysoff_estimate(pso.ts, n_samples, ts, delay);
	return SYSOFF_SUPPORTED;
}

int sysoff_probe(int fd, int n_samples)
{
	int64_t junk, delay;
	uint64_t ts;

	if (n_samples > PTP_MAX_SAMPLES) {
		fprintf(stderr, "warning: %d exceeds kernel max readings %d\n",
			n_samples, PTP_MAX_SAMPLES);
		fprintf(stderr, "falling back to clock_gettime method\n");
		return SYSOFF_RUN_TIME_MISSING;
	}

	return sysoff_measure(fd, n_samples, &junk, &ts, &delay);
}

#else /* !PTP_SYS_OFFSET */

int sysoff_measure(int fd, int n_samples,
		   int64_t *result, uint64_t *ts, int64_t *delay)
{
	return SYSOFF_COMPILE_TIME_MISSING;
}

int sysoff_probe(int fd, int n_samples)
{
	return SYSOFF_COMPILE_TIME_MISSING;
}

#endif /* PTP_SYS_OFFSET */

#include <math.h>
#include <string.h>
#include <stdlib.h>

struct stats {
	unsigned int num;
	double min;
	double max;
	double mean;
	double sum_sqr;
	double sum_diff_sqr;
};

struct stats *stats_create(void)
{
	struct stats *stats;

	stats = calloc(1, sizeof *stats);
	return stats;
}

void stats_destroy(struct stats *stats)
{
	free(stats);
}

void stats_add_value(struct stats *stats, double value)
{
	double old_mean = stats->mean;

	if (!stats->num || stats->max < value)
		stats->max = value;
	if (!stats->num || stats->min > value)
		stats->min = value;

	stats->num++;
	stats->mean = old_mean + (value - old_mean) / stats->num;
	stats->sum_sqr += value * value;
	stats->sum_diff_sqr += (value - old_mean) * (value - stats->mean);
}

unsigned int stats_get_num_values(struct stats *stats)
{
	return stats->num;
}

int stats_get_result(struct stats *stats, struct stats_result *result)
{
	if (!stats->num)
		return -1;

	result->min = stats->min;
	result->max = stats->max;
	result->max_abs = stats->max > -stats->min ? stats->max : -stats->min;
	result->mean = stats->mean;
	result->rms = sqrt(stats->sum_sqr / stats->num);
	result->stddev = sqrt(stats->sum_diff_sqr / stats->num);

	return 0;
}

void stats_reset(struct stats *stats)
{
	memset(stats, 0, sizeof *stats);
}

