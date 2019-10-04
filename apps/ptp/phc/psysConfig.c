
#include "phc2sys.h"

#include "_phc.c"

static struct servo *_psysServoAdd(struct PhcNode *node, struct PhcClock *clock)
{
	double ppb;
	int max_ppb;
	struct servo *servo;

	clockadj_init(clock->clkid);
	ppb = clockadj_get_freq(clock->clkid);
	/* The reading may silently fail and return 0, reset the frequency to
	   make sure ppb is the actual frequency of the clock. */
	clockadj_set_freq(clock->clkid, ppb);
	
	if (clock->clkid == CLOCK_REALTIME)
	{
		sysclk_set_leap(0);
		max_ppb = sysclk_max_freq();
	}
	else
	{
		max_ppb = phc_max_adj(clock->clkid);
		if (!max_ppb) {
			pr_err("clock is not adjustable");
			return NULL;
		}
	}

	servo = servo_create(phc2sys_config, node->servoType, -ppb, max_ppb, 0);
	if (!servo) {
		pr_err("Failed to create servo");
		return NULL;
	}

	servo_sync_interval(servo, node->phc_interval);

	return servo;
}

static void _psysClockReinit(struct PhcNode *node, struct PhcClock *clock, int new_state)
{
	int phc_index = -1, phc_switched = 0;
	int state, timestamping, ret = -1;
	struct PhcPort *p;
	struct servo *servo;
	struct sk_ts_info ts_info;
	char iface[IFNAMSIZ];
	clockid_t clkid = CLOCK_INVALID;

	LIST_FOREACH(p, &node->ports, list) {
		if (p->clock == clock)
		{
			ret = psysPmcGetPortProperties(node, 1000, p->number, &state, &timestamping, iface);
			if (ret > 0)
				p->state = psysNormalizeState(state);
		}
	}

	if (ret > 0 && timestamping != TS_SOFTWARE) {
		/* Check if device changed */
		if (strcmp(clock->device, iface)) {
			free(clock->device);
			clock->device = strdup(iface);
		}
		/* Check if phc index changed */
		if (!sk_get_ts_info(clock->device, &ts_info) &&
		    clock->phc_index != ts_info.phc_index) {
			clkid = clock_open(clock->device, &phc_index);
			if (clkid == CLOCK_INVALID)
				return;

			phc_close(clock->clkid);
			clock->clkid = clkid;
			clock->phc_index = phc_index;

			servo = _psysServoAdd(node, clock);
			if (servo) {
				servo_destroy(clock->servo);
				clock->servo = servo;
			}

			phc_switched = 1;
		}
	}

	if (new_state == PS_MASTER || phc_switched) {
		servo_reset(clock->servo);
		clock->servo_state = SERVO_UNLOCKED;

		if (clock->offset_stats) {
			stats_reset(clock->offset_stats);
			stats_reset(clock->freq_stats);
			stats_reset(clock->delay_stats);
		}
	}
}

static void _psysClockUpdateStats(struct PhcClock *clock, unsigned int max_count,
			       int64_t offset, double freq, int64_t delay)
{
	struct stats_result offset_stats, freq_stats, delay_stats;

	stats_add_value(clock->offset_stats, offset);
	stats_add_value(clock->freq_stats, freq);
	if (delay >= 0)
		stats_add_value(clock->delay_stats, delay);

	if (stats_get_num_values(clock->offset_stats) < max_count)
		return;

	stats_get_result(clock->offset_stats, &offset_stats);
	stats_get_result(clock->freq_stats, &freq_stats);

	if (!stats_get_result(clock->delay_stats, &delay_stats)) {
		pr_info("%s "
			"rms %4.0f max %4.0f "
			"freq %+6.0f +/- %3.0f "
			"delay %5.0f +/- %3.0f",
			clock->device,
			offset_stats.rms, offset_stats.max_abs,
			freq_stats.mean, freq_stats.stddev,
			delay_stats.mean, delay_stats.stddev);
	} else {
		pr_info("%s "
			"rms %4.0f max %4.0f "
			"freq %+6.0f +/- %3.0f",
			clock->device,
			offset_stats.rms, offset_stats.max_abs,
			freq_stats.mean, freq_stats.stddev);
	}

	stats_reset(clock->offset_stats);
	stats_reset(clock->freq_stats);
	stats_reset(clock->delay_stats);
}


static int64_t __getSyncOffset(struct PhcNode *node, struct PhcClock *dst)
{
	int direction = node->forced_sync_offset;

	if (!direction)
		direction = dst->is_utc - node->master->is_utc;
	return (int64_t)dst->sync_offset * NS_PER_SEC * direction;
}

