/**
 * @file unicast_client.c
 * @brief Unicast client implementation
 * @note Copyright (C) 2018 Richard Cochran <richardcochran@gmail.com>
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
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA.
 */

/* used in SIGNAL message only */

#include "port.h"
#include "portPrivate.h"
#include "unicast_client.h"


#include "clockPrivate.h"


/* unicast client state machine */
enum unicast_state unicast_fsm(enum unicast_state state, enum unicast_event ev)
{
	enum unicast_state next = state;

	switch (state) {
		case UC_WAIT:
			switch (ev) {
			case UC_EV_GRANT_ANN:
				next = UC_HAVE_ANN;
				break;
			case UC_EV_SELECTED:
			case UC_EV_GRANT_SYDY:
			case UC_EV_UNSELECTED:
			case UC_EV_CANCEL:
				break;
			}
			break;
		case UC_HAVE_ANN:
			switch (ev) {
			case UC_EV_GRANT_ANN:
				break;
			case UC_EV_SELECTED:
				next = UC_NEED_SYDY;
				break;
			case UC_EV_GRANT_SYDY:
			case UC_EV_UNSELECTED:
				break;
			case UC_EV_CANCEL:
				next = UC_WAIT;
				break;
			}
			break;
		case UC_NEED_SYDY:
			switch (ev) {
			case UC_EV_GRANT_ANN:
			case UC_EV_SELECTED:
				break;
			case UC_EV_GRANT_SYDY:
				next = UC_HAVE_SYDY;
				break;
			case UC_EV_UNSELECTED:
				next = UC_HAVE_ANN;
				break;
			case UC_EV_CANCEL:
				next = UC_WAIT;
				break;
			}
			break;
		case UC_HAVE_SYDY:
			switch (ev) {
			case UC_EV_GRANT_ANN:
			case UC_EV_SELECTED:
			case UC_EV_GRANT_SYDY:
				break;
			case UC_EV_UNSELECTED:
				next = UC_HAVE_ANN;
				break;
			case UC_EV_CANCEL:
				next = UC_WAIT;
				break;
			}
			break;
	}
	return next;
}


#define E2E_SYDY_MASK	(1 << ANNOUNCE | 1 << SYNC | 1 << DELAY_RESP)
#define P2P_SYDY_MASK	(1 << ANNOUNCE | 1 << SYNC)

static struct PortIdentity wildcard = {
	.clockIdentity = {
		{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
	},
	.portNumber = 0xffff,
};

static int attach_ack(struct ptp_message *msg, uint8_t message_type_flags)
{
	struct ack_cancel_unicast_xmit_tlv *ack;
	struct tlv_extra *extra;

	extra = msg_tlv_append(msg, sizeof(*ack));
	if (!extra) {
		return -1;
	}
	ack = (struct ack_cancel_unicast_xmit_tlv *) extra->tlv;
	ack->type = TLV_ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION;
	ack->length = sizeof(*ack) - sizeof(ack->type) - sizeof(ack->length);
	ack->message_type_flags = message_type_flags;

	return 0;
}

static int attach_request(struct ptp_message *msg, int log_period,
			  uint8_t message_type, int duration)
{
	struct request_unicast_xmit_tlv *req;
	struct tlv_extra *extra;

	extra = msg_tlv_append(msg, sizeof(*req));
	if (!extra) {
		return -1;
	}
	req = (struct request_unicast_xmit_tlv *) extra->tlv;
	req->type = TLV_REQUEST_UNICAST_TRANSMISSION;
	req->length = sizeof(*req) - sizeof(req->type) - sizeof(req->length);
	req->message_type = message_type << 4;
	req->logInterMessagePeriod = log_period;
	req->durationField = duration;

	return 0;
}

static int unicast_client_announce(struct PtpPort *p,
				   struct unicast_master_address *dst)
{
	struct ptp_message *msg;
	int err;

	msg = port_signaling_construct(p, &dst->address, &dst->portIdentity);
	if (!msg) {
		return -1;
	}
	err = attach_request(msg, p->logAnnounceInterval, ANNOUNCE,
			     p->unicast_req_duration);
	if (err) {
		goto out;
	}
	err = port_prepare_and_send(p, msg, TRANS_GENERAL);
	if (err) {
		pr_err(PORT_STR_FORMAT"signaling message failed", PORT_NAME(p));
	}
out:
	msg_put(msg);
	return err;
}

static struct unicast_master_address *unicast_client_ok(struct PtpPort *p,
							struct ptp_message *m)
{
	struct unicast_master_address *ucma;

