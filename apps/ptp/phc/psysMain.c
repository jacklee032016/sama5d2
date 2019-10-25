
#include "phc2sys.h"

struct PtpConfig *phc2sys_config;


int psysReadPhc(clockid_t clkid, clockid_t sysclk, int readings,
		    int64_t *offset, uint64_t *ts, int64_t *delay)
{
	struct timespec tdst1, tdst2, tsrc;
	int i;
	int64_t interval, best_interval = INT64_MAX;

	/* Pick the quickest clkid reading. */
	for (i = 0; i < readings; i++) {
		if (PTP_GET_SYS_TIME_NOW(sysclk, &tdst1) ||
				PTP_GET_SYS_TIME_NOW(clkid, &tsrc) ||
				PTP_GET_SYS_TIME_NOW(sysclk, &tdst2))
		{
			pr_err("failed to read clock: %m");
			return 0;
		}

		interval = (tdst2.tv_sec - tdst1.tv_sec) * NS_PER_SEC +
			tdst2.tv_nsec - tdst1.tv_nsec;

		if (best_interval > interval) {
			best_interval = interval;
			*offset = (tdst1.tv_sec - tsrc.tv_sec) * NS_PER_SEC +
				tdst1.tv_nsec - tsrc.tv_nsec + interval / 2;
			*ts = tdst2.tv_sec * NS_PER_SEC + tdst2.tv_nsec;
		}
	}
	*delay = best_interval;

	return 1;
}

static int update_needed(struct PhcClock *c)
{
	switch (c->state) {
	case PS_FAULTY:
	case PS_DISABLED:
	case PS_LISTENING:
	case PS_PRE_MASTER:
	case PS_MASTER:
	case PS_PASSIVE:
		return 1;
	case PS_UNCALIBRATED:
	case PS_SLAVE:
		break;
	}
	return 0;
}


int psysNormalizeState(int state)
{
	if (state != PS_MASTER && state != PS_SLAVE &&
		state != PS_PRE_MASTER && state != PS_UNCALIBRATED)
	{
		/* treat any other state as "not a master nor a slave" */
		state = PS_DISABLED;
	}
	return state;
}

void _psysCleanupPmc(struct PhcNode *node)
{
	if( !node->pmc )
		return;
	
	pmc_destroy(node->pmc);
	node->pmc = NULL;
}


static void _psysCleanupClocks(struct PhcNode *node)
{
	struct PhcClock *c, *tmp;

	LIST_FOREACH_SAFE(c, &node->clocks, list, tmp)
	{
		if (c->servo) {
			servo_destroy(c->servo);
		}
		if (c->sanity_check) {
			clockcheck_destroy(c->sanity_check);
		}
		if (c->delay_stats) {
			stats_destroy(c->delay_stats);
		}
		if (c->freq_stats) {
			stats_destroy(c->freq_stats);
		}
		if (c->offset_stats) {
			stats_destroy(c->offset_stats);
		}
		if (c->device) {
			free(c->device);
		}
		free(c);
	}
}


static void _psysCleanupPorts(struct PhcNode *node)
{
	struct PhcPort *p, *tmp;

	LIST_FOREACH_SAFE(p, &node->ports, list, tmp) {
		free(p);
	}
}

static void usage(char *progname)
{
	fprintf(stderr,
		"\n"
		"usage: %s [options]\n\n"
		"\n"
		" automatic configuration:\n"
		" -a             turn on autoconfiguration\n"
		" -r             synchronize system (realtime) clock\n"
		"                repeat -r to consider it also as a time source\n"
		" manual configuration:\n"
		" -c [dev|name]  slave clock (CLOCK_REALTIME)\n"
		" -d [dev]       master PPS device\n"
		" -s [dev|name]  master clock\n"
		" -O [offset]    slave-master time offset (0)\n"
		" -w             wait for ptp4l\n"
		" common options:\n"
		" -f [file]      configuration file\n"
		" -E [pi|linreg] clock servo (pi)\n"
		" -P [kp]        proportional constant (0.7)\n"
		" -I [ki]        integration constant (0.3)\n"
		" -S [step]      step threshold (disabled)\n"
		" -F [step]      step threshold only on start (0.00002)\n"
		" -R [rate]      slave clock update rate in HZ (1.0)\n"
		" -N [num]       number of master clock readings per update (5)\n"
		" -L [limit]     sanity frequency limit in ppb (200000000)\n"
		" -M [num]       NTP SHM segment number (0)\n"
		" -u [num]       number of clock updates in summary stats (0)\n"
		" -n [num]       domain number (0)\n"
		" -x             apply leap seconds by servo instead of kernel\n"
		" -z [path]      server address for UDS ("PTP_RUN_HOME"/ptp4l)\n"
		" -l [num]       set the logging level to 'num' (6)\n"
		" -t [tag]       add tag to log messages\n"
		" -m             print messages to stdout\n"
		" -q             do not print messages to the syslog\n"
		" -v             prints the software version and exits\n"
		" -h             prints this message and exits\n"
		"\n",
		progname);
}

