/**
 * @file port_private.h
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
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef HAVE_PORT_PRIVATE_H
#define HAVE_PORT_PRIVATE_H

#include <sys/queue.h>

#include "clock.h"
#include "ptpProtocol.h"
#include "tmv.h"

#define NSEC2SEC 1000000000LL

enum SyncFollowupState {
	SF_EMPTY,
	SF_HAVE_SYNC,
	SF_HAVE_FUP,
};

enum link_state {
	LINK_DOWN  = (1<<0),
	LINK_UP  = (1<<1),
	LINK_STATE_CHANGED = (1<<3),
	TS_LABEL_CHANGED  = (1<<4),
};

struct nrate_estimator {
	double ratio;
	tmv_t origin1;
	tmv_t ingress1;
	unsigned int max_count;
	unsigned int count;
	int ratio_valid;
};

struct tc_txd
{
	TAILQ_ENTRY(tc_txd)		list;
	struct ptp_message		*msg;
	
	tmv_t					residence;
	int						ingress_port;
};

struct PtpPort
{
	char						*name;
	
	struct PtpInterface		*iface;
	struct PtpClock			*clock;
	struct transport			*trp;
	
	enum timestamp_type		timestamping;
	struct FdArray			fda;

	int						fault_fd;		/* fd of CLOCK_MONOLITHIC, index is N_POLLFD*/
	int						phc_index;	/* index of /dev/ptp; when readtime clock is used, it is -1 */

	void					(*cbDispatch)(struct PtpPort *p, enum PORT_EVENT event, int mdiff);
	/* get event from fd polled */
	enum PORT_EVENT	(*cbEvent)(struct PtpPort *p, int fd_index);

	int							jbod;
	struct foreign_clock			*best;
	enum SyncFollowupState		syfu;
	struct ptp_message			*last_syncfup;

	/* peer delay messages, only used in transparent clock  */
	struct ptp_message			*peer_delay_req;
	struct ptp_message			*peer_delay_resp;
	struct ptp_message			*peer_delay_fup;
	int							peer_portid_valid;
	struct PortIdentity				peer_portid;
	
	struct
	{
		UInteger16		announce;
		UInteger16		delayreq;
		UInteger16		signaling;
		UInteger16		sync;
	} seqnum;
	
	tmv_t	peer_delay;


	struct TimestampProcess		*tsproc;
	int							log_sync_interval;
	struct nrate_estimator nrate;
	unsigned int pdr_missing;
	unsigned int multiple_seq_pdr_count;
	unsigned int multiple_pdr_detected;
	
	enum PORT_STATE (*state_machine)(enum PORT_STATE state, enum PORT_EVENT event, int mdiff);

	/* portDS */
	struct PortIdentity				portIdentity;
	enum PORT_STATE			state; /*portState*/
	Integer64           asymmetry;
	int                 asCapable;
	Integer8            logMinDelayReqInterval;
	TimeInterval        peerMeanPathDelay;
	Integer8            logAnnounceInterval;
	UInteger8           announceReceiptTimeout;
	int                 announce_span;
	UInteger8           syncReceiptTimeout;
	UInteger8           transportSpecific;
	UInteger8           localPriority;
	Integer8            logSyncInterval;
	Enumeration8        delayMechanism;
	Integer8            logMinPdelayReqInterval;
	UInteger32          neighborPropDelayThresh;
	int                 follow_up_info;
	int                 freq_est_interval;
	int                 hybrid_e2e;
	int                 master_only;
	int                 match_transport_specific;
	int                 min_neighbor_prop_delay;
	int                 net_sync_monitor;
	int                 path_trace_enabled;
	int                 tc_spanning_tree;
	Integer64           rx_timestamp_offset;
	Integer64           tx_timestamp_offset;
	int                 unicast_req_duration;
	
	enum link_state			link_status;
	struct fault_interval flt_interval_pertype[FT_CNT];
	enum fault_type     last_fault_type;
	unsigned int        versionNumber; /*UInteger4*/
	
	/* unicast client mode */
	struct unicast_master_table			*unicast_master_table;
	/* unicast service mode */
	struct unicast_service *unicast_service;
	int inhibit_multicast_service;
	
	LIST_ENTRY(PtpPort)					list;
	TAILQ_HEAD(delay_req, ptp_message)	delay_req;
	
	/* foreignMasterDS */
	LIST_HEAD(fm, foreign_clock)			foreign_masters;
	
	/* TC book keeping */
	TAILQ_HEAD(tct, tc_txd)				tc_transmitted;

};

