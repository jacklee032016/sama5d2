
#include <stdlib.h>
#include <string.h>

#include "port.h"
#include "portPrivate.h"
#include "missing.h"
#include "unicast_client.h"
#include "unicast_service.h"

#include "clockPrivate.h"

#include "ptpProtocol.h"

/**
 * Run the state machine for a BC or OC port.
 * @param state  The current state of the port.
 * @param event  The event to be processed.
 * @param mdiff  Whether a new master has been selected.
 * @return       The new state for the port.
 */
/* maste SM ??? */ 
static enum PORT_STATE _ptpFsm(enum PORT_STATE state, enum PORT_EVENT event, int mdiff)
{
	enum PORT_STATE next = state;

	if (EV_INITIALIZE == event || EV_POWERUP == event)
		return PS_INITIALIZING;

	switch (state)
	{
		case PS_INITIALIZING:
			switch (event)
			{
				case EV_FAULT_DETECTED:
					next = PS_FAULTY;
					break;
				case EV_INIT_COMPLETE:
					next = PS_LISTENING;
					break;
				default:
					break;
			}
			break;

		case PS_FAULTY:
			switch (event)
			{
				case EV_DESIGNATED_DISABLED:
					next = PS_DISABLED;
					break;
				case EV_FAULT_CLEARED:
					next = PS_INITIALIZING;
					break;
				default:
					break;
			}
			break;

		case PS_DISABLED:
			if (EV_DESIGNATED_ENABLED == event)
				next = PS_INITIALIZING;
			break;

		case PS_LISTENING:
			switch (event) {
			case EV_DESIGNATED_DISABLED:
				next = PS_DISABLED;
				break;
			case EV_FAULT_DETECTED:
				next = PS_FAULTY;
				break;
			case EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES:
				next = PS_MASTER;
				break;
			case EV_RS_MASTER:
				next = PS_PRE_MASTER;
				break;
			case EV_RS_GRAND_MASTER:
				next = PS_GRAND_MASTER;
				break;
			case EV_RS_SLAVE:
				next = PS_UNCALIBRATED;
				break;
			case EV_RS_PASSIVE:
				next = PS_PASSIVE;
				break;
			default:
				break;
			}
			break;

		case PS_PRE_MASTER:
			switch (event) {
			case EV_DESIGNATED_DISABLED:
				next = PS_DISABLED;
				break;
			case EV_FAULT_DETECTED:
				next = PS_FAULTY;
				break;
			case EV_QUALIFICATION_TIMEOUT_EXPIRES:
				next = PS_MASTER;
				break;
			case EV_RS_SLAVE:
				next = PS_UNCALIBRATED;
				break;
			case EV_RS_PASSIVE:
				next = PS_PASSIVE;
				break;
			default:
				break;
			}
			break;

		case PS_MASTER:
		case PS_GRAND_MASTER:
			switch (event) {
			case EV_DESIGNATED_DISABLED:
				next = PS_DISABLED;
				break;
			case EV_FAULT_DETECTED:
				next = PS_FAULTY;
				break;
			case EV_RS_SLAVE:
				next = PS_UNCALIBRATED;
				break;
			case EV_RS_PASSIVE:
				next = PS_PASSIVE;
				break;
			default:
				break;
			}
			break;

		case PS_PASSIVE:
			switch (event) {
			case EV_DESIGNATED_DISABLED:
				next = PS_DISABLED;
				break;
			case EV_FAULT_DETECTED:
				next = PS_FAULTY;
				break;
			case EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES:
				next = PS_MASTER;
				break;
			case EV_RS_MASTER:
				next = PS_PRE_MASTER;
				break;
			case EV_RS_GRAND_MASTER:
				next = PS_GRAND_MASTER;
				break;
			case EV_RS_SLAVE:
				next = PS_UNCALIBRATED;
				break;
			default:
				break;
			}
			break;

		case PS_UNCALIBRATED:
			switch (event) {
			case EV_DESIGNATED_DISABLED:
				next = PS_DISABLED;
				break;
			case EV_FAULT_DETECTED:
				next = PS_FAULTY;
				break;
			case EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES:
				next = PS_MASTER;
				break;
			case EV_MASTER_CLOCK_SELECTED:
				next = PS_SLAVE;
				break;
			case EV_RS_MASTER:
				next = PS_PRE_MASTER;
				break;
			case EV_RS_GRAND_MASTER:
				next = PS_GRAND_MASTER;
				break;
			case EV_RS_SLAVE:
				next = PS_UNCALIBRATED;
				break;
			case EV_RS_PASSIVE:
				next = PS_PASSIVE;
				break;
			default:
				break;
			}
			break;

		case PS_SLAVE:
			switch (event) {
			case EV_DESIGNATED_DISABLED:
				next = PS_DISABLED;
				break;
			case EV_FAULT_DETECTED:
				next = PS_FAULTY;
				break;
			case EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES:
				next = PS_MASTER;
				break;
			case EV_SYNCHRONIZATION_FAULT:
				next = PS_UNCALIBRATED;
				break;
			case EV_RS_MASTER:
				next = PS_PRE_MASTER;
				break;
			case EV_RS_GRAND_MASTER:
				next = PS_GRAND_MASTER;
				break;
			case EV_RS_SLAVE:
				if (mdiff)
					next = PS_UNCALIBRATED;
				break;
			case EV_RS_PASSIVE:
				next = PS_PASSIVE;
				break;
			default:
				break;
			}
			break;
		}

