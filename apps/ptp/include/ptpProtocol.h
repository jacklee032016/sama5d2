/*
* definitions of PTP protocol, IEEE1855-2008
 */
#ifndef __PTP_PROTOCOL_H__
#define __PTP_PROTOCOL_H__

/* constants of PTP */

#define PTP_VERSION 2

/* Values for the messageType field */
/* EVENT message, accurate timestamp is required for them */
#define SYNC							0x0
#define DELAY_REQ					0x1
#define PDELAY_REQ					0x2
#define PDELAY_RESP					0x3

/* GENERAL message, not accurate timestamp */
#define FOLLOW_UP					0x8
#define DELAY_RESP					0x9
#define PDELAY_RESP_FOLLOW_UP		0xA
#define ANNOUNCE					0xB
#define SIGNALING					0xC
#define MANAGEMENT					0xD


/* Bits for flagField[0] */
#define ALT_MASTER     (1<<0)
#define TWO_STEP       (1<<1)
#define UNICAST        (1<<2)

/* Bits for flagField[1] */
#define LEAP_61        (1<<0)
#define LEAP_59        (1<<1)
#define UTC_OFF_VALID  (1<<2)
#define PTP_TIMESCALE  (1<<3)
#define TIME_TRACEABLE (1<<4)
#define FREQ_TRACEABLE (1<<5)


#define PTP_PORT_EVENT						319
#define PTP_PORT_GENERAL					320

#define PTP_PRIMARY_MCAST_IPADDR			"224.0.1.129"
#define PTP_PDELAY_MCAST_IPADDR			"224.0.0.107"


#define PTP_PRIMARY_MCAST_IP6ADDR			"FF0E:0:0:0:0:0:0:181"
#define PTP_PDELAY_MCAST_IP6ADDR			"FF02:0:0:0:0:0:0:6B"


/************* basic types for PTP protocol ******************/
#define PACKED __attribute__((packed))

typedef Integer64		TimeInterval; /* nanoseconds << 16 */

/** On the wire time stamp format. */
struct WireTimeStamp
{
	uint16_t		seconds_msb;	/* 16 bits + */
	uint32_t		seconds_lsb;	/* 32 bits = 48 bits*/
	
	UInteger32	nanoseconds;
} PACKED;

/** Internal binary time stamp format. */
struct LocalTimeStamp
{
	uint64_t		sec;
	UInteger32	nsec;
};

struct ClockIdentity
{
	Octet id[8];
};

struct PortIdentity
{
	struct ClockIdentity	clockIdentity;
	UInteger16			portNumber;
} PACKED;

struct PortAddress
{
	Enumeration16	networkProtocol;
	UInteger16		addressLength;
	Octet			address[0];
} PACKED;

struct PhysicalAddress
{
	UInteger16 length;
	Octet      address[0];
} PACKED;

struct ClockQuality
{
	UInteger8		clockClass;
	Enumeration8		clockAccuracy;
	UInteger16		offsetScaledLogVariance;
} PACKED;

struct TLV
{
	Enumeration16	type;
	UInteger16		length; /* must be even */
	Octet			value[0];
} PACKED;

struct PTPText
{
	UInteger8	length;
	Octet		text[0];
} PACKED;

/* A static_ptp_text is like a PTPText but includes space to store the
 * text inside the struct. The text array must always be
 * null-terminated. Also tracks a maximum number of symbols. Note in
 * UTF-8, # symbols != # bytes.
 */
#define MAX_PTP_OCTETS 255
struct static_ptp_text {
	/* null-terminated array of UTF-8 symbols */
	Octet text[MAX_PTP_OCTETS + 1];
	/* number of used bytes in text, not including trailing null */
	int length;
	/* max number of UTF-8 symbols that can be in text */
	int max_symbols;
};

struct FaultRecord
{
	UInteger16				faultRecordLength;
	struct WireTimeStamp		faultTime;
	Enumeration8				severityCode;
	
	struct PTPText			faultName;
	struct PTPText			faultValue;
	struct PTPText			faultDescription;
};


/******************* definitions of message types ********************/

struct ptp_header
{
	uint8_t				tsmt; /* transportSpecific | messageType */
	uint8_t				ver;  /* reserved          | versionPTP  */
	UInteger16			messageLength;
	UInteger8			domainNumber;
	
	Octet				reserved1;
	Octet				flagField[2];
	
	Integer64			correction; /* nano second << 16 */
	UInteger32			reserved2;
	
