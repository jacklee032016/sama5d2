/**
 * @file port.h
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
#ifndef HAVE_PORT_H
#define HAVE_PORT_H

#include "ptpCompact.h"
#include "ptpProtocol.h"
#include "ptpImplements.h"

//#define	PORT_STR_FORMAT		"PORT#%d: "
#define	PORT_STR_FORMAT		"PORT#%s: "

/* forward declarations */
struct PtpInterface;
struct PtpClock;

/** Opaque type. */
struct PtpPort;

/* 
* Naming rule: 
*     pktPortXXX, name can be used by other module
*     portXXX, name used in pktPort module itself
*     _XXX, name for static functions and variables
*/


/**
 * Close a port and free its associated resources. After this call
 * returns, @a port is no longer a valid port instance.
 *
 * @param port A pointer previously obtained via port_open().
 */
void portDestory(struct PtpPort *port);

/**
 * Computes the 'best' foreign master discovered on a port. This has
 * the side effect of updating the 'dataset' field of the returned
 * foreign master.
 *
 * @param port A pointer previously obtained via port_open().
 * @return A pointer to the port's best foreign master, or NULL.
 */
struct foreign_clock *port_compute_best(struct PtpPort *port);



/**
 * Forward a message on a given port.
 * @param port    A pointer previously obtained via port_open().
 * @param msg     The message to send. Must be in network byte order.
 * @return        Zero on success, non-zero otherwise.
 */
int port_forward(struct PtpPort *p, struct ptp_message *msg);

/**
 * Forward a message on a given port to the address stored in the message.
 * @param port    A pointer previously obtained via port_open().
 * @param msg     The message to send. Must be in network byte order.
 * @return        Zero on success, non-zero otherwise.
 */
int port_forward_to(struct PtpPort *p, struct ptp_message *msg);

/**
 * Prepare message for transmission and send it to a given port. Note that
 * a single message cannot be sent several times using this function, that
 * would lead to corrupted data being sent. Use msg_pre_send and
 * port_forward if you need to send single message to several ports.
 * @param p        A pointer previously obtained via port_open().
 * @param msg      The message to send.
 * @param event    One of the @ref transport_event enumeration values.
 */
int port_prepare_and_send(struct PtpPort *p, struct ptp_message *msg,
			  enum transport_event event);


/**
 * Manage a port according to a given message.
 * @param p        A pointer previously obtained via port_open().
 * @param ingress  The port on which 'msg' was received.
 * @param msg      A management message.
 * @return         1 if the message was responded to, 0 if it did not apply
 *                 to the port, -1 if it was invalid.
 */
int port_manage(struct PtpPort *p, struct PtpPort *ingress, struct ptp_message *msg);

/**
 * Send a management error status message.
 * @param pid       The id of the responding port.
 * @param ingress   Port on which the 'req' was received.
 * @param req       The management message which triggered the error.
 * @param error_id  One of the management error ID values.
 * @return          Zero on success, non-zero otherwise.
 */
int port_management_error(struct PortIdentity pid, struct PtpPort *ingress,
			  struct ptp_message *req, Enumeration16 error_id);

/**
 * Allocate a reply to a management message.
 *
 * Messages are reference counted, and newly allocated messages have a
 * reference count of one. Allocated messages are freed using the
 * function @ref msg_put().
 *
 * @param pid      The id of the responding port.
 * @param ingress  The port on which 'req' was received.
 * @param req      A management message.
 * @return         Pointer to a message on success, NULL otherwise.
 */
struct ptp_message *port_management_reply(struct PortIdentity pid,
					  struct PtpPort *ingress,
					  struct ptp_message *req);

/**
 * Allocate a standalone reply management message.
 *
 * See note in @ref port_management_reply description about freeing the
 * message. Also note that the constructed message does not have
 * targetPortIdentity and sequenceId filled.
 *
 * @param pid      The id of the responding port.
 * @param port     The port to which the message will be sent.
 * @return         Pointer to a message on success, NULL otherwise.
 */
struct ptp_message *port_management_notify(struct PortIdentity pid,
					   struct PtpPort *port);

