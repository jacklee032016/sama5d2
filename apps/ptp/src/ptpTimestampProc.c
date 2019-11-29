/**
 * @file tsproc.c
 * @note Copyright (C) 2015 Miroslav Lichvar <mlichvar@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <inttypes.h>

#include "ptpCompact.h"
#include "tmv.h"

struct PtpFilter
{
	void (*destroy)(struct PtpFilter *filter);

	tmv_t (*sample)(struct PtpFilter *filter, tmv_t sample);

	void (*reset)(struct PtpFilter *filter);
};

/* Implements a moving median */
struct mmedian {
	struct PtpFilter filter;
	int cnt;
	int len;
	int index;
	/* Indices sorted by value. */
	int *order;
	/* Values stored in circular buffer. */
	tmv_t *samples;
};

static void __mmedianDestroy(struct PtpFilter *_filter)
{
	struct mmedian *m = container_of(_filter, struct mmedian, filter);
	free(m->order);
	free(m->samples);
	free(m);
}

static tmv_t __mmedianSample(struct PtpFilter *_filter, tmv_t sample)
{
	struct mmedian *m = container_of(_filter, struct mmedian, filter);
	int i;

	m->samples[m->index] = sample;
	if (m->cnt < m->len) {
		m->cnt++;
	} else {
		/* Remove index of the replaced value from order. */
		for (i = 0; i < m->cnt; i++)
			if (m->order[i] == m->index)
				break;
		for (; i + 1 < m->cnt; i++)
			m->order[i] = m->order[i + 1];
	}

	/* Insert index of the new value to order. */
	for (i = m->cnt - 1; i > 0; i--) {
		if (tmv_cmp(m->samples[m->order[i - 1]],
			    m->samples[m->index]) <= 0)
			break;
		m->order[i] = m->order[i - 1];
	}
	m->order[i] = m->index;

	m->index = (1 + m->index) % m->len;

	if (m->cnt % 2)
		return m->samples[m->order[m->cnt / 2]];
	else
		return tmv_div(tmv_add(m->samples[m->order[m->cnt / 2 - 1]],
				       m->samples[m->order[m->cnt / 2]]), 2);
}

static void __mmedianReset(struct PtpFilter *_filter)
{
	struct mmedian *m = container_of(_filter, struct mmedian, filter);
	m->cnt = 0;
	m->index = 0;
}

struct PtpFilter *_mmedianCreate(int length)
{
	struct mmedian *m;

	if (length < 1)
		return NULL;
	m = calloc(1, sizeof(*m));
	if (!m)
		return NULL;
	m->filter.destroy = __mmedianDestroy;
	m->filter.sample = __mmedianSample;
	m->filter.reset = __mmedianReset;
	m->order = calloc(1, length * sizeof(*m->order));
	if (!m->order) {
		free(m);
		return NULL;
	}
	m->samples = calloc(1, length * sizeof(*m->samples));
	if (!m->samples) {
		free(m->order);
		free(m);
		return NULL;
	}
	m->len = length;
	return &m->filter;
}

/* Implements a moving average */
struct mave
{
	struct PtpFilter filter;
	int cnt;
	int len;
	int index;
	tmv_t sum;
	tmv_t *val;
};

static void __maveDestroy(struct PtpFilter *_filter)
{
	struct mave *m = container_of(_filter, struct mave, filter);
	free(m->val);
	free(m);
}

static tmv_t __maveAccumulate(struct PtpFilter *_filter, tmv_t val)
{
	struct mave *m = container_of(_filter, struct mave, filter);

	m->sum = tmv_sub(m->sum, m->val[m->index]);
	m->val[m->index] = val;
	m->index = (1 + m->index) % m->len;
	m->sum = tmv_add(m->sum, val);
	if (m->cnt < m->len) {
		m->cnt++;
	}
	return tmv_div(m->sum, m->cnt);
}

static void __maveReset(struct PtpFilter *_filter)
{
	struct mave *m = container_of(_filter, struct mave, filter);

	m->cnt = 0;
	m->index = 0;
	m->sum = tmv_zero();
	memset(m->val, 0, m->len * sizeof(*m->val));
}

struct PtpFilter *_maveCreate(int length)
{
	struct mave *m;
	m = calloc(1, sizeof(*m));
	if (!m) {
		return NULL;
	}
	m->filter.destroy = __maveDestroy;
	m->filter.sample = __maveAccumulate;
	m->filter.reset = __maveReset;
	
	m->val = calloc(1, length * sizeof(*m->val));
	if (!m->val)
	{
		free(m);
		return NULL;
	}
	
	m->len = length;
	return &m->filter;
}

struct PtpFilter *filter_create(enum filter_type type, int length)
{
	switch (type)
	{
		case FILTER_MOVING_AVERAGE:
			pr_debug("Filter MovingAverage is created");
			return _maveCreate(length);
			
		case FILTER_MOVING_MEDIAN:
			pr_debug("Filter MovingMedia is created");
			return _mmedianCreate(length);
			
		default:
			pr_err("Filter is invalidate type: %d", type);
			return NULL;
	}
}


void filter_destroy(struct PtpFilter *filter)
{
	filter->destroy(filter);
}

tmv_t filter_sample(struct PtpFilter *filter, tmv_t sample)
{
	return filter->sample(filter, sample);
}

void filter_reset(struct PtpFilter *filter)
{
	filter->reset(filter);
}



