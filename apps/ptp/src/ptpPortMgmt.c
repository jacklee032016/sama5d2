
#include <stdlib.h>
#include <string.h>

#include "port.h"
#include "portPrivate.h"
#include "missing.h"
#include "unicast_client.h"
#include "unicast_service.h"

#include "clockPrivate.h"

static void port_management_send_error(struct PtpPort *p, struct PtpPort *ingress,
				       struct ptp_message *msg, int error_id)
{
	if (port_management_error(p->portIdentity, ingress, msg, error_id))
		pr_err(PORT_STR_FORMAT"management error failed", PORT_NAME(p));
}

/* 0x68, 0x97, 0xE8 is GROUP ID of SMPTE OUI */
static const Octet profile_id_smpte[] = 
		{0x68, 0x97, 0xE8, 0x00, 0x01, 0x00};
#if 0
static const Octet profile_id_drr[] = 
		{0x00, 0x1B, 0x19, 0x00, 0x01, 0x00};	/* default */
static const Octet profile_id_telecom[] = 
		{0x00, 0x19, 0xA7, 0x00, 0x01, 0x00};	/*+- 1.5us */
static const Octet profile_id_power[] = 
		{0x1C, 0x12, 0x9D, 0x00, 0x00, 0x00};	/*+- 1.5us */
#endif

static const Octet profile_id_p2p[] = 
		{0x00, 0x1B, 0x19, 0x00, 0x02, 0x00};	/* default P2P */

static int port_management_fill_response(struct PtpPort *target,
					 struct ptp_message *rsp, int id)
{
	struct mgmt_clock_description *cd;
	struct management_tlv_datum *mtd;
	struct ClockDescription *desc;
	struct port_properties_np *ppn;
	struct management_tlv *tlv;
	struct port_ds_np *pdsnp;
	struct tlv_extra *extra;
	struct portDS *pds;
	uint16_t u16;
	uint8_t *buf;
	int datalen;

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
		case TLV_NULL_MANAGEMENT:
			datalen = 0;
			break;
			
		case TLV_CLOCK_DESCRIPTION:
			cd = &extra->cd;
			buf = tlv->data;
			cd->clockType = (UInteger16 *) buf;
			buf += sizeof(*cd->clockType);
			*cd->clockType = clock_type(target->clock);
			cd->physicalLayerProtocol = (struct PTPText *) buf;
			switch(TransportType(target->trp))
			{
				case TRANS_UDP_IPV4:
				case TRANS_UDP_IPV6:
				case TRANS_IEEE_802_3:
					ptp_text_set(cd->physicalLayerProtocol, "IEEE 802.3");
					break;
				default:
					ptp_text_set(cd->physicalLayerProtocol, NULL);
					break;
			}
			buf += sizeof(struct PTPText) + cd->physicalLayerProtocol->length;

			cd->physicalAddress = (struct PhysicalAddress *) buf;
			u16 = transport_physical_addr(target->trp, cd->physicalAddress->address);
			memcpy(&cd->physicalAddress->length, &u16, 2);
			buf += sizeof(struct PhysicalAddress) + u16;

			cd->protocolAddress = (struct PortAddress *) buf;
			u16 = TransportType(target->trp);
			memcpy(&cd->protocolAddress->networkProtocol, &u16, 2);
			u16 = transport_protocol_addr(target->trp, cd->protocolAddress->address);
			memcpy(&cd->protocolAddress->addressLength, &u16, 2);
			buf += sizeof(struct PortAddress) + u16;

			desc = clock_description(target->clock);
			cd->manufacturerIdentity = buf;
			memcpy(cd->manufacturerIdentity, desc->manufacturerIdentity, OUI_LEN);
			buf += OUI_LEN;
			*(buf++) = 0; /* reserved */

			cd->productDescription = (struct PTPText *) buf;
			ptp_text_copy(cd->productDescription, &desc->productDescription);
			buf += sizeof(struct PTPText) + cd->productDescription->length;

