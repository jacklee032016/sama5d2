
#include <errno.h>
#include <linux/net_tstamp.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/queue.h>

#include "clock.h"
#include "missing.h"

#include "clockPrivate.h"
#include "portPrivate.h"

static int __isForwarding(struct PtpClock *c, struct PtpPort *p)
{
	enum PORT_STATE ps = PORT_STATE(p);
	
	switch (ps)
	{
		case PS_MASTER:
		case PS_GRAND_MASTER:
		case PS_SLAVE:
		case PS_UNCALIBRATED:
		case PS_PRE_MASTER:
			return 1;
		default:
			break;
	}
	
	if (p == c->uds_port && ps != PS_FAULTY)
	{
		return 1;
	}
	
	return 0;
}


static int _clockDoForwardMgmt(struct PtpClock *c, struct PtpPort *in, struct PtpPort *out, struct ptp_message *msg, int *pre_sent)
{
	if (in == out || !__isForwarding(c, out))
		return 0;

	/* Don't forward any requests to the UDS port. */
	if (out == c->uds_port) {
		switch (management_action(msg)) {
		case GET:
		case SET:
		case COMMAND:
			return 0;
		}
	}

	if (!*pre_sent) {
		/* delay calling msg_pre_send until actually forwarding */
		msg_pre_send(msg);
		*pre_sent = 1;
	}
	
	return port_forward(out, msg);
}


static void clock_management_send_error(struct PtpPort *p, struct ptp_message *msg, int error_id)
{
	if (port_management_error(PORT_IDENTITY(p), p, msg, error_id))
		pr_err("failed to send management error status");
}


static int clock_management_get_response(struct PtpClock *c, struct PtpPort *p, int id, struct ptp_message *req)
{
	struct PortIdentity pid = PORT_IDENTITY(p);
	struct ptp_message *rsp;
	int respond;

	rsp = port_management_reply(pid, p, req);
	if (!rsp) {
		return 0;
	}
	respond = clock_management_fill_response(c, p, req, rsp, id);
	if (respond)
		port_prepare_and_send(p, rsp, TRANS_GENERAL);
	msg_put(rsp);
	return respond;
}

static int clock_management_set(struct PtpClock *c, struct PtpPort *p, int id, struct ptp_message *req, int *changed)
{
	int respond = 0;
	struct management_tlv *tlv;
	struct management_tlv_datum *mtd;
	struct grandmaster_settings_np *gsn;
	struct subscribe_events_np *sen;

	tlv = (struct management_tlv *) req->management.suffix;

	switch (id) {
		case TLV_PRIORITY1:
			mtd = (struct management_tlv_datum *) tlv->data;
			c->dds.priority1 = mtd->val;
			*changed = 1;
			respond = 1;
			break;
		case TLV_PRIORITY2:
			mtd = (struct management_tlv_datum *) tlv->data;
			c->dds.priority2 = mtd->val;
			*changed = 1;
			respond = 1;
			break;
		case TLV_GRANDMASTER_SETTINGS_NP:
			gsn = (struct grandmaster_settings_np *) tlv->data;
			c->dds.clockQuality = gsn->clockQuality;
			c->utc_offset = gsn->utc_offset;
			c->time_flags = gsn->time_flags;
			c->time_source = gsn->time_source;
			*changed = 1;
			respond = 1;
			break;
		case TLV_SUBSCRIBE_EVENTS_NP:
			sen = (struct subscribe_events_np *)tlv->data;
			clock_update_subscription(c, req, sen->bitmask,
						  sen->duration);
			respond = 1;
			break;
	}
	if (respond && !clock_management_get_response(c, p, id, req))
		pr_err("failed to send management set response");
	return respond ? 1 : 0;
}


static void _clockForwardMgmtMsg(struct PtpClock *c, struct PtpPort *p, struct ptp_message *msg)
{
	struct PtpPort *piter;
	int pdulen = 0, msg_ready = 0;

	if (__isForwarding(c, p) && msg->management.boundaryHops)
	{
		pdulen = msg->header.messageLength;
		msg->management.boundaryHops--;
		
		LIST_FOREACH(piter, &c->clkPorts, list)
		{
			if (_clockDoForwardMgmt(c, p, piter, msg, &msg_ready))
			{
				pr_err("port %d: management forward failed", PORT_NUMBER(piter));
			}
		}
		
		if (_clockDoForwardMgmt(c, p, c->uds_port, msg, &msg_ready))
		{
			pr_err("uds port: management forward failed");
		}

		if (msg_ready)
		{
			ptpMsgReceive(msg, pdulen);
			msg->management.boundaryHops++;
		}
	}
}