	if (!unicast_client_enabled(p)) {
		return NULL;
	}
	if (!pid_eq(&m->signaling.targetPortIdentity, &p->portIdentity) &&
	    !pid_eq(&m->signaling.targetPortIdentity, &wildcard)) {
		return NULL;
	}
	STAILQ_FOREACH(ucma, &p->unicast_master_table->addrs, list) {
		if (addreq(TransportType(p->trp), &ucma->address, &m->address)) {
			break;
		}
	}
	if (!ucma) {
		pr_warning(PORT_STR_FORMAT"received rogue unicast grant or cancel", PORT_NAME(p));
		return NULL;
	}
	return ucma;
}

static int unicast_client_peer_renew(struct PtpPort *p)
{
	struct unicast_master_address *peer;
	struct ptp_message *msg;
	struct timespec now;
	int err;

	if (!p->unicast_master_table->peer_name) {
		return 0;
	}
	err = PTP_GET_SYS_TIME_MONOTONIC( &now);
	if (err) {
		pr_err("clock_gettime failed: %m");
		return err;
	}
	peer = &p->unicast_master_table->peer_addr;
	if (now.tv_sec < peer->renewal_tmo) {
		return 0;
	}
	peer->renewal_tmo = 0;
	pr_debug(PORT_STR_FORMAT"time to renew P2P unicast subscription", PORT_NAME(p));

	msg = port_signaling_construct(p, &peer->address, &peer->portIdentity);
	if (!msg) {
		return -1;
	}
	err = attach_request(msg, p->logMinPdelayReqInterval, PDELAY_RESP,
			     p->unicast_req_duration);
	if (err) {
		goto out;
	}
	err = port_prepare_and_send(p, msg, TRANS_GENERAL);
	if (err) {
		pr_err(PORT_STR_FORMAT"P2P signaling message failed", PORT_NAME(p));
	}
out:
	msg_put(msg);
	return err;
}

static int unicast_client_renew(struct PtpPort *p,
				struct unicast_master_address *dst)
{
	struct ptp_message *msg;
	struct timespec now;
	int err;

	err = PTP_GET_SYS_TIME_MONOTONIC( &now);
	if (err) {
		pr_err("clock_gettime failed: %m");
		return err;
	}
	if (now.tv_sec < dst->renewal_tmo) {
		return 0;
	}
	dst->renewal_tmo = 0;
	pr_debug(PORT_STR_FORMAT"time to renew unicast subscriptions", PORT_NAME(p));

	msg = port_signaling_construct(p, &dst->address, &dst->portIdentity);
	if (!msg) {
		return -1;
	}
	err = attach_request(msg, p->logAnnounceInterval, ANNOUNCE,
			     p->unicast_req_duration);
	if (err) {
		goto out;
	}

	if (dst->state == UC_HAVE_SYDY) {
		err = attach_request(msg, p->logSyncInterval, SYNC,
				     p->unicast_req_duration);
		if (err) {
			goto out;
		}
		if (p->delayMechanism != DM_P2P) {
			err = attach_request(msg, p->logMinDelayReqInterval,
					     DELAY_RESP,
					     p->unicast_req_duration);
			if (err) {
				goto out;
			}
		}
	}

	err = port_prepare_and_send(p, msg, TRANS_GENERAL);
	if (err) {
		pr_err(PORT_STR_FORMAT"signaling message failed", PORT_NAME(p));
	}
out:
	msg_put(msg);
	return err;
}

static void unicast_client_set_renewal(struct PtpPort *p,
				       struct unicast_master_address *master,
				       long duration)
{
	struct timespec now;
	long tmo;

	if (PTP_GET_SYS_TIME_MONOTONIC(&now)) {
		pr_err("clock_gettime failed: %m");
		return;
	}
	duration = (3 * duration) / 4;
	tmo = now.tv_sec + duration;
	if (!master->renewal_tmo || tmo < master->renewal_tmo) {
		master->renewal_tmo = tmo;
		pr_debug(PORT_STR_FORMAT"renewal timeout at %ld", PORT_NAME(p), tmo);
	}
}

static int unicast_client_sydy(struct PtpPort *p,
			       struct unicast_master_address *dst)
{
	struct ptp_message *msg;
	int err;