			cd->revisionData = (struct PTPText *) buf;
			ptp_text_copy(cd->revisionData, &desc->revisionData);
			buf += sizeof(struct PTPText) + cd->revisionData->length;

			cd->userDescription = (struct PTPText *) buf;
			ptp_text_copy(cd->userDescription, &desc->userDescription);
			buf += sizeof(struct PTPText) + cd->userDescription->length;

			if (target->delayMechanism == DM_P2P)
			{
				memcpy(buf, profile_id_p2p, PROFILE_ID_LEN);
			}
			else
			{
				memcpy(buf, profile_id_smpte, PROFILE_ID_LEN);
			}
			
			buf += PROFILE_ID_LEN;
			datalen = buf - tlv->data;
			break;
		case TLV_PORT_DATA_SET:
			pds = (struct portDS *) tlv->data;
			pds->portIdentity            = target->portIdentity;
			if (target->state == PS_GRAND_MASTER) {
				pds->portState = PS_MASTER;
			} else {
				pds->portState = target->state;
			}
			pds->logMinDelayReqInterval  = target->logMinDelayReqInterval;
			pds->peerMeanPathDelay       = target->peerMeanPathDelay;
			pds->logAnnounceInterval     = target->logAnnounceInterval;
			pds->announceReceiptTimeout  = target->announceReceiptTimeout;
			pds->logSyncInterval         = target->logSyncInterval;
			if (target->delayMechanism) {
				pds->delayMechanism = target->delayMechanism;
			} else {
				pds->delayMechanism = DM_E2E;
			}
			pds->logMinPdelayReqInterval = target->logMinPdelayReqInterval;
			pds->versionNumber           = target->versionNumber;
			datalen = sizeof(*pds);
			break;
		case TLV_LOG_ANNOUNCE_INTERVAL:
			mtd = (struct management_tlv_datum *) tlv->data;
			mtd->val = target->logAnnounceInterval;
			datalen = sizeof(*mtd);
			break;
		case TLV_ANNOUNCE_RECEIPT_TIMEOUT:
			mtd = (struct management_tlv_datum *) tlv->data;
			mtd->val = target->announceReceiptTimeout;
			datalen = sizeof(*mtd);
			break;
		case TLV_LOG_SYNC_INTERVAL:
			mtd = (struct management_tlv_datum *) tlv->data;
			mtd->val = target->logSyncInterval;
			datalen = sizeof(*mtd);
			break;
		case TLV_VERSION_NUMBER:
			mtd = (struct management_tlv_datum *) tlv->data;
			mtd->val = target->versionNumber;
			datalen = sizeof(*mtd);
			break;
		case TLV_DELAY_MECHANISM:
			mtd = (struct management_tlv_datum *) tlv->data;
			if (target->delayMechanism)
				mtd->val = target->delayMechanism;
			else
				mtd->val = DM_E2E;
			datalen = sizeof(*mtd);
			break;
		case TLV_LOG_MIN_PDELAY_REQ_INTERVAL:
			mtd = (struct management_tlv_datum *) tlv->data;
			mtd->val = target->logMinPdelayReqInterval;
			datalen = sizeof(*mtd);
			break;
		case TLV_PORT_DATA_SET_NP:
			pdsnp = (struct port_ds_np *) tlv->data;
			pdsnp->neighborPropDelayThresh = target->neighborPropDelayThresh;
			pdsnp->asCapable = target->asCapable;
			datalen = sizeof(*pdsnp);
			break;
		case TLV_PORT_PROPERTIES_NP:
			ppn = (struct port_properties_np *)tlv->data;
			ppn->portIdentity = target->portIdentity;
			if (target->state == PS_GRAND_MASTER)
				ppn->port_state = PS_MASTER;
			else
				ppn->port_state = target->state;
			ppn->timestamping = target->timestamping;
			ptp_text_set(&ppn->intfTxt, target->iface->ts_label);
			datalen = sizeof(*ppn) + ppn->intfTxt.length;
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

static int port_management_get_response(struct PtpPort *target,
					struct PtpPort *ingress, int id,
					struct ptp_message *req)
{
	struct PortIdentity pid = PORT_IDENTITY(target);
	struct ptp_message *rsp;
	int respond;

