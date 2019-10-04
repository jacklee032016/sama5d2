/*
 * implementation related definitions
 */
 
#ifndef __PTP_COMPACT_H__
#define __PTP_COMPACT_H__


#include <stdint.h>

/* Primitive data types */
#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif

#define	PTP_FALSE					0
#define	PTP_TRUE					1

#define	PTP_FAILED					-1
#define	PTP_SUCCESS				0


typedef	int       Boolean;
typedef uint8_t   Enumeration8;
typedef uint16_t  Enumeration16;
typedef int8_t    Integer8;
typedef uint8_t   UInteger8;
typedef int16_t   Integer16;
typedef uint16_t  UInteger16;
typedef int32_t   Integer32;
typedef uint32_t  UInteger32;
typedef int64_t   Integer64;
typedef uint8_t   Octet;


#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define MAX_IFNAME_SIZE 108 /* = UNIX_PATH_MAX */

#define NS_PER_SEC			1000000000LL		/* 10**9 */

/**
 * Defines the possible delay mechanisms.
 */
enum delay_mechanism {

	/** Start as E2E, but switch to P2P if a peer is detected. */
	DM_AUTO,

	/** Delay request-response mechanism. */
	DM_E2E,

	/** Peer delay mechanism. */
	DM_P2P,
};


enum NOTIFICATION
{
	NOTIFY_PORT_STATE,
};


#include <stddef.h>

/*
 * This macro borrowed from the Linux kernel.
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type, member) );	\
})


#define N_TIMER_FDS 8

/*
 * The order matters here.  The DELAY timer must appear before the
 * ANNOUNCE and SYNC_RX timers in order to correctly handle the case
 * when the DELAY timer and one of the other two expire during the
 * same call to poll().
 */
enum
{
	FD_EVENT,				/* port 319, 2 multicasts, 224.0.1.129(primary), 224.0.0.127(pdelay). ont socket join 2 groups */
	FD_GENERAL,			/* port 320, 2 multicasts, 224.0.1.129(primary), 224.0.0.127(pdelay). ont socket join 2 groups */
	FD_DELAY_TIMER,
	FD_ANNOUNCE_TIMER,
	FD_SYNC_RX_TIMER,
	FD_QUALIFICATION_TIMER,
	FD_MANNO_TIMER,		/* master TX announce */
	FD_SYNC_TX_TIMER,
	FD_UNICAST_REQ_TIMER,
	FD_UNICAST_SRV_TIMER,
	FD_RTNL,
	N_POLLFD,
};


#define FD_FIRST_TIMER FD_DELAY_TIMER

struct FdArray
{
	int fd[N_POLLFD];
};


#include <netinet/in.h>
#include <netpacket/packet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <net/if_arp.h>

struct address {
	socklen_t len;
	union {
		struct sockaddr_storage ss;
		struct sockaddr_ll sll;
		struct sockaddr_in sin;
		struct sockaddr_in6 sin6;
		struct sockaddr_un sun;
		struct sockaddr sa;
	};
};



#include <stdint.h>

#define EUI48 6
#define EUI64 8

#define MAC_LEN  EUI48
#define GUID_LEN EUI64

#define GUID_OFFSET 36

typedef uint8_t eth_addr[MAC_LEN];

struct eth_hdr {
	eth_addr dst;
	eth_addr src;
	uint16_t type;
} __attribute__((packed));

#define VLAN_HLEN 4

struct vlan_hdr {
	eth_addr dst;
	eth_addr src;
	uint16_t tpid;
	uint16_t tci;
	uint16_t type;
} __attribute__((packed));

#define OFF_ETYPE (2 * sizeof(eth_addr))

#include <stdio.h>
/**
 * Print the software version string into the given file.
 * @param fp  File pointer open for writing.
 */
void version_show(FILE *fp);

/**
 * Provide the software version as a human readable string.
 * @return  Pointer to a static global buffer holding the result.
 */