	struct PortIdentity		sourcePortIdentity;
	UInteger16			sequenceId;
	UInteger8			control;
	Integer8				logMessageInterval;
} PACKED;	/* 34 bytes */

struct announce_msg
{
	struct ptp_header			hdr;
	struct WireTimeStamp		originTimestamp;
	Integer16				currentUtcOffset;
	Octet					reserved;
	
	UInteger8				grandmasterPriority1;
	struct ClockQuality		grandmasterClockQuality;
	UInteger8				grandmasterPriority2;
	struct ClockIdentity grandmasterIdentity;
	UInteger16           stepsRemoved;
	Enumeration8         timeSource;
	uint8_t              suffix[0];
} PACKED;

struct sync_msg
{
	struct ptp_header			hdr;
	struct WireTimeStamp		originTimestamp;
} PACKED;

struct delay_req_msg
{
	struct ptp_header			hdr;
	struct WireTimeStamp		originTimestamp;
	uint8_t					suffix[0];
} PACKED;

struct follow_up_msg {
	struct ptp_header   hdr;
	struct WireTimeStamp    preciseOriginTimestamp;
	uint8_t             suffix[0];
} PACKED;

struct delay_resp_msg {
	struct ptp_header   hdr;
	struct WireTimeStamp    receiveTimestamp;
	struct PortIdentity requestingPortIdentity;
	uint8_t             suffix[0];
} PACKED;

struct pdelay_req_msg {
	struct ptp_header   hdr;
	struct WireTimeStamp    originTimestamp;
	struct PortIdentity reserved;
} PACKED;

struct pdelay_resp_msg {
	struct ptp_header   hdr;
	struct WireTimeStamp    requestReceiptTimestamp;
	struct PortIdentity requestingPortIdentity;
} PACKED;

struct pdelay_resp_fup_msg {
	struct ptp_header   hdr;
	struct WireTimeStamp    responseOriginTimestamp;
	struct PortIdentity requestingPortIdentity;
	uint8_t             suffix[0];
} PACKED;

struct signaling_msg {
	struct ptp_header   hdr;
	struct PortIdentity targetPortIdentity;
	uint8_t             suffix[0];
} PACKED;

struct management_msg
{
	struct ptp_header		hdr;
	struct PortIdentity		targetPortIdentity;
	UInteger8			startingBoundaryHops;
	UInteger8			boundaryHops;
	uint8_t				flags; /* reserved | actionField */
	uint8_t				reserved;
	uint8_t				suffix[0];
} PACKED;

struct message_data
{
	uint8_t				buffer[1500];
} PACKED;




/********************* clock data sets *******************/

#define DDS_TWO_STEP_FLAG (1<<0)
#define DDS_SLAVE_ONLY    (1<<1)

struct defaultDS {
	UInteger8            flags;
	UInteger8            reserved1;
	UInteger16           numberPorts;
	UInteger8            priority1;
	struct ClockQuality  clockQuality;
	UInteger8            priority2;
	struct ClockIdentity clockIdentity;
	UInteger8            domainNumber;
	UInteger8            reserved2;
} PACKED;

#define OUI_LEN 3
struct ClockDescription {
	struct static_ptp_text productDescription;
	struct static_ptp_text revisionData;
	struct static_ptp_text userDescription;
	Octet manufacturerIdentity[OUI_LEN];
};

struct dataset
{
	UInteger8				priority1;
	struct ClockIdentity		identity;
	struct ClockQuality		quality;
	UInteger8				priority2;
	UInteger8				localPriority; /* Telecom Profile only */
	UInteger16				stepsRemoved;
	
	struct PortIdentity			sender;
	struct PortIdentity			receiver;
};

struct currentDS {
	UInteger16   stepsRemoved;
	TimeInterval offsetFromMaster;
	TimeInterval meanPathDelay;
} PACKED;

struct parentDS {
	struct PortIdentity  parentPortIdentity;
	UInteger8            parentStats;
	UInteger8            reserved;
	UInteger16           observedParentOffsetScaledLogVariance;
	Integer32            observedParentClockPhaseChangeRate;
	UInteger8            grandmasterPriority1;
	struct ClockQuality  grandmasterClockQuality;
	UInteger8            grandmasterPriority2;
	struct ClockIdentity grandmasterIdentity;
} PACKED;

struct parent_ds {
	struct parentDS pds;
	struct ClockIdentity *ptl;
	unsigned int path_length;
};

#define CURRENT_UTC_OFFSET			37 /* 1 Jan 2017 */
#define INTERNAL_OSCILLATOR		0xA0

