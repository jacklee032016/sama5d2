/**
 * @file port.c
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
#include <arpa/inet.h>
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/queue.h>
#include <net/if.h>

#include "clock.h"
#include "missing.h"
#include "port.h"
#include "portPrivate.h"
#include "sk.h"
#include "tmv.h"
#include "util.h"

#include "clockPrivate.h"

#define ALLOWED_LOST_RESPONSES 3


int port_capable(struct PtpPort *p);
void port_nrate_initialize(struct PtpPort *p);


static void announce_to_dataset(struct ptp_message *m, struct PtpPort *p, struct dataset *out)
{
	struct announce_msg *a = &m->announce;
	out->priority1    = a->grandmasterPriority1;
	out->identity     = a->grandmasterIdentity;
	out->quality      = a->grandmasterClockQuality;
	out->priority2    = a->grandmasterPriority2;
	out->localPriority = p->localPriority;
	out->stepsRemoved = a->stepsRemoved;
	out->sender       = m->header.sourcePortIdentity;
	out->receiver     = p->portIdentity;
}

int clear_fault_asap(struct fault_interval *faint)
{
	switch (faint->type) {
	case FTMO_LINEAR_SECONDS:
		return faint->val == 0 ? 1 : 0;
	case FTMO_LOG2_SECONDS:
		return faint->val == FRI_ASAP ? 1 : 0;
	case FTMO_CNT:
		return 0;
	}
	return 0;
}


static int msg_current(struct ptp_message *m, struct timespec now)
{
	int64_t t1, t2, tmo;

	t1 = m->ts.host.tv_sec * NSEC2SEC + m->ts.host.tv_nsec;
	t2 = now.tv_sec * NSEC2SEC + now.tv_nsec;

	if (m->header.logMessageInterval < -63) {
		tmo = 0;
	} else if (m->header.logMessageInterval > 31) {
		tmo = INT64_MAX;
	} else if (m->header.logMessageInterval < 0) {
		tmo = 4LL * NSEC2SEC / (1 << -m->header.logMessageInterval);
	} else {
		tmo = 4LL * (1 << m->header.logMessageInterval) * NSEC2SEC;
	}

	return t2 - t1 < tmo;
}


void fault_interval(struct PtpPort *port, enum fault_type ft, struct fault_interval *i)
{
	i->type = port->flt_interval_pertype[ft].type;
	i->val = port->flt_interval_pertype[ft].val;
}

int set_tmo_log(int fd, unsigned int scale, int log_seconds)
{
	struct itimerspec tmo = {
		{0, 0}, {0, 0}
	};
	uint64_t ns;
	int i;

	if (log_seconds < 0) {

		log_seconds *= -1;
		for (i = 1, ns = scale * 500000000ULL; i < log_seconds; i++) {
			ns >>= 1;
		}
		tmo.it_value.tv_nsec = ns;

		while (tmo.it_value.tv_nsec >= NS_PER_SEC) {
			tmo.it_value.tv_nsec -= NS_PER_SEC;
			tmo.it_value.tv_sec++;
		}

	} else
		tmo.it_value.tv_sec = scale * (1 << log_seconds);

	return timerfd_settime(fd, 0, &tmo, NULL);
}

int set_tmo_lin(int fd, int seconds)
{
	struct itimerspec tmo = {
		{0, 0}, {0, 0}
	};

	tmo.it_value.tv_sec = seconds;
	return timerfd_settime(fd, 0, &tmo, NULL);
}

int set_tmo_random(int fd, int min, int span, int log_seconds)
{
	uint64_t value_ns, min_ns, span_ns;
	struct itimerspec tmo = {
		{0, 0}, {0, 0}
	};

	if (log_seconds >= 0) {
		min_ns = min * NS_PER_SEC << log_seconds;
		span_ns = span * NS_PER_SEC << log_seconds;
	} else {
		min_ns = min * NS_PER_SEC >> -log_seconds;
		span_ns = span * NS_PER_SEC >> -log_seconds;
	}

	value_ns = min_ns + (span_ns * (random() % (1 << 15) + 1) >> 15);

	tmo.it_value.tv_sec = value_ns / NS_PER_SEC;
	tmo.it_value.tv_nsec = value_ns % NS_PER_SEC;

	return timerfd_settime(fd, 0, &tmo, NULL);
}

int port_set_fault_timer_log(struct PtpPort *port, unsigned int scale, int log_seconds)
{
	return set_tmo_log(port->fault_fd, scale, log_seconds);
}

int port_set_fault_timer_lin(struct PtpPort *port, int seconds)
{
	return set_tmo_lin(port->fault_fd, seconds);
}

void fc_clear(struct foreign_clock *fc)
{
	struct ptp_message *m;

	while (fc->n_messages) {
		m = TAILQ_LAST(&fc->messages, messages);
		TAILQ_REMOVE(&fc->messages, m, list);
		fc->n_messages--;
		msg_put(m);
	}
}

void fc_prune(struct foreign_clock *fc)
{
	struct timespec now;
	struct ptp_message *m;

	PTP_GET_SYS_TIME_MONOTONIC( &now);

	while (fc->n_messages > FOREIGN_MASTER_THRESHOLD) {
		m = TAILQ_LAST(&fc->messages, messages);
		TAILQ_REMOVE(&fc->messages, m, list);
		fc->n_messages--;
		msg_put(m);
	}

	while (!TAILQ_EMPTY(&fc->messages)) {
		m = TAILQ_LAST(&fc->messages, messages);
		if (msg_current(m, now))
			break;
		TAILQ_REMOVE(&fc->messages, m, list);
		fc->n_messages--;
		msg_put(m);
	}
}

static int delay_req_current(struct ptp_message *m, struct timespec now)
{
	int64_t t1, t2, tmo = 5 * NSEC2SEC;

	t1 = m->ts.host.tv_sec * NSEC2SEC + m->ts.host.tv_nsec;
	t2 = now.tv_sec * NSEC2SEC + now.tv_nsec;

	return t2 - t1 < tmo;
}

void delay_req_prune(struct PtpPort *p)
{
	struct timespec now;
	struct ptp_message *m;
	
	PTP_GET_SYS_TIME_MONOTONIC(&now);

	while (!TAILQ_EMPTY(&p->delay_req)) {
		m = TAILQ_LAST(&p->delay_req, delay_req);
		if (delay_req_current(m, now)) {
			break;
		}
		TAILQ_REMOVE(&p->delay_req, m, list);
		msg_put(m);
	}
}

void ts_add(tmv_t *ts, Integer64 correction)
{
	if (!correction) {
		return;
	}
	*ts = tmv_add(*ts, correction_to_tmv(correction));
}

static int follow_up_info_append(struct ptp_message *m)
{
	struct follow_up_info_tlv *fui;
	struct tlv_extra *extra;

	extra = msg_tlv_append(m, sizeof(*fui));
	if (!extra) {
		return -1;
	}
	fui = (struct follow_up_info_tlv *) extra->tlv;
	fui->type = TLV_ORGANIZATION_EXTENSION;
	fui->length = sizeof(*fui) - sizeof(fui->type) - sizeof(fui->length);
	memcpy(fui->id, ieee8021_id, sizeof(ieee8021_id));
	fui->subtype[2] = 1;

	return 0;
}


static int path_trace_append(struct PtpPort *p, struct ptp_message *m,
			     struct parent_ds *dad)
{
	int length = 1 + dad->path_length, ptt_len, tlv_len;
	struct path_trace_tlv *ptt;
	struct tlv_extra *extra;

	if (length > PATH_TRACE_MAX) {
		return -1;
	}

	ptt_len = length * sizeof(struct ClockIdentity);
	tlv_len = ptt_len + sizeof(ptt->type) + sizeof(ptt->length);

	extra = msg_tlv_append(m, tlv_len);
	if (!extra) {
		return -1;
	}
	ptt = (struct path_trace_tlv *) extra->tlv;
	ptt->type = TLV_PATH_TRACE;
	ptt->length = ptt_len;
	memcpy(ptt->cid, dad->ptl, ptt->length);
	ptt->cid[length - 1] = clock_identity(p->clock);

	return 0;
}


int peer_prepare_and_send(struct PtpPort *p, struct ptp_message *msg,
				 enum transport_event event)
{
	int cnt;
	if (msg_pre_send(msg)) {
		return -1;
	}
	if (msg_unicast(msg)) {
		cnt = transport_sendto(p->trp, &p->fda, event, msg);
	} else {
		cnt = transport_peer(p->trp, &p->fda, event, msg);
	}
	if (cnt <= 0) {
		return -1;
	}
	if (msg_sots_valid(msg)) {
		ts_add(&msg->hwts.ts, p->tx_timestamp_offset);
	}
	return 0;
}

int port_capable(struct PtpPort *p)
{
	if (!PORT_IS_IEEE8021_AS(p)) {
		/* Normal 1588 ports are always capable. */
		goto capable;
	}

	if (tmv_to_nanoseconds(p->peer_delay) >	p->neighborPropDelayThresh) {
		if (p->asCapable)
			pr_debug(PORT_STR_FORMAT"peer_delay (%" PRId64 ") > neighborPropDelayThresh "
				"(%" PRId32 "), resetting asCapable", PORT_NAME(p),
				tmv_to_nanoseconds(p->peer_delay),
				p->neighborPropDelayThresh);
		goto not_capable;
	}

	if (tmv_to_nanoseconds(p->peer_delay) <	p->min_neighbor_prop_delay) {
		if (p->asCapable)
			pr_debug(PORT_STR_FORMAT"peer_delay (%" PRId64 ") < min_neighbor_prop_delay "
				"(%" PRId32 "), resetting asCapable", PORT_NAME(p),
				tmv_to_nanoseconds(p->peer_delay),
				p->min_neighbor_prop_delay);
		goto not_capable;
	}

	if (p->pdr_missing > ALLOWED_LOST_RESPONSES) {
		if (p->asCapable)
			pr_debug(PORT_STR_FORMAT"missed %d peer delay resp, "
				"resetting asCapable", PORT_NAME(p), p->pdr_missing);
		goto not_capable;
	}

	if (p->multiple_seq_pdr_count) {
		if (p->asCapable)
			pr_debug(PORT_STR_FORMAT"multiple sequential peer delay resp, "
				"resetting asCapable", PORT_NAME(p));
		goto not_capable;
	}

	if (!p->peer_portid_valid) {
		if (p->asCapable)
			pr_debug(PORT_STR_FORMAT"invalid peer port id, resetting asCapable", PORT_NAME(p));
		goto not_capable;
	}

	if (!p->nrate.ratio_valid) {
		if (p->asCapable)
			pr_debug(PORT_STR_FORMAT"invalid nrate, resetting asCapable", PORT_NAME(p));
		goto not_capable;
	}

