/**
 * @file udp.c
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
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ptpCompact.h"
#include "ptpProtocol.h"
#include "ptpImplements.h"

#include "ptpConfig.h"
#include "sk.h"
#include "transportPrivate.h"


struct PtpUdp
{
	struct transport	t;
	struct address	ip;
	struct address	mac;
};

static int mcast_bind(int fd, int index)
{
	int err;
	struct ip_mreqn req;
	memset(&req, 0, sizeof(req));
	req.imr_ifindex = index;
	err = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &req, sizeof(req));
	if (err) {
		pr_err("setsockopt IP_MULTICAST_IF failed: %m");
		return -1;
	}
	return 0;
}

static int mcast_join(int fd, int index, const struct sockaddr_in *sa)
{
	int err, off = 0;
	struct ip_mreqn req;

	memset(&req, 0, sizeof(req));
	memcpy(&req.imr_multiaddr, &sa->sin_addr, sizeof(struct in_addr));
	req.imr_ifindex = index;
	err = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req));
	if (err) {
		pr_err("setsockopt IP_ADD_MEMBERSHIP failed: %m");
		return -1;
	}
	err = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &off, sizeof(off));
	if (err) {
		pr_err("setsockopt IP_MULTICAST_LOOP failed: %m");
		return -1;
	}
	return 0;
}

static int _udpClose(struct transport *t, struct FdArray *fda)
{
	close(fda->fd[0]);
	close(fda->fd[1]);
	return 0;
}

/* UDP socket of port/device, then join 2 multicast addresses */
static int _udpOpenSocket(const char *devName, struct in_addr mc_addr[2], short port, int ttl)
{
	struct sockaddr_in addr;
	int fd, index, on = 1;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0)
	{
		pr_err("socket failed: %m");
		goto no_socket;
	}
	index = sk_interface_index(fd, devName);
	if (index < 0)
		goto no_option;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
	{
		pr_err("setsockopt SO_REUSEADDR failed: %m");
		goto no_option;
	}

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)))
	{
		pr_err("bind to local address failed: %m");
		goto no_option;
	}
	
	if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, devName, strlen(devName)))
	{
		pr_err("setsockopt SO_BINDTODEVICE to device %s failed: %m", devName);
		goto no_option;
	}

	if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)))
	{
		pr_err("setsockopt IP_MULTICAST_TTL failed: %m");
		goto no_option;
	}
	
	addr.sin_addr = mc_addr[0];
	if (mcast_join(fd, index, &addr))
	{
		pr_err("mcast_join failed");
		goto no_option;
	}
	
	addr.sin_addr = mc_addr[1];
	if (mcast_join(fd, index, &addr))
	{
		pr_err("mcast_join failed");
		goto no_option;
	}
	
	if (mcast_bind(fd, index))
	{
		goto no_option;
	}
	return fd;
no_option:
	close(fd);
no_socket:
	return -1;
}

enum { MC_PRIMARY, MC_PDELAY };

static struct in_addr mcast_addr[2];

static int _udpOpen(struct transport *t, struct PtpInterface *iface, struct FdArray *fda, enum timestamp_type ts_type)
{
	struct PtpUdp *udp = container_of(t, struct PtpUdp, t);
	uint8_t event_dscp, general_dscp;
	int efd, gfd, ttl;
	char *name = iface->name;

	ttl = config_get_int(t->cfg, name, "udp_ttl");
	udp->mac.len = 0;
	sk_interface_macaddr(name, &udp->mac);

	udp->ip.len = 0;
	sk_interface_addr(name, AF_INET, &udp->ip);

	if (!inet_aton(PTP_PRIMARY_MCAST_IPADDR, &mcast_addr[MC_PRIMARY]))
		return -1;

	if (!inet_aton(PTP_PDELAY_MCAST_IPADDR, &mcast_addr[MC_PDELAY]))
		return -1;

	efd = _udpOpenSocket(name, mcast_addr, PTP_PORT_EVENT, ttl);
	if (efd < 0)
		goto no_event;

	gfd = _udpOpenSocket(name, mcast_addr, PTP_PORT_GENERAL, ttl);
	if (gfd < 0)
		goto no_general;

	if (sk_timestamping_init(efd, iface->ts_label, ts_type, TRANS_UDP_IPV4))
		goto no_timestamping;

	if (sk_general_init(gfd))
		goto no_timestamping;

	event_dscp = config_get_int(t->cfg, NULL, "dscp_event");
	general_dscp = config_get_int(t->cfg, NULL, "dscp_general");

	if (event_dscp && sk_set_priority(efd, event_dscp))
	{
		pr_warning("Failed to set event DSCP priority.");
	}

	if (general_dscp && sk_set_priority(gfd, general_dscp))
	{
		pr_warning("Failed to set general DSCP priority.");
	}

	fda->fd[FD_EVENT] = efd;
	fda->fd[FD_GENERAL] = gfd;
	return 0;

no_timestamping:
	close(gfd);
no_general:
	close(efd);
no_event:
	return -1;
}

