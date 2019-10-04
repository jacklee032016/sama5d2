#include <stdlib.h>
#include <string.h>

#include "port.h"
#include "portPrivate.h"
#include "missing.h"
#include "unicast_client.h"
#include "unicast_service.h"

#include "clockPrivate.h"

static struct ptp_message *_managementConstruct(struct PortIdentity pid, struct PtpPort *ingress,
			  UInteger16 sequenceId,
			  struct PortIdentity *targetPortIdentity,
			  UInteger8 boundaryHops, uint8_t action)
{
	struct ptp_message *msg;

	msg = msg_allocate();
	if (!msg)
		return NULL;

	msg->hwts.type = ingress->timestamping;

	msg->header.tsmt               = MANAGEMENT | ingress->transportSpecific;
	msg->header.ver                = PTP_VERSION;
	msg->header.messageLength      = sizeof(struct management_msg);
	msg->header.domainNumber       = clock_domain_number(ingress->clock);
	msg->header.sourcePortIdentity = pid;
	msg->header.sequenceId         = sequenceId;
	msg->header.control            = CTL_MANAGEMENT;
	msg->header.logMessageInterval = 0x7f;

	if (targetPortIdentity)
		msg->management.targetPortIdentity = *targetPortIdentity;
	msg->management.startingBoundaryHops = boundaryHops;
	msg->management.boundaryHops = boundaryHops;

	switch (action) {
	case GET: case SET:
		msg->management.flags = RESPONSE;
		break;
	case COMMAND:
		msg->management.flags = ACKNOWLEDGE;
		break;
	}
	return msg;
}

struct ptp_message *port_management_reply(struct PortIdentity pid,
					  struct PtpPort *ingress,
					  struct ptp_message *req)
{
	UInteger8 boundaryHops;

	boundaryHops = req->management.startingBoundaryHops -
		       req->management.boundaryHops;
	return _managementConstruct(pid, ingress,
					 req->header.sequenceId,
					 &req->header.sourcePortIdentity,
					 boundaryHops,
					 management_action(req));
}

struct ptp_message *port_management_notify(struct PortIdentity pid, struct PtpPort *port)
{
	return _managementConstruct(pid, port, 0, NULL, 1, GET);
}


/* signal message, related with unicast client/service */
struct ptp_message *port_signaling_construct(struct PtpPort *p,
					     struct address *address,
					     struct PortIdentity *tpid)
{
	struct ptp_message *msg;

	msg = msg_allocate();
	if (!msg) {
		return NULL;
	}
	msg->hwts.type                 = p->timestamping;
	msg->header.tsmt               = SIGNALING | p->transportSpecific;
	msg->header.ver                = PTP_VERSION;
	msg->header.messageLength      = sizeof(struct signaling_msg);
	msg->header.domainNumber       = clock_domain_number(p->clock);
	msg->header.sourcePortIdentity = p->portIdentity;
	msg->header.sequenceId         = p->seqnum.signaling++;
	msg->header.control            = CTL_OTHER;
	msg->header.logMessageInterval = 0x7F;
	msg->signaling.targetPortIdentity = *tpid;
	msg->header.flagField[0] |= UNICAST;
	msg->address = *address;

	return msg;
}

int portMsgProSignaling(struct PtpPort *p, struct ptp_message *m)
{
	struct tlv_extra *extra;
	int err = 0, result;

	switch (p->state) {
	case PS_INITIALIZING:
	case PS_FAULTY:
	case PS_DISABLED:
		return 0;
	case PS_LISTENING:
	case PS_PRE_MASTER:
	case PS_MASTER:
	case PS_GRAND_MASTER:
	case PS_PASSIVE:
	case PS_UNCALIBRATED:
	case PS_SLAVE:
		break;
	}

	TAILQ_FOREACH(extra, &m->tlv_list, list) {
		switch (extra->tlv->type) {
		case TLV_REQUEST_UNICAST_TRANSMISSION:
			result = unicast_service_add(p, m, extra);
			switch (result) {
			case SERVICE_GRANTED:
				err = unicast_service_grant(p, m, extra);
				break;
			case SERVICE_DENIED:
				err = unicast_service_deny(p, m, extra);
				break;
			case SERVICE_DISABLED:
			default:
				break;
			}
			break;

		case TLV_GRANT_UNICAST_TRANSMISSION:
			unicast_client_grant(p, m, extra);
			break;

		case TLV_CANCEL_UNICAST_TRANSMISSION:
			err = unicast_client_cancel(p, m, extra);
			unicast_service_remove(p, m, extra);
			break;

		case TLV_ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION:
			break;
		}
	}
	return err;
}