	return next;
}

/* slave-only SM */
static enum PORT_STATE _ptpSlaveFsm(enum PORT_STATE state, enum PORT_EVENT event, int mdiff)
{
	enum PORT_STATE next = state;

	if (EV_INITIALIZE == event || EV_POWERUP == event)
		return PS_INITIALIZING;

	switch (state) {
	case PS_INITIALIZING:
		switch (event) {
		case EV_FAULT_DETECTED:
			next = PS_FAULTY;
			break;
		case EV_INIT_COMPLETE:
			next = PS_LISTENING;
			break;
		default:
			break;
		}
		break;

	case PS_FAULTY:
		switch (event) {
		case EV_DESIGNATED_DISABLED:
			next = PS_DISABLED;
			break;
		case EV_FAULT_CLEARED:
			next = PS_INITIALIZING;
			break;
		default:
			break;
		}
		break;

	case PS_DISABLED:
		if (EV_DESIGNATED_ENABLED == event)
			next = PS_INITIALIZING;
		break;

	case PS_LISTENING:
		switch (event) {
		case EV_DESIGNATED_DISABLED:
			next = PS_DISABLED;
			break;
		case EV_FAULT_DETECTED:
			next = PS_FAULTY;
			break;
		case EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES:
		case EV_RS_MASTER:
		case EV_RS_GRAND_MASTER:
		case EV_RS_PASSIVE:
			next = PS_LISTENING;
			break;
		case EV_RS_SLAVE:
			next = PS_UNCALIBRATED;
			break;
		default:
			break;
		}
		break;

	case PS_UNCALIBRATED:
		switch (event) {
		case EV_DESIGNATED_DISABLED:
			next = PS_DISABLED;
			break;
		case EV_FAULT_DETECTED:
			next = PS_FAULTY;
			break;
		case EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES:
		case EV_RS_MASTER:
		case EV_RS_GRAND_MASTER:
		case EV_RS_PASSIVE:
			next = PS_LISTENING;
			break;
		case EV_MASTER_CLOCK_SELECTED:
			next = PS_SLAVE;
			break;
		default:
			break;
		}
		break;

	case PS_SLAVE:
		switch (event) {
		case EV_DESIGNATED_DISABLED:
			next = PS_DISABLED;
			break;
		case EV_FAULT_DETECTED:
			next = PS_FAULTY;
			break;
		case EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES:
		case EV_RS_MASTER:
		case EV_RS_GRAND_MASTER:
		case EV_RS_PASSIVE:
			next = PS_LISTENING;
			break;
		case EV_SYNCHRONIZATION_FAULT:
			next = PS_UNCALIBRATED;
			break;
		case EV_RS_SLAVE:
			if (mdiff)
				next = PS_UNCALIBRATED;
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return next;
}

int port_initialize(struct PtpPort *p)
{
	struct PtpConfig *cfg = clock_config(p->clock);
	int fd[N_TIMER_FDS], i;

	p->multiple_seq_pdr_count  = 0;
	p->multiple_pdr_detected   = 0;
	p->last_fault_type         = FT_UNSPECIFIED;
	p->logMinDelayReqInterval  = config_get_int(cfg, p->name, "logMinDelayReqInterval");
	p->peerMeanPathDelay       = 0;
	p->logAnnounceInterval     = config_get_int(cfg, p->name, "logAnnounceInterval");
	p->announceReceiptTimeout  = config_get_int(cfg, p->name, "announceReceiptTimeout");
	p->syncReceiptTimeout      = config_get_int(cfg, p->name, "syncReceiptTimeout");
	p->transportSpecific       = config_get_int(cfg, p->name, "transportSpecific");
	p->transportSpecific     <<= 4;
	p->match_transport_specific = !config_get_int(cfg, p->name, "ignore_transport_specific");
	p->master_only             = config_get_int(cfg, p->name, "masterOnly");
	p->localPriority           = config_get_int(cfg, p->name, "G.8275.portDS.localPriority");
	p->logSyncInterval         = config_get_int(cfg, p->name, "logSyncInterval");
	p->logMinPdelayReqInterval = config_get_int(cfg, p->name, "logMinPdelayReqInterval");
	p->neighborPropDelayThresh = config_get_int(cfg, p->name, "neighborPropDelayThresh");
	p->min_neighbor_prop_delay = config_get_int(cfg, p->name, "min_neighbor_prop_delay");

	for (i = 0; i < N_TIMER_FDS; i++) {
		fd[i] = -1;
	}
	for (i = 0; i < N_TIMER_FDS; i++) {
		fd[i] = timerfd_create(CLOCK_MONOTONIC, 0);
		if (fd[i] < 0) {
			pr_err("timerfd_create: %s", strerror(errno));
			goto no_timers;
		}
	}
	
	if (transport_open(p->trp, p->iface, &p->fda, p->timestamping))
		goto no_tropen;

	for (i = 0; i < N_TIMER_FDS; i++) {
		p->fda.fd[FD_FIRST_TIMER + i] = fd[i];
	}

	if (port_set_announce_tmo(p)) {
		goto no_tmo;
	}
	if (unicast_client_enabled(p) && unicast_client_set_tmo(p)) {
		goto no_tmo;
	}

	/* No need to open rtnl socket on UDS port. */
	if (TransportType(p->trp) != TRANS_UDS)
	{
		if (p->fda.fd[FD_RTNL] == -1)
			p->fda.fd[FD_RTNL] = rtnl_open();
		if (p->fda.fd[FD_RTNL] >= 0)
			rtnl_link_query(p->fda.fd[FD_RTNL], p->iface->name);
	}

	port_nrate_initialize(p);

	clock_fda_changed(p->clock);
	return 0;

no_tmo:
	transport_close(p->trp, &p->fda);
	
no_tropen:
no_timers:
	for (i = 0; i < N_TIMER_FDS; i++) {
		if (fd[i] >= 0)
			close(fd[i]);
	}
	return -1;
}


#define ANNOUNCE_SPAN 1

/* when readtime clock is used, phc_index==-1 */
struct PtpPort *portCreate(int phc_index, enum timestamp_type timestamping, int number, struct PtpInterface *ptpIntf, struct PtpClock *clock)
{
	enum CLOCK_TYPE type = clock_type(clock);
	struct PtpConfig *cfg = clock_config(clock);
	struct PtpPort *p = malloc(sizeof(*p));
	enum transport_type transport;
	int i;

	if (!p)
	{
		return NULL;
	}

	pr_info("Port#%d (PHC#%d, %s) is created..."EXT_NEW_LINE, number,  phc_index, ptpIntf->name);

	memset(p, 0, sizeof(*p));
	TAILQ_INIT(&p->tc_transmitted);

	switch (type)
	{
		case CLOCK_TYPE_ORDINARY:
		case CLOCK_TYPE_BOUNDARY:
			p->cbDispatch = bc_dispatch;
			p->cbEvent = bc_event;
			break;
		case CLOCK_TYPE_P2P:
			p->cbDispatch = p2p_dispatch;
			p->cbEvent = p2p_event;
			break;
		case CLOCK_TYPE_E2E:
			p->cbDispatch = e2e_dispatch;
			p->cbEvent = e2e_event;
			break;
			
		case CLOCK_TYPE_MANAGEMENT:
			goto err_port;
	}

	p->state_machine = clock_slave_only(clock) ? _ptpSlaveFsm : _ptpFsm;
	p->phc_index = phc_index;
	p->jbod = config_get_int(cfg, ptpIntf->name, "boundary_clock_jbod");
	transport = config_get_int(cfg, ptpIntf->name, "network_transport");

	if (transport == TRANS_UDS)
	{
		; /* UDS cannot have a PHC. */
	}
	else if (!ptpIntf->ts_info.valid)
	{
		pr_warning("port %d: get_ts_info not supported", number);
	}
	else if (phc_index >= 0 && phc_index != ptpIntf->ts_info.phc_index)
	{
		if (p->jbod) {
			pr_warning("port %d: just a bunch of devices", number);
			p->phc_index = ptpIntf->ts_info.phc_index;
		} else {
			pr_err("port %d: PHC device mismatch", number);
			pr_err("port %d: /dev/ptp%d requested, ptp%d attached", number, phc_index, ptpIntf->ts_info.phc_index);
			goto err_port;
		}
	}

	p->name = ptpIntf->name;
	p->iface = ptpIntf;
	p->asymmetry = config_get_int(cfg, p->name, "delayAsymmetry");
	p->asymmetry <<= 16;
	p->announce_span = transport == TRANS_UDS ? 0 : ANNOUNCE_SPAN;
	p->follow_up_info = config_get_int(cfg, p->name, "follow_up_info");
	p->freq_est_interval = config_get_int(cfg, p->name, "freq_est_interval");
	p->net_sync_monitor = config_get_int(cfg, p->name, "net_sync_monitor");
	p->path_trace_enabled = config_get_int(cfg, p->name, "path_trace_enabled");
	p->tc_spanning_tree = config_get_int(cfg, p->name, "tc_spanning_tree");
	p->rx_timestamp_offset = config_get_int(cfg, p->name, "ingressLatency");
	p->rx_timestamp_offset <<= 16;
	p->tx_timestamp_offset = config_get_int(cfg, p->name, "egressLatency");
	p->tx_timestamp_offset <<= 16;
	p->link_status = LINK_UP;
	p->clock = clock;
	
	p->trp = transport_create(cfg, transport);
	if (!p->trp)
	{
		goto err_port;
	}
	
	p->timestamping = timestamping;
	p->portIdentity.clockIdentity = clock_identity(clock);
	p->portIdentity.portNumber = number;
	p->state = PS_INITIALIZING;
	p->delayMechanism = config_get_int(cfg, p->name, "delay_mechanism");
	p->versionNumber = PTP_VERSION;

	if (number && unicast_client_claim_table(p)) {
		goto err_port;
	}
	if (unicast_client_enabled(p) && config_set_section_int(cfg, p->name, "hybrid_e2e", 1)) {
		goto err_port;
	}
	if (number && unicast_service_initialize(p)) {
		goto err_port;
	}
	p->hybrid_e2e = config_get_int(cfg, p->name, "hybrid_e2e");

	if (number && type == CLOCK_TYPE_P2P && p->delayMechanism != DM_P2P)
	{
		pr_err("port %d: P2P TC needs P2P ports", number);
		goto err_port;
	}
	
	if (number && type == CLOCK_TYPE_E2E && p->delayMechanism != DM_E2E)
	{
		pr_err("port %d: E2E TC needs E2E ports", number);
		goto err_port;
	}
	
	if (p->hybrid_e2e && p->delayMechanism != DM_E2E)
	{
		pr_warning("port %d: hybrid_e2e only works with E2E", number);
	}
	
	if (p->net_sync_monitor && !p->hybrid_e2e)
	{
		pr_warning("port %d: net_sync_monitor needs hybrid_e2e", number);
	}

	/* Set fault timeouts to a default value */
	for (i = 0; i < FT_CNT; i++) {
		p->flt_interval_pertype[i].type = FTMO_LOG2_SECONDS;
		p->flt_interval_pertype[i].val = 4;
	}
	p->flt_interval_pertype[FT_BAD_PEER_NETWORK].type = FTMO_LINEAR_SECONDS;
	p->flt_interval_pertype[FT_BAD_PEER_NETWORK].val = config_get_int(cfg, p->name, "fault_badpeernet_interval");

	p->flt_interval_pertype[FT_UNSPECIFIED].val = config_get_int(cfg, p->name, "fault_reset_interval");

	p->tsproc = tsproc_create(config_get_int(cfg, p->name, "tsproc_mode"),
				  config_get_int(cfg, p->name, "delay_filter"),
				  config_get_int(cfg, p->name, "delay_filter_length"));
	if (!p->tsproc)
	{
		pr_err("Failed to create time stamp processor");
		goto err_transport;
	}
	p->nrate.ratio = 1.0;

	port_clear_fda(p, N_POLLFD);
	p->fault_fd = -1;

	if (number)
	{
		p->fault_fd = timerfd_create(CLOCK_MONOTONIC, 0);
		if (p->fault_fd < 0) {
			pr_err("timerfd_create failed: %m");
			goto err_tsproc;
		}
	}
	pr_info("Port#%d (PHC#%d, %s) creation ended successfully!!!"EXT_NEW_LINE, number,  phc_index, ptpIntf->name);

	return p;

err_tsproc:
	tsproc_destroy(p->tsproc);
err_transport:
	transport_destroy(p->trp);
err_port:
	free(p);
	return NULL;
}

void portDestory(struct PtpPort *p)
{
	if (portIsEnabled(p))
	{
		portOpDisable(p);
	}

	if (p->fda.fd[FD_RTNL] >= 0)
	{
		rtnl_close(p->fda.fd[FD_RTNL]);
	}

	unicast_service_cleanup(p);
	transport_destroy(p->trp);
	tsproc_destroy(p->tsproc);

	if (p->fault_fd >= 0)
	{
		close(p->fault_fd);
	}
	free(p);
}



