/*
* msg process for ordinary and boundary clock, including Announce
*/

 #include <stdlib.h>
 
#include "port.h"
#include "portPrivate.h"


#include "clockPrivate.h"

static int __announceCompare(struct ptp_message *m1, struct ptp_message *m2)
{
	struct announce_msg *a = &m1->announce, *b = &m2->announce;
	int len =
		sizeof(a->grandmasterPriority1) +
		sizeof(a->grandmasterClockQuality) +
		sizeof(a->grandmasterPriority2) +
		sizeof(a->grandmasterIdentity) +
		sizeof(a->stepsRemoved);

	return memcmp(&a->grandmasterPriority1, &b->grandmasterPriority1, len);
}

static int __msgSourceEqual(struct ptp_message *m1, struct foreign_clock *fc)
{
	struct PortIdentity *id1, *id2;
	id1 = &m1->header.sourcePortIdentity;
	id2 = &fc->dataset.sender;
	return 0 == memcmp(id1, id2, sizeof(*id1));
}

/*
 * Returns non-zero if the announce message is different than last.
 */
static int _addForeignMaster(struct PtpPort *p, struct ptp_message *m)
{
	struct foreign_clock *fc;
	struct ptp_message *tmp;
	int broke_threshold = 0, diff = 0;

	LIST_FOREACH(fc, &p->foreign_masters, list)
	{
		if (__msgSourceEqual(m, fc)) {
			break;
		}
	}
	
	if (!fc) {
		pr_notice(PORT_STR_FORMAT"new foreign master %s", PORT_NAME(p), pid2str(&m->header.sourcePortIdentity));

		fc = malloc(sizeof(*fc));
		if (!fc)
		{
			pr_err("low memory, failed to add foreign master");
			return 0;
		}
		
		memset(fc, 0, sizeof(*fc));
		TAILQ_INIT(&fc->messages);
		LIST_INSERT_HEAD(&p->foreign_masters, fc, list);
		fc->port = p;
		fc->dataset.sender = m->header.sourcePortIdentity;
		/* We do not count this first message, see 9.5.3(b) */
		return 0;
	}

	/*
	 * If this message breaks the threshold, that is an important change.
	 */
	fc_prune(fc);
	if (FOREIGN_MASTER_THRESHOLD - 1 == fc->n_messages) {
		broke_threshold = 1;
	}

	/*
	 * Okay, go ahead and add this announcement.
	 */
	msg_get(m);
	fc->n_messages++;
	TAILQ_INSERT_HEAD(&fc->messages, m, list);

	/*
	 * Test if this announcement contains changed information.
	 */
	if (fc->n_messages > 1) {
		tmp = TAILQ_NEXT(m, list);
		diff = __announceCompare(m, tmp);
	}

	return broke_threshold || diff;
}


/*
 * Returns non-zero if the announce message is different than last.
 */
static int _updateCurrentMaster(struct PtpPort *p, struct ptp_message *m)
{
	struct foreign_clock *fc = p->best;
	struct ptp_message *tmp;
	struct parent_ds *dad;
	struct path_trace_tlv *ptt;
	struct timePropertiesDS tds;

	if (!__msgSourceEqual(m, fc))
		return _addForeignMaster(p, m);

	if (p->state != PS_PASSIVE) {
		tds.currentUtcOffset = m->announce.currentUtcOffset;
		tds.flags = m->header.flagField[1];
		tds.timeSource = m->announce.timeSource;
		clock_update_time_properties(p->clock, tds);
	}
	if (p->path_trace_enabled) {
		ptt = (struct path_trace_tlv *) m->announce.suffix;
		dad = clock_parent_ds(p->clock);
		memcpy(dad->ptl, ptt->cid, ptt->length);
		dad->path_length = path_length(ptt);
	}
	port_set_announce_tmo(p);
	fc_prune(fc);
	msg_get(m);
	fc->n_messages++;
	TAILQ_INSERT_HEAD(&fc->messages, m, list);

	if (fc->n_messages > 1)
	{
		tmp = TAILQ_NEXT(m, list);
		return __announceCompare(m, tmp);
	}
	return 0;
}


/*
 * Returns non-zero if the announce message is both qualified and different.
 */