/* Returns: non-zero to skip clock update */
static int _psysClockHandleLeap(struct PhcNode *node, struct PhcClock *clock, int64_t offset, uint64_t ts)
{
	int clock_leap, node_leap = node->leap;

	clock->sync_offset = node->sync_offset;

	if ((node_leap || clock->leap_set) &&
	    clock->is_utc != node->master->is_utc) {
		/* If the master clock is in UTC, get a time stamp from it, as
		   it is the clock which will include the leap second. */
		if (node->master->is_utc) {
			struct timespec tp;

			if (PTP_GET_SYS_TIME_NOW(node->master->clkid, &tp))
			{
				pr_err("failed to read clock: %m");
				return -1;
			}
			ts = tp.tv_sec * NS_PER_SEC + tp.tv_nsec;
		}

		/* If the clock will be stepped, the time stamp has to be the
		   new time. Ignore possible 1 second error in UTC offset. */
		if (clock->is_utc && clock->servo_state == SERVO_UNLOCKED)
			ts -= offset + __getSyncOffset(node, clock);

		/* Suspend clock updates in the last second before midnight. */
		if (is_utc_ambiguous(ts)) {
			pr_info("clock update suspended due to leap second");
			return 1;
		}

		clock_leap = leap_second_status(ts, clock->leap_set,
						&node_leap,
						&clock->sync_offset);

		if (clock->leap_set != clock_leap) {
			/* Only the system clock can leap. */
			if (clock->clkid == CLOCK_REALTIME &&
			    node->kernel_leap)
				sysclk_set_leap(clock_leap);
			else
				servo_leap(clock->servo, clock_leap);
			clock->leap_set = clock_leap;
		}
	}

	if (node->utc_offset_traceable && clock->utc_offset_set != clock->sync_offset)
	{
		if (clock->clkid == CLOCK_REALTIME)
		{
			sysclk_set_tai_offset(clock->sync_offset);
		}
		clock->utc_offset_set = clock->sync_offset;
	}

	return 0;
}

/* after compared the master, then call this function to update dest clock  */
void psysClockUpdate(struct PhcNode *node, struct PhcClock *clock, int64_t offset, uint64_t ts, int64_t delay)
{
	enum servo_state state;
	double ppb;

	if (_psysClockHandleLeap(node, clock, offset, ts))
		return;

	offset += __getSyncOffset(node, clock);

	if (clock->sanity_check && clockcheck_sample(clock->sanity_check, ts))
		servo_reset(clock->servo);

	ppb = servo_sample(clock->servo, offset, ts, 1.0, &state);
	clock->servo_state = state;

	switch (state)
	{
		case SERVO_UNLOCKED:
			break;
			
		case SERVO_JUMP:
			clockadj_step(clock->clkid, -offset);
			if (clock->sanity_check)
			{
				clockcheck_step(clock->sanity_check, -offset);
			}
			
			/* Fall through. */
		case SERVO_LOCKED:
			clockadj_set_freq(clock->clkid, -ppb);
			if (clock->clkid == CLOCK_REALTIME)
				sysclk_set_sync();
			if (clock->sanity_check)
				clockcheck_set_freq(clock->sanity_check, -ppb);
			break;
	}

	if (clock->offset_stats) {
		_psysClockUpdateStats(clock, node->stats_max_count, offset, ppb, delay);
	}
	else
	{
		if (delay >= 0) {
			pr_info("%s %s offset %9" PRId64 " s%d freq %+7.0f "
				"delay %6" PRId64,
				clock->device, node->master->source_label,
				offset, state, ppb, delay);
		} else {
			pr_info("%s %s offset %9" PRId64 " s%d freq %+7.0f",
				clock->device, node->master->source_label,
				offset, state, ppb);
		}
	}
}

/* create a clock and add into node's clocks list, not dest clock list */
struct PhcClock *psysClockAdd(struct PhcNode *node, char *device)
{
	struct PhcClock *c;
	clockid_t clkid = CLOCK_INVALID;
	int phc_index = -1;

	if (device) {
		clkid = clock_open(device, &phc_index);
		if (clkid == CLOCK_INVALID)
			return NULL;
	}

	c = calloc(1, sizeof(*c));
	if (!c) {
		pr_err("failed to allocate memory for a clock");
		return NULL;
	}
	c->clkid = clkid;
	c->phc_index = phc_index;
	c->servo_state = SERVO_UNLOCKED;
	c->device = device ? strdup(device) : NULL;

	if (c->clkid == CLOCK_REALTIME)
	{
		c->source_label = "sys";
		c->is_utc = 1;
	}
	else
	{
		c->source_label = "phc";
	}

	if (node->stats_max_count > 0)
	{
		c->offset_stats = stats_create();
		c->freq_stats = stats_create();
		c->delay_stats = stats_create();
		if (!c->offset_stats ||
		    !c->freq_stats ||
		    !c->delay_stats) {
			pr_err("failed to create stats");
			return NULL;
		}
	}
	
	if (node->sanity_freq_limit) {
		c->sanity_check = clockcheck_create(node->sanity_freq_limit);
		if (!c->sanity_check) {
			pr_err("failed to create clock check");
			return NULL;
		}
	}

