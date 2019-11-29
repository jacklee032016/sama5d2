/**
 * @file clock.h
 * @brief Implements a PTP clock.
 * @note Copyright (C) 2011 Richard Cochran <richardcochran@gmail.com>
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
#ifndef HAVE_CLOCK_H
#define HAVE_CLOCK_H

#include "ptpCompact.h"
#include "ptpProtocol.h"
#include "ptpImplements.h"

#include "ptpConfig.h"
#include "servo.h"
#include "tmv.h"


struct ptp_message; /*forward declaration*/

/** Opaque type. */
struct PtpClock;

enum CLOCK_TYPE {
	CLOCK_TYPE_ORDINARY   = 0x8000,
	CLOCK_TYPE_BOUNDARY   = 0x4000,
	CLOCK_TYPE_P2P        = 0x2000,
	CLOCK_TYPE_E2E        = 0x1000,
	CLOCK_TYPE_MANAGEMENT = 0x0800,
};




/**
 * Obtain the clockClass attribute from a clock.
 * @param c  The clock instance.
 * @return   The value of the clock's class.
 */
UInteger8 clock_class(struct PtpClock *c);



/**
 * Obtains the clock's data set comparison function.
 * @param c  The clock instance.
 * @return   A pointer to the data set comparison function, without fail.
 */
int (*clock_dscmp(struct PtpClock *c))(struct dataset *a, struct dataset *b);

/**
 * Obtains the required time stamping mode.
 * @param c  The clock instance.
 * @return   The value of required time stamping mode, which is a bit mask
 *           of SOF_TIMESTAMPING_ flags.
 */
int clock_required_modes(struct PtpClock *c);

/**
 * Create a clock instance. There can only be one clock in any system,
 * so subsequent calls will destroy the previous clock instance.
 *
 * @param type         Specifies which type of clock to create.
 * @param config       Pointer to the configuration database.
 * @param phc_device   PTP hardware clock device to use. Pass NULL for automatic
 *                     selection based on the network interface.
 * @return             A pointer to the single global clock instance.
 */
struct PtpClock *clock_create(enum CLOCK_TYPE type, struct PtpConfig *config, const char *phc_device);

/**
 * Obtains a clock's default data set.
 * @param c  The clock instance.
 * @return   A pointer to the data set of the clock.
 */
struct dataset *clock_default_ds(struct PtpClock *c);

/**
 * Free all of the resources associated with a clock.
 * @param c  The clock instance.
 */
void clock_destroy(struct PtpClock *c);



/**
 * Provide the follow_up info TLV from a slave port.
 * @param c  The clock instance.
 * @param f  Pointer to the TLV.
 */
void clock_follow_up_info(struct PtpClock *c, struct follow_up_info_tlv *f);



/**
 * Manage the clock according to a given message.
 * @param c    The clock instance.
 * @param p    The port on which the message arrived.
 * @param msg  A management message.
 * @return     One if the management action caused a change that
 *             implies a state decision event, zero otherwise.
 */
//int clock_manage(struct PtpClock *c, struct PtpPort *p, struct ptp_message *msg);

/**
 * Send notification about an event to all subscribers.
 * @param c      The clock instance.
 * @param msg    The PTP message to send, in network byte order.
 * @param event  The event that occured.
 */
void clock_send_notification(struct PtpClock *c, struct ptp_message *msg, enum NOTIFICATION event);

/**
 * Construct and send notification to subscribers about an event that
 * occured on the clock.
 * @param c      The clock instance.
 * @param event  The identification of the event.
 */
void clock_notify_event(struct PtpClock *c, enum NOTIFICATION event);



/**
 * Provide a data point to estimate the path delay.
 * @param c           The clock instance.
 * @param req         The transmission time of the delay request message.
 * @param rx          The reception time of the delay request message,
 *                    as reported in the delay response message, including
 *                    correction.
 */
void ptpClockPathDelay(struct PtpClock *c, tmv_t req, tmv_t rx);

/**
 * Provide the estimated peer delay from a slave port.
 * @param c           The clock instance.
 * @param ppd         The peer delay as measured on a slave port.
 * @param req         The transmission time of the pdelay request message.
 * @param rx          The reception time of the pdelay request message.
 * @param nrr         The neighbor rate ratio as measured on a slave port.
 */
void clock_peer_delay(struct PtpClock *c, tmv_t ppd, tmv_t req, tmv_t rx,
		      double nrr);


/**
 * Poll for events and dispatch them.
 * @param c A pointer to a clock instance obtained with clock_create().
 * @return  Zero on success, non-zero otherwise.
 */
int clock_poll(struct PtpClock *c);



/**
 * Switch to a new PTP Hardware Clock, for use with the "jbod" mode.
 * @param c          The clock instance.
 * @param phc_index  The index of the PHC device to use.
 * @return           Zero on success, non-zero otherwise.
 */
int clock_switch_phc(struct PtpClock *c, int phc_index);

/**
 * Provide a data point to synchronize the clock.
 * @param c            The clock instance to synchronize.
 * @param ingress      The ingress time stamp on the sync message.
 * @param origin       The reported transmission time of the sync message,
                       including any corrections.
 * @param correction1  The correction field of the sync message.
 * @param correction2  The correction field of the follow up message.
 *                     Pass zero in the case of one step operation.
 * @return             The state of the clock's servo.
 */
enum servo_state ptpClockSynchronize(struct PtpClock *c, tmv_t ingress, tmv_t origin);

/**
 * Inform a slaved clock about the master's sync interval.
 * @param c  The clock instance.
 * @param n  The logarithm base two of the sync interval.
 */
void clock_sync_interval(struct PtpClock *c, int n);





/**
 * Perform a sanity check on a time stamp made by a clock.
 * @param c  The clock instance.
 * @param ts The time stamp.
 */
void clock_check_ts(struct PtpClock *c, uint64_t ts);

/**
 * Obtain ratio between master's frequency and current clock frequency.
 * @param c  The clock instance.
 * @return   The rate ratio, 1.0 is returned when not known.
 */
double clock_rate_ratio(struct PtpClock *c);


int clockInitFpga(void);



#endif