int portMsgProAnnounce(struct PtpPort *p, struct ptp_message *m)
{
	int result = 0;

	/* Do not qualify announce messages with stepsRemoved >= 255, see
	 * IEEE1588-2008 section 9.3.2.5 (d)
	 */
	if (m->announce.stepsRemoved >= 255)
	{
		return result;
	}

	switch (p->state)
	{
		case PS_INITIALIZING:
		case PS_FAULTY:
		case PS_DISABLED:
			break;
			
		case PS_LISTENING:
		case PS_PRE_MASTER:
		case PS_MASTER:
		case PS_GRAND_MASTER:
			result = _addForeignMaster(p, m);
			break;
			
		case PS_PASSIVE:
		case PS_UNCALIBRATED:
		case PS_SLAVE:
			result = _updateCurrentMaster(p, m);
			break;
	}
	
	return result;
}

enum syfu_event {
	SYNC_MISMATCH,
	SYNC_MATCH,
	FUP_MISMATCH,
	FUP_MATCH,
};

static int _fupSync_ok(struct ptp_message *fup, struct ptp_message *sync)
{
	/*
	 * NB - If the sk_check_fupsync option is not enabled, then
	 * both of these time stamps will be zero.
	 */
	if (tmv_cmp(fup->hwts.sw, sync->hwts.sw) < 0) {
		return 0;
	}
	return 1;
}

static int port_set_sync_rx_tmo(struct PtpPort *p)
{
	return set_tmo_log(p->fda.fd[FD_SYNC_RX_TIMER],
			   p->syncReceiptTimeout, p->logSyncInterval);
}


static void __portSynchronize(struct PtpPort *p, tmv_t ingress_ts, struct LocalTimeStamp origin_ts, Integer64 correction1, Integer64 correction2)
{
	enum servo_state state;
	tmv_t t1, t1c, t2, c1, c2;

#if 0//PTP_NOISE_DEBUG
//	pr_info("ingress %10" PRId64"; \torigin_ts %" PRId64 "; \tCorrect1:%" PRId64 "; \tCorrect2:%10" PRId64 ,
//			tmv_to_nanoseconds(ingress_ts), origin_ts.sec, correction1, correction2);
	pr_info("ingress %10" PRId64"; \torigin_ts %" PRId64, tmv_to_nanoseconds(ingress_ts), origin_ts.sec);
#endif

	port_set_sync_rx_tmo(p);

	t1 = timestamp_to_tmv(origin_ts);
	t2 = ingress_ts;
	c1 = correction_to_tmv(correction1);
	c2 = correction_to_tmv(correction2);
	t1c = tmv_add(t1, tmv_add(c1, c2));

	state = ptpClockSynchronize(p->clock, t2, t1c);
	switch (state)
	{
		case SERVO_UNLOCKED:
			PORT_DISPATCH(p, EV_SYNCHRONIZATION_FAULT, 0);
			break;
		case SERVO_JUMP:
			PORT_DISPATCH(p, EV_SYNCHRONIZATION_FAULT, 0);
			flush_delay_req(p);
			if (p->peer_delay_req) {
				msg_put(p->peer_delay_req);
				p->peer_delay_req = NULL;
			}
			break;
		case SERVO_LOCKED:
			PORT_DISPATCH(p, EV_MASTER_CLOCK_SELECTED, 0);
			break;
	}
}


/*  FSM of sync and follow up */
/*
 * Handle out of order packets. The network stack might
 * provide the follow up _before_ the sync message. After all,
 * they can arrive on two different ports. In addition, time
 * stamping in PHY devices might delay the event packets.
 */