capable:
	if (!p->asCapable)
		pr_debug(PORT_STR_FORMAT"setting asCapable", PORT_NAME(p));
	p->asCapable = 1;
	return 1;

not_capable:
	if (p->asCapable)
		port_nrate_initialize(p);
	p->asCapable = 0;
	return 0;
}

int port_clr_tmo(int fd)
{
	struct itimerspec tmo = {
		{0, 0}, {0, 0}
	};
	return timerfd_settime(fd, 0, &tmo, NULL);
}


/*
 * Test whether a 802.1AS port may transmit a sync message.
 */
static int port_sync_incapable(struct PtpPort *p)
{
	struct ClockIdentity cid;
	struct PortIdentity pid;

	if (!PORT_IS_IEEE8021_AS(p)) {
		return 0;
	}
	if (clock_gm_capable(p->clock)) {
		return 0;
	}
	cid = clock_identity(p->clock);
	pid = clock_parent_identity(p->clock);
	if (cid_eq(&cid, &pid.clockIdentity)) {
		/*
		 * We are the GM, but without gmCapable set.
		 */
		return 1;
	}
	return 0;
}


void port_nrate_initialize(struct PtpPort *p)
{
	int shift = p->freq_est_interval - p->logMinPdelayReqInterval;

	if (shift < 0)
		shift = 0;
	else if (shift >= sizeof(int) * 8) {
		shift = sizeof(int) * 8 - 1;
		pr_warning("freq_est_interval is too long");
	}

	/* We start in the 'incapable' state. */
	p->pdr_missing = ALLOWED_LOST_RESPONSES + 1;
	p->asCapable = 0;

	p->peer_portid_valid = 0;

	p->nrate.origin1 = tmv_zero();
	p->nrate.ingress1 = tmv_zero();
	p->nrate.max_count = (1 << shift);
	p->nrate.count = 0;
	p->nrate.ratio = 1.0;
	p->nrate.ratio_valid = 0;
}


