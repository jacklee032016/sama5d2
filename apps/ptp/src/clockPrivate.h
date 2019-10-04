#ifndef	__CLOCK_PRIVATE_H__
#define	__CLOCK_PRIVATE_H__

#define N_CLOCK_PFD		(N_POLLFD + 1) /* one extra per port, for the fault timer */
#define POW2_41			((double)(1ULL << 41))


struct ClockPort {
	LIST_ENTRY(PtpPort) list;	/* duplex link list, replace 'struct PtpPort *list ' */
};

struct freq_estimator
{
	tmv_t		origin1;
	tmv_t		ingress1;
	
	unsigned int	max_count;
	unsigned int	count;
};

struct clock_stats
{
	struct stats *offset;
	struct stats *freq;
	struct stats *delay;
	unsigned int max_count;
};

/* only one subscription for one target, for all messages of this target */
struct clock_subscriber
{
	LIST_ENTRY(clock_subscriber) list;

	uint8_t				events[EVENT_BITMASK_CNT];

	struct PortIdentity		targetPortIdentity;
	struct address		addr;
	UInteger16			sequenceId;
	time_t				expiration;
};

/* opaque type foe other modules */
struct PtpClock
{
	enum CLOCK_TYPE type;
	struct PtpConfig *config;
	clockid_t clkid;
	
	struct servo *servo;
	enum servo_type servo_type;
	
	int (*dscmp)(struct dataset *a, struct dataset *b);

	struct defaultDS dds;
	struct dataset default_dataset;
	struct currentDS cur;
	struct parent_ds dad;

	struct timePropertiesDS tds;

	struct ClockIdentity ptl[PATH_TRACE_MAX];
	struct foreign_clock *best;
	struct ClockIdentity best_id;

	LIST_HEAD(ports_head, ClockPort)	clkPorts;	/* ports_head is name of new struct, port is `struct port`; ports is field name; UDS port is not in this list */
	struct PtpPort					*uds_port;
	
	struct pollfd *pollfd;
	int pollfd_valid;
	
	int								nports; /* does not include the UDS PtpPort */
	
	int last_port_number;
	int sde;		/* is State Decision Event */
	int free_running;
	int freq_est_interval;
	int grand_master_capable; /* for 802.1AS only */
	int utc_timescale;
	int utc_offset_set;
	int leap_set;
	int kernel_leap;
	int utc_offset;
	int time_flags;  /* grand master role */
	int time_source; /* grand master role */

	enum servo_state servo_state;
	enum timestamp_type timestamping;

	tmv_t master_offset;
	tmv_t	path_delay;	/* delay from t1,t2,t3 and t4*/
	tmv_t ingress_ts;
	tmv_t initial_delay;

	struct TimestampProcess *tsproc;		/* timestamp process options */
	struct freq_estimator fest;
	struct time_status_np status;

	double master_local_rr; /* maintained when free_running */
	double nrr;

	struct ClockDescription desc;
	struct clock_stats stats;
	int stats_interval;

	struct clockcheck *sanity_check;
	struct PtpInterface uds_interface;

	LIST_HEAD(clock_subscribers_head, clock_subscriber) subscribers;
};


#if 0
/**
 * Obtain the clockClass attribute from a clock.
 * @param c  The clock instance.
 * @return   The value of the clock's class.
 */
UInteger8 clock_class(struct PtpClock *c);
UInteger8 clock_class(struct PtpClock *c)
{
	return c->dds.clockQuality.clockClass;
}

/**
 * Obtains a reference to the configuration database.
 * @param c  The clock instance.
 * @return   A pointer to the configuration, without fail.
 */
struct PtpConfig *clock_config(struct PtpClock *c);
struct PtpConfig *clock_config(struct PtpClock *c)
{
	return c->config;
}


/**
 * Obtains a reference to the current dataset.
 * @param c  The clock instance.
 * @return   A pointer to the current dataset, without fail.
 */
