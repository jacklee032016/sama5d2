/**
 * @file util.c
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
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ptpCompact.h"
#include "ptpProtocol.h"
#include "ptpImplements.h"

#include "sk.h"
#include "util.h"

#define NS_PER_HOUR (3600 * NS_PER_SEC)
#define NS_PER_DAY (24 * NS_PER_HOUR)

static int running = 1;


void ptpMsgDebug(struct ptp_message *m, FILE *fp)
{
	fprintf(fp,
		"\t"
		"%-10s "
//		"versionPTP         0x%02X "
//		"messageLength      %hu "
//		"domainNumber       %u "
//		"reserved1          0x%02X "
//		"flagField          0x%02X%02X "
//		"correction         %lld "
//		"reserved2          %u "
//		"sourcePortIdentity ... "
		"sequenceId %4hu "
//		"control            %u "
//		"logMessageInterval %d "
		,
		ptpMsgTypeString(msg_type(m)),
//		m->header.ver,
//		m->header.messageLength,
//		m->header.domainNumber,
//		m->header.reserved1,
//		m->header.flagField[0],
//		m->header.flagField[1],
//		m->header.correction,
//		m->header.reserved2,
//		m->header.sourcePortIdentity,
		m->header.sequenceId
//		m->header.control,
//		m->header.logMessageInterval
		);
	
	fprintf(fp, "\n");
}

const char *ptpMsgTypeString(int type)
{
	switch (type)
	{
		case SYNC:
			return "SYNC";
		case DELAY_REQ:
			return "DELAY_REQ";
		case PDELAY_REQ:
			return "PDELAY_REQ";
		case PDELAY_RESP:
			return "PDELAY_RESP";
		case FOLLOW_UP:
			return "FOLLOW_UP";
		case DELAY_RESP:
			return "DELAY_RESP";
		case PDELAY_RESP_FOLLOW_UP:
			return "PDELAY_RESP_FOLLOW_UP";
		case ANNOUNCE:
			return "ANNOUNCE";
		case SIGNALING:
			return "SIGNALING";
		case MANAGEMENT:
			return "MANAGEMENT";
	}
	return "unknown";
}

const char *ptpPortStateString(int state)
{
	switch (state)
	{
		case 0:
			return "NONE";
		case PS_INITIALIZING:
			return "INITIALIZING";
		case PS_FAULTY:
			return "FAULTY";
		case PS_DISABLED:
			return "DISABLED";
		case PS_LISTENING:
			return "LISTENING";
		case PS_PRE_MASTER:
			return "PRE_MASTER";
		case PS_MASTER:
			return "MASTER";
		case PS_PASSIVE:
			return "PASSIVE";
		case PS_UNCALIBRATED:
			return "UNCALIBRATED";
		case PS_SLAVE:
			return "SLAVE";
		case PS_GRAND_MASTER:
			return "GRAND_MASTER";
	}
	
	return "unknown";
}

const char *ptpPortEventString(int event)
{
	switch (event)
	{
		case EV_NONE:
			return "NONE";
		case EV_POWERUP:
			return "POWERUP";
		case EV_INITIALIZE:
			return "INITIALIZE";
		case EV_DESIGNATED_ENABLED:
			return "DESIGNATED_ENABLED";
		case EV_DESIGNATED_DISABLED:
			return "DESIGNATED_DISABLED";
		case EV_FAULT_CLEARED:
			return "FAULT_CLEARED";
		case EV_FAULT_DETECTED:
			return "FAULT_DETECTED";
		case EV_STATE_DECISION_EVENT:
			return "STATE_DECISION_EVENT";
		case EV_QUALIFICATION_TIMEOUT_EXPIRES:
			return "QUALIFICATION_TIMEOUT_EXPIRES";
		case EV_ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES:
			return "ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES";
		case EV_SYNCHRONIZATION_FAULT:
			return "SYNCHRONIZATION_FAULT";
		case EV_MASTER_CLOCK_SELECTED:
			return "MASTER_CLOCK_SELECTED";
		case EV_INIT_COMPLETE:
			return "INIT_COMPLETE";
		case EV_RS_MASTER:
			return "RS_MASTER";
		case EV_RS_GRAND_MASTER:
			return "RS_GRAND_MASTER";
		case EV_RS_SLAVE:
			return "RS_SLAVE";
		case EV_RS_PASSIVE:
			return "RS_PASSIVE";
	}
	
	return "unknown";
}


int addreq(enum transport_type type, struct address *a, struct address *b)
{
	void *bufa, *bufb;
	int len;

	switch (type) {
	case TRANS_UDP_IPV4:
		bufa = &a->sin.sin_addr;
		bufb = &b->sin.sin_addr;
		len = sizeof(a->sin);
		break;
	case TRANS_IEEE_802_3:
		bufa = &a->sll.sll_addr;
		bufb = &b->sll.sll_addr;
		len = MAC_LEN;
		break;
	case TRANS_UDS:
	case TRANS_UDP_IPV6:
	case TRANS_DEVICENET:
	case TRANS_CONTROLNET:
	case TRANS_PROFINET:
	default:
		pr_err("sorry, cannot compare addresses for this transport");
		return 0;
	}
	return memcmp(bufa, bufb, len) == 0 ? 1 : 0;
}

char *bin2str_impl(Octet *data, int len, char *buf, int buf_len)
{
	int i, offset = 0;
	if (len > MAX_PRINT_BYTES)
		len = MAX_PRINT_BYTES;
	buf[0] = '\0';
	if (!data)
		return buf;
	if (len)
		offset += snprintf(buf, buf_len, "%02hhx", data[0]);
	for (i = 1; i < len; i++) {
		if (offset >= buf_len)
			/* truncated output */
			break;
		offset += snprintf(buf + offset, buf_len - offset, ":%02hhx", data[i]);
	}
	return buf;
}

