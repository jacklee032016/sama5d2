/*
* message process for transparent clock, P2P and E2E
*/

#include "port.h"
#include "portPrivate.h"
#include "clockPrivate.h"


int portMsgProPDelayReq(struct PtpPort *p, struct ptp_message *m)
{
	struct ptp_message *rsp, *fup;
	enum transport_event event;
	int err;

	switch (p->timestamping) {
	case TS_SOFTWARE:
	case TS_LEGACY_HW:
	case TS_HARDWARE:
	case TS_ONESTEP:
		event = TRANS_EVENT;
		break;
	case TS_P2P1STEP:
		event = TRANS_P2P1STEP;
		break;
	default:
		return -1;
	}

	if (p->delayMechanism == DM_E2E) {
		pr_warning(PORT_STR_FORMAT"pdelay_req on E2E port", PORT_NAME(p));
		return 0;
	}
	if (p->delayMechanism == DM_AUTO) {
		pr_info(PORT_STR_FORMAT"peer detected, switch to P2P", PORT_NAME(p));
		p->delayMechanism = DM_P2P;
		port_set_delay_tmo(p);
	}
	if (p->peer_portid_valid) {
		if (!pid_eq(&p->peer_portid, &m->header.sourcePortIdentity)) {
			pr_err(PORT_STR_FORMAT"received pdelay_req msg with "
				"unexpected peer port id %s",
				PORT_NAME(p), pid2str(&m->header.sourcePortIdentity));
			p->peer_portid_valid = 0;
			port_capable(p);
		}
	} else {
		p->peer_portid_valid = 1;
		p->peer_portid = m->header.sourcePortIdentity;
		pr_debug(PORT_STR_FORMAT"peer port id set to %s", PORT_NAME(p), pid2str(&p->peer_portid));
	}

	rsp = msg_allocate();
	if (!rsp) {
		return -1;
	}

	fup = msg_allocate();
	if (!fup) {
		msg_put(rsp);
		return -1;
	}

	rsp->hwts.type = p->timestamping;

	rsp->header.tsmt               = PDELAY_RESP | p->transportSpecific;
	rsp->header.ver                = PTP_VERSION;
	rsp->header.messageLength      = sizeof(struct pdelay_resp_msg);
	rsp->header.domainNumber       = m->header.domainNumber;
	rsp->header.sourcePortIdentity = p->portIdentity;
	rsp->header.sequenceId         = m->header.sequenceId;
	rsp->header.control            = CTL_OTHER;
	rsp->header.logMessageInterval = 0x7f;

	/*
	 * NB - We do not have any fraction nanoseconds for the correction
	 * fields, neither in the response or the follow up.
	 */
	if (p->timestamping == TS_P2P1STEP) {
		rsp->header.correction = m->header.correction;
		rsp->header.correction += p->tx_timestamp_offset;
		rsp->header.correction += p->rx_timestamp_offset;
	} else {
		rsp->header.flagField[0] |= TWO_STEP;
		rsp->pdelay_resp.requestReceiptTimestamp =
			tmv_to_Timestamp(m->hwts.ts);
	}
	rsp->pdelay_resp.requestingPortIdentity = m->header.sourcePortIdentity;

	if (msg_unicast(m)) {
		rsp->address = m->address;
		rsp->header.flagField[0] |= UNICAST;
	}

	err = peer_prepare_and_send(p, rsp, event);
	if (err) {
		pr_err(PORT_STR_FORMAT"send peer delay response failed", PORT_NAME(p));
		goto out;
	}
	if (p->timestamping == TS_P2P1STEP) {
		goto out;
	} else if (msg_sots_missing(rsp)) {
		pr_err("missing timestamp on transmitted peer delay response");
		err = -1;
		goto out;
	}

	/*
	 * Send the follow up message right away.
	 */
	fup->hwts.type = p->timestamping;

	fup->header.tsmt               = PDELAY_RESP_FOLLOW_UP | p->transportSpecific;
	fup->header.ver                = PTP_VERSION;
	fup->header.messageLength      = sizeof(struct pdelay_resp_fup_msg);
	fup->header.domainNumber       = m->header.domainNumber;
	fup->header.correction         = m->header.correction;
	fup->header.sourcePortIdentity = p->portIdentity;
	fup->header.sequenceId         = m->header.sequenceId;
	fup->header.control            = CTL_OTHER;
	fup->header.logMessageInterval = 0x7f;

	fup->pdelay_resp_fup.requestingPortIdentity = m->header.sourcePortIdentity;

	fup->pdelay_resp_fup.responseOriginTimestamp =
		tmv_to_Timestamp(rsp->hwts.ts);

	if (msg_unicast(m)) {
		fup->address = m->address;
		fup->header.flagField[0] |= UNICAST;
	}

	err = peer_prepare_and_send(p, fup, TRANS_GENERAL);
	if (err) {
		pr_err(PORT_STR_FORMAT"send pdelay_resp_fup failed", PORT_NAME(p));
	}
out:
	msg_put(rsp);
	msg_put(fup);
	return err;
}


