/**
 * @file clock.c
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
#include <errno.h>
#include <linux/net_tstamp.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/queue.h>

#include "clock.h"
#include "missing.h"
#include "port.h"
#include "servo.h"
#include "util.h"

//#include "ptpCompact.h"
//#include "ptpProtocol.h"
//#include "ptpImplements.h"

#include "clockPrivate.h"
#include "portPrivate.h"

int (*clock_dscmp(struct PtpClock *c))(struct dataset *a, struct dataset *b)
{
	return c->dscmp;
}

void clock_send_notification(struct PtpClock *c, struct ptp_message *msg, enum NOTIFICATION event)
{
	unsigned int event_pos = event / 8;
	uint8_t mask = 1 << (event % 8);
	struct PtpPort *uds = c->uds_port;
	struct clock_subscriber *s;

	LIST_FOREACH(s, &c->subscribers, list) {
		if (!(s->events[event_pos] & mask))
			continue;
		/* send event */
		msg->header.sequenceId = htons(s->sequenceId);
		s->sequenceId++;
		msg->management.targetPortIdentity.clockIdentity =
			s->targetPortIdentity.clockIdentity;
		msg->management.targetPortIdentity.portNumber =
			htons(s->targetPortIdentity.portNumber);
		msg->address = s->addr;
		port_forward_to(uds, msg);
	}
}


/* The 'p' and 'req' paremeters are needed for the GET actions that operate
 * on per-client datasets. If such actions do not apply to the caller, it is
 * allowed to pass both of them as NULL.
 */
int clock_management_fill_response(struct PtpClock *c, struct PtpPort *p,
					  struct ptp_message *req,
					  struct ptp_message *rsp, int id)
{
	struct grandmaster_settings_np *gsn;
	struct management_tlv_datum *mtd;
	struct subscribe_events_np *sen;
	struct management_tlv *tlv;
	struct time_status_np *tsn;
	struct tlv_extra *extra;
	struct PTPText *text;
	int datalen = 0;

	extra = tlv_extra_alloc();
	if (!extra) {
		pr_err("failed to allocate TLV descriptor");
		return 0;
	}
	extra->tlv = (struct TLV *) rsp->management.suffix;

	tlv = (struct management_tlv *) rsp->management.suffix;
	tlv->type = TLV_MANAGEMENT;
	tlv->id = id;

