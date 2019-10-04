/*
* port event handlers of ordinary and boundary clock
*/

#include <errno.h>

#include "port.h"
#include "portPrivate.h"
#include "clockPrivate.h"


static int __setSyncTxTmo(struct PtpPort *p)
{
	return set_tmo_log(p->fda.fd[FD_SYNC_TX_TIMER], 1, p->logSyncInterval);
}

/* operations needed when state is changed */
static void _e2eTransition(struct PtpPort *p, enum PORT_STATE next)
{
	port_clr_tmo(p->fda.fd[FD_ANNOUNCE_TIMER]);
	port_clr_tmo(p->fda.fd[FD_SYNC_RX_TIMER]);
	port_clr_tmo(p->fda.fd[FD_DELAY_TIMER]);
	port_clr_tmo(p->fda.fd[FD_QUALIFICATION_TIMER]);
	port_clr_tmo(p->fda.fd[FD_MANNO_TIMER]);
	port_clr_tmo(p->fda.fd[FD_SYNC_TX_TIMER]);
	/* Leave FD_UNICAST_REQ_TIMER running. */

	switch (next) {
		case PS_INITIALIZING:
			break;
		case PS_FAULTY:
		case PS_DISABLED:
			portOpDisable(p);
			break;
		case PS_LISTENING:
			port_set_announce_tmo(p);
			break;
		case PS_PRE_MASTER:
			port_set_qualification_tmo(p);
			break;
		case PS_MASTER:
		case PS_GRAND_MASTER:
			set_tmo_log(p->fda.fd[FD_MANNO_TIMER], 1, -10); /*~1ms*/
			__setSyncTxTmo(p);
			break;
			
		case PS_PASSIVE:
			port_set_announce_tmo(p);
			break;
			
		case PS_UNCALIBRATED:
			flush_last_sync(p);
			flush_delay_req(p);
			
			/* fall through */
		case PS_SLAVE:
			port_set_announce_tmo(p);
			port_set_delay_tmo(p);
			break;
	};
}


static void _p2pTransition(struct PtpPort *p, enum PORT_STATE next)
{
	port_clr_tmo(p->fda.fd[FD_ANNOUNCE_TIMER]);
	port_clr_tmo(p->fda.fd[FD_SYNC_RX_TIMER]);
	/* Leave FD_DELAY_TIMER running. */
	port_clr_tmo(p->fda.fd[FD_QUALIFICATION_TIMER]);
	port_clr_tmo(p->fda.fd[FD_MANNO_TIMER]);
	port_clr_tmo(p->fda.fd[FD_SYNC_TX_TIMER]);
	/* Leave FD_UNICAST_REQ_TIMER running. */

	switch (next) {
		case PS_INITIALIZING:
			break;
		case PS_FAULTY:
		case PS_DISABLED:
			portOpDisable(p);
			break;
		case PS_LISTENING:
			port_set_announce_tmo(p);
			port_set_delay_tmo(p);
			break;
		case PS_PRE_MASTER:
			port_set_qualification_tmo(p);
			break;
		case PS_MASTER:
		case PS_GRAND_MASTER:
			set_tmo_log(p->fda.fd[FD_MANNO_TIMER], 1, -10); /*~1ms*/
			__setSyncTxTmo(p);
			break;
		case PS_PASSIVE:
			port_set_announce_tmo(p);
			break;
		case PS_UNCALIBRATED:
			flush_last_sync(p);
			flush_peer_delay(p);
			/* fall through */
		case PS_SLAVE:
			port_set_announce_tmo(p);
			break;
	};
}


/* ordinary and boundary clock */
void bc_dispatch(struct PtpPort *p, enum PORT_EVENT event, int mdiff)
{
	pr_debug(PORT_STR_FORMAT"dispatch %s event, masterDiff:%d", PORT_NAME(p), ptpPortEventString(event), mdiff );

	if (clock_slave_only(p->clock))
	{
		if (event == EV_RS_MASTER || event == EV_RS_GRAND_MASTER)
		{
			pr_warning(PORT_STR_FORMAT"master state recommended in slave only mode", PORT_NAME(p));
			pr_warning(PORT_STR_FORMAT"defaultDS.priority1 probably misconfigured", PORT_NAME(p) );
		}
	}
	TRACE();

	if (!portStateUpdate(p, event, mdiff))
	{
		return;
	}

	TRACE();
	/* ordinary clock: P2P delay mechanism ?? */
	if (p->delayMechanism == DM_P2P)
	{/* P2P delay in boundary clock */
		_p2pTransition(p, p->state);
	}
	else
	{
		_e2eTransition(p, p->state);
	}

	if (p->jbod && p->state == PS_UNCALIBRATED)
	{
		if (clock_switch_phc(p->clock, p->phc_index))
		{
			p->last_fault_type = FT_SWITCH_PHC;
			PORT_DISPATCH(p, EV_FAULT_DETECTED, 0);
			return;
		}
		clock_sync_interval(p->clock, p->log_sync_interval);
	}
	
	TRACE();
}