struct timePropertiesDS
{
	Integer16    currentUtcOffset;
	UInteger8    flags;
	Enumeration8 timeSource;
} PACKED;

struct portDS {
	struct PortIdentity portIdentity;
	Enumeration8        portState;
	Integer8            logMinDelayReqInterval;
	TimeInterval        peerMeanPathDelay;
	Integer8            logAnnounceInterval;
	UInteger8           announceReceiptTimeout;
	Integer8            logSyncInterval;
	Enumeration8        delayMechanism;
	Integer8            logMinPdelayReqInterval;
	UInteger8           versionNumber;
} PACKED;

#define FRI_ASAP (-128)

/*****************  TLV ************************/
/* TLV types */
#define TLV_MANAGEMENT					0x0001
#define TLV_MANAGEMENT_ERROR_STATUS			0x0002
#define TLV_ORGANIZATION_EXTENSION			0x0003
#define TLV_REQUEST_UNICAST_TRANSMISSION		0x0004
#define TLV_GRANT_UNICAST_TRANSMISSION			0x0005
#define TLV_CANCEL_UNICAST_TRANSMISSION			0x0006
#define TLV_ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION	0x0007
#define TLV_PATH_TRACE					0x0008
#define TLV_ALTERNATE_TIME_OFFSET_INDICATOR		0x0009
#define TLV_AUTHENTICATION				0x2000
#define TLV_AUTHENTICATION_CHALLENGE			0x2001
#define TLV_SECURITY_ASSOCIATION_UPDATE			0x2002
#define TLV_CUM_FREQ_SCALE_FACTOR_OFFSET		0x2003
#define TLV_PTPMON_REQ					0x21FE
#define TLV_PTPMON_RESP					0x21FF

/* specs sec.15, p.158 table 38 */
enum management_action
{
	GET = 0,
	SET = 1,
	RESPONSE = 2,
	COMMAND = 3,
	ACKNOWLEDGE = 4,
	M_ACTION_UKNOWN
};

/* Clock management ID values */
#define TLV_USER_DESCRIPTION						0x0002
#define TLV_SAVE_IN_NON_VOLATILE_STORAGE			0x0003
#define TLV_RESET_NON_VOLATILE_STORAGE			0x0004
#define TLV_INITIALIZE								0x0005
#define TLV_FAULT_LOG								0x0006
#define TLV_FAULT_LOG_RESET						0x0007

#define TLV_DEFAULT_DATA_SET						0x2000
#define TLV_CURRENT_DATA_SET						0x2001
#define TLV_PARENT_DATA_SET						0x2002
#define TLV_TIME_PROPERTIES_DATA_SET				0x2003
#define TLV_PRIORITY1								0x2005
#define TLV_PRIORITY2								0x2006
#define TLV_DOMAIN									0x2007
#define TLV_SLAVE_ONLY								0x2008
#define TLV_TIME										0x200F
#define TLV_CLOCK_ACCURACY						0x2010
#define TLV_UTC_PROPERTIES							0x2011
#define TLV_TRACEABILITY_PROPERTIES				0x2012
#define TLV_TIMESCALE_PROPERTIES					0x2013
#define TLV_PATH_TRACE_LIST							0x2015
#define TLV_PATH_TRACE_ENABLE						0x2016
#define TLV_GRANDMASTER_CLUSTER_TABLE			0x2017
#define TLV_ACCEPTABLE_MASTER_TABLE				0x201A
#define TLV_ACCEPTABLE_MASTER_MAX_TABLE_SIZE	0x201C
#define TLV_ALTERNATE_TIME_OFFSET_ENABLE			0x201E
#define TLV_ALTERNATE_TIME_OFFSET_NAME			0x201F
#define TLV_ALTERNATE_TIME_OFFSET_MAX_KEY		0x2020
#define TLV_ALTERNATE_TIME_OFFSET_PROPERTIES		0x2021

#define TLV_TRANSPARENT_CLOCK_DEFAULT_DATA_SET		0x4000
#define TLV_PRIMARY_DOMAIN								0x4002

#define TLV_TIME_STATUS_NP								0xC000
#define TLV_GRANDMASTER_SETTINGS_NP					0xC001
#define TLV_SUBSCRIBE_EVENTS_NP						0xC003