char *cid2str(struct ClockIdentity *id)
{
	static char buf[64];
	unsigned char *ptr = id->id;

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%02x%02x%02x.%02x%02x.%02x%02x%02x",
		 ptr[0], ptr[1], ptr[2], ptr[3],
		 ptr[4], ptr[5], ptr[6], ptr[7]);
	return buf;
}

int count_char(const char *str, char c)
{
	int num = 0;
	char s;
	while ((s = *(str++))) {
		if (s == c)
			num++;
	}
	return num;
}

char *pid2str(struct PortIdentity *id)
{
	static char buf[64];
	unsigned char *ptr = id->clockIdentity.id;
	snprintf(buf, sizeof(buf), "%02x%02x%02x.%02x%02x.%02x%02x%02x-%hu",
		 ptr[0], ptr[1], ptr[2], ptr[3],
		 ptr[4], ptr[5], ptr[6], ptr[7],
		 id->portNumber);
	return buf;
}

char *portaddr2str(struct PortAddress *addr)
{
	static char buf[BIN_BUF_SIZE];
	switch (align16(&addr->networkProtocol)) {
	case TRANS_UDP_IPV4:
		if (align16(&addr->addressLength) == 4
			&& inet_ntop(AF_INET, addr->address, buf, sizeof(buf)))
			return buf;
		break;
	case TRANS_UDP_IPV6:
		if (align16(&addr->addressLength) == 16
			&& inet_ntop(AF_INET6, addr->address, buf, sizeof(buf)))
			return buf;
		break;
	}
	bin2str_impl(addr->address, align16(&addr->addressLength), buf, sizeof(buf));
	return buf;
}

int str2addr(enum transport_type type, const char *s, struct address *addr)
{
	unsigned char mac[MAC_LEN];
	struct in_addr ipv4_addr;

	memset(addr, 0, sizeof(*addr));

	switch (type) {
	case TRANS_UDS:
	case TRANS_UDP_IPV6:
	case TRANS_DEVICENET:
	case TRANS_CONTROLNET:
	case TRANS_PROFINET:
		pr_err("sorry, cannot convert addresses for this transport");
		return -1;
	case TRANS_UDP_IPV4:
		if (!inet_aton(s, &ipv4_addr)) {
			pr_err("bad IPv4 address");
			return -1;
		}
		addr->sin.sin_family = AF_INET;
		addr->sin.sin_addr = ipv4_addr;
		addr->len = sizeof(addr->sin);
		break;
	case TRANS_IEEE_802_3:
		if (str2mac(s, mac)) {
			pr_err("bad Layer-2 address");
			return -1;
		}
		addr->sll.sll_family = AF_PACKET;
		addr->sll.sll_halen = MAC_LEN;
		memcpy(&addr->sll.sll_addr, mac, MAC_LEN);
		addr->len = sizeof(addr->sll);
		break;
	}
	return 0;
}