const char *version_string(void);



typedef void (*rtnl_callback)(void *ctx, int linkup, int ts_index);

/**
 * Close a RT netlink socket.
 * @param fd  A socket obtained via rtnl_open().
 * @return    Zero on success, non-zero otherwise.
 */
int rtnl_close(int fd);

/**
 * Get name of the slave interface which timestamps packets going through
 * a master interface (e.g. bond0)
 * @param device    Name of the master interface.
 * @param ts_device Buffer for the name of the slave interface, which must be
 *                  at least IF_NAMESIZE bytes long.
 * @return          Zero on success, or -1 on error.
 */
int rtnl_get_ts_device(char *device, char *ts_device);

/**
 * Request the link status from the kernel.
 * @param fd     A socket obtained via rtnl_open().
 * @param device Interface name. Request all iface's status if set NULL.
 * @return       Zero on success, non-zero otherwise.
 */
int rtnl_link_query(int fd, char *device);

/**
 * Read kernel messages looking for a link up/down events.
 * @param fd     Readable socket obtained via rtnl_open().
 * @param device The device which we need to get link info.
 * @param cb     Callback function to be invoked on each event.
 * @param ctx    Private context passed to the callback.
 * @return       Zero on success, non-zero otherwise.
 */
int rtnl_link_status(int fd, char *device, rtnl_callback cb, void *ctx);

/**
 * Open a RT netlink socket for monitoring link state.
 * @return    A valid socket, or -1 on error.
 */
int rtnl_open(void);


struct pqueue;

struct pqueue *pqueue_create(int max_length,
			     int (*compare)(void *a, void *b));

void pqueue_destroy(struct pqueue *q);

void *pqueue_extract(struct pqueue *q);

int pqueue_insert(struct pqueue *q, void *d);

int pqueue_length(struct pqueue *q);

void *pqueue_peek(struct pqueue *q);



struct hash;

/**
 * Create a new hash table.
 * @return  A pointer to a new hash table on success, NULL otherwise.
 */
struct hash *hash_create(void);

/**
 * Destroy an instance of a hash table.
 * @param ht   Pointer to a hash table obtained via @ref hash_create().
 * @param func Callback function, possibly NULL, to apply to the
 *             data of each element in the table.
 */
void hash_destroy(struct hash *ht, void (*func)(void *));

/**
 * Inserts an element into a hash table.
 * @param ht   Hash table into which the element is to be stored.
 * @param key  Key that identifies the element.
 * @param data Pointer to the user data to be stored.
 * @return Zero on success and non-zero on error.  Attempting to
 *         insert a duplicate key will fail with an error.
 */
int hash_insert(struct hash *ht, const char* key, void *data);

/**
 * Looks up an element from the hash table.
 * @param ht   Hash table to consult.
 * @param key  Key identifying the element of interest.
 * @return  Pointer to the element's data, or NULL if the key is not found.
 */
void *hash_lookup(struct hash *ht, const char* key);


/**
 * We implement the time value as a 64 bit signed integer containing
 * nanoseconds. Using this representation, we could really spare the
 * arithmetic functions such as @ref tmv_add() and the like, and just
 * use plain old math operators in the code.
 *
 * However, we are going to be a bit pedantic here and enforce the
 * use of the these functions, so that we can easily upgrade the code
 * to a finer representation later on. In that way, we can make use of
 * the fractional nanosecond parts of the correction fields, if and
 * when people start asking for them.
 */
typedef struct {
	int64_t ns;
} tmv_t;

/**
 * Defines the available filters.
 */
enum filter_type {
	FILTER_MOVING_AVERAGE,
	FILTER_MOVING_MEDIAN,
};


/**
 * Defines the available modes.
 */
enum tsproc_mode {
	TSPROC_FILTER,
	TSPROC_RAW,
	TSPROC_FILTER_WEIGHT,
	TSPROC_RAW_WEIGHT,
};