static int _psysMainLoop(struct PhcNode *node, int subscriptions)
{
	struct timespec interval;
	struct PhcClock *clock;
	uint64_t ts;
	int64_t offset, delay;

	interval.tv_sec = node->phc_interval;
	interval.tv_nsec = (node->phc_interval - interval.tv_sec) * 1e9;

	while (is_running())
	{
		clock_nanosleep(CLOCK_MONOTONIC, 0, &interval, NULL);

		/* update node sync_offset, leap from pmc */
		if (psysPmcUpdate(node, subscriptions) < 0)
			continue;

		if (subscriptions)
		{/* auto */
			psysPmcWaitEvents(node);
			
			if (node->state_changed)
			{
				/* force getting offset, as it may have
				 * changed after the port state change */
				if (psysPmcGetUtcOffset(node, 1000) <= 0)
				{
					pr_err("failed to get UTC offset");
					continue;
				}
				
				psysReconfigure(node);
			}
		}
		
		if (!node->master)
			continue;

		LIST_FOREACH(clock, &node->dst_clocks, dst_list)
		{
			if (!update_needed(clock))
				continue;

			/* don't try to synchronize the clock to itself */
			if (clock->clkid == node->master->clkid ||
			    (clock->phc_index >= 0 &&
			     clock->phc_index == node->master->phc_index) ||
			    !strcmp(clock->device, node->master->device))
			{
				continue;
			}

			/* get offset, ts, delay from master  */
			if (clock->clkid == CLOCK_REALTIME && node->master->sysoff_supported)
			{/* use sysoff */
				if (sysoff_measure(CLOCKID_TO_FD(node->master->clkid), node->phc_readings, &offset, &ts, &delay))
					return -1;
			}
			else
			{/* use phc */
				if (!psysReadPhc(node->master->clkid, clock->clkid, node->phc_readings, &offset, &ts, &delay))
					continue;
			}

			/* update offset, ts and delay to every dest clock */
			psysClockUpdate(node, clock, offset, ts, delay);
		}
	}
	
	return 0;
}

static int	_phcAutoConfig(struct PtpConfig *cfg, struct PhcNode *node, int rt)
{
	int ret; 
	EXT_INFOF("Working as auto...");
	
	if (psysPmcInit(cfg, node) )
	{
		return EXIT_FAILURE;
	}
	
	if (psysAutoInitPortsByPmc(node, rt) < 0)
	{
		return EXIT_FAILURE;
	}
	
	ret = _psysMainLoop(node, 1);

	return ret;
}

