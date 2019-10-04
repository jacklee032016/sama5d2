
#include <errno.h>
#include <linux/net_tstamp.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/queue.h>

#include "clock.h"
#include "missing.h"
//#include "port.h"

#include "clockPrivate.h"
#include "portPrivate.h"

static int _clockResizePollfd(struct PtpClock *c, int new_nports)
{
	struct pollfd *new_pollfd;

	/* Need to allocate one whole extra block of fds for UDS. */
	new_pollfd = realloc(c->pollfd, (new_nports + 1) * N_CLOCK_PFD * sizeof(struct pollfd));
	if (!new_pollfd)
	{
		return -1;
	}
	
	c->pollfd = new_pollfd;
	return 0;
}

static int _clockAddPort(struct PtpClock *c, int phc_index, enum timestamp_type timestamping, struct PtpInterface *iface)
{
	struct PtpPort *p, *piter, *lastp = NULL;

	if (_clockResizePollfd(c, c->nports + 1))
	{
		return -1;
	}
	
	p = portCreate(phc_index, timestamping, ++c->last_port_number, iface, c);
	if (!p)
	{
		/* No need to shrink pollfd */
		return -1;
	}
	
	LIST_FOREACH(piter, &c->clkPorts, list) {
		lastp = piter;
	}
	
	if (lastp) {
		LIST_INSERT_AFTER(lastp, p, list);
	}
	else
	{
		LIST_INSERT_HEAD(&c->clkPorts, p, list);
	}
	
	c->nports++;
	clock_fda_changed(c);

	return 0;
}


static struct PtpClock _theClock;

/*
 * If we do not have a slave or the rtnl query failed, then use our
 * own interface name as the time stamping interface name.
 */
#if 0
static void _ensureTsLabel(struct PtpInterface *iface)
{
	if (iface->ts_label[0] == '\0')
		strncpy(iface->ts_label, iface->name, MAX_IFNAME_SIZE);
}
#else
#define	_ensureTsLabel(iFace)	\
	if ((iFace)->ts_label[0] == '\0'){ \
		strncpy((iFace)->ts_label, (iFace)->name, MAX_IFNAME_SIZE);}
#endif

struct PtpClock *clock_create(enum CLOCK_TYPE type, struct PtpConfig *config, const char *phc_device)
{
	enum servo_type servo = config_get_int(config, NULL, "clock_servo");
	enum timestamp_type timestamping;
	int fadj = 0, max_adj = 0, sw_ts;
	int phc_index, required_modes = 0;
	struct PtpClock *c = &_theClock;
	struct PtpPort *p;
	unsigned char oui[OUI_LEN];
	char phc[32], *tmp;
	struct PtpInterface *iface, *udsif = &c->uds_interface;
	struct timespec ts;
	int sfl;

	/* clockit_t and clock_gettime is defined time.h, set/get high resolution timers */
	PTP_GET_SYS_TIME_REALTIME(&ts);
	srandom(ts.tv_sec ^ ts.tv_nsec);

	if (c->nports) {
		clock_destroy(c);
	}

	switch (type) {
		case CLOCK_TYPE_ORDINARY:
		case CLOCK_TYPE_BOUNDARY:
		case CLOCK_TYPE_P2P:
		case CLOCK_TYPE_E2E:
			c->type = type;
			break;
		case CLOCK_TYPE_MANAGEMENT:
			return NULL;
	}

	/* Initialize the defaultDS. */
	c->dds.clockQuality.clockClass = config_get_int(config, NULL, "clockClass");
	c->dds.clockQuality.clockAccuracy = config_get_int(config, NULL, "clockAccuracy");
	c->dds.clockQuality.offsetScaledLogVariance = config_get_int(config, NULL, "offsetScaledLogVariance");

	c->desc.productDescription.max_symbols = 64;
	c->desc.revisionData.max_symbols = 32;
	c->desc.userDescription.max_symbols = 128;

	tmp = config_get_string(config, NULL, "productDescription");
	if (count_char(tmp, ';') != 2 || static_ptp_text_set(&c->desc.productDescription, tmp))
	{
		pr_err("invalid productDescription '%s'", tmp);
		return NULL;
	}
	
	tmp = config_get_string(config, NULL, "revisionData");
	if (count_char(tmp, ';') != 2 || static_ptp_text_set(&c->desc.revisionData, tmp))
	{
		pr_err("invalid revisionData '%s'", tmp);
		return NULL;
	}
	tmp = config_get_string(config, NULL, "userDescription");
	if (static_ptp_text_set(&c->desc.userDescription, tmp))
	{
		pr_err("invalid userDescription '%s'", tmp);
		return NULL;
	}
	
	tmp = config_get_string(config, NULL, "manufacturerIdentity");
	if (OUI_LEN != sscanf(tmp, "%hhx:%hhx:%hhx", &oui[0], &oui[1], &oui[2]))
	{
		pr_err("invalid manufacturerIdentity '%s'", tmp);
		return NULL;
	}
	memcpy(c->desc.manufacturerIdentity, oui, OUI_LEN);

	c->dds.domainNumber = config_get_int(config, NULL, "domainNumber");