/** Opaque type */
struct TimestampProcess;

/** Opaque type */
struct PtpFilter;

/* process timestamp of t1, t2, t3, and t4, with Filter (Moving Average or Moving Media) */
struct TimestampProcess
{
	/* Processing options */
	enum tsproc_mode mode;

	/* Current ratio between remote and local clock frequency */
	double clock_rate_ratio;

	/* Latest down measurement */
	tmv_t t1;
	tmv_t t2;

	/* Latest up measurement */
	tmv_t t3;
	tmv_t t4;

	/* Current filtered delay */
	tmv_t filtered_delay;
	int filtered_delay_valid;

	/* Delay filter */
	struct PtpFilter *delay_filter;
};

/**
 * Create a new instance of the time stamp processor.
 * @param mode           Time stamp processing mode.
 * @param delay_filter   Type of the filter that will be applied to delay.
 * @param filter_length  Length of the filter.
 * @return               A pointer to a new tsproc on success, NULL otherwise.
 */
struct TimestampProcess *tsproc_create(enum tsproc_mode mode,
			     enum filter_type delay_filter, int filter_length);

#if 0
/**
 * Destroy a time stamp processor.
 * @param tsp       Pointer obtained via @ref tsproc_create().
 */
void tsproc_destroy(struct TimestampProcess *tsp);
void tsproc_destroy(struct TimestampProcess *tsp)
{
	filter_destroy(tsp->delay_filter);
	free(tsp);
}

/**
 * Feed a downstream measurement into a time stamp processor.
 * @param tsp       Pointer obtained via @ref tsproc_create().
 * @param remote_ts The remote transmission time.
 * @param local_ts  The local reception time.
 */
void tsproc_down_ts(struct TimestampProcess *tsp, tmv_t remote_ts, tmv_t local_ts);
void tsproc_down_ts(struct TimestampProcess *tsp, tmv_t remote_ts, tmv_t local_ts)
{
	tsp->t1 = remote_ts;
	tsp->t2 = local_ts;
}

/**
 * Feed an upstream measurement into a time stamp processor.
 * @param tsp       Pointer obtained via @ref tsproc_create().
 * @param local_ts  The local transmission time.
 * @param remote_ts The remote reception time.
 */
void tsproc_up_ts(struct TimestampProcess *tsp, tmv_t local_ts, tmv_t remote_ts);
void tsproc_up_ts(struct TimestampProcess *tsp, tmv_t local_ts, tmv_t remote_ts)
{
	tsp->t3 = local_ts;
	tsp->t4 = remote_ts;
}

/**
 * Set ratio between remote and local clock frequencies.
 * @param tsp               Pointer obtained via @ref tsproc_create().
 * @param clock_rate_ratio  The ratio between frequencies.
 */
void tsproc_set_clock_rate_ratio(struct TimestampProcess *tsp, double clock_rate_ratio);
void tsproc_set_clock_rate_ratio(struct TimestampProcess *tsp, double clock_rate_ratio)
{
	tsp->clock_rate_ratio = clock_rate_ratio;
}

/**
 * Set delay in a time stamp processor. This can be used to override the last
 * calculated value.
 * @param tsp    Pointer obtained via @ref tsproc_create().
 * @param delay  The new delay.
 */
void tsproc_set_delay(struct TimestampProcess *tsp, tmv_t delay);
void tsproc_set_delay(struct TimestampProcess *tsp, tmv_t delay)
{
	tsp->filtered_delay = delay;
	tsp->filtered_delay_valid = 1;
}

#else
#define tsproc_destroy(tsp)	\
		do{	filter_destroy((tsp)->delay_filter); free((tsp));}while(0)

#define	tsproc_down_ts(tsp, remote_ts, local_ts)	\
	do{ (tsp)->t1 = (remote_ts); (tsp)->t2 = (local_ts);}while(0)