int port_set_announce_tmo(struct PtpPort *p)
{
	return set_tmo_random(p->fda.fd[FD_ANNOUNCE_TIMER],
			      p->announceReceiptTimeout,
			      p->announce_span, p->logAnnounceInterval);
}

int port_set_delay_tmo(struct PtpPort *p)
{
	if (p->delayMechanism == DM_P2P) {
		return set_tmo_log(p->fda.fd[FD_DELAY_TIMER], 1,
			       p->logMinPdelayReqInterval);
	} else {
		return set_tmo_random(p->fda.fd[FD_DELAY_TIMER], 0, 2,
				p->logMinDelayReqInterval);
	}
}


int port_set_qualification_tmo(struct PtpPort *p)
{
	return set_tmo_log(p->fda.fd[FD_QUALIFICATION_TIMER],
		       1+clock_steps_removed(p->clock), p->logAnnounceInterval);
}


static int port_pdelay_request(struct PtpPort *p)
{
	struct ptp_message *msg;
	int err;

	/* If multiple pdelay resp were not detected the counter can be reset */
	if (!p->multiple_pdr_detected) {
		p->multiple_seq_pdr_count = 0;
	}
	p->multiple_pdr_detected = 0;

	msg = msg_allocate();
	if (!msg) {
		return -1;
	}

	msg->hwts.type = p->timestamping;

	msg->header.tsmt               = PDELAY_REQ | p->transportSpecific;
	msg->header.ver                = PTP_VERSION;
	msg->header.messageLength      = sizeof(struct pdelay_req_msg);
	msg->header.domainNumber       = clock_domain_number(p->clock);
	msg->header.correction         = -p->asymmetry;
	msg->header.sourcePortIdentity = p->portIdentity;
	msg->header.sequenceId         = p->seqnum.delayreq++;
	msg->header.control            = CTL_OTHER;
	msg->header.logMessageInterval = PORT_IS_IEEE8021_AS(p) ? p->logMinPdelayReqInterval : 0x7f;

	if (unicast_client_enabled(p) && p->unicast_master_table->peer_name) {
		msg->address = p->unicast_master_table->peer_addr.address;
		msg->header.flagField[0] |= UNICAST;
	}

	err = peer_prepare_and_send(p, msg, TRANS_EVENT);
	if (err) {
		pr_err(PORT_STR_FORMAT"send peer delay request failed", PORT_NAME(p));
		goto out;
	}
	if (msg_sots_missing(msg)) {
		pr_err("missing timestamp on transmitted peer delay request");
		goto out;
	}

	if (p->peer_delay_req) {
		if (port_capable(p)) {
			p->pdr_missing++;
		}
		msg_put(p->peer_delay_req);
	}
	p->peer_delay_req = msg;
	return 0;
out:
	msg_put(msg);
	return -1;
}