int main(int argc, char *argv[])
{
	/* src: /dev/ptp0, eth0, CLOCK_REALTIME; dst: /dev/ptp1, eth1, or CLOCK_REALTIME(default) */
	char *configFile = NULL, *dst_name = NULL, *progname, *src_name = NULL;
	
	struct PhcClock *src, *dst;
	struct PtpConfig *cfg;
	
	struct option *opts;
	int autocfg = 0, c, domain_number = 0, index, ntpshm_segment;
	int pps_fd = -1, print_level = LOG_INFO, r = -1, rt = 0, wait_sync = 0;
	double phc_rate, tmp;
	
	struct PhcNode node =
	{
		.phc_readings = 5,
		.phc_interval = 1.0,
	};

	handle_term_signals();

	cfg = phc2sys_config = config_create();
	if (!cfg) {
		return -1;
	}

	opts = config_long_options(cfg);

	config_set_double(cfg, "pi_proportional_const", KP);
	config_set_double(cfg, "pi_integral_const", KI);

	/* Process the command line arguments. */
	progname = strrchr(argv[0], '/');
	progname = progname ? 1+progname : argv[0];
	
	while (EOF != (c = getopt_long(argc, argv, "arc:d:f:s:E:P:I:S:F:R:N:O:L:M:i:u:wn:xz:l:t:mqvh", opts, &index)))
	{
		switch (c)
		{
			case 0:
				if (config_parse_option(cfg, opts[index].name, optarg)) {
					goto bad_usage;
				}
				break;
			case 'a':
				autocfg = 1;
				break;
			case 'r':
				rt++;
				break;
			case 'c':
				dst_name = strdup(optarg);
				break;
			case 'd':
				pps_fd = open(optarg, O_RDONLY);
				if (pps_fd < 0) {
					fprintf(stderr,
						"cannot open '%s': %m\n", optarg);
					goto end;
				}
				break;
			case 'f':
				configFile = optarg;
				break;
			case 'i':
				fprintf(stderr, 	"'-i' has been deprecated. please use '-s' instead.\n");
				
	            /* fallthrough */
			case 's':
				src_name = strdup(optarg);
				break;
				
			case 'E':
				if (!strcasecmp(optarg, "pi"))
				{
					config_set_int(cfg, "clock_servo", CLOCK_SERVO_PI);
				}
				else if (!strcasecmp(optarg, "linreg"))
				{
					config_set_int(cfg, "clock_servo", CLOCK_SERVO_LINREG);
				}
				else if (!strcasecmp(optarg, "ntpshm"))
				{
					config_set_int(cfg, "clock_servo", CLOCK_SERVO_NTPSHM);
				}
				else
				{
					fprintf(stderr, 	"invalid servo name %s\n", optarg);
					goto end;
				}
				break;
			case 'P':
				if (get_arg_val_d(c, optarg, &tmp, 0.0, DBL_MAX) ||
				    config_set_double(cfg, "pi_proportional_const", tmp))
					goto end;
				break;
			case 'I':
				if (get_arg_val_d(c, optarg, &tmp, 0.0, DBL_MAX) ||
				    config_set_double(cfg, "pi_integral_const", tmp))
					goto end;
				break;
			case 'S':
				if (get_arg_val_d(c, optarg, &tmp, 0.0, DBL_MAX) ||
				    config_set_double(cfg, "step_threshold", tmp))
					goto end;
				break;
			case 'F':
				if (get_arg_val_d(c, optarg, &tmp, 0.0, DBL_MAX) ||
				    config_set_double(cfg, "first_step_threshold", tmp))
					goto end;
				break;
			case 'R': /* not defined in config file */
				if (get_arg_val_d(c, optarg, &phc_rate, 1e-9, DBL_MAX))
					goto end;
				node.phc_interval = 1.0 / phc_rate;
				break;
			case 'N':/* not defined in config file */
				if (get_arg_val_i(c, optarg, &node.phc_readings, 1, INT_MAX))
					goto end;
				break;
			case 'O':/* not defined in config file */
				if (get_arg_val_i(c, optarg, &node.sync_offset,
						  INT_MIN, INT_MAX))
					goto end;
				node.forced_sync_offset = -1;
				break;
			case 'L':
				if (get_arg_val_i(c, optarg, &node.sanity_freq_limit, 0, INT_MAX) || config_set_int(cfg, "sanity_freq_limit", node.sanity_freq_limit)) {
					goto end;
				}
				break;
			case 'M':
				if (get_arg_val_i(c, optarg, &ntpshm_segment, INT_MIN, INT_MAX) || config_set_int(cfg, "ntpshm_segment", ntpshm_segment))
					goto end;
				break;
			case 'u':/* not defined in config file */
				if (get_arg_val_ui(c, optarg, &node.stats_max_count, 0, UINT_MAX))
					goto end;
				break;
			case 'w':
				wait_sync = 1;
				break;
			case 'n':
				if (get_arg_val_i(c, optarg, &domain_number, 0, 255) || config_set_int(cfg, "domainNumber", domain_number)) {
					goto end;
				}
				break;
			case 'x':
				if (config_set_int(cfg, "kernel_leap", 0)) {
					goto end;
				}
				break;
			case 'z':
				if (strlen(optarg) > MAX_IFNAME_SIZE)
				{
					fprintf(stderr, "path %s too long, max is %d\n",	optarg, MAX_IFNAME_SIZE);
					goto end;
				}
				
				if (config_set_string(cfg, "uds_address", optarg))
				{
					goto end;
				}
				break;
			case 'l':
				if (get_arg_val_i(c, optarg, &print_level, PRINT_LEVEL_MIN, PRINT_LEVEL_MAX) ||config_set_int(cfg, "logging_level", print_level)) {
					goto end;
				}
				break;
			case 't':
				if (config_set_string(cfg, "message_tag", optarg)) {
					goto end;
				}
				break;
			case 'm':
				if (config_set_int(cfg, "verbose", 1)) {
					goto end;
				}
				break;
			case 'q':
				if (config_set_int(cfg, "use_syslog", 0)) {
					goto end;
				}
				break;
			case 'v':
				version_show(stdout);
				config_destroy(cfg);
				return 0;
			case 'h':
				usage(progname);
				config_destroy(cfg);
				return 0;
			default:
				goto bad_usage;
			}
	}

	configFile = EXT_CONFIG_FILE_PHCD;
	if (configFile && (c = config_read(configFile, cfg)))
	{
		return c;
	}

	if (autocfg && (src_name || dst_name || pps_fd >= 0 || wait_sync || node.forced_sync_offset))
	{
		fprintf(stderr, 	"autoconfiguration cannot be mixed with manual config options.\n");
		goto bad_usage;
	}
	
	if (!autocfg && pps_fd < 0 && !src_name)
	{
		fprintf(stderr,	"autoconfiguration or valid source clock must be selected.\n");
		goto bad_usage;
	}

	if (!autocfg && !wait_sync && !node.forced_sync_offset)
	{
		fprintf(stderr,	"time offset must be specified using -w or -O\n");
		goto bad_usage;
	}

	if (node.servoType == CLOCK_SERVO_NTPSHM)
	{
		node.kernel_leap = 0;
		node.sanity_freq_limit = 0;
	}

	print_set_progname(progname);
	print_set_tag(config_get_string(cfg, NULL, "message_tag"));
	print_set_verbose(config_get_int(cfg, NULL, "verbose"));
	print_set_syslog(config_get_int(cfg, NULL, "use_syslog"));
	print_set_level(config_get_int(cfg, NULL, "logging_level"));

	node.servoType = config_get_int(cfg, NULL, "clock_servo");
	if (node.servoType == CLOCK_SERVO_NTPSHM)
	{
		config_set_int(cfg, "kernel_leap", 0);
		config_set_int(cfg, "sanity_freq_limit", 0);
	}
	node.kernel_leap = config_get_int(cfg, NULL, "kernel_leap");
	node.sanity_freq_limit = config_get_int(cfg, NULL, "sanity_freq_limit");
	EXT_INFOF("clock_servo is %d", node.servoType);

	if (autocfg)
	{
		_phcAutoConfig(cfg, &node, rt);
		goto end;
	}

TRACE();
	/* src can be /dev/ptp0, eth0 or CLOCK_REALTIME */
	src = psysClockAdd(&node, src_name);
	free(src_name);
	if (!src)
	{
		fprintf(stderr, 	"valid source clock must be selected.\n");
		goto bad_usage;
	}
	src->state = PS_SLAVE;
	node.master = src;

	dst = psysClockAdd(&node, dst_name ? dst_name : "CLOCK_REALTIME");
	free(dst_name);
	if (!dst)
	{
		fprintf(stderr, "valid destination clock must be selected.\n");
		goto bad_usage;
	}
	
	dst->state = PS_MASTER;
	LIST_INSERT_HEAD(&node.dst_clocks, dst, dst_list);

	if (pps_fd >= 0 && dst->clkid != CLOCK_REALTIME)
	{
		fprintf(stderr, "cannot use a pps device unless destination is CLOCK_REALTIME\n");
		goto bad_usage;
	}

	r = -1;

	if (wait_sync)
	{
		if (psysPmcInit(cfg, &node))
			goto end;

		while (is_running())
		{
			r = psysPmcWaitPtpdReady(&node, 1000);
			if (r < 0)
				goto end;
			if (r > 0)
				break;
			else
				pr_notice("Waiting for ptp4l...");
		}

		if (!node.forced_sync_offset)
		{
			r = psysPmcGetUtcOffset(&node, 1000);
			if (r <= 0) {
				pr_err("failed to get UTC offset");
				goto end;
			}
		}

		if (node.forced_sync_offset || (src->clkid != CLOCK_REALTIME && dst->clkid != CLOCK_REALTIME) || src->clkid == CLOCK_INVALID)
		{
			_psysCleanupPmc(&node);
		}
	}

	if (pps_fd >= 0)
	{
		/* only one destination clock allowed with PPS until we
		 * implement a mean to specify PTP port to PPS mapping */
		servo_sync_interval(dst->servo, 1.0);
		r = psysPpsLoop(&node, dst, pps_fd);
	}
	else
	{
		r = _psysMainLoop(&node, 0);
	}

end:
	_psysCleanupPmc(&node);
	_psysCleanupClocks(&node);
	_psysCleanupPorts(&node);
	config_destroy(cfg);
	msg_cleanup();
	
	return r;
	
bad_usage:
	usage(progname);
	config_destroy(cfg);
	return -1;
}