#define	tsproc_up_ts(tsp, local_ts, remote_ts)	\
	do{ (tsp)->t3 = (local_ts);  (tsp)->t4 = (remote_ts);}while(0)

#define tsproc_set_clock_rate_ratio(tsp, _clock_rate_ratio)	\
		(tsp)->clock_rate_ratio = (_clock_rate_ratio)

#define	tsproc_set_delay(tsp, delay)	\
	do{ (tsp)->filtered_delay = (delay);	(tsp)->filtered_delay_valid = 1;}while(0)


#endif

/**
 * Update delay in a time stamp processor using new measurements.
 * @param tsp    Pointer obtained via @ref tsproc_create().
 * @param delay  A pointer to store the new delay, may be NULL.
 * @return       0 on success, -1 when missing a measurement.
 */
int tsproc_update_delay(struct TimestampProcess *tsp, tmv_t *delay);

/**
 * Update offset in a time stamp processor using new measurements.
 * @param tsp    Pointer obtained via @ref tsproc_create().
 * @param offset A pointer to store the new offset.
 * @param weight A pointer to store the weight of the sample, may be NULL.
 * @return       0 on success, -1 when missing a measurement.
 */
int tsproc_update_offset(struct TimestampProcess *tsp, tmv_t *offset, double *weight);

/**
 * Reset a time stamp processor.
 * @param tsp    Pointer obtained via @ref tsproc_create().
 * @param full   0 to reset stored measurements (e.g. after clock was stepped),
 *               1 to reset everything (e.g. when remote clock changed).
 */
void tsproc_reset(struct TimestampProcess *tsp, int full);


#include <syslog.h>
#include "util.h"
#include "extLog.h"

#define PRINT_LEVEL_MIN LOG_EMERG
#define PRINT_LEVEL_MAX LOG_DEBUG

#if 0
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
#endif

void print(int level, char const *format, ...);

void print_set_progname(const char *name);
void print_set_tag(const char *tag);
void print_set_syslog(int value);
void print_set_level(int level);
void print_set_verbose(int value);

#if 0
#define pr_emerg(x...)   print(LOG_EMERG, x)
#define pr_alert(x...)   print(LOG_ALERT, x)
#define pr_crit(x...)    print(LOG_CRIT, x)
#define pr_err(x...)     print(LOG_ERR, x)
#define pr_warning(x...) print(LOG_WARNING, x)
#define pr_notice(x...)  print(LOG_NOTICE, x)
#define pr_info(x...)    print(LOG_INFO, x)
#define pr_debug(x...)   print(LOG_DEBUG, x)
#else
#define pr_emerg(x,...)		EXT_ERRORF(x, ##__VA_ARGS__)
#define pr_alert(x,...)			EXT_ERRORF(x, ##__VA_ARGS__)
#define pr_crit(x,...)			EXT_ERRORF(x, ##__VA_ARGS__)
#define pr_err(x,...)			EXT_ERRORF(x, ##__VA_ARGS__)
#define pr_warning(x,...)		EXT_INFOF(x, ##__VA_ARGS__)
#define pr_notice(x,...)		EXT_INFOF(x, ##__VA_ARGS__)
#define pr_info(x,...)			EXT_INFOF(x, ##__VA_ARGS__)
#define pr_debug(x,...)		EXT_DEBUGF(EXT_DBG_ON, x, ##__VA_ARGS__)
//#define pr_debug(x...)   EXT_INFOF( x)
#endif


#define PRINT_RL(l, i, x...) \
	do { \
		static time_t last = -i; \
		if (!rate_limited(i, &last)) \
			print(l, x); \
	} while (0);

