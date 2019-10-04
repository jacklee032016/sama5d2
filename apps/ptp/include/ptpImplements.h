/*
 *
 */

#ifndef __PTP_IMPLEMENTS_H__
#define __PTP_IMPLEMENTS_H__

#include <stdio.h>
#include <sys/queue.h>
#include <time.h>
#include <asm/byteorder.h>

#include "ptpProtocol.h"
#include "tmv.h"

struct tlv_extra {
	TAILQ_ENTRY(tlv_extra) list;
	struct TLV *tlv;
	union {
		struct mgmt_clock_description cd;
		struct nsm_resp_tlv_foot *foot;
	};
};

/**
 * Allocates a new tlv_extra structure.
 * @return  Pointer to a new structure on success or NULL otherwise.
 */
struct tlv_extra *tlv_extra_alloc(void);

/**
 * Release all of the memory in the tlv_extra cache.
 */
void tlv_extra_cleanup(void);

/**
 * Frees a tlv_extra structure.
 * @param extra  Pointer to the structure to free.
 */
void tlv_extra_recycle(struct tlv_extra *extra);

/**
 * Converts recognized value sub-fields into host byte order.
 * @param extra  TLV descriptor pointing to the protocol data.
 * @return Zero if successful, otherwise non-zero
 */
int tlv_post_recv(struct tlv_extra *extra);

/**
 * Converts recognized value sub-fields into network byte order.
 * @param tlv Pointer to a Type Length Value field.
 * @param extra Additional struct containing tlv data to send, can be
 * NULL.
 */
void tlv_pre_send(struct TLV *tlv, struct tlv_extra *extra);


enum timestamp_type {
	TS_SOFTWARE,
	TS_HARDWARE,
	TS_LEGACY_HW,
	TS_ONESTEP,
	TS_P2P1STEP,
};

struct hw_timestamp {
	enum timestamp_type type;
	tmv_t ts;
	tmv_t sw;
};

enum controlField {
	CTL_SYNC,
	CTL_DELAY_REQ,
	CTL_FOLLOW_UP,
	CTL_DELAY_RESP,
	CTL_MANAGEMENT,
	CTL_OTHER,
};

struct ptp_message
{
	union
	{
		struct ptp_header				header;
		struct announce_msg			announce;
		struct sync_msg				sync;
		struct delay_req_msg			delay_req;
		struct follow_up_msg			follow_up;
		struct delay_resp_msg			delay_resp;
		struct pdelay_req_msg			pdelay_req;
		struct pdelay_resp_msg		pdelay_resp;
		struct pdelay_resp_fup_msg		pdelay_resp_fup;
		struct signaling_msg			signaling;
		struct management_msg		management;
		struct message_data			data;
	} PACKED;
	
	/**/
	int	tail_room;
	int refcnt;
	
	TAILQ_ENTRY(ptp_message)		list;

	struct {
		/**
		 * Contains the time stamp from the packet data in a
		 * native binary format for the host machine. The
		 * exact source of the time stamp's value depends on
		 * the message type:
		 *
		 * - announce    originTimestamp
		 * - follow_up   preciseOriginTimestamp
		 * - sync        originTimestamp
		 * - delay_req   originTimestamp
		 * - pdelay_resp     requestReceiptTimestamp
		 * - pdelay_resp_fup responseOriginTimestamp
		 */
		struct LocalTimeStamp				pdu;
		/**
		 * Approximate ingress time stamp using the relative
		 * CLOCK_MONOTONIC. Used to determine when announce
		 * messages have expired.
		 */
		struct timespec					host;
	} ts;
	
	/* Contains the ingress time stamp obtained by the SO_TIMESTAMPING socket option */
	struct hw_timestamp					hwts;
	
	/* Contains the address this message was received from or should be sent to */
	struct address					address;
	
	/* List of TLV descriptors.  Each item in the list contains pointers to the appended TLVs */
	TAILQ_HEAD(tlv_list, tlv_extra)		tlv_list;
};

/**
 * Obtain the action field from a management message.
 * @param m  A management message.
 * @return   The value of the action field.
 */
static inline uint8_t management_action(struct ptp_message *m)
{
	return m->management.flags & 0x0f;
}