struct currentDS *clock_current_dataset(struct PtpClock *c);
struct currentDS *clock_current_dataset(struct PtpClock *c)
{
	return &c->cur;
}


/**
 * Obtains a reference to the best foreign master of a clock.
 * @param c  The clock instance.
 * @return   A pointer to the data set of the foreign master,
 *           or NULL if none has been yet discovered.
 */
struct dataset *clock_best_foreign(struct PtpClock *c);
struct dataset *clock_best_foreign(struct PtpClock *c)
{
	return c->best ? &c->best->dataset : NULL;
}

/**
 * Obtains a reference to the port with the best foreign master.
 * @param c  The clock instance.
 * @return   A pointer to the port with the best foreign master,
 *           or NULL if none has been yet discovered.
 */
struct PtpPort *clock_best_port(struct PtpClock *c);
struct PtpPort *clock_best_port(struct PtpClock *c)
{
	return c->best ? c->best->port : NULL;
}



/**
 * Obtain the domain number from a clock's default data set.
 * @param c  The clock instance.
 * @return   The PTP domain number.
 */
UInteger8 clock_domain_number(struct PtpClock *c);
UInteger8 clock_domain_number(struct PtpClock *c)
{
	return c->dds.domainNumber;
}

/**
 * Obtains a reference to the first port in the clock's list.
 * @param c  The clock instance.
 * @return   A pointer to a port, or NULL if no ports are present.
 */
struct PtpPort *clock_first_port(struct PtpClock *c);
struct PtpPort *clock_first_port(struct PtpClock *c)
{
	return LIST_FIRST(&c->clkPorts);
}


/**
 * Determine if a clock is free running or not.
 * @param c  The clock instance.
 * @return   One if the clock is free running or zero otherwise.
 */
int clock_free_running(struct PtpClock *c);
int clock_free_running(struct PtpClock *c)
{
	return c->free_running ? 1 : 0;
}

/**
 * Obtain the gmCapable flag from a clock's default data set.
 * This function is specific to the 802.1AS standard.
 * @param c  The clock instance.
 * @return One if the clock is capable of becoming grand master, zero otherwise.
 */
int clock_gm_capable(struct PtpClock *c);
int clock_gm_capable(struct PtpClock *c)
{
	return c->grand_master_capable;
}

/**
 * Obtain a clock's identity from its default data set.
 * @param c  The clock instance.
 * @return   The clock's identity.
 */
struct ClockIdentity clock_identity(struct PtpClock *c);
struct ClockIdentity clock_identity(struct PtpClock *c)
{
	return c->dds.clockIdentity;
}



/**
 * Informs clock that a file descriptor of one of its ports changed. The
 * clock will rebuild its array of file descriptors to poll.
 * @param c    The clock instance.
 */
void clock_fda_changed(struct PtpClock *c);
void clock_fda_changed(struct PtpClock *c)
{
	c->pollfd_valid = 0;
}

/**
 * Obtains the time of the latest synchronization.
 * @param c    The clock instance.
 * @return     The local time stamp of the last received Sync message.
 */
tmv_t clock_ingress_time(struct PtpClock *c);
tmv_t clock_ingress_time(struct PtpClock *c)
{
	return c->ingress_ts;
}


/**
 * Obtain a clock's parent data set.
 * @param c  The clock instance.
 * @return   A pointer to the parent data set of the clock.
 */
struct parent_ds *clock_parent_ds(struct PtpClock *c);
struct parent_ds *clock_parent_ds(struct PtpClock *c)
{
	return &c->dad;
}

/**
 * Obtain the parent port identity from a clock's parent data set.
 * @param c  The clock instance.
 * @return   The parent port identity.
 */
struct PortIdentity clock_parent_identity(struct PtpClock *c);
struct PortIdentity clock_parent_identity(struct PtpClock *c)
{
	return c->dad.pds.parentPortIdentity;
}

/**
 * Set clock sde
 * @param c     A pointer to a clock instance obtained with clock_create().
 * @param sde   Pass one (1) if need a decision event and zero if not.
 */