static int _renewTransport(struct PtpPort *p)
{
	int res;

	if (!portIsEnabled(p)) {
		return 0;
	}
	transport_close(p->trp, &p->fda);
	port_clear_fda(p, FD_FIRST_TIMER);
	res = transport_open(p->trp, p->iface, &p->fda, p->timestamping);
	/* Need to call clock_fda_changed even if transport_open failed in
	 * order to update clock to the now closed descriptors. */
	clock_fda_changed(p->clock);
	return res;
}


static int _setMannoTmo(struct PtpPort *p)
{
	return set_tmo_log(p->fda.fd[FD_MANNO_TIMER], 1, p->logAnnounceInterval);
}

static int __incapableIgnore(struct PtpPort *p, struct ptp_message *m)
{
	if (port_capable(p)) {
		return 0;
	}
	if (msg_type(m) == ANNOUNCE || msg_type(m) == SYNC) {
		return 1;
	}
	return 0;
}

static int __pathTraceIgnore(struct PtpPort *p, struct ptp_message *m)
{
	struct path_trace_tlv *ptt;
	struct ClockIdentity cid;
	struct tlv_extra *extra;
	int i, cnt;

	if (!p->path_trace_enabled) {
		return 0;
	}
	if (msg_type(m) != ANNOUNCE) {
		return 0;
	}
	TAILQ_FOREACH(extra, &m->tlv_list, list) {
		ptt = (struct path_trace_tlv *) extra->tlv;
		if (ptt->type != TLV_PATH_TRACE) {
			continue;
		}
		cnt = path_length(ptt);
		cid = clock_identity(p->clock);
		for (i = 0; i < cnt; i++) {
			if (cid_eq(&ptt->cid[i], &cid)) {
				return 1;
			}
		}
	}
	return 0;
}

static int _portIgnore(struct PtpPort *p, struct ptp_message *m)
{
	struct ClockIdentity c1, c2;

	if (__incapableIgnore(p, m)) {
		return 1;
	}
	if (__pathTraceIgnore(p, m)) {
		return 1;
	}
	if (p->match_transport_specific &&
	    msg_transport_specific(m) != p->transportSpecific) {
		return 1;
	}
	if (pid_eq(&m->header.sourcePortIdentity, &p->portIdentity)) {
		return 1;
	}
	if (m->header.domainNumber != clock_domain_number(p->clock)) {
		return 1;
	}

	c1 = clock_identity(p->clock);
	c2 = m->header.sourcePortIdentity.clockIdentity;

	if (cid_eq(&c1, &c2)) {
		return 1;
	}
	return 0;
}

/* ordinary and boundary clock. called when input of fd is available */
enum PORT_EVENT bc_event(struct PtpPort *p, int fd_index)
{
	enum PORT_EVENT event = EV_NONE;
	struct ptp_message *msg;
	TRACE();
	int cnt, fd = p->fda.fd[fd_index], err;

	TRACE();