int port_delay_request(struct PtpPort *p)
{
	struct ptp_message *msg;

	/* Time to send a new request, forget current pdelay resp and fup */
	if (p->peer_delay_resp) {
		msg_put(p->peer_delay_resp);
		p->peer_delay_resp = NULL;
	}
	if (p->peer_delay_fup) {
		msg_put(p->peer_delay_fup);
		p->peer_delay_fup = NULL;
	}

	if (p->delayMechanism == DM_P2P) {
		return port_pdelay_request(p);
	}

	msg = msg_allocate();
	if (!msg) {
		return -1;
	}

	msg->hwts.type = p->timestamping;

	msg->header.tsmt               = DELAY_REQ | p->transportSpecific;
	msg->header.ver                = PTP_VERSION;
	msg->header.messageLength      = sizeof(struct delay_req_msg);
	msg->header.domainNumber       = clock_domain_number(p->clock);
	msg->header.correction         = -p->asymmetry;
	msg->header.sourcePortIdentity = p->portIdentity;
	msg->header.sequenceId         = p->seqnum.delayreq++;
	msg->header.control            = CTL_DELAY_REQ;
	msg->header.logMessageInterval = 0x7f;

	if (p->hybrid_e2e) {
		struct ptp_message *dst = TAILQ_FIRST(&p->best->messages);
		msg->address = dst->address;
		msg->header.flagField[0] |= UNICAST;
	}

	if (port_prepare_and_send(p, msg, TRANS_EVENT)) {
		pr_err(PORT_STR_FORMAT"send delay request failed", PORT_NAME(p));
		goto out;
	}
	if (msg_sots_missing(msg)) {
		pr_err("missing timestamp on transmitted delay request");
		goto out;
	}

	TAILQ_INSERT_HEAD(&p->delay_req, msg, list);

	return 0;
out:
	msg_put(msg);
	return -1;
}

