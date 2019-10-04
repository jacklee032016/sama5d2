/*
* for PHC 2 Sys program
*/

#ifndef	__PHC_2_SYS_H__
#define	__PHC_2_SYS_H__

/**
 * @file phc2sys.c
 * @brief Utility program to synchronize two clocks via a PPS.
 * @note Copyright (C) 2012 Richard Cochran <richardcochran@gmail.com>
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
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <net/if.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/pps.h>
#include <linux/ptp_clock.h>

#include "ptpCompact.h"
#include "ptpProtocol.h"
#include "missing.h"
#include "servo.h"
#include "sk.h"
#include "util.h"
#include "ptpConfig.h"


#define	PHC_SYS_DBG_DATASET( datasetId)	\
			EXT_DEBUGF(PHC_SYS_DEBUG, "DataSet ID " #datasetId ":0x%x", datasetId )

#define KP 0.7
#define KI 0.3

#define PHC_PPS_OFFSET_LIMIT				10000000	/* 10**7 */

#define PMC_UPDATE_INTERVAL				(60 * NS_PER_SEC)
#define PMC_SUBSCRIBE_DURATION			180	/* 3 minutes */
/* Note that PMC_SUBSCRIBE_DURATION has to be longer than
 * PMC_UPDATE_INTERVAL otherwise subscription will time out before it is
 * renewed.
 */

/* clock of local(realtime, monolithic) or ptp clock (from ptp4l) */
struct PhcClock
{
	LIST_ENTRY(PhcClock)	list;
	LIST_ENTRY(PhcClock)	dst_list;
	
	clockid_t				clkid;
	int					phc_index;
	
	int					sysoff_supported;
	int					is_utc;
	int					dest_only;
	int					state;
	int					new_state;
	int					sync_offset;
	int					leap_set;
	int					utc_offset_set;
	
	struct servo			*servo;
	enum servo_state		servo_state;
	
	char					*device;

	const char 			*source_label;
	
	struct stats			*offset_stats;
	struct stats			*freq_stats;
	struct stats			*delay_stats;
	
	struct clockcheck		*sanity_check;
};

struct PhcPort
{
	LIST_ENTRY(PhcPort)		list;
	unsigned int				number;
	int						state;
	struct PhcClock			*clock;
};

struct PhcNode
{
	unsigned int							stats_max_count;
	int									sanity_freq_limit;
	enum servo_type						servoType;
	
	int									phc_readings;			/* how many times of master clock reading per one slave clock update */
	double								phc_interval;
	int									sync_offset;				/* offset from ptp pmc */
	int									forced_sync_offset;
	int									utc_offset_traceable;
	int									leap;					/* from ptp pmc; -1 for LEAP_59; 1 for LEAP_61 */
	int									kernel_leap;
	
	struct PtpMgmtClient					*pmc;
	int									pmc_ds_requested;	/* is data received from pmc */
	uint64_t								pmc_last_update;	/* timestamp of last update */
	
	int									state_changed;
	int									clock_identity_set;
	struct ClockIdentity					clock_identity;
	
	LIST_HEAD(port_head, PhcPort)			ports;
	LIST_HEAD(clock_head, PhcClock)		clocks;
	LIST_HEAD(dst_clock_head, PhcClock)	dst_clocks;	/* dest clocks */
	
	struct PhcClock						*master;	/* source clock */
};



int psysPmcInit(struct PtpConfig *cfg, struct PhcNode *node);

void psysPmcWaitEvents(struct PhcNode *node);
int psysPmcWaitPtpdReady(struct PhcNode *node, int timeout);
int psysPmcGetUtcOffset(struct PhcNode *node, int timeout);
int psysPmcGetPortProperties(struct PhcNode *node, int timeout, unsigned int port, int *state, int *tstamping, char *iface);
int psysPmcUpdate(struct PhcNode *node, int subscribe);


int psysAutoInitPortsByPmc(struct PhcNode *node, int add_rt /*add CLOCK_REALTIME */);



struct PhcClock *psysClockAdd(struct PhcNode *node, char *device);
void psysClockUpdate(struct PhcNode *node, struct PhcClock *clock, int64_t offset, uint64_t ts, int64_t delay);


struct PhcPort *psysPortAdd(struct PhcNode *node, unsigned int number, char *device);
struct PhcPort *psysPortGet(struct PhcNode *node, unsigned int number);

int psysNormalizeState(int state);

int psysReadPhc(clockid_t clkid, clockid_t sysclk, int readings, int64_t *offset, uint64_t *ts, int64_t *delay);

int psysPpsLoop(struct PhcNode *node, struct PhcClock *clock, int fd);

void psysReconfigure(struct PhcNode *node);


extern	struct PtpConfig *phc2sys_config;

#define	PSYS_ENABLE_SOFTWATR_TIMESTAMP			1

#define	PSYS_DEBUG_PMC		EXT_DBG_ON

#endif