void clock_set_sde(struct PtpClock *c, int sde);
void clock_set_sde(struct PtpClock *c, int sde)
{
	c->sde = sde;
}


/**
 * Obtain the slave-only flag from a clock's default data set.
 * @param c  The clock instance.
 * @return   The value of the clock's slave-only flag.
 */
int clock_slave_only(struct PtpClock *c);
int clock_slave_only(struct PtpClock *c)
{
	return c->dds.flags & DDS_SLAVE_ONLY;
}

/**
 * Obtain the steps removed field from a clock's current data set.
 * @param c  The clock instance.
 * @return   The value of the clock's steps removed field.
 */
UInteger16 clock_steps_removed(struct PtpClock *c);
UInteger16 clock_steps_removed(struct PtpClock *c)
{
	return c->cur.stepsRemoved;
}

/**
 * Obtain a clock's time properties data set.
 * @param c  The clock instance.
 * @return   A pointer to the time properties data set of the clock.
 */
struct timePropertiesDS *clock_time_properties(struct PtpClock *c);
struct timePropertiesDS *clock_time_properties(struct PtpClock *c)
{
	return &c->tds;
}

/**
 * Update a clock's time properties data set.
 * @param c   The clock instance.
 * @param tds The new time properties data set for the clock.
 */
void clock_update_time_properties(struct PtpClock *c, struct timePropertiesDS tds);
void clock_update_time_properties(struct PtpClock *c, struct timePropertiesDS tds)
{
	c->tds = tds;
}

/**
 * Obtain a clock's description.
 * @param c  The clock instance.
 * @return   A pointer to the ClockDescription of the clock.
 */
struct ClockDescription *clock_description(struct PtpClock *c);
struct ClockDescription *clock_description(struct PtpClock *c)
{
	return &c->desc;
}

/**
 * Obtain the type of a clock.
 * @param c  The clock instance.
 * @return   One of the @ref CLOCK_TYPE enumeration values.
 */
enum CLOCK_TYPE clock_type(struct PtpClock *c);
enum CLOCK_TYPE clock_type(struct PtpClock *c)
{
	return c->type;
}
#else
#define clock_class(clock)	\
		((clock)->dds.clockQuality.clockClass)

#define clock_config(clock)	\
		((clock)->config)

#define clock_current_dataset(clock)	\
 		(&(clock)->cur)
 
#define clock_best_foreign(clock)	\
		((clock)->best ? &(clock)->best->dataset : NULL)

#define	clock_best_port(clock)	\
		((clock)->best ? (clock)->best->port : NULL)

#define	clock_domain_number(clock)	\
		((clock)->dds.domainNumber)

#define	clock_first_port(clock)	\
		(LIST_FIRST(&(clock)->clkPorts))


#define	clock_free_running(clock)		\
		((clock)->free_running ? 1 : 0)

#define	clock_gm_capable(clock)	\
		((clock)->grand_master_capable)

#define	clock_identity(clock)		\
		((clock)->dds.clockIdentity)

#define	clock_fda_changed(clock)		\
		((clock)->pollfd_valid = 0)

#define	clock_ingress_time(clock)		\
		((clock)->ingress_ts)


#define	clock_parent_ds(clock)	\
		(&(clock)->dad)

#define	clock_parent_identity(clock)	\
		((clock)->dad.pds.parentPortIdentity)

#define	clock_set_sde(clock, sdeValue)		\
		((clock)->sde = sdeValue)


#define	 clock_slave_only(clock)	\
		((clock)->dds.flags & DDS_SLAVE_ONLY)

#define	clock_steps_removed(clock)	\
		((clock)->cur.stepsRemoved)


#define clock_time_properties(clock)	\
		(&(clock)->tds)

#define clock_update_time_properties(clock, tDs)	\
	(clock)->tds = (tDs)

#define	clock_description(clock)	\
	(&(clock)->desc)


#define	clock_type(clock) \
	((clock)->type)

#endif

#endif