int port_tx_announce(struct PtpPort *p, struct address *dst)
{
	struct timePropertiesDS *tp = clock_time_properties(p->clock);
	struct parent_ds *dad = clock_parent_ds(p->clock);
	struct ptp_message *msg;
	int err;

	if (p->inhibit_multicast_service && !dst) {
		return 0;
	}
	if (!port_capable(p)) {
		return 0;
	}
	msg = msg_allocate();
	if (!msg) {
		return -1;
	}

	msg->hwts.type = p->timestamping;

	msg->header.tsmt               = ANNOUNCE | p->transportSpecific;
	msg->header.ver                = PTP_VERSION;
	msg->header.messageLength      = sizeof(struct announce_msg);
	msg->header.domainNumber       = clock_domain_number(p->clock);
	msg->header.sourcePortIdentity = p->portIdentity;
	msg->header.sequenceId         = p->seqnum.announce++;
	msg->header.control            = CTL_OTHER;
	msg->header.logMessageInterval = p->logAnnounceInterval;

	msg->header.flagField[1] = tp->flags;

	if (dst) {
		msg->address = *dst;
		msg->header.flagField[0] |= UNICAST;
	}
	msg->announce.currentUtcOffset        = tp->currentUtcOffset;
	msg->announce.grandmasterPriority1    = dad->pds.grandmasterPriority1;
	msg->announce.grandmasterClockQuality = dad->pds.grandmasterClockQuality;
	msg->announce.grandmasterPriority2    = dad->pds.grandmasterPriority2;
	msg->announce.grandmasterIdentity     = dad->pds.grandmasterIdentity;
	msg->announce.stepsRemoved            = clock_steps_removed(p->clock);
	msg->announce.timeSource              = tp->timeSource;

	if (p->path_trace_enabled && path_trace_append(p, msg, dad))
	{
		pr_err(PORT_STR_FORMAT"append path trace failed", PORT_NAME(p));
	}

	err = port_prepare_and_send(p, msg, TRANS_GENERAL);
	if (err)
	{
		pr_err(PORT_STR_FORMAT"send announce failed", PORT_NAME(p));
	}
	msg_put(msg);
	return err;
}

