
#include <errno.h>
#include <linux/net_tstamp.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/queue.h>

#include "clock.h"
#include "clockPrivate.h"
#include "portPrivate.h"

void clock_update_grandmaster(struct PtpClock *c)
{
	struct parentDS *pds = &c->dad.pds;
	memset(&c->cur, 0, sizeof(c->cur));
	memset(c->ptl, 0, sizeof(c->ptl));
	
	pds->parentPortIdentity.clockIdentity   = c->dds.clockIdentity;
	pds->parentPortIdentity.portNumber      = 0;
	pds->grandmasterIdentity                = c->dds.clockIdentity;
	pds->grandmasterClockQuality            = c->dds.clockQuality;
	pds->grandmasterPriority1               = c->dds.priority1;
	pds->grandmasterPriority2               = c->dds.priority2;
	c->dad.path_length                      = 0;
	c->tds.currentUtcOffset                 = c->utc_offset;
	c->tds.flags                            = c->time_flags;
	c->tds.timeSource                       = c->time_source;
}

static void clock_update_slave(struct PtpClock *c)
{
	struct parentDS *pds = &c->dad.pds;
	struct ptp_message *msg        = TAILQ_FIRST(&c->best->messages);
	c->cur.stepsRemoved            = 1 + c->best->dataset.stepsRemoved;
	pds->parentPortIdentity        = c->best->dataset.sender;
	pds->grandmasterIdentity       = msg->announce.grandmasterIdentity;
	pds->grandmasterClockQuality   = msg->announce.grandmasterClockQuality;
	pds->grandmasterPriority1      = msg->announce.grandmasterPriority1;
	pds->grandmasterPriority2      = msg->announce.grandmasterPriority2;
	c->tds.currentUtcOffset        = msg->announce.currentUtcOffset;
	c->tds.flags                   = msg->header.flagField[1];
	c->tds.timeSource              = msg->announce.timeSource;
	
	if (!(c->tds.flags & PTP_TIMESCALE))
	{
		pr_warning("foreign master not using PTP timescale");
	}
	
	if (c->tds.currentUtcOffset < c->utc_offset)
	{
		pr_warning("running in a temporal vortex,  UtcOffset announced: %d, configured: %d", c->tds.currentUtcOffset, c->utc_offset);
	}
	
	if ((c->tds.flags & UTC_OFF_VALID && c->tds.flags & TIME_TRACEABLE) || (c->tds.currentUtcOffset > c->utc_offset))
	{
		pr_info("updating UTC offset to %d", c->tds.currentUtcOffset);
		c->utc_offset = c->tds.currentUtcOffset;
	}
}



static void __fillPollfd(struct pollfd *dest, struct PtpPort *p)
{
	struct FdArray *fda;
	int i;

	fda = &p->fda;
	for (i = 0; i < N_POLLFD; i++)
	{
		dest[i].fd = fda->fd[i];
		dest[i].events = POLLIN|POLLPRI;
	}
	
	dest[i].fd = p->fault_fd;
	dest[i].events = POLLIN|POLLPRI;
}

static void _checkPollfd(struct PtpClock *c)
{
	struct PtpPort *p;
	struct pollfd *dest = c->pollfd;

	if (c->pollfd_valid)
	{
		return;
	}

	/* normal ports */
	LIST_FOREACH(p, &c->clkPorts, list)
	{
		__fillPollfd(dest, p);
		dest += N_CLOCK_PFD;
	}

	/* UDS port */
	__fillPollfd(dest, c->uds_port);
	c->pollfd_valid = 1;
}


static int clock_fault_timeout(struct PtpPort *port, int set)
{
	struct fault_interval i;

	if (!set) {
		pr_debug("clearing fault on port %d", PORT_NUMBER(port));
		return port_set_fault_timer_lin(port, 0);
	}

	fault_interval(port, PORT_LAST_FAULT_TYPE(port), &i);

	if (i.type == FTMO_LINEAR_SECONDS)
	{
		pr_debug("waiting %d seconds to clear fault on port %d", i.val, PORT_NUMBER(port));
		return port_set_fault_timer_lin(port, i.val);
	}
	else if (i.type == FTMO_LOG2_SECONDS)
	{
		pr_debug("waiting 2^{%d} seconds to clear fault on port %d", i.val, PORT_NUMBER(port));
		return port_set_fault_timer_log(port, 1, i.val);
	}

	pr_err("Unsupported fault interval type %d", i.type);
	return -1;
}

static void clock_freq_est_reset(struct PtpClock *c)
{
	c->fest.origin1 = tmv_zero();
	c->fest.ingress1 = tmv_zero();
	c->fest.count = 0;
}

