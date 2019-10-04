
#include <errno.h>
#include <linux/net_tstamp.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/queue.h>

#include "clock.h"
#include "missing.h"	/* must added this, otherwise LIST_FOREACH_SAFE will fail */

#include "clockPrivate.h"

void remove_subscriber(struct clock_subscriber *s)
{
	LIST_REMOVE(s, list);
	free(s);
}

void clock_update_subscription(struct PtpClock *c, struct ptp_message *req, uint8_t *bitmask, uint16_t duration)
{
	struct clock_subscriber *s;
	int i, remove = 1;
	struct timespec now;

	for (i = 0; i < EVENT_BITMASK_CNT; i++)
	{
		if (bitmask[i])
		{
			remove = 0;
			break;
		}
	}

	LIST_FOREACH(s, &c->subscribers, list)
	{
		if (pid_eq(&s->targetPortIdentity, &req->header.sourcePortIdentity))
		{
			/* Found, update the transport address and event
			 * mask. */
			if (!remove)
			{
				s->addr = req->address;
				memcpy(s->events, bitmask, EVENT_BITMASK_CNT);
				PTP_GET_SYS_TIME_MONOTONIC(&now);
				s->expiration = now.tv_sec + duration;
			}
			else
			{
				remove_subscriber(s);
			}
			return;
		}
	}
	
	if (remove)
		return;

	/* Not present yet, add the subscriber. */
	s = malloc(sizeof(*s));
	if (!s)
	{
		pr_err("failed to allocate memory for a subscriber");
		return;
	}
	
	s->targetPortIdentity = req->header.sourcePortIdentity;
	s->addr = req->address;
	
	memcpy(s->events, bitmask, EVENT_BITMASK_CNT);
	PTP_GET_SYS_TIME_MONOTONIC( &now);
	s->expiration = now.tv_sec + duration;
	s->sequenceId = 0;
	
	LIST_INSERT_HEAD(&c->subscribers, s, list);
}


void clock_get_subscription(struct PtpClock *c, struct ptp_message *req, uint8_t *bitmask, uint16_t *duration)
{
	struct clock_subscriber *s;
	struct timespec now;

	LIST_FOREACH(s, &c->subscribers, list)
	{
		if (pid_eq(&s->targetPortIdentity, &req->header.sourcePortIdentity))
		{
			memcpy(bitmask, s->events, EVENT_BITMASK_CNT);
			PTP_GET_SYS_TIME_MONOTONIC( &now);
			if (s->expiration < now.tv_sec)
				*duration = 0;
			else
				*duration = s->expiration - now.tv_sec;
			
			return;
		}
	}
	/* A client without entry means the client has no subscriptions. */
	memset(bitmask, 0, EVENT_BITMASK_CNT);
	*duration = 0;
}

void clock_flush_subscriptions(struct PtpClock *c)
{
	struct clock_subscriber *s, *tmp;

	LIST_FOREACH_SAFE(s, &c->subscribers, list, tmp) {
		remove_subscriber(s);
	}
}

void clock_prune_subscriptions(struct PtpClock *c)
{
	struct clock_subscriber *s, *tmp;
	struct timespec now;

	PTP_GET_SYS_TIME_MONOTONIC(&now);
	
	LIST_FOREACH_SAFE(s, &c->subscribers, list, tmp)
	{
		if (s->expiration <= now.tv_sec)
		{
			pr_info("subscriber %s timed out", pid2str(&s->targetPortIdentity));
			remove_subscriber(s);
		}
	}
}