	if (config_get_int(config, NULL, "slaveOnly")) {
		c->dds.flags |= DDS_SLAVE_ONLY;
	}
	if (!config_get_int(config, NULL, "gmCapable") &&
	    c->dds.flags & DDS_SLAVE_ONLY) {
		pr_err("Cannot mix 1588 slaveOnly with 802.1AS !gmCapable");
		return NULL;
	}
	
	if (!config_get_int(config, NULL, "gmCapable") || c->dds.flags & DDS_SLAVE_ONLY) {
		c->dds.clockQuality.clockClass = 255;
	}
	c->default_dataset.localPriority = config_get_int(config, NULL, "G.8275.defaultDS.localPriority");

	/* Harmonize the twoStepFlag with the time_stamping option. */
	if (config_harmonize_onestep(config)) {
		return NULL;
	}
	
	if (config_get_int(config, NULL, "twoStepFlag")) {
		c->dds.flags |= DDS_TWO_STEP_FLAG;
	}
	timestamping = config_get_int(config, NULL, "time_stamping");
	if (timestamping == TS_SOFTWARE) {
		sw_ts = 1;
	} else {
		sw_ts = 0;
	}

	c->dds.priority1 = config_get_int(config, NULL, "priority1");
	c->dds.priority2 = config_get_int(config, NULL, "priority2");

	/* Check the time stamping mode on each interface. */
	c->timestamping = timestamping;
	required_modes = clock_required_modes(c);

	/* check interface status */
	STAILQ_FOREACH(iface, &config->intfs, list)
	{
		rtnl_get_ts_device(iface->name, iface->ts_label);
		
		pr_debug("interface '%s', ts label:%s", iface->name, iface->ts_label );
		
		_ensureTsLabel(iface);
		sk_get_ts_info(iface->ts_label, &iface->ts_info);
		if (iface->ts_info.valid && ((iface->ts_info.so_timestamping & required_modes) != required_modes))
		{
			pr_err("interface '%s' does not support requested timestamping mode 0x%x", iface->name, required_modes);
			return NULL;
		}
	}

	iface = STAILQ_FIRST(&config->intfs);

	/* determine PHC Clock index */
	if (config_get_int(config, NULL, "free_running"))
	{
		phc_index = -1;
	}
	else if (timestamping == TS_SOFTWARE || timestamping == TS_LEGACY_HW)
	{
		phc_index = -1;
	}
	else if (phc_device)
	{
		if (1 != sscanf(phc_device, "/dev/ptp%d", &phc_index))
		{
			pr_err("bad ptp device string");
			return NULL;
		}
	}
	else if (iface->ts_info.valid)
	{
		phc_index = iface->ts_info.phc_index;
	}
	else
	{
		pr_err("PTP device not specified and automatic determination"
		       " is not supported. Please specify PTP device.");
		return NULL;
	}
	
	if (phc_index >= 0)
	{
		pr_info("selected /dev/ptp%d as PTP clock", phc_index);
	}

	if (generate_clock_identity(&c->dds.clockIdentity, iface->name))
	{
		pr_err("failed to generate a clock identity");
		return NULL;
	}


	/* Configure the UDS. */
	snprintf(udsif->name, sizeof(udsif->name), "%s", config_get_string(config, NULL, "uds_address"));
	if (config_set_section_int(config, udsif->name, "announceReceiptTimeout", 0))
	{
		return NULL;
	}
	if (config_set_section_int(config, udsif->name, "delay_mechanism", DM_AUTO)) {
		return NULL;
	}
	if (config_set_section_int(config, udsif->name, "network_transport", TRANS_UDS))
	{
		return NULL;
	}
	if (config_set_section_int(config, udsif->name, "delay_filter_length", 1))
	{
		return NULL;
	}

	c->config = config;
	c->free_running = config_get_int(config, NULL, "free_running");
	c->freq_est_interval = config_get_int(config, NULL, "freq_est_interval");
	c->grand_master_capable = config_get_int(config, NULL, "gmCapable");
	c->kernel_leap = config_get_int(config, NULL, "kernel_leap");
	c->utc_offset = config_get_int(config, NULL, "utc_offset");
	c->time_source = config_get_int(config, NULL, "timeSource");

	if (c->free_running)
	{
		EXT_INFOF("Clock free running");
		c->clkid = CLOCK_INVALID;
		if (timestamping == TS_SOFTWARE || timestamping == TS_LEGACY_HW)
		{
			c->utc_timescale = 1;
		}
	}
	else if (phc_index >= 0)
	{
		EXT_INFOF("Clock use PTP clock");
		snprintf(phc, sizeof(phc), "/dev/ptp%d", phc_index);
		c->clkid = phc_open(phc);

		if (c->clkid == CLOCK_INVALID)
		{
			pr_err("Failed to open %s: %m", phc);
			return NULL;
		}
		max_adj = phc_max_adj(c->clkid);
		if (!max_adj) {
			pr_err("clock is not adjustable");
			return NULL;
		}
		clockadj_init(c->clkid);
	}
	else
	{
		EXT_INFOF("Clock use REALTIME clock");
		c->clkid = CLOCK_REALTIME;
		c->utc_timescale = 1;
		clockadj_init(c->clkid);
		max_adj = sysclk_max_freq();
		sysclk_set_leap(0);
	}
	