	msg = port_signaling_construct(p, &dst->address, &dst->portIdentity);
	if (!msg) {
		return -1;
	}
	err = attach_request(msg, p->logSyncInterval, SYNC,
			     p->unicast_req_duration);
	if (err) {
		goto out;
	}
	if (p->delayMechanism != DM_P2P) {
		err = attach_request(msg, p->logMinDelayReqInterval, DELAY_RESP,
				     p->unicast_req_duration);
		if (err) {
			goto out;
		}
	}
	err = port_prepare_and_send(p, msg, TRANS_GENERAL);
	if (err) {
		pr_err(PORT_STR_FORMAT"signaling message failed", PORT_NAME(p));
	}
out:
	msg_put(msg);
	return err;
}

/* public methods */

int unicast_client_cancel(struct PtpPort *p, struct ptp_message *m,
			  struct tlv_extra *extra)
{
	struct cancel_unicast_xmit_tlv *cancel;
	struct unicast_master_address *ucma;
	struct ptp_message *msg;
	uint8_t mtype;
	int err;

	ucma = unicast_client_ok(p, m);
	if (!ucma) {
		return 0;
	}
	cancel = (struct cancel_unicast_xmit_tlv *) extra->tlv;
	mtype = cancel->message_type_flags >> 4;
	switch (mtype) {
	case ANNOUNCE:
	case SYNC:
	case DELAY_RESP:
		break;
	default:
		return 0;
	}
	if (cancel->message_type_flags & CANCEL_UNICAST_MAINTAIN_GRANT) {
		return 0;
	}
	pr_warning(PORT_STR_FORMAT"server unilaterally canceled unicast %s grant", PORT_NAME(p), ptpMsgTypeString(mtype));

	ucma->state = unicast_fsm(ucma->state, UC_EV_CANCEL);
	ucma->granted &= ~(1 << mtype);

	/* Respond with ACK. */
	msg = port_signaling_construct(p, &ucma->address, &ucma->portIdentity);
	if (!msg) {
		return -1;
	}
	err = attach_ack(msg, cancel->message_type_flags);
	if (err) {
		goto out;
	}
	err = port_prepare_and_send(p, msg, TRANS_GENERAL);
	if (err) {
		pr_err(PORT_STR_FORMAT"signaling message failed", PORT_NAME(p));
	}
out:
	msg_put(msg);
	return err;
}

int unicast_client_claim_table(struct PtpPort *p)
{
	struct unicast_master_address *master, *peer;
	struct PtpConfig *cfg = clock_config(p->clock);
	struct unicast_master_table *table;
	int table_id;

	table_id = config_get_int(cfg, p->name, "unicast_master_table");
	if (!table_id) {
		return 0;
	}
	STAILQ_FOREACH(table, &cfg->unicast_master_tables, list) {
		if (table->table_index == table_id) {
			break;
		}
	}
	if (!table) {
		pr_err(PORT_STR_FORMAT"no table with id %d", PORT_NAME(p), table_id);
		return -1;
	}
	if (table->port) {
		pr_err(PORT_STR_FORMAT"table %d already claimed by port %d", PORT_NAME(p), table_id, table->port);
		return -1;
	}
	peer = &table->peer_addr;
	if (table->peer_name && str2addr(TransportType(p->trp),
					 table->peer_name, &peer->address))
	{
		pr_err(PORT_STR_FORMAT"bad peer address: %s", PORT_NAME(p), table->peer_name);
		return -1;
	}
	STAILQ_FOREACH(master, &table->addrs, list) {
		if (master->type != TransportType(p->trp)) {
			pr_warning(PORT_STR_FORMAT"unicast master transport mismatch", PORT_NAME(p));
		}
		if (p->delayMechanism == DM_P2P) {
			master->sydymsk = P2P_SYDY_MASK;
		} else {
			master->sydymsk = E2E_SYDY_MASK;
		}
	}
	table->port = PORT_NUMBER(p);
	p->unicast_master_table = table;
	p->unicast_req_duration =
		config_get_int(cfg, p->name, "unicast_req_duration");
	return 0;
}

int unicast_client_enabled(struct PtpPort *p)
{
	return p->unicast_master_table ? 1 : 0;
}

void unicast_client_grant(struct PtpPort *p, struct ptp_message *m,
			  struct tlv_extra *extra)
{
	struct unicast_master_address *ucma;
	struct grant_unicast_xmit_tlv *g;
	int mtype;

