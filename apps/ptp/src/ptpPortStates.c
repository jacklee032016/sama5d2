
#include <stdlib.h>
#include <string.h>

#include "port.h"
#include "portPrivate.h"
#include "missing.h"
#include "unicast_client.h"
#include "unicast_service.h"

#include "clockPrivate.h"

#include "ptpProtocol.h"

static void _portStateDebug(struct PtpPort *p, enum PORT_STATE next, enum PORT_EVENT event)
{
	if (event == EV_FAULT_DETECTED)
	{
		pr_notice(PORT_STR_FORMAT"State from %s to %s on event %s (%s)", PORT_NAME(p),
			  ptpPortStateString(p->state), ptpPortStateString(next), ptpPortEventString(event),
			  ft_str(PORT_LAST_FAULT_TYPE(p)));
	}
	else
	{
		pr_notice(PORT_STR_FORMAT"State from %s to %s on event %s", PORT_NAME(p),
			  ptpPortStateString(p->state), ptpPortStateString(next), ptpPortEventString(event) );
	}
}

int portIsEnabled(struct PtpPort *p)
{
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
	return 1;
}

static void _freeForeignMasters(struct PtpPort *p)
{
	struct foreign_clock *fc;
	
	while ((fc = LIST_FIRST(&p->foreign_masters)) != NULL)
	{
		LIST_REMOVE(fc, list);
		fc_clear(fc);
		free(fc);
	}
}


void portOpDisable(struct PtpPort *p)
{
	int i;

	tc_flush(p);
	flush_last_sync(p);
	flush_delay_req(p);
	flush_peer_delay(p);

	p->best = NULL;
	
	_freeForeignMasters(p);
	transport_close(p->trp, &p->fda);

	for (i = 0; i < N_TIMER_FDS; i++) {
		close(p->fda.fd[FD_FIRST_TIMER + i]);
	}

	/* Keep rtnl socket to get link status info. */
	port_clear_fda(p, FD_RTNL);
	clock_fda_changed(p->clock);
}


/* called in dispatch handler, for boundary and transparent clock */
int portStateUpdate(struct PtpPort *p, enum PORT_EVENT event, int mdiff)
{
	enum PORT_STATE next = p->state_machine(p->state, event, mdiff);

	if (PS_FAULTY == next)
	{
		struct fault_interval i;
		fault_interval(p, PORT_LAST_FAULT_TYPE(p), &i);
		
		if (clear_fault_asap(&i))
		{
			pr_notice(PORT_STR_FORMAT"clearing fault immediately", PORT_NAME(p));
			next = p->state_machine(next, EV_FAULT_CLEARED, 0);
		}
	}

	if (PS_INITIALIZING == next) {
		/*
		 * This is a special case. Since we initialize the
		 * port immediately, we can skip right to listening
		 * state if all goes well.
		 */
		if (portIsEnabled(p))
		{
			portOpDisable(p);
		}

		if (port_initialize(p))
		{
			event = EV_FAULT_DETECTED;
		}
		else
		{
			event = EV_INIT_COMPLETE;
		}
		
		next = p->state_machine(next, event, 0);
	}

	if (next != p->state)
	{
		_portStateDebug(p, next, event);
		p->state = next;
		port_notify_event(p, NOTIFY_PORT_STATE);
		unicast_client_state_changed(p);
		return 1;
	}

	return 0;
}