	rsp = port_management_reply(pid, ingress, req);
	if (!rsp) {
		return 0;
	}
	respond = port_management_fill_response(target, rsp, id);
	if (respond)
		port_prepare_and_send(ingress, rsp, TRANS_GENERAL);
	msg_put(rsp);
	return respond;
}

static int port_management_set(struct PtpPort *target,
			       struct PtpPort *ingress, int id,
			       struct ptp_message *req)
{
	int respond = 0;
	struct management_tlv *tlv;
	struct port_ds_np *pdsnp;

	tlv = (struct management_tlv *) req->management.suffix;

	switch (id) {
	case TLV_PORT_DATA_SET_NP:
		pdsnp = (struct port_ds_np *) tlv->data;
		target->neighborPropDelayThresh = pdsnp->neighborPropDelayThresh;
		respond = 1;
		break;
	}
	if (respond && !port_management_get_response(target, ingress, id, req))
		pr_err(PORT_STR_FORMAT"failed to send management set response", PORT_NAME(target));
	return respond ? 1 : 0;
}

/* after receiving management message, call this when clock manage is called */
int port_manage(struct PtpPort *p, struct PtpPort *ingress, struct ptp_message *msg)
{
	struct management_tlv *mgt;
	UInteger16 target = msg->management.targetPortIdentity.portNumber;

	if (target != PORT_NUMBER(p) && target != 0xffff) {
		return 0;
	}
	mgt = (struct management_tlv *) msg->management.suffix;

	switch (management_action(msg)) {
	case GET:
		if (port_management_get_response(p, ingress, mgt->id, msg))
			return 1;
		break;
	case SET:
		if (port_management_set(p, ingress, mgt->id, msg))
			return 1;
		break;
	case COMMAND:
		break;
	default:
		return -1;
	}

	switch (mgt->id) {
	case TLV_NULL_MANAGEMENT:
	case TLV_CLOCK_DESCRIPTION:
	case TLV_PORT_DATA_SET:
	case TLV_LOG_ANNOUNCE_INTERVAL:
	case TLV_ANNOUNCE_RECEIPT_TIMEOUT:
	case TLV_LOG_SYNC_INTERVAL:
	case TLV_VERSION_NUMBER:
	case TLV_ENABLE_PORT:
	case TLV_DISABLE_PORT:
	case TLV_UNICAST_NEGOTIATION_ENABLE:
	case TLV_UNICAST_MASTER_TABLE:
	case TLV_UNICAST_MASTER_MAX_TABLE_SIZE:
	case TLV_ACCEPTABLE_MASTER_TABLE_ENABLED:
	case TLV_ALTERNATE_MASTER:
	case TLV_TRANSPARENT_CLOCK_PORT_DATA_SET:
	case TLV_DELAY_MECHANISM:
	case TLV_LOG_MIN_PDELAY_REQ_INTERVAL:
		port_management_send_error(p, ingress, msg, TLV_NOT_SUPPORTED);
		break;
	default:
		port_management_send_error(p, ingress, msg, TLV_NO_SUCH_ID);
		return -1;
	}
	return 1;
}


void port_notify_event(struct PtpPort *p, enum NOTIFICATION event)
{
	struct PortIdentity pid = PORT_IDENTITY(p);
	struct ptp_message *msg;
	int id;

	switch (event) {
	case NOTIFY_PORT_STATE:
		id = TLV_PORT_DATA_SET;
		break;
	default:
		return;
	}
	/* targetPortIdentity and sequenceId will be filled by
	 * clock_send_notification */
	msg = port_management_notify(pid, p);
	if (!msg)
		return;
	if (!port_management_fill_response(p, msg, id))
		goto err;
	if (msg_pre_send(msg))
		goto err;
	clock_send_notification(p->clock, msg, event);
err:
	msg_put(msg);
}