/* Port management ID values */
#define TLV_NULL_MANAGEMENT				0x0000
#define TLV_CLOCK_DESCRIPTION				0x0001
#define TLV_PORT_DATA_SET				0x2004
#define TLV_LOG_ANNOUNCE_INTERVAL			0x2009
#define TLV_ANNOUNCE_RECEIPT_TIMEOUT			0x200A
#define TLV_LOG_SYNC_INTERVAL				0x200B
#define TLV_VERSION_NUMBER				0x200C
#define TLV_ENABLE_PORT					0x200D
#define TLV_DISABLE_PORT				0x200E
#define TLV_UNICAST_NEGOTIATION_ENABLE			0x2014
#define TLV_UNICAST_MASTER_TABLE			0x2018
#define TLV_UNICAST_MASTER_MAX_TABLE_SIZE		0x2019
#define TLV_ACCEPTABLE_MASTER_TABLE_ENABLED		0x201B
#define TLV_ALTERNATE_MASTER				0x201D
#define TLV_TRANSPARENT_CLOCK_PORT_DATA_SET		0x4001
#define TLV_DELAY_MECHANISM				0x6000
#define TLV_LOG_MIN_PDELAY_REQ_INTERVAL			0x6001
#define TLV_PORT_DATA_SET_NP				0xC002
#define TLV_PORT_PROPERTIES_NP				0xC004

/* Management error ID values */
#define TLV_RESPONSE_TOO_BIG						0x0001
#define TLV_NO_SUCH_ID								0x0002
#define TLV_WRONG_LENGTH							0x0003
#define TLV_WRONG_VALUE							0x0004
#define TLV_NOT_SETABLE								0x0005
#define TLV_NOT_SUPPORTED							0x0006
#define TLV_GENERAL_ERROR							0xFFFE


#define CANCEL_UNICAST_MAINTAIN_REQUEST	(1 << 0)
#define CANCEL_UNICAST_MAINTAIN_GRANT	(1 << 1)
#define GRANT_UNICAST_RENEWAL_INVITED	(1 << 0)

struct ack_cancel_unicast_xmit_tlv {
	Enumeration16   type;
	UInteger16      length;
	uint8_t         message_type_flags;
	uint8_t         reserved;
} PACKED;

struct cancel_unicast_xmit_tlv {
	Enumeration16   type;
	UInteger16      length;
	uint8_t         message_type_flags;
	uint8_t         reserved;
} PACKED;

struct grant_unicast_xmit_tlv {
	Enumeration16   type;
	UInteger16      length;
	uint8_t         message_type;
	Integer8        logInterMessagePeriod;
	UInteger32      durationField;
	uint8_t         reserved;
	uint8_t         flags;
} PACKED;

struct management_tlv
{
	Enumeration16	type;
	UInteger16		length;
	Enumeration16	id;
	Octet			data[0];
} PACKED;

struct management_tlv_datum
{
	uint8_t			val;
	uint8_t			reserved;
} PACKED;

struct management_error_status {
	Enumeration16 type;
	UInteger16    length;
	Enumeration16 error;
	Enumeration16 id;
	Octet         reserved[4];
	Octet         data[0];
} PACKED;

struct nsm_resp_tlv_head {
	Enumeration16           type;
	UInteger16              length;
	uint8_t                 port_state;
	uint8_t                 reserved;
	struct PortAddress      parent_addr;
} PACKED;

struct nsm_resp_tlv_foot {
	struct parentDS         parent;
	struct currentDS        current;
	struct timePropertiesDS timeprop;
	struct WireTimeStamp        lastsync;
} PACKED;

/* Organizationally Unique Identifiers */
#define IEEE_802_1_COMMITTEE		0x00, 0x80, 0xC2

extern uint8_t ieee8021_id[3];

struct organization_tlv {
	Enumeration16 type;
	UInteger16    length;
	Octet         id[3];
	Octet         subtype[3];
} PACKED;

#define PATH_TRACE_MAX \
	((sizeof(struct message_data) - sizeof(struct announce_msg) - sizeof(struct TLV)) / \
	 sizeof(struct ClockIdentity))

struct path_trace_tlv {
	Enumeration16 type;
	UInteger16    length;
	struct ClockIdentity cid[0];
} PACKED;

struct request_unicast_xmit_tlv {
	Enumeration16   type;
	UInteger16      length;
	uint8_t         message_type;
	Integer8        logInterMessagePeriod;
	UInteger32      durationField;
} PACKED;

static inline unsigned int path_length(struct path_trace_tlv *p)
{
	return p->length / sizeof(struct ClockIdentity);
}

typedef struct Integer96 {
	uint16_t nanoseconds_msb;
	uint64_t nanoseconds_lsb;
	uint16_t fractional_nanoseconds;
} PACKED ScaledNs;