	c->utc_offset_set = 0;
	c->leap_set = 0;
	c->time_flags = c->utc_timescale ? 0 : PTP_TIMESCALE;

	if (c->clkid != CLOCK_INVALID)
	{
		fadj = (int) clockadj_get_freq(c->clkid);
		/* Due to a bug in older kernels, the reading may silently fail
		   and return 0. Set the frequency back to make sure fadj is
		   the actual frequency of the clock. */
		clockadj_set_freq(c->clkid, fadj);
	}

	
	c->servo = servo_create(c->config, servo, -fadj, max_adj, sw_ts);
	if (!c->servo) {
		pr_err("Failed to create clock servo");
		return NULL;
	}
	c->servo_state = SERVO_UNLOCKED;
	c->servo_type = servo;
	if (config_get_int(config, NULL, "dataset_comparison") == DS_CMP_G8275)
	{
		c->dscmp = telecom_dscmp;
	}
	else
	{
		c->dscmp = dscmp;
	}
	
	c->tsproc = tsproc_create(config_get_int(config, NULL, "tsproc_mode"),
				  config_get_int(config, NULL, "delay_filter"),
				  config_get_int(config, NULL, "delay_filter_length"));
	if (!c->tsproc) {
		pr_err("Failed to create time stamp processor");
		return NULL;
	}
	
	c->initial_delay = dbl_tmv(config_get_int(config, NULL, "initial_delay"));
	c->master_local_rr = 1.0;
	c->nrr = 1.0;
	c->stats_interval = config_get_int(config, NULL, "summary_interval");
	c->stats.offset = stats_create();
	c->stats.freq = stats_create();
	c->stats.delay = stats_create();
	if (!c->stats.offset || !c->stats.freq || !c->stats.delay) {
		pr_err("failed to create stats");
		return NULL;
	}
	sfl = config_get_int(config, NULL, "sanity_freq_limit");
	if (sfl) {
		c->sanity_check = clockcheck_create(sfl);
		if (!c->sanity_check) {
			pr_err("Failed to create clock sanity check");
			return NULL;
		}
	}

	/* Initialize the parentDS. */
	clock_update_grandmaster(c);
	c->dad.pds.parentStats                           = 0;
	c->dad.pds.observedParentOffsetScaledLogVariance = 0xffff;
	c->dad.pds.observedParentClockPhaseChangeRate    = 0x7fffffff;
	c->dad.ptl = c->ptl;

	clock_sync_interval(c, 0);

	LIST_INIT(&c->subscribers);
	LIST_INIT(&c->clkPorts);
	c->last_port_number = 0;

	if (_clockResizePollfd(c, 0))
	{
		pr_err("failed to allocate pollfd");
		return NULL;
	}

	/* Create the UDS port from interface defined just now */
	c->uds_port = portCreate(phc_index, timestamping, 0, udsif, c);
	if (!c->uds_port) {
		pr_err("failed to open the UDS port");
		return NULL;
	}
	clock_fda_changed(c);

	/* Create the ports from interfaces defined by app . */
	STAILQ_FOREACH(iface, &config->intfs, list)
	{
		if (_clockAddPort(c, phc_index, timestamping, iface))
		{
			pr_err("failed to open port %s", iface->name);
			return NULL;
		}
	}

	c->dds.numberPorts = c->nports;

	LIST_FOREACH(p, &c->clkPorts, list)
	{
		PORT_DISPATCH(p, EV_INITIALIZE, 0);
	}
	PORT_DISPATCH(c->uds_port, EV_INITIALIZE, 0);

	return c;
}


static void _clockRemovePort(struct PtpClock *c, struct PtpPort *p)
{
	/* Do not call clock_resize_pollfd, it's pointless to shrink
	 * the allocated memory at this point, clock_destroy will free
	 * it all anyway. This function is usable from other parts of
	 * the code, but even then we don't mind if pollfd is larger
	 * than necessary. */
	LIST_REMOVE(p, list);
	c->nports--;
	clock_fda_changed(c);
	portDestory(p);
}

void clock_destroy(struct PtpClock *c)
{
	struct PtpPort *p, *tmp;

	clock_flush_subscriptions(c);

	LIST_FOREACH_SAFE(p, &c->clkPorts, list, tmp)
	{
		_clockRemovePort(c, p);
	}
	
	portDestory(c->uds_port);
	free(c->pollfd);
	if (c->clkid != CLOCK_REALTIME) {
		phc_close(c->clkid);
	}
	servo_destroy(c->servo);
	tsproc_destroy(c->tsproc);
	stats_destroy(c->stats.offset);
	stats_destroy(c->stats.freq);
	stats_destroy(c->stats.delay);
	if (c->sanity_check) {
		clockcheck_destroy(c->sanity_check);
	}
	memset(c, 0, sizeof(*c));
	msg_cleanup();
	tc_cleanup();
}