static int _udpRecv(struct transport *t, int fd, void *buf, int buflen,
		    struct address *addr, struct hw_timestamp *hwts)
{
	return sk_receive(fd, buf, buflen, addr, hwts, 0);
}

static int _udpSend(struct transport *t, struct FdArray *fda,
		    enum transport_event event, int peer, void *buf, int len,
		    struct address *addr, struct hw_timestamp *hwts)
{
	struct address addr_buf;
	unsigned char junk[1600];
	ssize_t cnt;
	int fd = -1;

	switch (event) {
	case TRANS_GENERAL:
		fd = fda->fd[FD_GENERAL];
		break;
	case TRANS_EVENT:
	case TRANS_ONESTEP:
	case TRANS_P2P1STEP:
	case TRANS_DEFER_EVENT:
		fd = fda->fd[FD_EVENT];
		break;
	}

	if (!addr) {
		memset(&addr_buf, 0, sizeof(addr_buf));
		addr_buf.sin.sin_family = AF_INET;
		addr_buf.sin.sin_addr = peer ? mcast_addr[MC_PDELAY] :
					       mcast_addr[MC_PRIMARY];
		addr_buf.len = sizeof(addr_buf.sin);
		addr = &addr_buf;
	}

	addr->sin.sin_port = htons(event ? PTP_PORT_EVENT : PTP_PORT_GENERAL);

	/*
	 * Extend the payload by two, for UDP checksum correction.
	 * This is not really part of the standard, but it is the way
	 * that the phyter works.
	 */
	if (event == TRANS_ONESTEP)
		len += 2;

	cnt = sendto(fd, buf, len, 0, &addr->sa, sizeof(addr->sin));
	if (cnt < 1) {
		pr_err("sendto failed: %m");
		return cnt;
	}
	/*
	 * Get the time stamp right away.
	 */
	/* get timestamp of last transmition, which is used in Follow UP when software clock is used JL */
	return event == TRANS_EVENT ? sk_receive(fd, junk, len, NULL, hwts, MSG_ERRQUEUE) : cnt;
}

static void _udpRelease(struct transport *t)
{
	struct PtpUdp *udp = container_of(t, struct PtpUdp, t);
	free(udp);
}

static int _udpPhysicalAddr(struct transport *t, uint8_t *addr)
{
	struct PtpUdp *udp = container_of(t, struct PtpUdp, t);
	int len = 0;

	if (udp->mac.len)
	{
		len = MAC_LEN;
		memcpy(addr, udp->mac.sll.sll_addr, len);
	}
	
	return len;
}

static int _udpProtocolAddr(struct transport *t, uint8_t *addr)
{
	struct PtpUdp *udp = container_of(t, struct PtpUdp, t);
	int len = 0;

	if (udp->ip.len)
	{
		len = sizeof(udp->ip.sin.sin_addr.s_addr);
		memcpy(addr, &udp->ip.sin.sin_addr.s_addr, len);
	}
	return len;
}

struct transport *udp_transport_create(void)
{
	struct PtpUdp *udp = calloc(1, sizeof(*udp));
	if (!udp)
		return NULL;
	
	udp->t.close = _udpClose;
	udp->t.open  = _udpOpen;
	udp->t.recv  = _udpRecv;
	udp->t.send  = _udpSend;
	udp->t.release = _udpRelease;
	udp->t.physical_addr = _udpPhysicalAddr;
	udp->t.protocol_addr = _udpProtocolAddr;
	
	return &udp->t;
}