struct follow_up_info_tlv {
	Enumeration16 type;
	UInteger16    length;
	Octet         id[3];
	Octet         subtype[3];
	Integer32     cumulativeScaledRateOffset;
	UInteger16    gmTimeBaseIndicator;
	ScaledNs      lastGmPhaseChange;
	Integer32     scaledLastGmPhaseChange;
} PACKED;

struct time_status_np {
	int64_t       master_offset; /*nanoseconds*/
	int64_t       ingress_time;  /*nanoseconds*/
	Integer32     cumulativeScaledRateOffset;
	Integer32     scaledLastGmPhaseChange;
	UInteger16    gmTimeBaseIndicator;
	ScaledNs      lastGmPhaseChange;
	Integer32     gmPresent;
	struct ClockIdentity gmIdentity;
} PACKED;

struct grandmaster_settings_np {
	struct ClockQuality clockQuality;
	Integer16 utc_offset;
	UInteger8 time_flags;
	Enumeration8 time_source;
} PACKED;

struct port_ds_np {
	UInteger32    neighborPropDelayThresh; /*nanoseconds*/
	Integer32     asCapable;
} PACKED;


#define EVENT_BITMASK_CNT 64

struct subscribe_events_np {
	uint16_t      duration; /* seconds */
	uint8_t       bitmask[EVENT_BITMASK_CNT];
} PACKED;

struct port_properties_np
{
	struct PortIdentity portIdentity;
	uint8_t port_state;
	uint8_t timestamping;

	struct PTPText	intfTxt;
} PACKED;

#define PROFILE_ID_LEN 6

struct mgmt_clock_description {
	UInteger16             *clockType;
	struct PTPText         *physicalLayerProtocol;
	struct PhysicalAddress *physicalAddress;
	struct PortAddress     *protocolAddress;
	Octet                  *manufacturerIdentity;
	struct PTPText         *productDescription;
	struct PTPText         *revisionData;
	struct PTPText         *userDescription;
	Octet                  *profileIdentity;
};


/******************** state machine ****************/
/* The finite state machine for ports on boundary and ordinary clocks */

/** Defines the state of a port. */
enum PORT_STATE {
	PS_INITIALIZING = 1,
	PS_FAULTY,
	PS_DISABLED,
	PS_LISTENING,
	PS_PRE_MASTER,
	PS_MASTER,
	PS_PASSIVE,
	PS_UNCALIBRATED,
	PS_SLAVE,
	PS_GRAND_MASTER, /*non-standard extension*/
};

/** Defines the events for the port state machine. */
enum PORT_EVENT {
	EV_NONE,
	EV_POWERUP,
	EV_INITIALIZE,
	EV_DESIGNATED_ENABLED,
	EV_DESIGNATED_DISABLED,
	EV_FAULT_CLEARED,
	EV_FAULT_DETECTED,
	EV_STATE_DECISION_EVENT,
	EV_QUALIFICATION_TIMEOUT_EXPIRES,
	EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES,
	EV_SYNCHRONIZATION_FAULT,
	EV_MASTER_CLOCK_SELECTED,	/* enter into SLAVE state */
	EV_INIT_COMPLETE,
	EV_RS_MASTER,
	EV_RS_GRAND_MASTER,
	EV_RS_SLAVE,
	EV_RS_PASSIVE,
};


/* Values from networkProtocol enumeration 7.4.1 Table 3 */
enum transport_type {
	/* 0 is Reserved in spec. Use it for UDS */
	TRANS_UDS = 0,
	TRANS_UDP_IPV4 = 1,
	TRANS_UDP_IPV6,
	TRANS_IEEE_802_3,
	TRANS_DEVICENET,
	TRANS_CONTROLNET,
	TRANS_PROFINET,
};

/**
 * Values for the 'event' parameter in transport_send() and
 * transport_peer().
 */
enum transport_event {
	TRANS_GENERAL,
	TRANS_EVENT,
	TRANS_ONESTEP,
	TRANS_P2P1STEP,
	TRANS_DEFER_EVENT,
};


#include <stdint.h>

enum fault_type {
	FT_UNSPECIFIED = 0,
	FT_BAD_PEER_NETWORK,
	FT_SWITCH_PHC,
	FT_CNT,
};

enum fault_tmo_type {
	FTMO_LINEAR_SECONDS = 0,
	FTMO_LOG2_SECONDS,
	FTMO_CNT,
};

struct fault_interval {
	enum fault_tmo_type type;
	int32_t val;
};

const char *ft_str(enum fault_type ft);

#endif