	switch (id) {
	case TLV_USER_DESCRIPTION:
		text = (struct PTPText *) tlv->data;
		text->length = c->desc.userDescription.length;
		memcpy(text->text, c->desc.userDescription.text, text->length);
		datalen = 1 + text->length;
		break;
	case TLV_DEFAULT_DATA_SET:
		memcpy(tlv->data, &c->dds, sizeof(c->dds));
		datalen = sizeof(c->dds);
		break;
	case TLV_CURRENT_DATA_SET:
		memcpy(tlv->data, &c->cur, sizeof(c->cur));
		datalen = sizeof(c->cur);
		break;
	case TLV_PARENT_DATA_SET:
		memcpy(tlv->data, &c->dad.pds, sizeof(c->dad.pds));
		datalen = sizeof(c->dad.pds);
		break;
	case TLV_TIME_PROPERTIES_DATA_SET:
		memcpy(tlv->data, &c->tds, sizeof(c->tds));
		datalen = sizeof(c->tds);
		break;
	case TLV_PRIORITY1:
		mtd = (struct management_tlv_datum *) tlv->data;
		mtd->val = c->dds.priority1;
		datalen = sizeof(*mtd);
		break;
	case TLV_PRIORITY2:
		mtd = (struct management_tlv_datum *) tlv->data;
		mtd->val = c->dds.priority2;
		datalen = sizeof(*mtd);
		break;
	case TLV_DOMAIN:
		mtd = (struct management_tlv_datum *) tlv->data;
		mtd->val = c->dds.domainNumber;
		datalen = sizeof(*mtd);
		break;
	case TLV_SLAVE_ONLY:
		mtd = (struct management_tlv_datum *) tlv->data;
		mtd->val = c->dds.flags & DDS_SLAVE_ONLY;
		datalen = sizeof(*mtd);
		break;
	case TLV_CLOCK_ACCURACY:
		mtd = (struct management_tlv_datum *) tlv->data;
		mtd->val = c->dds.clockQuality.clockAccuracy;
		datalen = sizeof(*mtd);
		break;
	case TLV_TRACEABILITY_PROPERTIES:
		mtd = (struct management_tlv_datum *) tlv->data;
		mtd->val = c->tds.flags & (TIME_TRACEABLE|FREQ_TRACEABLE);
		datalen = sizeof(*mtd);
		break;
	case TLV_TIMESCALE_PROPERTIES:
		mtd = (struct management_tlv_datum *) tlv->data;
		mtd->val = c->tds.flags & PTP_TIMESCALE;
		datalen = sizeof(*mtd);
		break;
	case TLV_TIME_STATUS_NP:
		tsn = (struct time_status_np *) tlv->data;
		tsn->master_offset = tmv_to_nanoseconds(c->master_offset);
		tsn->ingress_time = tmv_to_nanoseconds(c->ingress_ts);
		tsn->cumulativeScaledRateOffset =
			(Integer32) (c->status.cumulativeScaledRateOffset +
				      c->nrr * POW2_41 - POW2_41);
		tsn->scaledLastGmPhaseChange = c->status.scaledLastGmPhaseChange;
		tsn->gmTimeBaseIndicator = c->status.gmTimeBaseIndicator;
		tsn->lastGmPhaseChange = c->status.lastGmPhaseChange;
		if (cid_eq(&c->dad.pds.grandmasterIdentity, &c->dds.clockIdentity))
			tsn->gmPresent = 0;
		else
			tsn->gmPresent = 1;
		tsn->gmIdentity = c->dad.pds.grandmasterIdentity;
		datalen = sizeof(*tsn);
		break;
	case TLV_GRANDMASTER_SETTINGS_NP:
		gsn = (struct grandmaster_settings_np *) tlv->data;
		gsn->clockQuality = c->dds.clockQuality;
		gsn->utc_offset = c->utc_offset;
		gsn->time_flags = c->time_flags;
		gsn->time_source = c->time_source;
		datalen = sizeof(*gsn);
		break;
	case TLV_SUBSCRIBE_EVENTS_NP:
		if (p != c->uds_port) {
			/* Only the UDS port allowed. */
			break;
		}
		sen = (struct subscribe_events_np *)tlv->data;
		clock_get_subscription(c, req, sen->bitmask, &sen->duration);
		break;
	default:
		/* The caller should *not* respond to this message. */
		tlv_extra_recycle(extra);
		return 0;
	}
	if (datalen % 2) {
		tlv->data[datalen] = 0;
		datalen++;
	}
	tlv->length = sizeof(tlv->id) + datalen;
	rsp->header.messageLength += sizeof(*tlv) + datalen;
	msg_tlv_attach(rsp, extra);

	/* The caller can respond to this message. */
	return 1;
}

static void clock_stats_update(struct clock_stats *s, double offset, double freq)
{
	struct stats_result offset_stats, freq_stats, delay_stats;

	stats_add_value(s->offset, offset);
	stats_add_value(s->freq, freq);

	if (stats_get_num_values(s->offset) < s->max_count)
		return;

	stats_get_result(s->offset, &offset_stats);
	stats_get_result(s->freq, &freq_stats);

	/* Path delay stats are updated separately, they may be empty. */
	if (!stats_get_result(s->delay, &delay_stats)) {
		pr_info("rms %4.0f max %4.0f "
			"freq %+6.0f +/- %3.0f "
			"delay %5.0f +/- %3.0f",
			offset_stats.rms, offset_stats.max_abs,
			freq_stats.mean, freq_stats.stddev,
			delay_stats.mean, delay_stats.stddev);
	} else {
		pr_info("rms %4.0f max %4.0f "
			"freq %+6.0f +/- %3.0f",
			offset_stats.rms, offset_stats.max_abs,
			freq_stats.mean, freq_stats.stddev);
	}

	stats_reset(s->offset);
	stats_reset(s->freq);
	stats_reset(s->delay);
}

