/**
 * @file tmv.h
 * @brief Implements an abstract time value type.
 * @note Copyright (C) 2011 Richard Cochran <richardcochran@gmail.com>
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
#ifndef HAVE_TMV_H
#define HAVE_TMV_H

#include <time.h>



static inline tmv_t tmv_add(tmv_t a, tmv_t b)
{
	tmv_t t;
	t.ns = a.ns + b.ns;
	return t;
}

static inline tmv_t tmv_div(tmv_t a, int divisor)
{
	tmv_t t;
	t.ns = a.ns / divisor;
	return t;
}

static inline int tmv_cmp(tmv_t a, tmv_t b)
{
	return a.ns == b.ns ? 0 : a.ns > b.ns ? +1 : -1;
}

static inline int tmv_sign(tmv_t x)
{
	return x.ns == 0 ? 0 : x.ns > 0 ? +1 : -1;
}

static inline int tmv_is_zero(tmv_t x)
{
	return x.ns == 0 ? 1 : 0;
}

static inline tmv_t tmv_sub(tmv_t a, tmv_t b)
{
	tmv_t t;
	t.ns = a.ns - b.ns;
	return t;
}

static inline tmv_t tmv_zero(void)
{
	tmv_t t = { 0 };
	return t;
}

static inline tmv_t correction_to_tmv(Integer64 c)
{
	tmv_t t;
	t.ns = (c >> 16);
	return t;
}

static inline double tmv_dbl(tmv_t x)
{
	return (double) x.ns;
}

static inline tmv_t dbl_tmv(double x)
{
	tmv_t t;
	t.ns = x;
	return t;
}

static inline int64_t tmv_to_nanoseconds(tmv_t x)
{
	return x.ns;
}

static inline TimeInterval tmv_to_TimeInterval(tmv_t x)
{
	return x.ns << 16;
}

static inline struct WireTimeStamp tmv_to_Timestamp(tmv_t x)
{
	struct WireTimeStamp result;
	uint64_t sec, nsec;

	sec  = x.ns / 1000000000ULL;
	nsec = x.ns % 1000000000ULL;

	result.seconds_lsb = sec & 0xFFFFFFFF;
	result.seconds_msb = (sec >> 32) & 0xFFFF;
	result.nanoseconds = nsec;

	return result;
}

static inline tmv_t timespec_to_tmv(struct timespec ts)
{
	tmv_t t;
	t.ns = ts.tv_sec * NS_PER_SEC + ts.tv_nsec;
	return t;
}

static inline tmv_t timestamp_to_tmv(struct LocalTimeStamp ts)
{
	tmv_t t;
	t.ns = ts.sec * NS_PER_SEC + ts.nsec;
	return t;
}

#endif