	ucma = unicast_client_ok(p, m);
	if (!ucma) {
		return;
	}
	g = (struct grant_unicast_xmit_tlv *) extra->tlv;
	mtype = g->message_type >> 4;

	if (!g->durationField) {
		pr_warning(PORT_STR_FORMAT"unicast grant of %s rejected", PORT_NAME(p), ptpMsgTypeString(mtype));
		if (mtype != PDELAY_RESP) {
			ucma->state = UC_WAIT;
		}
		return;
	}
	pr_debug(PORT_STR_FORMAT"unicast %s granted for %u sec", PORT_NAME(p), ptpMsgTypeString(mtype), g->durationField);

	if (p->delayMechanism == DM_P2P) {
		switch (mtype) {
		case DELAY_RESP:
			return;
		case PDELAY_RESP:
			p->unicast_master_table->peer_addr.portIdentity =
				m->header.sourcePortIdentity;
			unicast_client_set_renewal(p,
				&p->unicast_master_table->peer_addr,
				g->durationField);
			p->logMinPdelayReqInterval = g->logInterMessagePeriod;
			return;
		default:
			break;
		}
	}

	ucma->granted |= 1 << mtype;

	switch (ucma->state) {
	case UC_WAIT:
		if (mtype == ANNOUNCE) {
			ucma->state = unicast_fsm(ucma->state, UC_EV_GRANT_ANN);
			ucma->portIdentity = m->header.sourcePortIdentity;
			unicast_client_set_renewal(p, ucma, g->durationField);
		}
		break;
	case UC_HAVE_ANN:
		break;
	case UC_NEED_SYDY:
		switch (mtype) {
		case DELAY_RESP:
			if ((ucma->granted & ucma->sydymsk) == ucma->sydymsk) {
				ucma->state = unicast_fsm(ucma->state,
							  UC_EV_GRANT_SYDY);
			}
			unicast_client_set_renewal(p, ucma, g->durationField);
			p->logMinDelayReqInterval = g->logInterMessagePeriod;
			break;
		case SYNC:
			if ((ucma->granted & ucma->sydymsk) == ucma->sydymsk) {
				ucma->state = unicast_fsm(ucma->state,
							  UC_EV_GRANT_SYDY);
			}
			unicast_client_set_renewal(p, ucma, g->durationField);
			clock_sync_interval(p->clock, g->logInterMessagePeriod);
			break;
		}
		break;
	case UC_HAVE_SYDY:
		switch (mtype) {
		case ANNOUNCE:
		case DELAY_RESP:
		case SYNC:
			unicast_client_set_renewal(p, ucma, g->durationField);
			break;
		}
		break;
	}
}

int unicast_client_set_tmo(struct PtpPort *p)
{
	return set_tmo_log(p->fda.fd[FD_UNICAST_REQ_TIMER], 1,
			   p->unicast_master_table->logQueryInterval);
}

void unicast_client_state_changed(struct PtpPort *p)
{
	struct unicast_master_address *ucma;
	struct PortIdentity pid;

	if (!unicast_client_enabled(p)) {
		return;
	}
	pid = clock_parent_identity(p->clock);

	STAILQ_FOREACH(ucma, &p->unicast_master_table->addrs, list) {
		if (pid_eq(&ucma->portIdentity, &pid)) {
			ucma->state = unicast_fsm(ucma->state, UC_EV_SELECTED);
		} else {
			ucma->state = unicast_fsm(ucma->state, UC_EV_UNSELECTED);
		}
	}
}

int unicast_client_timer(struct PtpPort *p)
{
	struct unicast_master_address *master;
	int err = 0;

	STAILQ_FOREACH(master, &p->unicast_master_table->addrs, list) {
		if (master->type != TransportType(p->trp)) {
			continue;
		}
		switch (master->state) {
		case UC_WAIT:
			err = unicast_client_announce(p, master);
			break;
		case UC_HAVE_ANN:
			err = unicast_client_renew(p, master);
			break;
		case UC_NEED_SYDY:
			err = unicast_client_sydy(p, master);
			break;
		case UC_HAVE_SYDY:
			err = unicast_client_renew(p, master);
			break;
		}
		if (p->delayMechanism == DM_P2P) {
			unicast_client_peer_renew(p);
		}
	}

	unicast_client_set_tmo(p);
	return err;
}