int port_tx_sync(struct PtpPort *p, struct address *dst)
{
	struct ptp_message *msg, *fup;
	int err, event;

	switch (p->timestamping) {
	case TS_SOFTWARE:
	case TS_LEGACY_HW:
	case TS_HARDWARE:
		event = TRANS_EVENT;
		break;
	case TS_ONESTEP:
		event = TRANS_ONESTEP;
		break;
	case TS_P2P1STEP:
		event = TRANS_P2P1STEP;
		break;
	default:
		return -1;
	}

	if (p->inhibit_multicast_service && !dst) {
		return 0;
	}
	if (!port_capable(p)) {
		return 0;
	}
	if (port_sync_incapable(p)) {
		return 0;
	}
	msg = msg_allocate();
	if (!msg) {
		return -1;
	}
	fup = msg_allocate();
	if (!fup) {
		msg_put(msg);
		return -1;
	}

	msg->hwts.type = p->timestamping;

	msg->header.tsmt               = SYNC | p->transportSpecific;
	msg->header.ver                = PTP_VERSION;
	msg->header.messageLength      = sizeof(struct sync_msg);
	msg->header.domainNumber       = clock_domain_number(p->clock);
	msg->header.sourcePortIdentity = p->portIdentity;
	msg->header.sequenceId         = p->seqnum.sync++;
	msg->header.control            = CTL_SYNC;
	msg->header.logMessageInterval = p->logSyncInterval;

	if (p->timestamping != TS_ONESTEP && p->timestamping != TS_P2P1STEP) {
		msg->header.flagField[0] |= TWO_STEP;
	}

	if (dst) {
		msg->address = *dst;
		msg->header.flagField[0] |= UNICAST;
		msg->header.logMessageInterval = 0x7f;
	}
	err = port_prepare_and_send(p, msg, event);
	if (err) {
		pr_err(PORT_STR_FORMAT"send sync failed", PORT_NAME(p));
		goto out;
	}
	if (p->timestamping == TS_ONESTEP || p->timestamping == TS_P2P1STEP) {
		goto out;
	} else if (msg_sots_missing(msg)) {
		pr_err("missing timestamp on transmitted sync");
		err = -1;
		goto out;
	}

	/*
	 * Send the follow up message right away.
	 */
	fup->hwts.type = p->timestamping;

	fup->header.tsmt               = FOLLOW_UP | p->transportSpecific;
	fup->header.ver                = PTP_VERSION;
	fup->header.messageLength      = sizeof(struct follow_up_msg);
	fup->header.domainNumber       = clock_domain_number(p->clock);
	fup->header.sourcePortIdentity = p->portIdentity;
	fup->header.sequenceId         = p->seqnum.sync - 1;
	fup->header.control            = CTL_FOLLOW_UP;
	fup->header.logMessageInterval = p->logSyncInterval;

	fup->follow_up.preciseOriginTimestamp = tmv_to_Timestamp(msg->hwts.ts);

	if (dst) {
		fup->address = *dst;
		fup->header.flagField[0] |= UNICAST;
	}

	if (p->follow_up_info && follow_up_info_append(fup))
	{
		pr_err(PORT_STR_FORMAT"append fup info failed", PORT_NAME(p));
		err = -1;
		goto out;
	}

	err = port_prepare_and_send(p, fup, TRANS_GENERAL);
	if (err) {
		pr_err(PORT_STR_FORMAT"send follow up failed", PORT_NAME(p));
	}
out:
	msg_put(msg);
	msg_put(fup);
	return err;
}


void flush_last_sync(struct PtpPort *p)
{
	if (p->syfu != SF_EMPTY) {
		msg_put(p->last_syncfup);
		p->syfu = SF_EMPTY;
	}
}

void flush_delay_req(struct PtpPort *p)
{
	struct ptp_message *m;
	while ((m = TAILQ_FIRST(&p->delay_req)) != NULL) {
		TAILQ_REMOVE(&p->delay_req, m, list);
		msg_put(m);
	}
}

void flush_peer_delay(struct PtpPort *p)
{
	if (p->peer_delay_req) {
		msg_put(p->peer_delay_req);
		p->peer_delay_req = NULL;
	}
	if (p->peer_delay_resp) {
		msg_put(p->peer_delay_resp);
		p->peer_delay_resp = NULL;
	}
	if (p->peer_delay_fup) {
		msg_put(p->peer_delay_fup);
		p->peer_delay_fup = NULL;
	}
}

void port_clear_fda(struct PtpPort *p, int count)
{
	int i;

	for (i = 0; i < count; i++)
		p->fda.fd[i] = -1;
}


/* public methods */

struct foreign_clock *port_compute_best(struct PtpPort *p)
{
	int (*dscmp)(struct dataset *a, struct dataset *b);
	struct foreign_clock *fc;
	struct ptp_message *tmp;

	dscmp = clock_dscmp(p->clock);
	p->best = NULL;

	if (p->master_only)
		return p->best;

	LIST_FOREACH(fc, &p->foreign_masters, list) {
		tmp = TAILQ_FIRST(&fc->messages);
		if (!tmp)
			continue;

		announce_to_dataset(tmp, p, &fc->dataset);

		fc_prune(fc);

		if (fc->n_messages < FOREIGN_MASTER_THRESHOLD)
			continue;

		if (!p->best)
			p->best = fc;
		else if (dscmp(&fc->dataset, &p->best->dataset) > 0)
			p->best = fc;
		else
			fc_clear(fc);
	}

	return p->best;
}


