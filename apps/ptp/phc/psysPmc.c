
#include "phc2sys.h"

static int clock_compute_state(struct PhcNode *node, struct PhcClock *clock)
{
	struct PhcPort *p;
	int state = PS_DISABLED;

	LIST_FOREACH(p, &node->ports, list) {
		if (p->clock != clock)
			continue;
		
		/* PS_SLAVE takes the highest precedence, PS_UNCALIBRATED
		 * after that, PS_MASTER is third, PS_PRE_MASTER fourth and
		 * all of that overrides PS_DISABLED, which corresponds
		 * nicely with the numerical values */
		if (p->state > state)
			state = p->state;
	}
	
	return state;
}


static int __isMsgMgt(struct ptp_message *msg)
{
	struct TLV *tlv;

	if (msg_type(msg) != MANAGEMENT)
		return 0;
	if (management_action(msg) != RESPONSE)
		return 0;
	if (msg_tlv_count(msg) != 1)
		return 0;
	tlv = (struct TLV *) msg->management.suffix;
	if (tlv->type == TLV_MANAGEMENT)
		return 1;
	if (tlv->type == TLV_MANAGEMENT_ERROR_STATUS)
		return -1;
	return 0;
}

static int __getMgtId(struct ptp_message *msg)
{
	struct management_tlv *mgt = (struct management_tlv *) msg->management.suffix;
	return mgt->id;
}

static void *__getMgtData(struct ptp_message *msg)
{
	struct management_tlv *mgt = (struct management_tlv *) msg->management.suffix;
	return mgt->data;
}

static int __getMgtErrId(struct ptp_message *msg)
{
	struct management_error_status *mgt;

	mgt = (struct management_error_status *)msg->management.suffix;
	return mgt->id;
}


static int __pmcRecvSubscribed(struct PhcNode *node, struct ptp_message *msg, int excluded)
{
	int mgt_id, state;
	struct portDS *pds;
	struct PhcPort *port;
	struct PhcClock *clock;

	mgt_id = __getMgtId(msg);
	if (mgt_id == excluded)
		return 0;
	
	switch (mgt_id)
	{
		case TLV_PORT_DATA_SET:
			
			pds = __getMgtData(msg);
			port = psysPortGet(node, pds->portIdentity.portNumber);

			if (!port)
			{
				pr_info("received data for unknown port %s", pid2str(&pds->portIdentity));
				return 1;
			}
			
			state = psysNormalizeState(pds->portState);
			if (port->state != state)
			{
				pr_info("port %s changed state", pid2str(&pds->portIdentity));
				port->state = state;
				clock = port->clock;
				
				state = clock_compute_state(node, clock);

				if (clock->state != state || clock->new_state)
				{
					clock->new_state = state;
					node->state_changed = 1;
				}
			}
			
			return 1;
	}
	
	return 0;
}

static void __pmcSendSubscription(struct PhcNode *node)
{
	struct subscribe_events_np sen;

	memset(&sen, 0, sizeof(sen));
	sen.duration = PMC_SUBSCRIBE_DURATION;
	sen.bitmask[0] = 1 << NOTIFY_PORT_STATE;
	
	pmc_send_set_action(node->pmc, TLV_SUBSCRIBE_EVENTS_NP, &sen, sizeof(sen));
}

static int __check_clock_identity(struct PhcNode *node, struct ptp_message *msg)
{
	if (!node->clock_identity_set)
		return 1;
	return cid_eq(&node->clock_identity, &msg->header.sourcePortIdentity.clockIdentity);
}


/* Return values:
 * 1: success
 * 0: timeout
 * -1: error reported by the other side
 * -2: local error, fatal
 */
static int _psysPmcRun(struct PhcNode *node, int timeout, int datasetId, struct ptp_message **msg)
{
#define N_FD 1
	struct pollfd pollfd[N_FD];
	int cnt, res;

	EXT_DEBUGF(PHC_SYS_DEBUG, "send dataset 0x%x...", datasetId);
	
	while (1)
	{
		pollfd[0].fd = pmc_get_transport_fd(node->pmc);
		pollfd[0].events = POLLIN|POLLPRI;
		
		if (!node->pmc_ds_requested && datasetId >= 0)
			pollfd[0].events |= POLLOUT;

		cnt = poll(pollfd, N_FD, timeout);
		if (cnt < 0)
		{
			pr_err("poll failed");
			return -2;
		}
		
		if (!cnt) {
			/* Request the data set again in the next run. */
			node->pmc_ds_requested = 0;
			return 0;
		}

		/* Send a new request if there are no pending messages. */
		if ((pollfd[0].revents & POLLOUT) && !(pollfd[0].revents & (POLLIN|POLLPRI)))
		{
			switch (datasetId)
			{
				case TLV_SUBSCRIBE_EVENTS_NP:
					__pmcSendSubscription(node);
					break;
					
				default:
					/* GET action for other ds ID */
					pmc_send_get_action(node->pmc, datasetId);
					break;
			}
			node->pmc_ds_requested = 1;
		}

		if (!(pollfd[0].revents & (POLLIN|POLLPRI)))
			continue;

		*msg = pmc_recv(node->pmc);

		if (!*msg)
			continue;

		if (!__check_clock_identity(node, *msg))
		{
			msg_put(*msg);
			*msg = NULL;
			continue;
		}

		res = __isMsgMgt(*msg);
		
		if (res < 0 && __getMgtErrId(*msg) == datasetId)
		{
			node->pmc_ds_requested = 0;
			return -1;
		}
		
		if ( (res <= 0) || __pmcRecvSubscribed(node, *msg, datasetId) || (__getMgtId(*msg) != datasetId) )
		{
			msg_put(*msg);
			*msg = NULL;
			continue;
		}
		
		node->pmc_ds_requested = 0;
		return 1;
	}
	
}