int str2mac(const char *s, unsigned char mac[MAC_LEN])
{
	unsigned char buf[MAC_LEN];
	int c;
	c = sscanf(s, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		   &buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5]);
	if (c != MAC_LEN) {
		return -1;
	}
	memcpy(mac, buf, MAC_LEN);
	return 0;
}

int str2pid(const char *s, struct PortIdentity *result)
{
	struct PortIdentity pid;
	unsigned char *ptr = pid.clockIdentity.id;
	int c;
	c = sscanf(s, " %02hhx%02hhx%02hhx.%02hhx%02hhx.%02hhx%02hhx%02hhx-%hu",
		   &ptr[0], &ptr[1], &ptr[2], &ptr[3],
		   &ptr[4], &ptr[5], &ptr[6], &ptr[7],
		   &pid.portNumber);
	if (c == 9) {
		*result = pid;
		return 0;
	}
	return -1;
}

int generate_clock_identity(struct ClockIdentity *ci, const char *name)
{
	struct address addr;

	if (sk_interface_macaddr(name, &addr))
		return -1;

	switch (addr.sll.sll_halen) {
		case EUI48:
			ci->id[0] = addr.sll.sll_addr[0];
			ci->id[1] = addr.sll.sll_addr[1];
			ci->id[2] = addr.sll.sll_addr[2];
			ci->id[3] = 0xFF;
			ci->id[4] = 0xFE;
			ci->id[5] = addr.sll.sll_addr[3];
			ci->id[6] = addr.sll.sll_addr[4];
			ci->id[7] = addr.sll.sll_addr[5];
			break;
		case EUI64:
			ci->id[0] = addr.sll.sll_addr[0];
			ci->id[1] = addr.sll.sll_addr[1];
			ci->id[2] = addr.sll.sll_addr[2];
			ci->id[3] = addr.sll.sll_addr[3];
			ci->id[4] = addr.sll.sll_addr[4];
			ci->id[5] = addr.sll.sll_addr[5];
			ci->id[6] = addr.sll.sll_addr[6];
			ci->id[7] = addr.sll.sll_addr[7];
			break;
		default:
			return -1;
	}

	return 0;
}

/* Naive count of utf8 symbols. Doesn't detect invalid UTF-8 and
 * probably doesn't count combining characters correctly. */
static size_t strlen_utf8(const Octet *s)
{
	size_t len = 0;
	char c;
	while ((c = *(s++))) {
		if ((c & 0xC0) != 0x80)
			len++;
	}
	return len;
}

int static_ptp_text_copy(struct static_ptp_text *dst, const struct PTPText *src)
{
	int len = src->length;
	if (dst->max_symbols > 0 && strlen_utf8(src->text) > dst->max_symbols)
		return -1;
	dst->length = len;
	memcpy(dst->text, src->text, len);
	dst->text[len] = '\0';
	return 0;
}

void ptp_text_copy(struct PTPText *dst, const struct static_ptp_text *src)
{
	dst->length = src->length;
	memcpy(dst->text, src->text, src->length);
}

int ptp_text_set(struct PTPText *dst, const char *src)
{
	size_t len;
	if (src) {
		len = strlen(src);
		if (len > MAX_PTP_OCTETS)
			return -1;
		dst->length = len;
		memcpy(dst->text, src, len);
	} else {
		dst->length = 0;
	}
	return 0;
}

int static_ptp_text_set(struct static_ptp_text *dst, const char *src)
{
	int len = strlen(src);
	if (len > MAX_PTP_OCTETS)
		return -1;
	if (dst->max_symbols > 0 && strlen_utf8((Octet *) src) > dst->max_symbols)
		return -1;
	dst->length = len;
	memcpy(dst->text, src, len);
	dst->text[len] = '\0';
	return 0;
}