static void port_nrate_calculate(struct PtpPort *p, tmv_t origin, tmv_t ingress)
{
	struct nrate_estimator *n = &p->nrate;

	/*
	 * We experienced a successful exchanges of peer delay request
	 * and response, reset pdr_missing for this port.
	 */
	p->pdr_missing = 0;

	if (tmv_is_zero(n->ingress1)) {
		n->ingress1 = ingress;
		n->origin1 = origin;
		return;
	}
	n->count++;
	if (n->count < n->max_count) {
		return;
	}
	if (tmv_cmp(ingress, n->ingress1) == 0) {
		pr_warning("bad timestamps in nrate calculation");
		return;
	}
	n->ratio =
		tmv_dbl(tmv_sub(origin, n->origin1)) /
		tmv_dbl(tmv_sub(ingress, n->ingress1));
	n->ingress1 = ingress;
	n->origin1 = origin;
	n->count = 0;
	n->ratio_valid = 1;
}

static void port_peer_delay(struct PtpPort *p)
{
	tmv_t c1, c2, t1, t2, t3, t3c, t4;
	struct ptp_message *req = p->peer_delay_req;
	struct ptp_message *rsp = p->peer_delay_resp;
	struct ptp_message *fup = p->peer_delay_fup;

	/* Check for response, validate port and sequence number. */

	if (!rsp)
		return;

	if (!pid_eq(&rsp->pdelay_resp.requestingPortIdentity, &p->portIdentity))
		return;

	if (rsp->header.sequenceId != ntohs(req->header.sequenceId))
		return;

	t1 = req->hwts.ts;
	t4 = rsp->hwts.ts;
	c1 = correction_to_tmv(rsp->header.correction + p->asymmetry);

	/* Process one-step response immediately. */
	if (one_step(rsp)) {
		t2 = tmv_zero();
		t3 = tmv_zero();
		c2 = tmv_zero();
		goto calc;
	}

	/* Check for follow up, validate port and sequence number. */

	if (!fup)
		return;

	if (!pid_eq(&fup->pdelay_resp_fup.requestingPortIdentity, &p->portIdentity))
		return;

	if (fup->header.sequenceId != rsp->header.sequenceId)
		return;

	if (!source_pid_eq(fup, rsp))
		return;

	/* Process follow up response. */
	t2 = timestamp_to_tmv(rsp->ts.pdu);
	t3 = timestamp_to_tmv(fup->ts.pdu);
	c2 = correction_to_tmv(fup->header.correction);
calc:
	t3c = tmv_add(t3, tmv_add(c1, c2));

	if (p->follow_up_info)
		port_nrate_calculate(p, t3c, t4);

	tsproc_set_clock_rate_ratio(p->tsproc, p->nrate.ratio*clock_rate_ratio(p->clock) );
	tsproc_up_ts(p->tsproc, t1, t2);
	tsproc_down_ts(p->tsproc, t3c, t4);
	if (tsproc_update_delay(p->tsproc, &p->peer_delay))
		return;

	p->peerMeanPathDelay = tmv_to_TimeInterval(p->peer_delay);

	if (p->state == PS_UNCALIBRATED || p->state == PS_SLAVE) {
		clock_peer_delay(p->clock, p->peer_delay, t1, t2,
				 p->nrate.ratio);
	}

	msg_put(p->peer_delay_req);
	p->peer_delay_req = NULL;
}

int portMsgProPDelayResp(struct PtpPort *p, struct ptp_message *m)
{
	if (p->peer_delay_resp) {
		if (!source_pid_eq(p->peer_delay_resp, m)) {
			pr_err(PORT_STR_FORMAT"multiple peer responses", PORT_NAME(p));
			if (!p->multiple_pdr_detected) {
				p->multiple_pdr_detected = 1;
				p->multiple_seq_pdr_count++;
			}
			if (p->multiple_seq_pdr_count >= 3) {
				p->last_fault_type = FT_BAD_PEER_NETWORK;
				return -1;
			}
		}
	}
	if (!p->peer_delay_req) {
		pr_err(PORT_STR_FORMAT"rogue peer delay response", PORT_NAME(p));
		return -1;
	}
	if (p->peer_portid_valid) {
		if (!pid_eq(&p->peer_portid, &m->header.sourcePortIdentity)) {
			pr_err(PORT_STR_FORMAT"received pdelay_resp msg with "
				"unexpected peer port id %s",
				PORT_NAME(p),
				pid2str(&m->header.sourcePortIdentity));
			p->peer_portid_valid = 0;
			port_capable(p);
		}
	} else {
		p->peer_portid_valid = 1;
		p->peer_portid = m->header.sourcePortIdentity;
		pr_debug(PORT_STR_FORMAT"peer port id set to %s", PORT_NAME(p),
			pid2str(&p->peer_portid));
	}

	if (p->peer_delay_resp) {
		msg_put(p->peer_delay_resp);
	}
	msg_get(m);
	p->peer_delay_resp = m;
	port_peer_delay(p);
	return 0;
}

void portMsgProPDelayRespFollowUp(struct PtpPort *p, struct ptp_message *m)
{
	if (!p->peer_delay_req) {
		return;
	}

	if (p->peer_delay_fup) {
		msg_put(p->peer_delay_fup);
	}

	msg_get(m);
	p->peer_delay_fup = m;
	port_peer_delay(p);
}