/* use info in ANNOUNCE to select best master clock and update local clock port */
static void handle_state_decision_event(struct PtpClock *c)
{
	struct foreign_clock *best = NULL, *fc;
	struct ClockIdentity best_id;
	struct PtpPort *piter;
	int fresh_best = 0;

	/* iterate foreign clock of every port */
	LIST_FOREACH(piter, &c->clkPorts, list)
	{
		fc = port_compute_best(piter);
		if (!fc)
			continue;
		
		if (!best || c->dscmp(&fc->dataset, &best->dataset) > 0)
			best = fc;
	}

	if (best)
	{
		best_id = best->dataset.identity;
	}
	else
	{
		best_id = c->dds.clockIdentity;
	}

	if (cid_eq(&best_id, &c->dds.clockIdentity))
	{
		pr_notice("selected local clock %s as best master", cid2str(&best_id));
	}
	else
	{
		pr_notice("selected best master clock %s", cid2str(&best_id));
	}

	if (!cid_eq(&best_id, &c->best_id))
	{
		clock_freq_est_reset(c);
		tsproc_reset(c->tsproc, 1);
		if (!tmv_is_zero(c->initial_delay))
			tsproc_set_delay(c->tsproc, c->initial_delay);
		c->ingress_ts = tmv_zero();
		c->path_delay = c->initial_delay;
		c->nrr = 1.0;
		fresh_best = 1;
	}

	c->best = best;
	c->best_id = best_id;

	LIST_FOREACH(piter, &c->clkPorts, list)
	{
		enum PORT_STATE ps;
		enum PORT_EVENT event;

		/* BMC state decision process, for event of STATE_DECISION_EVENT */
		ps = bmc_state_decision(c, piter, c->dscmp);
		
		switch (ps)
		{
			case PS_LISTENING:
				event = EV_NONE;
				break;
			case PS_GRAND_MASTER:
				pr_notice("assuming the grand master role");
				clock_update_grandmaster(c);
				event = EV_RS_GRAND_MASTER;
				break;
			case PS_MASTER:
				event = EV_RS_MASTER;
				break;
			case PS_PASSIVE:
				event = EV_RS_PASSIVE;
				break;
			case PS_SLAVE:
				clock_update_slave(c);
				event = EV_RS_SLAVE;
				break;
			default:
				event = EV_FAULT_DETECTED;
				break;
		}
		
		PORT_DISPATCH(piter, event, fresh_best);
	}
}

int clock_poll(struct PtpClock *c)
{
	int cnt, i;
	enum PORT_EVENT event;
	struct pollfd *cur;
	struct PtpPort *p;

	_checkPollfd(c);
	
	cnt = poll(c->pollfd, (c->nports + 1) * N_CLOCK_PFD, -1);
	if (cnt < 0)
	{
		if (EINTR == errno)
		{
			return 0;
		}
		else
		{
			pr_emerg("poll failed");
			return -1;
		}
	}
	else if (!cnt)
	{
		return 0;
	}

	cur = c->pollfd;

	LIST_FOREACH(p, &c->clkPorts, list)
	{
		/* Let the ports handle their events. */
		for (i = 0; i < N_POLLFD; i++)
		{
			if (cur[i].revents & (POLLIN|POLLPRI))
			{
				event = p->cbEvent(p, i);
				
				if (EV_STATE_DECISION_EVENT == event)
				{/* after ANNOUNCE or MANAGEMENT */
					pr_debug(PORT_STR_FORMAT"detect %s event", PORT_NAME(p), ptpPortEventString(event) );
					c->sde = 1;
				}
				
				if (EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES == event)
				{
					c->sde = 1;
				}
				
				PORT_DISPATCH(p, event, 0);
				
				/* Clear any fault after a little while. */
				if (PS_FAULTY == PORT_STATE(p))
				{
					clock_fault_timeout(p, 1);
					break;
				}
			}
		}

		/* When the fault timer expires we clear the fault, but only if the link is up */
		if (cur[N_POLLFD].revents & (POLLIN|POLLPRI))
		{
			clock_fault_timeout(p, 0);

			if (PORT_LINK_STATUS(p))
			{
				PORT_DISPATCH(p, EV_FAULT_CLEARED, 0);
			}
		}

		cur += N_CLOCK_PFD;
	}

	/* Check the UDS port. */
	for (i = 0; i < N_POLLFD; i++)
	{
		if (cur[i].revents & (POLLIN|POLLPRI))
		{
#if 0		
			if(!p)
			{
			}
			if( !p->cbEvent )
			{
				TRACE();
			}
			event = p->cbEvent(c->uds_port, i);
			if (EV_STATE_DECISION_EVENT == event)
			{
				c->sde = 1;
			}
#else
			event = c->uds_port->cbEvent(c->uds_port, i);
			if (EV_STATE_DECISION_EVENT == event)
			{
				c->sde = 1;
			}
#endif
		}

		/* no dispatch for UDS port, so no FSM for it */
	}

	if (c->sde)
	{
		handle_state_decision_event(c);
		c->sde = 0;
	}
	
	clock_prune_subscriptions(c);
	return 0;
}