/* Rate limited versions */
#define pl_emerg(i, x...)   PRINT_RL(LOG_EMERG, i, x)
#define pl_alert(i, x...)   PRINT_RL(LOG_ALERT, i, x)
#define pl_crit(i, x...)    PRINT_RL(LOG_CRIT, i, x)
#define pl_err(i, x...)     PRINT_RL(LOG_ERR, i, x)
#define pl_warning(i, x...) PRINT_RL(LOG_WARNING, i, x)
#define pl_notice(i, x...)  PRINT_RL(LOG_NOTICE, i, x)
#define pl_info(i, x...)    PRINT_RL(LOG_INFO, i, x)
#define pl_debug(i, x...)   PRINT_RL(LOG_DEBUG, i, x)


#include <inttypes.h>
#include <time.h>

/**
 * Initialize state needed when adjusting or reading the clock.
 * @param clkid A clock ID obtained using phc_open() or CLOCK_REALTIME.
 */
void clockadj_init(clockid_t clkid);

/**
 * Set clock's frequency offset.
 * @param clkid A clock ID obtained using phc_open() or CLOCK_REALTIME.
 * @param freq  The frequency offset in parts per billion (ppb).
 */
void clockadj_set_freq(clockid_t clkid, double freq);

/**
 * Read clock's frequency offset.
 * @param clkid A clock ID obtained using phc_open() or CLOCK_REALTIME.
 * @return      The frequency offset in parts per billion (ppb).
 */
double clockadj_get_freq(clockid_t clkid);

/**
 * Step clock's time.
 * @param clkid A clock ID obtained using phc_open() or CLOCK_REALTIME.
 * @param step  The time step in nanoseconds.
 */
void clockadj_step(clockid_t clkid, int64_t step);

/**
 * Set the system clock to insert/delete leap second at midnight.
 * @param leap  +1 to insert leap second, -1 to delete leap second,
 *              0 to reset the leap state.
 */
void sysclk_set_leap(int leap);

/**
 * Set the TAI offset of the system clock to have correct CLOCK_TAI.
 * @param offset The TAI-UTC offset in seconds.
 */
void sysclk_set_tai_offset(int offset);

/**
 * Read maximum frequency adjustment of the system clock (CLOCK_REALTIME).
 * @return The maximum frequency adjustment in parts per billion (ppb).
 */
int sysclk_max_freq(void);

/**
 * Mark the system clock as synchronized to let the kernel synchronize
 * the real-time clock (RTC) to it.
 */
void sysclk_set_sync(void);

#include <stdint.h>

/** Opaque type */
struct clockcheck;

/**
 * Create a new instance of a clock sanity check.
 * @param freq_limit The maximum allowed frequency offset between uncorrected
 *                   clock and the system monotonic clock in ppb.
 * @return A pointer to a new clock check on success, NULL otherwise.
 */
struct clockcheck *clockcheck_create(int freq_limit);

/**
 * Perform the sanity check on a time stamp.
 * @param cc Pointer to a clock check obtained via @ref clockcheck_create().
 * @param ts Time stamp made by the clock in nanoseconds.
 * @return Zero if ts passed the check, non-zero otherwise.
 */
int clockcheck_sample(struct clockcheck *cc, uint64_t ts);

/**
 * Inform clock check about changes in current frequency of the clock.
 * @param cc   Pointer to a clock check obtained via @ref clockcheck_create().
 * @param freq Frequency correction applied to the clock in ppb.
 */
void clockcheck_set_freq(struct clockcheck *cc, int freq);

/**
 * Inform clock check that the clock was stepped.
 * @param cc   Pointer to a clock check obtained via @ref clockcheck_create().
 * @param step Step correction applied to the clock in nanoseconds.
 */
void clockcheck_step(struct clockcheck *cc, int64_t step);

/**
 * Destroy a clock check.
 * @param cc Pointer to a clock check obtained via @ref clockcheck_create().
 */
void clockcheck_destroy(struct clockcheck *cc);



#include "missing.h"

/**
 * Opens a PTP hardware clock device.
 * @param phc The device to open.
 * @return A valid clock ID on success, CLOCK_INVALID otherwise.
 */
clockid_t phc_open(char *phc);