/**
 * Test a given bit in a message's flag field.
 * @param m      Message to test.
 * @param index  Index into flag field, either 0 or 1.
 * @param bit    Bit mask of one bit to test.
 * @return       One if bit is set, zero otherwise.
 */
static inline Boolean field_is_set(struct ptp_message *m, int index, Octet bit)
{
	return m->header.flagField[index] & bit ? TRUE : FALSE;
}

/**
 * Append a new TLV onto a message for transmission.
 *
 * This is a high level API designed for the transmit path.  The
 * function allocates a new descriptor, initializes its .tlv field,
 * and ensures that the TLV will fit into the message buffer.  This
 * function increments the message length field by 'length' before
 * returning.
 *
 * @param msg     A message obtained using msg_allocate().  At a mininum,
 *                the message type and length fields must set by the caller.
 * @param length  The length of the TLV to append.
 * @return        A pointer to a TLV descriptor on success or NULL otherwise.
 */
struct tlv_extra *msg_tlv_append(struct ptp_message *msg, int length);

/**
 * Place a TLV descriptor into a message's list of TLVs.
 *
 * @param msg     A message obtained using msg_allocate().
 * @param extra   The TLV to be added to the list.
 */
void msg_tlv_attach(struct ptp_message *msg, struct tlv_extra *extra);

/*
 * Return the number of TLVs attached to a message.
 * @param msg  A message obtained using @ref msg_allocate().
 * @return     The number of attached TLVs.
 */
int msg_tlv_count(struct ptp_message *msg);

/**
 * Obtain the transportSpecific field from a message.
 * @param m  Message to test.
 * @return   The value of the transportSpecific field. Note that the
 *           value is returned unshifted, in the upper nibble.
 */
static inline UInteger8 msg_transport_specific(struct ptp_message *m)
{
	return m->header.tsmt & 0xf0;
}

/**
 * Obtain the message type.
 * @param m  Message to test.
 * @return   The value of the messageType field.
 */
static inline int msg_type(struct ptp_message *m)
{
	return m->header.tsmt & 0x0f;
}

/**
 * Allocate a new message instance.
 *
 * Messages are reference counted, and newly allocated messages have a
 * reference count of one. Allocated messages are freed using the
 * function @ref msg_put().
 *
 * @return Pointer to a message on success, NULL otherwise.
 */
struct ptp_message *msg_allocate(void);

/**
 * Release all of the memory in the message cache.
 */
void msg_cleanup(void);

/**
 * Duplicate a message instance.
 *
 * This function accepts a message in network byte order and returns a
 * duplicate in host byte.
 *
 * Messages are reference counted, and newly allocated messages have a
 * reference count of one. Allocated messages are freed using the
 * function @ref msg_put().
 *
 * @param msg  A message obtained using @ref msg_allocate().
 *             The passed message must be in network byte order, not
 *             having been passed to @ref ptpMsgReceive().
 *
 * @return     Pointer to a message on success, NULL otherwise.
 *             The returned message will be in host byte order, having
 *             been passed to @ref ptpMsgReceive().
 */
struct ptp_message *msg_duplicate(struct ptp_message *msg, int cnt);

/**
 * Obtain a reference to a message, increasing its reference count by one.
 * @param m A message obtained using @ref msg_allocate().
 */
void msg_get(struct ptp_message *m);

/**
 * Process messages after reception.
 * @param m    A message obtained using @ref msg_allocate().
 * @param cnt  The size of 'm' in bytes.
 * @return   Zero on success, non-zero if the message is invalid.
 */
int ptpMsgReceive(struct ptp_message *m, int cnt);

/**
 * Prepare messages for transmission.
 * @param m  A message obtained using @ref msg_allocate().
 * @return   Zero on success, non-zero if the message is invalid.
 */
int msg_pre_send(struct ptp_message *m);


const char *ptpMsgTypeString(int type);
void ptpMsgDebug(struct ptp_message *m, FILE *fp);

const char *ptpPortStateString(int state);
const char *ptpPortEventString(int event);


/**
 * Release a reference to a message, decreasing its reference count by one.
 * @param m A message obtained using @ref msg_allocate().
 */