static void port_syfufsm(struct PtpPort *p, enum syfu_event event, struct ptp_message *m)
{
	struct ptp_message *syn, *fup;

	switch (p->syfu)
	{
		case SF_EMPTY:
			switch (event)
			{
				case SYNC_MISMATCH:
					msg_get(m);
					p->last_syncfup = m;
					p->syfu = SF_HAVE_SYNC;
					break;
				case FUP_MISMATCH:
					msg_get(m);
					p->last_syncfup = m;
					p->syfu = SF_HAVE_FUP;
					break;
				case SYNC_MATCH:
					break;
				case FUP_MATCH:
					break;
			}
			break;

		case SF_HAVE_SYNC:
			switch (event)
			{
				case SYNC_MISMATCH:
					msg_put(p->last_syncfup);
					msg_get(m);
					p->last_syncfup = m;
					break;
				case SYNC_MATCH:
					break;
				case FUP_MISMATCH:
					msg_put(p->last_syncfup);
					msg_get(m);
					p->last_syncfup = m;
					p->syfu = SF_HAVE_FUP;
					break;
				case FUP_MATCH:
					syn = p->last_syncfup;
					__portSynchronize(p, syn->hwts.ts, m->ts.pdu,
							 syn->header.correction,
							 m->header.correction);
					msg_put(p->last_syncfup);
					p->syfu = SF_EMPTY;
					break;
			}
			break;

		case SF_HAVE_FUP:
			switch (event)
			{
				case SYNC_MISMATCH:
					msg_put(p->last_syncfup);
					msg_get(m);
					p->last_syncfup = m;
					p->syfu = SF_HAVE_SYNC;
					break;
				case SYNC_MATCH:
					fup = p->last_syncfup;
					__portSynchronize(p, m->hwts.ts, fup->ts.pdu,
							 m->header.correction,
							 fup->header.correction);
					msg_put(p->last_syncfup);
					p->syfu = SF_EMPTY;
					break;
				case FUP_MISMATCH:
					msg_put(p->last_syncfup);
					msg_get(m);
					p->last_syncfup = m;
					break;
				case FUP_MATCH:
					break;
			}
			break;
	}
}

void portMsgProSync(struct PtpPort *p, struct ptp_message *m)
{
	enum syfu_event event;
	struct PortIdentity master;
	
	switch (p->state)
	{
		case PS_INITIALIZING:
		case PS_FAULTY:
		case PS_DISABLED:
		case PS_LISTENING:
		case PS_PRE_MASTER:
		case PS_MASTER:
		case PS_GRAND_MASTER:
		case PS_PASSIVE:
			return;
			
		case PS_UNCALIBRATED:
		case PS_SLAVE:
			break;
	}
	
	master = clock_parent_identity(p->clock);
	if (!pid_eq(&master, &m->header.sourcePortIdentity))
	{
		return;
	}

	if (!msg_unicast(m) &&
	    m->header.logMessageInterval != p->log_sync_interval) {
		p->log_sync_interval = m->header.logMessageInterval;
		clock_sync_interval(p->clock, p->log_sync_interval);
	}

	m->header.correction += p->asymmetry;

	if (one_step(m))
	{
		__portSynchronize(p, m->hwts.ts, m->ts.pdu, m->header.correction, 0);
		flush_last_sync(p);
		return;
	}

	if (p->syfu == SF_HAVE_FUP && _fupSync_ok(p->last_syncfup, m) &&
	    p->last_syncfup->header.sequenceId == m->header.sequenceId)
	{
		event = SYNC_MATCH;
	}
	else
	{
		event = SYNC_MISMATCH;
	}
	
	port_syfufsm(p, event, m);
}

static struct follow_up_info_tlv *_followUpInfoExtract(struct ptp_message *m)
{
	struct follow_up_info_tlv *f;
	struct tlv_extra *extra;

	TAILQ_FOREACH(extra, &m->tlv_list, list) {
		f = (struct follow_up_info_tlv *) extra->tlv;
		if (f->type == TLV_ORGANIZATION_EXTENSION &&
		    f->length == sizeof(*f) - sizeof(f->type) - sizeof(f->length) &&
//		    memcmp(f->id, ieee8021_id, sizeof(ieee8021_id)) &&
		    !f->subtype[0] && !f->subtype[1] && f->subtype[2] == 1) {
			return f;
		}
	}
	return NULL;
}

void portMsgProFollowUp(struct PtpPort *p, struct ptp_message *m)
{
	enum syfu_event event;
	struct PortIdentity master;
	switch (p->state) {
	case PS_INITIALIZING:
	case PS_FAULTY:
	case PS_DISABLED:
	case PS_LISTENING:
	case PS_PRE_MASTER:
	case PS_MASTER:
	case PS_GRAND_MASTER:
	case PS_PASSIVE:
		return;
	case PS_UNCALIBRATED:
	case PS_SLAVE:
		break;
	}
	master = clock_parent_identity(p->clock);
	if (!pid_eq(&master, &m->header.sourcePortIdentity)) {
		return;
	}

	if (p->follow_up_info) {
		struct follow_up_info_tlv *fui = _followUpInfoExtract(m);
		if (!fui)
			return;
		clock_follow_up_info(p->clock, fui);
	}

	if (p->syfu == SF_HAVE_SYNC &&
	    p->last_syncfup->header.sequenceId == m->header.sequenceId) {
		event = FUP_MATCH;
	} else {
		event = FUP_MISMATCH;
	}
	port_syfufsm(p, event, m);
}