/**
 * Closes a PTP hardware clock device.
 *
 * @param clkid A clock ID obtained using phc_open().
 */
void phc_close(clockid_t clkid);

/**
 * Query the maximum frequency adjustment of a PTP hardware clock device.
 *
 * @param clkid A clock ID obtained using phc_open().
 *
 * @return The clock's maximum frequency adjustment in parts per billion.
 */
int phc_max_adj(clockid_t clkid);

/**
 * Checks whether the given PTP hardware clock device supports PPS output.
 *
 * @param clkid A clock ID obtained using phc_open().
 *
 * @return Zero if PPS output is not supported by the clock, non-zero
 * otherwise.
 */
int phc_has_pps(clockid_t clkid);



/*  Implements the system offset estimation method */
#include <stdint.h>

enum {
	SYSOFF_SUPPORTED,
	SYSOFF_COMPILE_TIME_MISSING,
	SYSOFF_RUN_TIME_MISSING,
};

/**
 * Check to see if the PTP_SYS_OFFSET ioctl is supported.
 * @param fd  An open file descriptor to a PHC device.
 * @return  One of the SYSOFF_ enumeration values.
 */
int sysoff_probe(int fd, int n_samples);

/**
 * Measure the offset between a PHC and the system time.
 * @param fd         An open file descriptor to a PHC device.
 * @param n_samples  The number of consecutive readings to make.
 * @param result     The estimated offset in nanoseconds.
 * @param ts         The system time corresponding to the 'result'.
 * @param delay      The delay in reading of the clock in nanoseconds.
 * @return  One of the SYSOFF_ enumeration values.
 */
int sysoff_measure(int fd, int n_samples,
		   int64_t *result, uint64_t *ts, int64_t *delay);


/** Opaque type */
struct stats;

/**
 * Create a new instance of statistics.
 * @return A pointer to a new stats on success, NULL otherwise.
 */
struct stats *stats_create(void);

/**
 * Destroy an instance of stats.
 * @param servo Pointer to stats obtained via @ref stats_create().
 */
void stats_destroy(struct stats *stats);

/**
 * Add a new value to the stats.
 * @param stats Pointer to stats obtained via @ref stats_create().
 * @param value The measured value.
 */
void stats_add_value(struct stats *stats, double value);

/**
 * Get the number of values collected in the stats so far.
 * @param stats Pointer to stats obtained via @ref stats_create().
 * @return      The number of values.
 */
unsigned int stats_get_num_values(struct stats *stats);

struct stats_result {
	double min;
	double max;
	double max_abs;
	double mean;
	double rms;
	double stddev;
};

/**
 * Obtain the results of the calculated statistics.
 * @param stats        Pointer to stats obtained via @ref stats_create().
 * @param stats_result Pointer to stats_result to store the results.
 * @return             Zero on success, non-zero if no values were added.
 */
int stats_get_result(struct stats *stats, struct stats_result *result);

/**
 * Reset all statistics.
 * @param stats Pointer to stats obtained via @ref stats_create().
 */
void stats_reset(struct stats *stats);

#define	PTP_GET_SYS_TIME_NOW(clockId, timeSpecNow)		\
			clock_gettime(clockId, (timeSpecNow) )


/*
* the timevalue can be jump forwards and backwards as the system time-of-day clock is changed, including by NTP
*/
#define	PTP_GET_SYS_TIME_REALTIME( timeSpecNow)		\
			PTP_GET_SYS_TIME_NOW(CLOCK_REALTIME, (timeSpecNow) )


/*
* monotonic clock, is wall time, the time is continual, no matter time_of_day is changed or not;
* so it is used to measure how long one event has happened
*/
#define	PTP_GET_SYS_TIME_MONOTONIC( timeSpecNow)		\
			PTP_GET_SYS_TIME_NOW(CLOCK_MONOTONIC, (timeSpecNow) )
			

#endif