/* receive the management message from one port, and and reply */
int clock_manage(struct PtpClock *c, struct PtpPort *p, struct ptp_message *msg)
{
	int changed = 0, res, answers;
	struct PtpPort *piter;
	struct management_tlv *mgt;

	struct ClockIdentity *tcid, wildcard =
	{
		{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
	};

	/* Forward this message out all eligible ports. */
	_clockForwardMgmtMsg(c, p, msg);

	/* Apply this message to the local clock and ports. */
	tcid = &msg->management.targetPortIdentity.clockIdentity;
	if (!cid_eq(tcid, &wildcard) && !cid_eq(tcid, &c->dds.clockIdentity))
	{
		return changed;
	}
	
	if (msg_tlv_count(msg) != 1)
	{
		return changed;
	}
	mgt = (struct management_tlv *) msg->management.suffix;

	/*
	  The correct length according to the management ID is checked
	  in tlv.c, but management TLVs with empty bodies are also
	  received successfully to support GETs and CMDs. At this
	  point the TLV either has the correct length or length 2.
	*/
	switch (management_action(msg))
	{
		case GET:
			if (clock_management_get_response(c, p, mgt->id, msg))
				return changed;
			break;
			
		case SET:
			if (mgt->length == 2 && mgt->id != TLV_NULL_MANAGEMENT)
			{
				clock_management_send_error(p, msg, TLV_WRONG_LENGTH);
				return changed;
			}
			
			if (p != c->uds_port)
			{
				/* Sorry, only allowed on the UDS port. */
				clock_management_send_error(p, msg, TLV_NOT_SUPPORTED);
				return changed;
			}
			
			if (clock_management_set(c, p, mgt->id, msg, &changed))
				return changed;
			break;
			
		case COMMAND:
			break;
			
		default:
			return changed;
	}

	switch (mgt->id)
	{
		case TLV_PORT_PROPERTIES_NP:
			if (p != c->uds_port) {
				/* Only the UDS port allowed. */
				clock_management_send_error(p, msg, TLV_NOT_SUPPORTED);
				return 0;
			}
	}

	switch (mgt->id) {
	case TLV_USER_DESCRIPTION:
	case TLV_SAVE_IN_NON_VOLATILE_STORAGE:
	case TLV_RESET_NON_VOLATILE_STORAGE:
	case TLV_INITIALIZE:
	case TLV_FAULT_LOG:
	case TLV_FAULT_LOG_RESET:
	case TLV_DEFAULT_DATA_SET:
	case TLV_CURRENT_DATA_SET:
	case TLV_PARENT_DATA_SET:
	case TLV_TIME_PROPERTIES_DATA_SET:
	case TLV_PRIORITY1:
	case TLV_PRIORITY2:
	case TLV_DOMAIN:
	case TLV_SLAVE_ONLY:
	case TLV_TIME:
	case TLV_CLOCK_ACCURACY:
	case TLV_UTC_PROPERTIES:
	case TLV_TRACEABILITY_PROPERTIES:
	case TLV_TIMESCALE_PROPERTIES:
	case TLV_PATH_TRACE_LIST:
	case TLV_PATH_TRACE_ENABLE:
	case TLV_GRANDMASTER_CLUSTER_TABLE:
	case TLV_ACCEPTABLE_MASTER_TABLE:
	case TLV_ACCEPTABLE_MASTER_MAX_TABLE_SIZE:
	case TLV_ALTERNATE_TIME_OFFSET_ENABLE:
	case TLV_ALTERNATE_TIME_OFFSET_NAME:
	case TLV_ALTERNATE_TIME_OFFSET_MAX_KEY:
	case TLV_ALTERNATE_TIME_OFFSET_PROPERTIES:
	case TLV_TRANSPARENT_CLOCK_DEFAULT_DATA_SET:
	case TLV_PRIMARY_DOMAIN:
	case TLV_TIME_STATUS_NP:
	case TLV_GRANDMASTER_SETTINGS_NP:
	case TLV_SUBSCRIBE_EVENTS_NP:
		clock_management_send_error(p, msg, TLV_NOT_SUPPORTED);
		break;
		
	default:
		answers = 0;
		LIST_FOREACH(piter, &c->clkPorts, list) {
			res = port_manage(piter, p, msg);
			if (res < 0)
				return changed;
			if (res > 0)
				answers++;
		}
		if (!answers) {
			/* IEEE 1588 Interpretation #21 suggests to use
			 * TLV_WRONG_VALUE for ports that do not exist */
			clock_management_send_error(p, msg, TLV_WRONG_VALUE);
		}
		break;
	}
	
	return changed;
}