static int _psysPmcGetNumberPorts(struct PhcNode *node, int timeout)
{
	struct ptp_message *msg;
	int res;
	struct defaultDS *dds;

	PHC_SYS_DBG_DATASET(TLV_DEFAULT_DATA_SET);
	res = _psysPmcRun(node, timeout, TLV_DEFAULT_DATA_SET, &msg);
	if (res <= 0)
		return res;

	dds = (struct defaultDS *)__getMgtData(msg);
	res = dds->numberPorts;
	msg_put(msg);
	return res;
}

static int _psysPmcSubscribe(struct PhcNode *node, int timeout)
{
	struct ptp_message *msg;
	int res;

	PHC_SYS_DBG_DATASET(TLV_SUBSCRIBE_EVENTS_NP);
	res = _psysPmcRun(node, timeout, TLV_SUBSCRIBE_EVENTS_NP, &msg);
	if (res <= 0)
		return res;
	msg_put(msg);
	return 1;
}


/* default ds: get clock_id */
static int _psysPmcGetClockIdentity(struct PhcNode *node, int timeout)
{
	struct ptp_message *msg;
	struct defaultDS *dds;
	int res;

	PHC_SYS_DBG_DATASET(TLV_DEFAULT_DATA_SET);
	res = _psysPmcRun(node, timeout, TLV_DEFAULT_DATA_SET, &msg);
	if (res <= 0)
		return res;

	dds = (struct defaultDS *)__getMgtData(msg);
	
	memcpy(&node->clock_identity, &dds->clockIdentity, sizeof(struct ClockIdentity));
	
	node->clock_identity_set = 1;
	msg_put(msg);
	
	EXT_DEBUGF(PSYS_DEBUG_PMC, "Found Clock ID is '%s'", cid2str(&node->clock_identity));
	return 1;
}


int psysPmcWaitPtpdReady(struct PhcNode *node, int timeout)
{
	struct ptp_message *msg;
	int res;
	void *data;
	Enumeration8 portState;

	while (1)
	{
		PHC_SYS_DBG_DATASET(TLV_PORT_DATA_SET);
		res = _psysPmcRun(node, timeout, TLV_PORT_DATA_SET, &msg);
		if (res <= 0)
			return res;

		data = __getMgtData(msg);
		portState = ((struct portDS *)data)->portState;
		msg_put(msg);

		switch (portState)
		{
			case PS_MASTER:
			case PS_SLAVE:
				return 1;
		}
		
		/* try to get more data sets (for other ports) */
		node->pmc_ds_requested = 1;
	}
}


/* update node->sync_offset, node->leap */
int psysPmcGetUtcOffset(struct PhcNode *node, int timeout)
{
	struct ptp_message *msg;
	struct timePropertiesDS *tds;
	int res;

	PHC_SYS_DBG_DATASET(TLV_TIME_PROPERTIES_DATA_SET);
	res = _psysPmcRun(node, timeout, TLV_TIME_PROPERTIES_DATA_SET, &msg);
	if (res <= 0)
		return res;

	tds = (struct timePropertiesDS *)__getMgtData(msg);

	if (tds->flags & PTP_TIMESCALE)
	{
		node->sync_offset = tds->currentUtcOffset;
		if (tds->flags & LEAP_61)
			node->leap = 1;
		else if (tds->flags & LEAP_59)
			node->leap = -1;
		else
			node->leap = 0;
		
		node->utc_offset_traceable = tds->flags & UTC_OFF_VALID && tds->flags & TIME_TRACEABLE;
	}
	else
	{
		node->sync_offset = 0;
		node->leap = 0;
		node->utc_offset_traceable = 0;
	}
	
	msg_put(msg);
	return 1;
}

/* wait receiving msg from ptp service */
void psysPmcWaitEvents(struct PhcNode *node)
{
	struct ptp_message *msg;

	_psysPmcRun(node, 0, -1, &msg);
}