/**
 * Construct and send notification to subscribers about an event that
 * occured on the port.
 * @param p        The port.
 * @param event    The identification of the event.
 */
void port_notify_event(struct PtpPort *p, enum NOTIFICATION event);

/**
 * Open a network port.
 * @param phc_index     The PHC device index for the network device.
 * @param timestamping  The timestamping mode for this port.
 * @param number	An arbitrary number assigned to this port.
 * @param interface     The interface data
 * @param clock         A pointer to the system PTP clock.
 * @return A pointer to an open port on success, or NULL otherwise.
 */
struct PtpPort *portCreate(int phc_index,
		       enum timestamp_type timestamping,
		       int number,
		       struct PtpInterface *interface,
		       struct PtpClock *clock);


/**
 * Update a port's current state based on a given event.
 * @param p        A pointer previously obtained via port_open().
 * @param event    One of the @a PORT_EVENT codes.
 * @param mdiff    Whether a new master has been selected.
 * @return         One (1) if the port state has changed, zero otherwise.
 */
int portStateUpdate(struct PtpPort *p, enum PORT_EVENT event, int mdiff);



/**
 * Utility function for setting or resetting a file descriptor timer.
 *
 * This function sets the timer 'fd' to the value M(2^N), where M is
 * the value of the 'scale' parameter and N in the value of the
 * 'log_seconds' parameter.
 *
 * Passing both 'scale' and 'log_seconds' as zero disables the timer.
 *
 * @param fd A file descriptor previously opened with timerfd_create(2).
 * @param scale The multiplicative factor for the timer.
 * @param log_seconds The exponential factor for the timer.
 * @return Zero on success, non-zero otherwise.
 */
int set_tmo_log(int fd, unsigned int scale, int log_seconds);

/**
 * Utility function for setting a file descriptor timer.
 *
 * This function sets the timer 'fd' to a random value between M * 2^N and
 * (M + S) * 2^N, where M is the value of the 'min' parameter, S is the value
 * of the 'span' parameter, and N in the value of the 'log_seconds' parameter.
 *
 * @param fd A file descriptor previously opened with timerfd_create(2).
 * @param min The minimum value for the timer.
 * @param span The span value for the timer. Must be a positive value.
 * @param log_seconds The exponential factor for the timer.
 * @return Zero on success, non-zero otherwise.
 */
int set_tmo_random(int fd, int min, int span, int log_seconds);

/**
 * Utility function for setting or resetting a file descriptor timer.
 *
 * This function sets the timer 'fd' to the value of the 'seconds' parameter.
 *
 * Passing 'seconds' as zero disables the timer.
 *
 * @param fd A file descriptor previously opened with timerfd_create(2).
 * @param seconds The timeout value for the timer.
 * @return Zero on success, non-zero otherwise.
 */
int set_tmo_lin(int fd, int seconds);

/**
 * Sets port's fault file descriptor timer.
 * Passing both 'scale' and 'log_seconds' as zero disables the timer.
 *
 * @param fd		A port instance.
 * @param scale		The multiplicative factor for the timer.
 * @param log_seconds	The exponential factor for the timer.
 * @return		Zero on success, non-zero otherwise.
 */
int port_set_fault_timer_log(struct PtpPort *port,
			     unsigned int scale, int log_seconds);

/**
 * Sets port's fault file descriptor timer.
 * Passing 'seconds' as zero disables the timer.
 *
 * @param fd		A port instance.
 * @param seconds	The timeout value for the timer.
 * @return		Zero on success, non-zero otherwise.
 */
int port_set_fault_timer_lin(struct PtpPort *port, int seconds);


/**
 * Fills passed in struct fault_interval with the value associated to a
 * port and fault type.
 *
 * @param port        A port instance.
 * @param ft          Fault type.
 * @param i           Pointer to the struct which will be filled in.
 */
void fault_interval(struct PtpPort *port, enum fault_type ft,
		    struct fault_interval *i);

/**
 * Release all of the memory in the TC transmit descriptor cache.
 */
void tc_cleanup(void);

#endif
