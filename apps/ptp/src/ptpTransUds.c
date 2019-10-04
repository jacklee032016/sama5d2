/**
 * @file uds.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "ptpCompact.h"
#include "ptpProtocol.h"
#include "ptpImplements.h"
#include "ptpConfig.h"
#include "transportPrivate.h"

#define UDS_FILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) /*0660*/

struct UDS
{
	struct transport	t;
	struct address	address;
};

static int _udsClose(struct transport *t, struct FdArray *fda)
{
	struct sockaddr_un sa;
	socklen_t len = sizeof(sa);

	if (!getsockname(fda->fd[FD_GENERAL], (struct sockaddr *) &sa, &len) && sa.sun_family == AF_LOCAL)
	{
		unlink(sa.sun_path);
	}

	close(fda->fd[FD_GENERAL]);
	return 0;
}

static int _udsOpen(struct transport *t, struct PtpInterface *iface, struct FdArray *fda, enum timestamp_type tt)
{
	int fd, err;
	struct sockaddr_un sa;
	struct UDS *uds = container_of(t, struct UDS, t);
	char *uds_path = config_get_string(t->cfg, NULL, "uds_address");
	char *name = iface->name;

	fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		pr_err("uds: failed to create socket: %m");
		return -1;
	}
	
	memset(&sa, 0, sizeof(sa));
	sa.sun_family = AF_LOCAL;
	strncpy(sa.sun_path, name, sizeof(sa.sun_path) - 1);

	unlink(name);

	err = bind(fd, (struct sockaddr *) &sa, sizeof(sa));
	if (err < 0)
	{
		pr_err("uds: bind to %s failed: %m", name);
		close(fd);
		return -1;
	}

	/* For client use, pre load the server path. */
	memset(&sa, 0, sizeof(sa));
	sa.sun_family = AF_LOCAL;
	strncpy(sa.sun_path, uds_path, sizeof(sa.sun_path) - 1);
	uds->address.sun = sa;
	uds->address.len = sizeof(sa);

	chmod(name, UDS_FILEMODE);
	
	fda->fd[FD_EVENT] = -1;
	fda->fd[FD_GENERAL] = fd;

	return 0;
}

static int _udsRecv(struct transport *t, int fd, void *buf, int buflen, struct address *addr, struct hw_timestamp *hwts)
{
	int cnt;
	struct UDS *uds = container_of(t, struct UDS, t);

	addr->len = sizeof(addr->sun);
	
	cnt = recvfrom(fd, buf, buflen, 0, &addr->sa, &addr->len);
	if (cnt <= 0)
	{
		pr_err("uds: recvfrom failed: %m");
		return cnt;
	}
	uds->address = *addr;
	return cnt;
}

static int _udsSend(struct transport *t, struct FdArray *fda, 
		    enum transport_event event, int peer, void *buf, int buflen,
		    struct address *addr, struct hw_timestamp *hwts)
{
	int cnt, fd = fda->fd[FD_GENERAL];
	struct UDS *uds = container_of(t, struct UDS, t);

	if (!addr)
		addr = &uds->address;

	cnt = sendto(fd, buf, buflen, 0, &addr->sa, addr->len);
	if (cnt <= 0 && errno != ECONNREFUSED)
	{
		pr_err("uds: sendto failed: %m");
	}
	return cnt;
}

static void _udsRelease(struct transport *t)
{
	struct UDS *uds = container_of(t, struct UDS, t);
	free(uds);
}

struct transport *uds_transport_create(void)
{
	struct UDS *uds;
	uds = calloc(1, sizeof(*uds));
	if (!uds)
		return NULL;
	
	uds->t.close   = _udsClose;
	uds->t.open    = _udsOpen;
	uds->t.recv    = _udsRecv;
	uds->t.send    = _udsSend;
	uds->t.release = _udsRelease;
	return &uds->t;
}