	switch (fd_index)
	{
		case FD_ANNOUNCE_TIMER:
		case FD_SYNC_RX_TIMER:
			pr_debug(PORT_STR_FORMAT"%s timeout", PORT_NAME(p), fd_index == FD_SYNC_RX_TIMER ? "RX SYNC" : "ANNOUNCE");
			if (p->best)
				fc_clear(p->best);
			port_set_announce_tmo(p);
			delay_req_prune(p);
			
			if (clock_slave_only(p->clock) && p->delayMechanism != DM_P2P && _renewTransport(p))
			{
				return EV_FAULT_DETECTED;
			}
			return EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES;

		case FD_DELAY_TIMER:
			pr_debug(PORT_STR_FORMAT"DELAY timeout", PORT_NAME(p));
			port_set_delay_tmo(p);
			delay_req_prune(p);
			return port_delay_request(p) ? EV_FAULT_DETECTED : EV_NONE;

		case FD_QUALIFICATION_TIMER:
			pr_debug(PORT_STR_FORMAT"QUALIFICATION timeout", PORT_NAME(p));
			return EV_QUALIFICATION_TIMEOUT_EXPIRES;

		case FD_MANNO_TIMER:
			pr_debug(PORT_STR_FORMAT"Master TX Announce timeout", PORT_NAME(p));
			_setMannoTmo(p);
			return port_tx_announce(p, NULL) ? EV_FAULT_DETECTED : EV_NONE;

		case FD_SYNC_TX_TIMER:
			pr_debug(PORT_STR_FORMAT"master sync timeout", PORT_NAME(p));
			__setSyncTxTmo(p);
			return port_tx_sync(p, NULL) ? EV_FAULT_DETECTED : EV_NONE;

		case FD_UNICAST_SRV_TIMER:
			pr_debug(PORT_STR_FORMAT"unicast service timeout", PORT_NAME(p));
			return unicast_service_timer(p) ? EV_FAULT_DETECTED : EV_NONE;

		case FD_UNICAST_REQ_TIMER:
			pr_debug(PORT_STR_FORMAT"unicast request timeout", PORT_NAME(p));
			return unicast_client_timer(p) ? EV_FAULT_DETECTED : EV_NONE;

		case FD_RTNL:
			pr_debug(PORT_STR_FORMAT"received link status notification", PORT_NAME(p));
			rtnl_link_status(fd, p->name, port_link_status, p);
			if (p->link_status == (LINK_UP | LINK_STATE_CHANGED))
				return EV_FAULT_CLEARED;
			else if ((p->link_status == (LINK_DOWN | LINK_STATE_CHANGED)) ||
				 (p->link_status & TS_LABEL_CHANGED))
				return EV_FAULT_DETECTED;
			else
				return EV_NONE;
	}

	TRACE();
	pr_debug(PORT_STR_FORMAT"event %s is availbale", PORT_NAME(p), (fd_index==FD_EVENT)?"EVENT":(fd_index==FD_GENERAL)?"GENERAL":"Unknown");
	TRACE();
	/* for event and general */
	msg = msg_allocate();
	if (!msg)
		return EV_FAULT_DETECTED;

	msg->hwts.type = p->timestamping;

	cnt = transport_recv(p->trp, fd, msg);
	if (cnt <= 0)
	{
		pr_err(PORT_STR_FORMAT"recv message failed", PORT_NAME(p));
		msg_put(msg);
		return EV_FAULT_DETECTED;
	}
	
	err = ptpMsgReceive(msg, cnt);
	if (err) 
	{
		switch (err)
		{
			case -EBADMSG:
				pr_err(PORT_STR_FORMAT"bad message", PORT_NAME(p));
				break;
			case -EPROTO:
				pr_debug(PORT_STR_FORMAT"ignoring message", PORT_NAME(p));
				break;
		}
		
		msg_put(msg);
		return EV_NONE;
	}
	
	TRACE();
	if (_portIgnore(p, msg))
	{
		msg_put(msg);
		return EV_NONE;
	}
	
	if (msg_sots_missing(msg) && !(p->timestamping == TS_P2P1STEP && msg_type(msg) == PDELAY_REQ))
	{
		pr_err(PORT_STR_FORMAT"received %s without timestamp", PORT_NAME(p), ptpMsgTypeString(msg_type(msg)));
		msg_put(msg);
		return EV_NONE;
	}
	
	if (msg_sots_valid(msg))
	{
		ts_add(&msg->hwts.ts, -p->rx_timestamp_offset);
		clock_check_ts(p->clock, tmv_to_nanoseconds(msg->hwts.ts));
	}

	switch (msg_type(msg))
	{
		case SYNC:
			portMsgProSync(p, msg);
			break;
			
		case DELAY_REQ:
			if (portMsgProDelayReq(p, msg))
				event = EV_FAULT_DETECTED;
			break;
			
		case PDELAY_REQ:
			if (portMsgProPDelayReq(p, msg))
				event = EV_FAULT_DETECTED;
			break;
			
		case PDELAY_RESP:
			if (portMsgProPDelayResp(p, msg))
				event = EV_FAULT_DETECTED;
			break;
			
		case FOLLOW_UP:
			portMsgProFollowUp(p, msg);
			break;
			
		case DELAY_RESP:
			portMsgProDelayResp(p, msg);
			break;
			
		case PDELAY_RESP_FOLLOW_UP:
			portMsgProPDelayRespFollowUp(p, msg);
			break;
			
		case ANNOUNCE:
			if (portMsgProAnnounce(p, msg))
				event = EV_STATE_DECISION_EVENT;
			break;
			
		case SIGNALING:
			if (portMsgProSignaling(p, msg))
			{
				event = EV_FAULT_DETECTED;
			}
			break;
			
		case MANAGEMENT:
			/* management message is handled by clock, not port */
			if (clock_manage(p->clock, p, msg))
				event = EV_STATE_DECISION_EVENT;
			break;
	}

	msg_put(msg);
	return event;
}