/* callback in rntl_link_status */
void port_link_status(void *ctx, int linkup, int ts_index)
{
	struct PtpPort *p = ctx;
	int link_state;
	char ts_label[MAX_IFNAME_SIZE + 1] = {0};
	int required_modes;

	link_state = linkup ? LINK_UP : LINK_DOWN;
	if (p->link_status & link_state) {
		p->link_status = link_state;
	} else {
		p->link_status = link_state | LINK_STATE_CHANGED;
		pr_notice(PORT_STR_FORMAT"link %s", PORT_NAME(p), linkup ? "up" : "down");
	}

	/* ts_label changed */
	if (if_indextoname(ts_index, ts_label) && strcmp(p->iface->ts_label, ts_label)) {
		strncpy(p->iface->ts_label, ts_label, MAX_IFNAME_SIZE);
		p->link_status |= TS_LABEL_CHANGED;
		pr_notice(PORT_STR_FORMAT"ts label changed to %s", PORT_NAME(p), ts_label);
	}

	/* Both link down/up and change ts_label may change phc index. */
	if (p->link_status & LINK_UP &&
	    (p->link_status & LINK_STATE_CHANGED || p->link_status & TS_LABEL_CHANGED)) {
		sk_get_ts_info(p->iface->ts_label, &p->iface->ts_info);

		/* Only switch phc with HW time stamping mode */
		if (p->phc_index >= 0 && p->iface->ts_info.valid) {
			required_modes = clock_required_modes(p->clock);
			if ((p->iface->ts_info.so_timestamping & required_modes) != required_modes) {
				pr_err("interface '%s' does not support requested "
				       "timestamping mode, set link status down by force.",
				       p->iface->ts_label);
				p->link_status = LINK_DOWN | LINK_STATE_CHANGED;
			} else if (p->phc_index != p->iface->ts_info.phc_index) {
				p->phc_index = p->iface->ts_info.phc_index;

				if (clock_switch_phc(p->clock, p->phc_index))
				{
					p->last_fault_type = FT_SWITCH_PHC;
					PORT_DISPATCH(p, EV_FAULT_DETECTED, 0);
					return;
				}
				clock_sync_interval(p->clock, p->log_sync_interval);
			}
		}
	}

	/*
	 * A port going down can affect the BMCA result.
	 * Force a state decision event.
	 */
	if (p->link_status & LINK_DOWN)
		clock_set_sde(p->clock, 1);
}

int port_forward(struct PtpPort *p, struct ptp_message *msg)
{
	int cnt;
	cnt = transport_send(p->trp, &p->fda, TRANS_GENERAL, msg);
	return cnt <= 0 ? -1 : 0;
}

int port_forward_to(struct PtpPort *p, struct ptp_message *msg)
{
	int cnt;
	cnt = transport_sendto(p->trp, &p->fda, TRANS_GENERAL, msg);
	return cnt <= 0 ? -1 : 0;
}

int port_prepare_and_send(struct PtpPort *p, struct ptp_message *msg,
			  enum transport_event event)
{
	int cnt;

	if (msg_pre_send(msg)) {
		return -1;
	}
	if (msg_unicast(msg)) {
		cnt = transport_sendto(p->trp, &p->fda, event, msg);
	} else {
		cnt = transport_send(p->trp, &p->fda, event, msg);
	}
	if (cnt <= 0) {
		return -1;
	}
	if (msg_sots_valid(msg)) {
		ts_add(&msg->hwts.ts, p->tx_timestamp_offset);
	}
	return 0;
}

int port_management_error(struct PortIdentity pid, struct PtpPort *ingress,
			  struct ptp_message *req, Enumeration16 error_id)
{
	struct management_error_status *mes;
	struct management_tlv *mgt;
	struct ptp_message *msg;
	struct tlv_extra *extra;
	int err = 0;

	mgt = (struct management_tlv *) req->management.suffix;
	msg = port_management_reply(pid, ingress, req);
	if (!msg) {
		return -1;
	}

	extra = msg_tlv_append(msg, sizeof(*mes));
	if (!extra) {
		msg_put(msg);
		return -ENOMEM;
	}
	mes = (struct management_error_status *) extra->tlv;
	mes->type = TLV_MANAGEMENT_ERROR_STATUS;
	mes->length = 8;
	mes->error = error_id;
	mes->id = mgt->id;

	err = port_prepare_and_send(ingress, msg, TRANS_GENERAL);
	msg_put(msg);
	return err;
}