void msg_put(struct ptp_message *m);

/**
 * Test whether an event message received a valid SO_TIMESTAMPING time stamp.
 * @param m  Message to test.
 * @return   One if the message is an event without a time stamp, zero otherwise.
 */
int msg_sots_missing(struct ptp_message *m);

/**
 * Test whether a message has a valid SO_TIMESTAMPING time stamp.
 * @param m  Message to test.
 * @return   One if the message has a valid time stamp, zero otherwise.
 */
static inline int msg_sots_valid(struct ptp_message *m)
{
	return !tmv_is_zero(m->hwts.ts);
}

/**
 * Test whether a message is a unicast message.
 * @param m  Message to test.
 * @return   One if the message is unicast, zero otherwise.
 */
static inline Boolean msg_unicast(struct ptp_message *m)
{
	return field_is_set(m, 0, UNICAST);
}

/**
 * Work around buggy 802.1AS switches.
 */
extern int assume_two_step;

/**
 * Test whether a message is one-step message.
 * @param m  Message to test.
 * @return   One if the message is a one-step, zero otherwise.
 */
static inline Boolean one_step(struct ptp_message *m)
{
	if (assume_two_step)
		return 0;
	return !field_is_set(m, 0, TWO_STEP);
}

/**
 * Convert a 64 bit word into network byte order.
 */
static inline int64_t host2net64(int64_t val)
{
	return __cpu_to_be64(val);
}

/**
 * Convert a 64 bit word into host byte order.
 */
static inline int64_t net2host64(int64_t val)
{
	return __be64_to_cpu(val);
}



#include "port.h"

#define FOREIGN_MASTER_THRESHOLD 2

struct foreign_clock {
	/**
	 * Pointer to next foreign_clock in list.
	 */
	LIST_ENTRY(foreign_clock) list;

	/**
	 * A list of received announce messages.
	 *
	 * The data set field, foreignMasterPortIdentity, is the
	 * sourcePortIdentity of the first message.
	 */
	TAILQ_HEAD(messages, ptp_message) messages;

	/**
	 * Number of elements in the message list,
	 * aka foreignMasterAnnounceMessages.
	 */
	unsigned int n_messages;

	/**
	 * Pointer to the associated port.
	 */
	struct PtpPort *port;

	/**
	 * Contains the information from the latest announce message
	 * in a form suitable for comparision in the BMCA.
	 */
	struct dataset dataset;
};


/* master table implementation */
enum unicast_state {
	UC_WAIT,
	UC_HAVE_ANN,
	UC_NEED_SYDY,
	UC_HAVE_SYDY,
};

enum unicast_event {
	UC_EV_GRANT_ANN,
	UC_EV_SELECTED,
	UC_EV_GRANT_SYDY,
	UC_EV_UNSELECTED,
	UC_EV_CANCEL,
};

struct unicast_master_address
{
	STAILQ_ENTRY(unicast_master_address) list;
	
	struct PortIdentity			portIdentity;
	enum transport_type		type;
	enum unicast_state		state;
	struct address			address;
	
	unsigned int				granted;
	unsigned int				sydymsk;
	time_t					renewal_tmo;
};

struct unicast_master_table
{
	STAILQ_HEAD(addrs_head, unicast_master_address) addrs;
	
	STAILQ_ENTRY(unicast_master_table)			list;
	Integer8										logQueryInterval;
	int											table_index;
	int											count;
	int											port;
	
	/* for use with P2P delay mechanism: */
	struct unicast_master_address					peer_addr;
	char											*peer_name;
};


#include <time.h>
#include <inttypes.h>

struct PtpConfig;
struct PtpInterface;

struct transport;

int transport_close(struct transport *t, struct FdArray *fda);

int transport_open(struct transport *t, struct PtpInterface *iface,
		   struct FdArray *fda, enum timestamp_type tt);

int transport_recv(struct transport *t, int fd, struct ptp_message *msg);