static enum servo_state clock_no_adjust(struct PtpClock *c, tmv_t ingress, tmv_t origin)
{
	double fui;
	double ratio, freq;
	struct freq_estimator *f = &c->fest;
	enum servo_state state = SERVO_UNLOCKED;
	/*
	 * The ratio of the local clock freqency to the master clock
	 * is estimated by:
	 *
	 *    (ingress_2 - ingress_1) / (origin_2 - origin_1)
	 *
	 * Both of the origin time estimates include the path delay,
	 * but we assume that the path delay is in fact constant.
	 * By leaving out the path delay altogther, we can avoid the
	 * error caused by our imperfect path delay measurement.
	 */
	if (tmv_is_zero(f->ingress1)) {
		f->ingress1 = ingress;
		f->origin1 = origin;
		return state;
	}
	f->count++;
	if (f->count < f->max_count) {
		return state;
	}
	if (tmv_cmp(ingress, f->ingress1) == 0) {
		pr_warning("bad timestamps in rate ratio calculation");
		return state;
	}

	ratio = tmv_dbl(tmv_sub(origin, f->origin1)) /
		tmv_dbl(tmv_sub(ingress, f->ingress1));
	freq = (1.0 - ratio) * 1e9;

	if (c->stats.max_count > 1) {
		clock_stats_update(&c->stats, tmv_dbl(c->master_offset), freq);
	} else {
		pr_info("master offset %10" PRId64 " s%d freq %+7.0f "
			"path delay %9" PRId64,
			tmv_to_nanoseconds(c->master_offset), state, freq,
			tmv_to_nanoseconds(c->path_delay));
	}

	fui = 1.0 + (c->status.cumulativeScaledRateOffset + 0.0) / POW2_41;

	pr_debug("peer/local    %.9f", c->nrr);
	pr_debug("fup_info      %.9f", fui);
	pr_debug("product       %.9f", fui * c->nrr);
	pr_debug("sum-1         %.9f", fui + c->nrr - 1.0);
	pr_debug("master/local  %.9f", ratio);
	pr_debug("diff         %+.9f", ratio - (fui + c->nrr - 1.0));

	f->ingress1 = ingress;
	f->origin1 = origin;
	f->count = 0;

	c->master_local_rr = ratio;

	return state;
}

static int clock_utc_correct(struct PtpClock *c, tmv_t ingress)
{
	struct timespec offset;
	int utc_offset, leap, clock_leap;
	uint64_t ts;

	if (!c->utc_timescale)
		return 0;

	utc_offset = c->utc_offset;

	if (c->tds.flags & LEAP_61) {
		leap = 1;
	} else if (c->tds.flags & LEAP_59) {
		leap = -1;
	} else {
		leap = 0;
	}

	/* Handle leap seconds. */
	if ((leap || c->leap_set) && c->clkid == CLOCK_REALTIME) {
		/* If the clock will be stepped, the time stamp has to be the
		   target time. Ignore possible 1 second error in utc_offset. */
		if (c->servo_state == SERVO_UNLOCKED) {
			ts = tmv_to_nanoseconds(tmv_sub(ingress,
							c->master_offset));
			if (c->tds.flags & PTP_TIMESCALE)
				ts -= utc_offset * NS_PER_SEC;
		} else {
			ts = tmv_to_nanoseconds(ingress);
		}

		/* Suspend clock updates in the last second before midnight. */
		if (is_utc_ambiguous(ts)) {
			pr_info("clock update suspended due to leap second");
			return -1;
		}

		clock_leap = leap_second_status(ts, c->leap_set,
						&leap, &utc_offset);
		if (c->leap_set != clock_leap) {
			if (c->kernel_leap)
				sysclk_set_leap(clock_leap);
			else
				servo_leap(c->servo, clock_leap);
			c->leap_set = clock_leap;
		}
	}

	/* Update TAI-UTC offset of the system clock if valid and traceable. */
	if (c->tds.flags & UTC_OFF_VALID && c->tds.flags & TIME_TRACEABLE &&
	    c->utc_offset_set != utc_offset && c->clkid == CLOCK_REALTIME) {
		sysclk_set_tai_offset(utc_offset);
		c->utc_offset_set = utc_offset;
	}

	if (!(c->tds.flags & PTP_TIMESCALE))
		return 0;

	offset.tv_sec = utc_offset;
	offset.tv_nsec = 0;
	/* Local clock is UTC, but master is TAI. */
	c->master_offset = tmv_add(c->master_offset, timespec_to_tmv(offset));
	return 0;
}