int is_utc_ambiguous(uint64_t ts)
{
	/* The Linux kernel inserts leap second by stepping the clock backwards
	   at 0:00 UTC, the last second before midnight is played twice. */
	if (NS_PER_DAY - ts % NS_PER_DAY <= NS_PER_SEC)
		return 1;
	return 0;
}

int leap_second_status(uint64_t ts, int leap_set, int *leap, int *utc_offset)
{
	int leap_status = leap_set;

	/* The leap bits obtained by PTP should be set at most 12 hours before
	   midnight and unset at most 2 announce intervals after midnight.
	   Split updates which are too early and which are too late at 6 hours
	   after midnight. */
	if (ts % NS_PER_DAY > 6 * NS_PER_HOUR) {
		if (!leap_status)
			leap_status = *leap;
	} else {
		if (leap_status)
			leap_status = 0;
	}

	/* Fix early or late update of leap and utc_offset. */
	if (!*leap && leap_status) {
		*utc_offset -= leap_status;
		*leap = leap_status;
	} else if (*leap && !leap_status) {
		*utc_offset += *leap;
		*leap = leap_status;
	}

	return leap_status;
}

enum parser_result get_ranged_int(const char *str_val, int *result,
				  int min, int max)
{
	long parsed_val;
	char *endptr = NULL;
	errno = 0;
	parsed_val = strtol(str_val, &endptr, 0);
	if (*endptr != '\0' || endptr == str_val)
		return MALFORMED;
	if (errno == ERANGE || parsed_val < min || parsed_val > max)
		return OUT_OF_RANGE;
	*result = parsed_val;
	return PARSED_OK;
}

enum parser_result get_ranged_uint(const char *str_val, unsigned int *result,
				   unsigned int min, unsigned int max)
{
	unsigned long parsed_val;
	char *endptr = NULL;
	errno = 0;
	parsed_val = strtoul(str_val, &endptr, 0);
	if (*endptr != '\0' || endptr == str_val)
		return MALFORMED;
	if (errno == ERANGE || parsed_val < min || parsed_val > max)
		return OUT_OF_RANGE;
	*result = parsed_val;
	return PARSED_OK;
}

enum parser_result get_ranged_double(const char *str_val, double *result,
				     double min, double max)
{
	double parsed_val;
	char *endptr = NULL;
	errno = 0;
	parsed_val = strtod(str_val, &endptr);
	if (*endptr != '\0' || endptr == str_val)
		return MALFORMED;
	if (errno == ERANGE || parsed_val < min || parsed_val > max)
		return OUT_OF_RANGE;
	*result = parsed_val;
	return PARSED_OK;
}

int get_arg_val_i(int op, const char *optarg, int *val, int min, int max)
{
	enum parser_result r;
	r = get_ranged_int(optarg, val, min, max);
	if (r == MALFORMED) {
		fprintf(stderr,
			"-%c: %s is a malformed value\n", op, optarg);
		return -1;
	}
	if (r == OUT_OF_RANGE) {
		fprintf(stderr,
			"-%c: %s is out of range. Must be in the range %d to %d\n",
			op, optarg, min, max);
		return -1;
	}
	return 0;
}

int get_arg_val_ui(int op, const char *optarg, unsigned int *val,
		   unsigned int min, unsigned int max)
{
	enum parser_result r;
	r = get_ranged_uint(optarg, val, min, max);
	if (r == MALFORMED) {
		fprintf(stderr,
			"-%c: %s is a malformed value\n", op, optarg);
		return -1;
	}
	if (r == OUT_OF_RANGE) {
		fprintf(stderr,
			"-%c: %s is out of range. Must be in the range %u to %u\n",
			op, optarg, min, max);
		return -1;
	}
	return 0;
}

int get_arg_val_d(int op, const char *optarg, double *val,
		  double min, double max)
{
	enum parser_result r;
	r = get_ranged_double(optarg, val, min, max);
	if (r == MALFORMED) {
		fprintf(stderr,
			"-%c: %s is a malformed value\n", op, optarg);
		return -1;
	}
	if (r == OUT_OF_RANGE) {
		fprintf(stderr,
			"-%c: %s is out of range. Must be in the range %e to %e\n",
			op, optarg, min, max);
		return -1;
	}
	return 0;
}