/**
 * Sends the PTP message using the given transport. The message is sent to
 * the default (usually multicast) address, any address field in the
 * ptp_message itself is ignored.
 * @param t	The transport.
 * @param fda	The array of descriptors filled in by transport_open.
 * @param event	One of the @ref transport_event enumeration values.
 * @param msg	The message to send.
 * @return	Number of bytes send, or negative value in case of an error.
 */
int transport_send(struct transport *t, struct FdArray *fda,
		   enum transport_event event, struct ptp_message *msg);

/**
 * Sends the PTP message using the given transport. The message is sent to
 * the address used for p2p delay measurements (usually a multicast
 * address), any address field in the ptp_message itself is ignored.
 * @param t	The transport.
 * @param fda	The array of descriptors filled in by transport_open.
 * @param event	One of the @ref transport_event enumeration values.
 * @param msg	The message to send.
 * @return	Number of bytes send, or negative value in case of an error.
 */
int transport_peer(struct transport *t, struct FdArray *fda,
		   enum transport_event event, struct ptp_message *msg);

/**
 * Sends the PTP message using the given transport. The address has to be
 * provided in the address field of the message.
 * @param t	The transport.
 * @param fda	The array of descriptors filled in by transport_open.
 * @param event	One of the @ref transport_event enumeration values.
 * @param msg	The message to send. The address of the destination has to
 *		be set in the address field.
 * @return	Number of bytes send, or negative value in case of an error.
 */
int transport_sendto(struct transport *t, struct FdArray *fda,
		     enum transport_event event, struct ptp_message *msg);

/**
 * Fetches the transmit time stamp for a PTP message that was sent
 * with the TRANS_DEFER_EVENT flag.
 *
 * @param fda	The array of descriptors filled in by transport_open.
 * @param msg	The message previously sent using transport_send(),
 *              transport_peer(), or transport_sendto().
 * @return	Zero on success, or negative value in case of an error.
 */
int transport_txts(struct FdArray *fda,
		   struct ptp_message *msg);

/**
 * Returns the transport's type.
 */
enum transport_type TransportType(struct transport *t);

#define TRANSPORT_ADDR_LEN 16

/**
 * Gets the transport's physical address.
 * @param t    The transport.
 * @param addr The address will be written to this buffer.
 * @return     The number of bytes written to the buffer. Will be 0-16
 *             bytes
 */
int transport_physical_addr(struct transport *t, uint8_t *addr);

/**
 * Gets the transport's protocol address.
 * @param t    The transport.
 * @param addr The address will be written to this buffer.
 * @return     The number of bytes written to the buffer. Will be 0-16
 *             bytes
 */
int transport_protocol_addr(struct transport *t, uint8_t *addr);

/**
 * Allocate an instance of the specified transport.
 * @param config Pointer to the configuration database.
 * @param type  Which transport to obtain.
 * @return      Pointer to a transport instance on success, NULL otherwise.
 */
struct transport *transport_create(struct PtpConfig *cfg,
				   enum transport_type type);

/**
 * Free an instance of a transport.
 * @param t Pointer obtained by calling transport_create().
 */
void transport_destroy(struct transport *t);


struct transport *uds_transport_create(void);
struct transport *udp6_transport_create(void);
struct transport *udp_transport_create(void);
struct transport *raw_transport_create(void);


/* Ptp Management Client */

#define BAD_ACTION   -1
#define BAD_ID       -1
#define AMBIGUOUS_ID -2
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define P41 ((double)(1ULL << 41))

struct PtpMgmtClient;

struct PtpMgmtClient *pmc_create(struct PtpConfig *cfg, enum transport_type transport_type,
		       const char *iface_name, UInteger8 boundary_hops,
		       UInteger8 domain_number, UInteger8 transport_specific,
		       int zero_datalen);

void pmc_destroy(struct PtpMgmtClient *pmc);

int pmc_get_transport_fd(struct PtpMgmtClient *pmc);

int pmc_send_get_action(struct PtpMgmtClient *pmc, int id);

int pmc_send_set_action(struct PtpMgmtClient *pmc, int id, void *data, int datasize);

struct ptp_message *pmc_recv(struct PtpMgmtClient *pmc);

int pmc_target(struct PtpMgmtClient *pmc, struct PortIdentity *pid);
void pmc_target_port(struct PtpMgmtClient *pmc, UInteger16 portNumber);
void pmc_target_all(struct PtpMgmtClient *pmc);