/* public methods */


int clock_required_modes(struct PtpClock *c)
{
	int required_modes = 0;

	switch (c->timestamping)
	{
		case TS_SOFTWARE:
			required_modes |= SOF_TIMESTAMPING_TX_SOFTWARE |
				SOF_TIMESTAMPING_RX_SOFTWARE |
				SOF_TIMESTAMPING_SOFTWARE; /* timerstamp reporting, software */
			break;
			
		case TS_LEGACY_HW:
			required_modes |= SOF_TIMESTAMPING_TX_HARDWARE |
				SOF_TIMESTAMPING_RX_HARDWARE |
				SOF_TIMESTAMPING_SYS_HARDWARE;/* timerstamp reporting, deprecated */
			break;
			
		case TS_HARDWARE:
		case TS_ONESTEP:
		case TS_P2P1STEP:
			required_modes |= SOF_TIMESTAMPING_TX_HARDWARE |
				SOF_TIMESTAMPING_RX_HARDWARE |
				SOF_TIMESTAMPING_RAW_HARDWARE;/* timerstamp reporting, hardware */
			break;
		default:
			break;
	}

	return required_modes;
}


struct dataset *clock_default_ds(struct PtpClock *c)
{
	struct dataset *out = &c->default_dataset;
	struct defaultDS *in = &c->dds;

	out->priority1              = in->priority1;
	out->identity               = in->clockIdentity;
	out->quality                = in->clockQuality;
	out->priority2              = in->priority2;
	out->stepsRemoved           = 0;
	out->sender.clockIdentity   = in->clockIdentity;
	out->sender.portNumber      = 0;
	out->receiver.clockIdentity = in->clockIdentity;
	out->receiver.portNumber    = 0;

	return out;
}

void clock_follow_up_info(struct PtpClock *c, struct follow_up_info_tlv *f)
{
	c->status.cumulativeScaledRateOffset = f->cumulativeScaledRateOffset;
	c->status.scaledLastGmPhaseChange = f->scaledLastGmPhaseChange;
	c->status.gmTimeBaseIndicator = f->gmTimeBaseIndicator;
	memcpy(&c->status.lastGmPhaseChange, &f->lastGmPhaseChange,
	       sizeof(c->status.lastGmPhaseChange));
}

void clock_notify_event(struct PtpClock *c, enum NOTIFICATION event)
{
	struct PtpPort *uds = c->uds_port;
	struct PortIdentity pid = PORT_IDENTITY(uds);
	struct ptp_message *msg;
	int id;

	switch (event) {
	/* set id */
	default:
		return;
	}
	/* targetPortIdentity and sequenceId will be filled by
	 * clock_send_notification */
	msg = port_management_notify(pid, uds);
	if (!msg)
		return;
	if (!clock_management_fill_response(c, NULL, NULL, msg, id))
		goto err;
	if (msg_pre_send(msg))
		goto err;
	clock_send_notification(c, msg, event);
err:
	msg_put(msg);
}

/* update path delay after delay_resp is received */
void ptpClockPathDelay(struct PtpClock *c, tmv_t req, tmv_t rx)
{
	tsproc_up_ts(c->tsproc, req, rx);

	if (tsproc_update_delay(c->tsproc, &c->path_delay))
		return;

	c->cur.meanPathDelay = tmv_to_TimeInterval(c->path_delay);

	if (c->stats.delay)
		stats_add_value(c->stats.delay, tmv_dbl(c->path_delay));
}

void clock_peer_delay(struct PtpClock *c, tmv_t ppd, tmv_t req, tmv_t rx,
		      double nrr)
{
	c->path_delay = ppd;
	c->nrr = nrr;

	tsproc_set_delay(c->tsproc, ppd);
	tsproc_up_ts(c->tsproc, req, rx);

	if (c->stats.delay)
		stats_add_value(c->stats.delay, tmv_dbl(ppd));
}