/* NSM: NetSync Monitor */
static int _portNsmReply(struct PtpPort *p, struct ptp_message *m)
{
	struct tlv_extra *extra;

	if (!p->net_sync_monitor) {
		return 0;
	}
	if (!p->hybrid_e2e) {
		return 0;
	}
	if (!msg_unicast(m)) {
		return 0;
	}
	TAILQ_FOREACH(extra, &m->tlv_list, list) {
		if (extra->tlv->type == TLV_PTPMON_REQ) {
			return 1;
		}
	}
	return 0;
}

static void __extractAddress(struct ptp_message *m, struct PortAddress *paddr)
{
	int len = 0;

	switch (paddr->networkProtocol) {
		case TRANS_UDP_IPV4:
			len = sizeof(m->address.sin.sin_addr.s_addr);
			memcpy(paddr->address, &m->address.sin.sin_addr.s_addr, len);
			break;
		case TRANS_UDP_IPV6:
			len = sizeof(m->address.sin6.sin6_addr.s6_addr);
			memcpy(paddr->address, &m->address.sin6.sin6_addr.s6_addr, len);
			break;
		case TRANS_IEEE_802_3:
			len = MAC_LEN;
			memcpy(paddr->address, &m->address.sll.sll_addr, len);
			break;
		default:
			return;
	}
	paddr->addressLength = len;
}


static int _netSyncRespAppend(struct PtpPort *p, struct ptp_message *m)
{
	struct timePropertiesDS *tp = clock_time_properties(p->clock);
	struct ClockIdentity cid = clock_identity(p->clock), pid;
	struct currentDS *cds = clock_current_dataset(p->clock);
	struct parent_ds *dad = clock_parent_ds(p->clock);
	struct PtpPort *best = clock_best_port(p->clock);
	struct nsm_resp_tlv_head *head;
	struct WireTimeStamp last_sync;
	struct PortAddress *paddr;
	struct ptp_message *tmp;
	struct tlv_extra *extra;
	unsigned char *ptr;
	int tlv_len;

	uint8_t buf[sizeof(*paddr) + sizeof(struct sockaddr_storage)];

	last_sync = tmv_to_Timestamp(clock_ingress_time(p->clock));
	pid = dad->pds.parentPortIdentity.clockIdentity;
	paddr = (struct PortAddress *)buf;

	if (best && !cid_eq(&cid, &pid)) {
		/* Extract the parent's protocol address. */
		paddr->networkProtocol = TransportType(best->trp);
		paddr->addressLength =
			transport_protocol_addr(best->trp, paddr->address);
		if (best->best) {
			tmp = TAILQ_FIRST(&best->best->messages);
			__extractAddress(tmp, paddr);
		}
	}
	else
	{
		/* We are our own parent. */
		paddr->networkProtocol = TransportType(p->trp);
		paddr->addressLength =
			transport_protocol_addr(p->trp, paddr->address);
	}

	tlv_len = sizeof(*head) + sizeof(*extra->foot) + paddr->addressLength;

	extra = msg_tlv_append(m, tlv_len);
	if (!extra) {
		return -1;
	}

	head = (struct nsm_resp_tlv_head *) extra->tlv;
	head->type = TLV_PTPMON_RESP;
	head->length = tlv_len - sizeof(head->type) - sizeof(head->length);
	head->port_state = p->state == PS_GRAND_MASTER ? PS_MASTER : p->state;
	head->parent_addr.networkProtocol = paddr->networkProtocol;
	head->parent_addr.addressLength = paddr->addressLength;
	memcpy(head->parent_addr.address, paddr->address, paddr->addressLength);

	ptr = (unsigned char *) head;
	ptr += sizeof(*head) + paddr->addressLength;
	extra->foot = (struct nsm_resp_tlv_foot *) ptr;

	memcpy(&extra->foot->parent, &dad->pds, sizeof(extra->foot->parent));
	memcpy(&extra->foot->current, cds, sizeof(extra->foot->current));
	memcpy(&extra->foot->timeprop, tp, sizeof(extra->foot->timeprop));
	memcpy(&extra->foot->lastsync, &last_sync, sizeof(extra->foot->lastsync));

	return 0;
}