#define	PORT_NUMBER(p)				(p->portIdentity.portNumber)
#define	PORT_NAME(p)				(p->name)

void e2e_dispatch(struct PtpPort *p, enum PORT_EVENT event, int mdiff);
enum PORT_EVENT e2e_event(struct PtpPort *p, int fd_index);

void p2p_dispatch(struct PtpPort *p, enum PORT_EVENT event, int mdiff);
enum PORT_EVENT p2p_event(struct PtpPort *p, int fd_index);

void bc_dispatch(struct PtpPort *p, enum PORT_EVENT event, int mdiff);
enum PORT_EVENT bc_event(struct PtpPort *p, int fd_index);

int clear_fault_asap(struct fault_interval *faint);
void delay_req_prune(struct PtpPort *p);
void fc_clear(struct foreign_clock *fc);
void flush_delay_req(struct PtpPort *p);
void flush_last_sync(struct PtpPort *p);
int port_clr_tmo(int fd);
int port_delay_request(struct PtpPort *p);
void portOpDisable(struct PtpPort *p);
int port_initialize(struct PtpPort *p);
int portIsEnabled(struct PtpPort *p);
void port_link_status(void *ctx, int index, int linkup);
int port_set_announce_tmo(struct PtpPort *p);
int port_set_delay_tmo(struct PtpPort *p);
int port_set_qualification_tmo(struct PtpPort *p);
struct ptp_message *port_signaling_construct(struct PtpPort *p,
					     struct address *address,
					     struct PortIdentity *tpid);
int port_tx_announce(struct PtpPort *p, struct address *dst);
int port_tx_sync(struct PtpPort *p, struct address *dst);
int portMsgProAnnounce(struct PtpPort *p, struct ptp_message *m);
int portMsgProDelayReq(struct PtpPort *p, struct ptp_message *m);
void portMsgProDelayResp(struct PtpPort *p, struct ptp_message *m);
void portMsgProFollowUp(struct PtpPort *p, struct ptp_message *m);
int portMsgProPDelayReq(struct PtpPort *p, struct ptp_message *m);
int portMsgProPDelayResp(struct PtpPort *p, struct ptp_message *m);
void portMsgProPDelayRespFollowUp(struct PtpPort *p, struct ptp_message *m);
int portMsgProSignaling(struct PtpPort *p, struct ptp_message *m);
void portMsgProSync(struct PtpPort *p, struct ptp_message *m);
void ts_add(tmv_t *ts, Integer64 correction);



#define source_pid_eq(m1, m2)	\
		(pid_eq(&(m1)->header.sourcePortIdentity, &(m2)->header.sourcePortIdentity))

#define	PORT_LAST_FAULT_TYPE(port)	\
		((port)->last_fault_type)


#define	PORT_IDENTITY(port)	\
		((port)->portIdentity)

#define	 PORT_STATE(port) \
		((port)->state)

#define	PORT_IS_IEEE8021_AS(port)	\
		((port)->follow_up_info ? 1 : 0)

/**
 * Returns the dataset from a port's best foreign clock record, if any
 * has yet been discovered. This function does not bring the returned
 * dataset up to date, so the caller should invoke port_compute_best()
 * beforehand.
 */
#define	PORT_BEST_FOREIGN_DS(port)	\
		((port)->best ? &(port)->best->dataset : NULL)


#define	PORT_LINK_STATUS(port)	\
		(!!((port)->link_status & LINK_UP))


#define	 PORT_DISPATCH(port, event, masterDiff) \
		((port)->cbDispatch( (port), (event), (masterDiff)))

#endif