int clock_switch_phc(struct PtpClock *c, int phc_index)
{
	struct servo *servo;
	int fadj, max_adj;
	clockid_t clkid;
	char phc[32];

	snprintf(phc, sizeof(phc), "/dev/ptp%d", phc_index);
	clkid = phc_open(phc);
	if (clkid == CLOCK_INVALID) {
		pr_err("Switching PHC, failed to open %s: %m", phc);
		return -1;
	}
	
	max_adj = phc_max_adj(clkid);
	if (!max_adj) {
		pr_err("Switching PHC, clock is not adjustable");
		phc_close(clkid);
		return -1;
	}
	fadj = (int) clockadj_get_freq(clkid);
	clockadj_set_freq(clkid, fadj);
	servo = servo_create(c->config, c->servo_type, -fadj, max_adj, 0);
	if (!servo) {
		pr_err("Switching PHC, failed to create clock servo");
		phc_close(clkid);
		return -1;
	}
	phc_close(c->clkid);
	servo_destroy(c->servo);
	c->clkid = clkid;
	c->servo = servo;
	c->servo_state = SERVO_UNLOCKED;
	return 0;
}

enum servo_state ptpClockSynchronize(struct PtpClock *c, tmv_t ingress, tmv_t origin)
{
	double adj, weight;
	enum servo_state state = SERVO_UNLOCKED;

	c->ingress_ts = ingress;

	tsproc_down_ts(c->tsproc, origin, ingress);

	if (tsproc_update_offset(c->tsproc, &c->master_offset, &weight)) {
		if (c->free_running) {
			return clock_no_adjust(c, ingress, origin);
		} else {
			return state;
		}
	}

	if (clock_utc_correct(c, ingress)) {
		return c->servo_state;
	}

	c->cur.offsetFromMaster = tmv_to_TimeInterval(c->master_offset);

	if (c->free_running) {
		return clock_no_adjust(c, ingress, origin);
	}

	adj = servo_sample(c->servo, tmv_to_nanoseconds(c->master_offset),
			   tmv_to_nanoseconds(ingress), weight, &state);
	c->servo_state = state;

	if (c->stats.max_count > 1)
	{
		clock_stats_update(&c->stats, tmv_dbl(c->master_offset), adj);
	}
	else
	{
		pr_info("master offset %10" PRId64 " s%d freq %+7.0f path delay %9" PRId64,
			tmv_to_nanoseconds(c->master_offset), state, adj, tmv_to_nanoseconds(c->path_delay));
	}

	tsproc_set_clock_rate_ratio(c->tsproc, clock_rate_ratio(c));

	switch (state)
	{
		case SERVO_UNLOCKED:
			break;
		case SERVO_JUMP:
			clockadj_set_freq(c->clkid, -adj);
			clockadj_step(c->clkid, -tmv_to_nanoseconds(c->master_offset));
			c->ingress_ts = tmv_zero();
			if (c->sanity_check) {
				clockcheck_set_freq(c->sanity_check, -adj);
				clockcheck_step(c->sanity_check,
						-tmv_to_nanoseconds(c->master_offset));
			}
			tsproc_reset(c->tsproc, 0);
			break;
		case SERVO_LOCKED:
			clockadj_set_freq(c->clkid, -adj);
			if (c->clkid == CLOCK_REALTIME) {
				sysclk_set_sync();
			}
			if (c->sanity_check) {
				clockcheck_set_freq(c->sanity_check, -adj);
			}
			break;
	}
	return state;
}

void clock_sync_interval(struct PtpClock *c, int n)
{
	int shift;

	shift = c->freq_est_interval - n;
	if (shift < 0)
		shift = 0;
	else if (shift >= sizeof(int) * 8) {
		shift = sizeof(int) * 8 - 1;
		pr_warning("freq_est_interval is too long");
	}
	c->fest.max_count = (1 << shift);

	shift = c->stats_interval - n;
	if (shift < 0)
		shift = 0;
	else if (shift >= sizeof(int) * 8) {
		shift = sizeof(int) * 8 - 1;
		pr_warning("summary_interval is too long");
	}
	c->stats.max_count = (1 << shift);

	servo_sync_interval(c->servo, n < 0 ? 1.0 / (1 << -n) : 1 << n);
}


void clock_check_ts(struct PtpClock *c, uint64_t ts)
{
	if (c->sanity_check && clockcheck_sample(c->sanity_check, ts)) {
		servo_reset(c->servo);
	}
}

double clock_rate_ratio(struct PtpClock *c)
{
	if (c->free_running) {
		return c->master_local_rr;
	}
	return servo_rate_ratio(c->servo);
}