static void handleSignalInt(int s)
{
	EXT_INFOF("Signal SIGINT received");
	running = 0;
}

static void handleSignalQuit(int s)
{
	EXT_INFOF("Signal QUIT received");
	running = 0;
}

static void handleSiganlTerm(int s)
{
	EXT_INFOF("Signal TERM received");
	running = 0;
}


int handle_term_signals(void)
{
	if (SIG_ERR == signal(SIGINT, handleSignalInt))
	{
		fprintf(stderr, "cannot handle SIGINT\n");
		return -1;
	}
	if (SIG_ERR == signal(SIGQUIT, handleSignalQuit))
	{
		fprintf(stderr, "cannot handle SIGQUIT\n");
		return -1;
	}
	if (SIG_ERR == signal(SIGTERM, handleSiganlTerm))
	{
		fprintf(stderr, "cannot handle SIGTERM\n");
		return -1;
	}
	return 0;
}

int is_running(void)
{
	return running;
}

void *xmalloc(size_t size)
{
	void *r;

	r = malloc(size);
	if (!r) {
		pr_err("failed to allocate memory");
		exit(1);
	}

	return r;
}

void *xcalloc(size_t nmemb, size_t size)
{
	void *r;

	r = calloc(nmemb, size);
	if (!r) {
		pr_err("failed to allocate memory");
		exit(1);
	}

	return r;
}

void *xrealloc(void *ptr, size_t size)
{
	void *r;

	r = realloc(ptr, size);
	if (!r) {
		pr_err("failed to allocate memory");
		exit(1);
	}

	return r;
}

char *xstrdup(const char *s)
{
	void *r;

	r = strdup(s);
	if (!r) {
		pr_err("failed to allocate memory");
		exit(1);
	}

	return r;
}

char *string_newf(const char *format, ...)
{
	va_list ap;
	char *s;

	va_start(ap, format);
	if (vasprintf(&s, format, ap) < 0) {
		pr_err("failed to allocate memory");
		exit(1);
	}
	va_end(ap);

	return s;
}

void string_append(char **s, const char *str)
{
	size_t len1, len2;

	len1 = strlen(*s);
	len2 = strlen(str);
	*s = xrealloc(*s, len1 + len2 + 1);
	memcpy((*s) + len1, str, len2 + 1);
}

void string_appendf(char **s, const char *format, ...)
{
	va_list ap;
	size_t len1;
	int len2;
	char *s2;

	len1 = strlen(*s);

	va_start(ap, format);
	len2 = vasprintf(&s2, format, ap);
	va_end(ap);

	if (len2 < 0) {
		*s = NULL;
		return;
	}

	*s = xrealloc(*s, len1 + len2 + 1);
	memcpy((*s) + len1, s2, len2 + 1);
	free(s2);
}

void **parray_new(void)
{
	void **a;

	a = xmalloc(sizeof(*a));
	*a = NULL;

	return a;
}

void parray_append(void ***a, void *p)
{
	parray_extend(a, p, NULL);
}

void parray_extend(void ***a, ...)
{
	va_list ap;
	int ilen, len, alloced;
	void *p;

	for (len = 0; (*a)[len]; len++)
		;
	len++;

	va_start(ap, a);
	for (ilen = 0; va_arg(ap, void *); ilen++)
		;
	va_end(ap);

	/* Reallocate in exponentially increasing sizes. */
	for (alloced = 1; alloced < len; alloced <<= 1)
		;
	if (alloced < len + ilen) {
		while (alloced < len + ilen)
			alloced *= 2;
		*a = xrealloc(*a, alloced * sizeof **a);
	}

	va_start(ap, a);
	while ((p = va_arg(ap, void *)))
		(*a)[len++ - 1] = p;
	va_end(ap);
	(*a)[len - 1] = NULL;
}

int rate_limited(int interval, time_t *last)
{
	struct timespec ts;

	if (PTP_GET_SYS_TIME_MONOTONIC( &ts))
		return 1;
	if (*last + interval > ts.tv_sec)
		return 1;

	*last = ts.tv_sec;

	return 0;
}