int psysPmcGetPortProperties(struct PhcNode *node, int timeout, unsigned int port, int *state, int *tstamping, char *iface)
{
	struct ptp_message *msg;
	int res, len;
	struct port_properties_np *ppn;

	pmc_target_port(node->pmc, port);
	while (1)
	{
		PHC_SYS_DBG_DATASET(TLV_PORT_PROPERTIES_NP);
		res = _psysPmcRun(node, timeout, TLV_PORT_PROPERTIES_NP, &msg);
		if (res <= 0)
			goto out;

		ppn = __getMgtData(msg);
		if (ppn->portIdentity.portNumber != port) {
			msg_put(msg);
			continue;
		}

		*state = ppn->port_state;
		*tstamping = ppn->timestamping;
		len = ppn->intfTxt.length;
		if (len > IFNAMSIZ - 1)
			len = IFNAMSIZ - 1;
		memcpy(iface, ppn->intfTxt.text, len);
		iface[len] = '\0';

		msg_put(msg);
		res = 1;
		break;
	}
out:
	pmc_target_all(node->pmc);
	return res;
}


int psysPmcInit(struct PtpConfig *cfg, struct PhcNode *node)
{
	char uds_local[MAX_IFNAME_SIZE + 1];
	snprintf(uds_local, sizeof(uds_local), PHC_SYS_UNIX_SOCKET".%d", getpid());
	
	node->pmc = pmc_create(cfg, TRANS_UDS, uds_local, 0, 
			       config_get_int(cfg, NULL, "domainNumber"),
			       config_get_int(cfg, NULL, "transportSpecific") << 4, 1);
	if (!node->pmc)
	{
		pr_err("failed to create pmc");
		return -1;
	}

	return 0;
}



/* Returns: -1 in case of error, 0 otherwise */
int psysPmcUpdate(struct PhcNode *node, int subscribe)
{
	struct timespec tp;
	uint64_t ts;

	if (PTP_GET_SYS_TIME_MONOTONIC(&tp))
	{
		pr_err("failed to read clock: %m");
		return -1;
	}
	ts = tp.tv_sec * NS_PER_SEC + tp.tv_nsec;

	if (node->pmc && !(ts > node->pmc_last_update && ts - node->pmc_last_update < PMC_UPDATE_INTERVAL))
	{
		if (subscribe)
			_psysPmcSubscribe(node, 0);
		
		if (psysPmcGetUtcOffset(node, 0) > 0)
			node->pmc_last_update = ts;
	}

	return 0;
}


int psysAutoInitPortsByPmc(struct PhcNode *node, int add_rt /*add CLOCK_REALTIME */)
{
	struct PhcPort *port;
	struct PhcClock *clock;
	int number_ports, res;
	unsigned int i;
	int state, timestamping;
	char iface[IFNAMSIZ];

	while (1)
	{
		if (!is_running())
			return -1;
		
		res = _psysPmcGetClockIdentity(node, 1000);
		if (res < 0)
			return -1;
		if (res > 0)
			break;
		/* res == 0, timeout */
		pr_notice("Waiting for ptp4l...");
	}

	number_ports = _psysPmcGetNumberPorts(node, 1000);
	if (number_ports <= 0)
	{
		pr_err("failed to get number of ports");
		return -1;
	}
	EXT_DEBUGF(PHC_SYS_DEBUG, "number of ports=%d", number_ports);
	
	res = _psysPmcSubscribe(node, 1000);
	if (res <= 0)
	{
		pr_err("failed to subscribe");
		return -1;
	}

	for (i = 1; i <= number_ports; i++)
	{
		res = psysPmcGetPortProperties(node, 1000, i, &state, &timestamping, iface);
		if (res == -1) {
			/* port does not exist, ignore the port */
			continue;
		}

		if (res <= 0)
		{
			pr_err("failed to get port properties");
			return -1;
		}
		
#if PSYS_ENABLE_SOFTWATR_TIMESTAMP		
#else
		if (timestamping == TS_SOFTWARE)
		{
			/* ignore ports with software time stamping */
			EXT_INFOF("Software TS, ignored");
			continue;
		}
#endif

		EXT_DEBUGF(PSYS_DEBUG_PMC, "Port#%d: iface: %s; timestamping:%d; state=%s(%d)", i, iface, timestamping, ptpPortStateString(state), state );

		/* add this port and its clock into port list and clock list */
		port = psysPortAdd(node, i, iface);
		if (!port)
			return -1;
		port->state = psysNormalizeState(state);
	}

	if (LIST_EMPTY(&node->clocks))
	{
		pr_err("no suitable ports available");
		return -1;
	}
	
	LIST_FOREACH(clock, &node->clocks, list)
	{
		clock->new_state = clock_compute_state(node, clock);
	}
	node->state_changed = 1;

	if (add_rt)
	{/* add CLOCK_REALTIME to clock list */
		clock = psysClockAdd(node, "CLOCK_REALTIME");
		if (!clock)
			return -1;
		
		if (add_rt == 1)
		{
			clock->dest_only = 1;
		}
	}

	/* get initial offset */
	if (psysPmcGetUtcOffset(node, 1000) <= 0)
	{
		pr_err("failed to get UTC offset");
		return -1;
	}
	return 0;
}