	if (clkid != CLOCK_INVALID)
		c->servo = _psysServoAdd(node, c);

	if (clkid != CLOCK_INVALID && clkid != CLOCK_REALTIME)
	{
		c->sysoff_supported = (SYSOFF_SUPPORTED ==   sysoff_probe(CLOCKID_TO_FD(clkid), node->phc_readings));
	}

	LIST_INSERT_HEAD(&node->clocks, c, list);
	return c;
}



struct PhcPort *psysPortGet(struct PhcNode *node, unsigned int number)
{
	struct PhcPort *p;

	LIST_FOREACH(p, &node->ports, list) {
		if (p->number == number)
			return p;
	}
	return NULL;
}

struct PhcPort *psysPortAdd(struct PhcNode *node, unsigned int number, char *device)
{
	struct PhcPort *p;
	struct PhcClock *c = NULL, *tmp;

	p = psysPortGet(node, number);
	if (p)
		return p;
	
	/* port is a new one, look whether we have the device already on a different port */
	LIST_FOREACH(tmp, &node->clocks, list)
	{
		if (!strcmp(tmp->device, device))
		{
			c = tmp;
			break;
		}
	}
	
	if (!c)
	{
		c = psysClockAdd(node, device);
		if (!c)
			return NULL;
	}
	
	p = malloc(sizeof(*p));
	if (!p) {
		pr_err("failed to allocate memory for a port");
		return NULL;
	}
	p->number = number;
	p->clock = c;
	LIST_INSERT_HEAD(&node->ports, p, list);
	return p;
}

static struct PhcClock *_findDestClock(struct PhcNode *node, int phc_index)
{
	struct PhcClock *c = NULL;
	
	LIST_FOREACH(c, &node->dst_clocks, dst_list)
	{
		if (c->phc_index == phc_index)
		{
			break;
		}
	}
	return c;
}

void psysReconfigure(struct PhcNode *node)
{
	struct PhcClock *c, *rt = NULL, *src = NULL, *last = NULL, *dup = NULL;
	int src_cnt = 0, dst_cnt = 0;

	pr_info("reconfiguring after port state change");
	node->state_changed = 0;

	while (node->dst_clocks.lh_first != NULL)
	{
		LIST_REMOVE(node->dst_clocks.lh_first, dst_list);
	}

	LIST_FOREACH(c, &node->clocks, list)
	{
		if (c->clkid == CLOCK_REALTIME) {
			rt = c;
			continue;
		}

		if (c->new_state)
		{
			_psysClockReinit(node, c, c->new_state);
			c->state = c->new_state;
			c->new_state = 0;
		}

		switch (c->state)
		{
			case PS_FAULTY:
			case PS_DISABLED:
			case PS_LISTENING:
			case PS_PRE_MASTER:
			case PS_MASTER:
			case PS_PASSIVE:
				dup = _findDestClock(node, c->phc_index);
				if (!dup)
				{
					pr_info("selecting %s for synchronization", c->device);
					dst_cnt++;
					
					LIST_INSERT_HEAD(&node->dst_clocks, c, dst_list);
				}
				else
				{
					pr_info("skipping %s: %s has the same clock and is already selected", c->device, dup->device);
				}
				break;
				
			case PS_UNCALIBRATED:
				src_cnt++;
				break;
				
			case PS_SLAVE:
				src = c;
				src_cnt++;
				break;
		}
		
		last = c;
	}
	
	if (dst_cnt > 1 && !src)
	{
		if (!rt || rt->dest_only)
		{
			node->master = last;
			/* Reset to original state in next reconfiguration. */
			node->master->new_state = node->master->state;
			node->master->state = PS_SLAVE;
			if (rt)
				rt->state = PS_SLAVE;
			
			pr_info("no source, selecting %s as the default clock", last->device);
			return;
		}
	}
	
	if (src_cnt > 1)
	{
		pr_info("multiple master clocks available, postponing sync...");
		node->master = NULL;
		return;
	}
	
	if (src_cnt > 0 && !src)
	{
		pr_info("master clock not ready, waiting...");
		node->master = NULL;
		return;
	}
	
	if (!src_cnt && !dst_cnt)
	{
		pr_info("no PHC ready, waiting...");
		node->master = NULL;
		return;
	}
	
	if ((!src_cnt && (!rt || rt->dest_only)) || (!dst_cnt && !rt))
	{
		pr_info("nothing to synchronize");
		node->master = NULL;
		return;
	}
	
	if (!src_cnt)
	{
		src = rt;
		rt->state = PS_SLAVE;
	}
	else if (rt)
	{
		if (rt->state != PS_MASTER)
		{
			rt->state = PS_MASTER;
			_psysClockReinit(node, rt, rt->state);
		}
		
		LIST_INSERT_HEAD(&node->dst_clocks, rt, dst_list);
		pr_info("selecting %s for synchronization", rt->device);
	}
	
	node->master = src;
	pr_info("selecting %s as the master clock", src->device);
}