const char *pmc_action_string(int action);
int pmc_do_command(struct PtpMgmtClient *pmc, char *str);


/* BMC: Best Master Clock algorithm */
#define A_BETTER_TOPO  2
#define A_BETTER       1
#define B_BETTER      -1
#define B_BETTER_TOPO -2

enum {
	DS_CMP_IEEE1588,
	DS_CMP_G8275,
};

/**
 * BMC state decision algorithm.
 * @param c  The local clock.
 * @param r  The port in question.
 * @param compare  The data set comparison algorithm.
 * @return   A @ref PORT_STATE value as the recommended state.
 */
enum PORT_STATE bmc_state_decision(struct PtpClock *c, struct PtpPort *r,
				   int (*comapre)(struct dataset *a, struct dataset *b));

/**
 * Compare two data sets using the algorithm defined in IEEE 1588.
 * @param a A dataset to compare.
 * @param b A dataset to compare.
 * @return An integer less than, equal to, or greater than zero
 *         if the dataset @a a is found, respectively, to be
 *         less than, to match, or be greater than @a b.
 */
int dscmp(struct dataset *a, struct dataset *b);

/**
 * Second part of the data set comparison algorithm, not for general
 * public use.
 */
int dscmp2(struct dataset *a, struct dataset *b);

/**
 * Compare two data sets using the algorithm defined in the Telecom
 * Profiles according to ITU-T G.8275.1 and G.8275.2.
 *
 * @param a A dataset to compare.
 * @param b A dataset to compare.
 * @return An integer less than, equal to, or greater than zero
 *         if the dataset @a a is found, respectively, to be
 *         less than, to match, or be greater than @a b.
 */
int telecom_dscmp(struct dataset *a, struct dataset *b);



/* Transparent Clock */
/**
 * Flushes the list of remembered residence times.
 * @param q    Port whose list should be flushed
 */
void tc_flush(struct PtpPort *q);

/**
 * Forwards a given general message out all other ports.
 * @param q    The ingress port
 * @param msg  The message to be sent
 * @return     Zero on success, non-zero otherwise.
 */
int tc_forward(struct PtpPort *q, struct ptp_message *msg);

/**
 * Forwards a given Follow-Up message out all other ports.
 *
 * This function adds the unique, per egress port residence time into
 * the correction field for the transmitted follow up message.
 *
 * @param q    The ingress port
 * @param msg  The message to be sent
 * @return     Zero on success, non-zero otherwise.
 */
int tc_fwd_folup(struct PtpPort *q, struct ptp_message *msg);

/**
 * Forwards a given delay request message out all other ports.
 *
 * This function computes the unique residence time for each egress
 * port, remembering it in that egress port.
 *
 * @param q    The ingress port
 * @param msg  The message to be sent
 * @return     Zero on success, non-zero otherwise.
 */
int tc_fwd_request(struct PtpPort *q, struct ptp_message *msg);

/**
 * Forwards a given response message out all other ports.
 *
 * This function adds the unique, per egress port residence time into
 * the correction field for the transmitted delay response message.
 *
 * @param q    The ingress port
 * @param msg  The message to be sent
 * @return     Zero on success, non-zero otherwise.
 */
int tc_fwd_response(struct PtpPort *q, struct ptp_message *msg);

/**
 * Forwards a given sync message out all other ports.
 *
 * This function computes the unique residence time for each egress
 * port, remembering it in that egress port.
 *
 * @param q    The ingress port
 * @param msg  The message to be sent
 * @return     Zero on success, non-zero otherwise.
 */
int tc_fwd_sync(struct PtpPort *q, struct ptp_message *msg);

/**
 * Determines whether the local clock should ignore a given message.
 *
 * @param q    The ingress port
 * @param msg  The message to test
 * @return     One if the message should be ignored, zero otherwise.
 */
int tc_ignore(struct PtpPort *q, struct ptp_message *m);

/**
 * Prunes stale entries from the list of remembered residence times.
 * @param q    Port whose list should be pruned.
 */
void tc_prune(struct PtpPort *q);

#endif