static int _isWeighting(struct TimestampProcess *tsp)
{
	switch (tsp->mode)
	{
		case TSPROC_FILTER:
		case TSPROC_RAW:
			return 0;

		case TSPROC_FILTER_WEIGHT:
		case TSPROC_RAW_WEIGHT:
			return 1;
	}
	return 0;
}

struct TimestampProcess *tsproc_create(enum tsproc_mode mode,
			     enum filter_type delay_filter, int filter_length)
{
	struct TimestampProcess *tsp;

	tsp = calloc(1, sizeof(*tsp));
	if (!tsp)
		return NULL;

	switch (mode) {
		case TSPROC_FILTER:
		case TSPROC_RAW:
		case TSPROC_FILTER_WEIGHT:
		case TSPROC_RAW_WEIGHT:
			tsp->mode = mode;
			break;
		default:
			free(tsp);
			return NULL;
	}

	tsp->delay_filter = filter_create(delay_filter, filter_length);
	if (!tsp->delay_filter) {
		free(tsp);
		return NULL;
	}

	tsp->clock_rate_ratio = 1.0;

	return tsp;
}



static tmv_t _getRawDelay(struct TimestampProcess *tsp)
{
	tmv_t t23, t41, delay;

	/* delay = ((t2 - t3) * rr + (t4 - t1)) / 2 */

	t23 = tmv_sub(tsp->t2, tsp->t3);
	if (tsp->clock_rate_ratio != 1.0)
		t23 = dbl_tmv(tmv_dbl(t23) * tsp->clock_rate_ratio);
	t41 = tmv_sub(tsp->t4, tsp->t1);
	delay = tmv_div(tmv_add(t23, t41), 2);

	if (tmv_sign(delay) < 0)
	{/* delay <=0, something is wrong */
		pr_debug("negative delay %10" PRId64, tmv_to_nanoseconds(delay));
		pr_debug("delay = (t2 - t3) * rr + (t4 - t1)");
		pr_debug("t2 - t3 = %+10" PRId64, tmv_to_nanoseconds(t23));
		pr_debug("t4 - t1 = %+10" PRId64, tmv_to_nanoseconds(t41));
		pr_debug("rr = %.9f", tsp->clock_rate_ratio);
	}

	return delay;
}


/* calculate path delay after  delay_resp or pdelay_resp is received */
int tsproc_update_delay(struct TimestampProcess *tsp, tmv_t *delay)
{
	tmv_t raw_delay;

	if (tmv_is_zero(tsp->t2) || tmv_is_zero(tsp->t3))
	{/* t1, t2, t3, t4 must be set before this operation */
		return -1;
	}

	raw_delay = _getRawDelay(tsp);
	tsp->filtered_delay = filter_sample(tsp->delay_filter, raw_delay);
	tsp->filtered_delay_valid = 1;

	pr_debug("delay   filtered %10" PRId64 "   raw %10" PRId64,
		 tmv_to_nanoseconds(tsp->filtered_delay),
		 tmv_to_nanoseconds(raw_delay));

	if (!delay)
	{
		return 0;
	}

	switch (tsp->mode) {
		case TSPROC_FILTER:
		case TSPROC_FILTER_WEIGHT:
			*delay = tsp->filtered_delay;
			break;
			
		case TSPROC_RAW:
		case TSPROC_RAW_WEIGHT:
			*delay = raw_delay;
			break;
	}

	return 0;
}

/* after receive FOLLOW_UP(when 2-step is used) or receive SYN, this is called 
* caculate offset and weight?? from t1, t2, t3 and t4
*/
int tsproc_update_offset(struct TimestampProcess *tsp, tmv_t *offset, double *weight)
{
	tmv_t delay = tmv_zero(), raw_delay = tmv_zero();

	if (tmv_is_zero(tsp->t1) || tmv_is_zero(tsp->t2))
		return -1;

	switch (tsp->mode)
	{
		case TSPROC_FILTER:
			if (!tsp->filtered_delay_valid)
			{
#if PTP_NOISE_DEBUG			
				pr_info("TSPROC_FILTER: tsp->filtered_delay_valid is 0");
#endif				
				return -1;
			}
			delay = tsp->filtered_delay;
			break;
		case TSPROC_RAW:
		case TSPROC_RAW_WEIGHT:
			if (tmv_is_zero(tsp->t3)) {
				return -1;
			}
			raw_delay = _getRawDelay(tsp);
			delay = raw_delay;
			break;
			
		case TSPROC_FILTER_WEIGHT:
			if (tmv_is_zero(tsp->t3) || !tsp->filtered_delay_valid) {
				return -1;
			}
			raw_delay = _getRawDelay(tsp);
			delay = tsp->filtered_delay;
			break;
	}

	/* offset = t2 - t1 - delay */
	*offset = tmv_sub(tmv_sub(tsp->t2, tsp->t1), delay);

	if (!weight)
		return 0;

	if (_isWeighting(tsp) && tmv_sign(tsp->filtered_delay) > 0 && tmv_sign(raw_delay) > 0)
	{
		*weight = tmv_dbl(tsp->filtered_delay) / tmv_dbl(raw_delay);
		if (*weight > 1.0)
			*weight = 1.0;
	}
	else {
		*weight = 1.0;
	}

	return 0;
}

void tsproc_reset(struct TimestampProcess *tsp, int full)
{
	tsp->t1 = tmv_zero();
	tsp->t2 = tmv_zero();
	tsp->t3 = tmv_zero();
	tsp->t4 = tmv_zero();

	if (full) {
		tsp->clock_rate_ratio = 1.0;
		filter_reset(tsp->delay_filter);
		tsp->filtered_delay_valid = 0;
	}
}