int portMsgProDelayReq(struct PtpPort *p, struct ptp_message *m)
{
	int err, nsm, saved_seqnum_sync;
	struct ptp_message *msg;

	nsm = _portNsmReply(p, m);

	if (!nsm && p->state != PS_MASTER && p->state != PS_GRAND_MASTER) {
		return 0;
	}

	if (p->delayMechanism == DM_P2P) {
		pr_warning(PORT_STR_FORMAT"delay request on P2P port", PORT_NAME(p));
		return 0;
	}

	msg = msg_allocate();
	if (!msg) {
		return -1;
	}

	msg->hwts.type = p->timestamping;

	msg->header.tsmt               = DELAY_RESP | p->transportSpecific;
	msg->header.ver                = PTP_VERSION;
	msg->header.messageLength      = sizeof(struct delay_resp_msg);
	msg->header.domainNumber       = m->header.domainNumber;
	msg->header.correction         = m->header.correction;
	msg->header.sourcePortIdentity = p->portIdentity;
	msg->header.sequenceId         = m->header.sequenceId;
	msg->header.control            = CTL_DELAY_RESP;
	msg->header.logMessageInterval = p->logMinDelayReqInterval;

	msg->delay_resp.receiveTimestamp = tmv_to_Timestamp(m->hwts.ts);

	msg->delay_resp.requestingPortIdentity = m->header.sourcePortIdentity;

	if (p->hybrid_e2e && msg_unicast(m)) {
		msg->address = m->address;
		msg->header.flagField[0] |= UNICAST;
		msg->header.logMessageInterval = 0x7f;
	}
	if (nsm && _netSyncRespAppend(p, msg))
	{
		pr_err(PORT_STR_FORMAT"append NSM failed", PORT_NAME(p));
		err = -1;
		goto out;
	}
	
	err = port_prepare_and_send(p, msg, TRANS_GENERAL);
	if (err) {
		pr_err(PORT_STR_FORMAT"send delay response failed", PORT_NAME(p));
		goto out;
	}
	if (nsm) {
		saved_seqnum_sync = p->seqnum.sync;
		p->seqnum.sync = m->header.sequenceId;
		err = port_tx_sync(p, &m->address);
		p->seqnum.sync = saved_seqnum_sync;
	}
out:
	msg_put(msg);
	return err;
}

/*
* received delayReq
*/
void portMsgProDelayResp(struct PtpPort *p, struct ptp_message *m)
{
	struct delay_resp_msg *rsp = &m->delay_resp;
	struct PortIdentity master;
	struct ptp_message *req;
	tmv_t c3, t3, t4, t4c;

	master = clock_parent_identity(p->clock);

	if (p->state != PS_UNCALIBRATED && p->state != PS_SLAVE) {
		return;
	}
	if (!pid_eq(&rsp->requestingPortIdentity, &p->portIdentity)) {
		return;
	}
	if (!pid_eq(&master, &m->header.sourcePortIdentity)) {
		return;
	}
	TAILQ_FOREACH(req, &p->delay_req, list) {
		if (rsp->hdr.sequenceId == ntohs(req->delay_req.hdr.sequenceId)) {
			break;
		}
	}
	if (!req) {
		return;
	}

	c3 = correction_to_tmv(m->header.correction);
	t3 = req->hwts.ts;
	t4 = timestamp_to_tmv(m->ts.pdu);
	t4c = tmv_sub(t4, c3);

	ptpClockPathDelay(p->clock, t3, t4c);

	TAILQ_REMOVE(&p->delay_req, req, list);
	msg_put(req);

	if (p->logMinDelayReqInterval == rsp->hdr.logMessageInterval) {
		return;
	}
	if (msg_unicast(m)) {
		/* Unicast responses have logMinDelayReqInterval set to 0x7F. */
		return;
	}
	if (rsp->hdr.logMessageInterval < -10 ||
	    rsp->hdr.logMessageInterval > 22) {//?? 300
		pl_info(300, PORT_STR_FORMAT"ignore bogus delay request interval 2^%d", PORT_NAME(p), rsp->hdr.logMessageInterval);
		return;
	}
	p->logMinDelayReqInterval = rsp->hdr.logMessageInterval;
	pr_notice(PORT_STR_FORMAT"minimum delay request interval 2^%d", PORT_NAME(p), p->logMinDelayReqInterval);
	port_set_delay_tmo(p);
}

