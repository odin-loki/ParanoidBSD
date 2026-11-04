/*
 * Batch b0196s3 oracle: original HardenedBSD C sources concatenated.
 * Each function renamed with ref_ prefix; bodies otherwise unmodified.
 * Infrastructure (types, mocks) added for standalone compilation.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef IPPROTO_UDP
#define IPPROTO_UDP 17
#endif
#ifndef IPPROTO_TCP
#define IPPROTO_TCP 6
#endif

typedef int bool_t;
#define TRUE 1
#define FALSE 0

typedef uint32_t rpcprog_t;
typedef uint32_t rpcvers_t;
typedef uint32_t u_long;
typedef uint32_t u_int32_t;

struct netbuf {
	unsigned int maxlen;
	unsigned int len;
	void *buf;
};

struct netconfig {
	char *nc_netid;
	unsigned long nc_semantics;
	unsigned long nc_flag;
	char *nc_protofmly;
	char *nc_proto;
	char *nc_device;
	unsigned long nc_nlookups;
	char **nc_lookups;
	unsigned long nc_unused[9];
};

#define NC_LOOPBACK "loopback"

/* ---- mock control (harness-visible) ---- */

int mock_getconfip_udp_null = 0;
int mock_getconfip_tcp_null = 0;
int mock_uaddr2taddr_null = 0;
bool_t mock_rpcb_set_result = TRUE;
bool_t mock_rpcb_unset_udp_result = FALSE;
bool_t mock_rpcb_unset_tcp_result = FALSE;

void
mock_reset_b0196s3(void)
{
	mock_getconfip_udp_null = 0;
	mock_getconfip_tcp_null = 0;
	mock_uaddr2taddr_null = 0;
	mock_rpcb_set_result = TRUE;
	mock_rpcb_unset_udp_result = FALSE;
	mock_rpcb_unset_tcp_result = FALSE;
}

static int
validate_uaddr(const char *addr)
{
	unsigned hi, lo;

	if (addr == NULL)
		return 0;
	if (sscanf(addr, "0.0.0.0.%u.%u", &hi, &lo) != 2)
		return 0;
	if (hi > 255U || lo > 255U)
		return 0;
	return 1;
}

struct netconfig *
__rpc_getconfip(const char *proto)
{
	static struct netconfig udp_nc;
	static struct netconfig tcp_nc;

	if (strcmp(proto, "udp") == 0) {
		if (mock_getconfip_udp_null)
			return NULL;
		udp_nc.nc_protofmly = NC_LOOPBACK;
		udp_nc.nc_proto = "udp";
		return &udp_nc;
	}
	if (strcmp(proto, "tcp") == 0) {
		if (mock_getconfip_tcp_null)
			return NULL;
		tcp_nc.nc_protofmly = NC_LOOPBACK;
		tcp_nc.nc_proto = "tcp";
		return &tcp_nc;
	}
	return NULL;
}

void
freenetconfigent(struct netconfig *nc)
{
	(void)nc;
}

struct netbuf *
uaddr2taddr(const struct netconfig *nc, const char *addr)
{
	struct netbuf *nb;

	(void)nc;
	if (mock_uaddr2taddr_null)
		return NULL;
	if (!validate_uaddr(addr))
		return NULL;
	nb = (struct netbuf *)malloc(sizeof(struct netbuf));
	if (nb == NULL)
		return NULL;
	nb->maxlen = 16;
	nb->len = 4;
	nb->buf = (void *)0x1234;
	return nb;
}

bool_t
rpcb_set(rpcprog_t program, rpcvers_t version,
    const struct netconfig *nconf, const struct netbuf *na)
{
	(void)program;
	(void)version;
	(void)nconf;
	(void)na;
	return mock_rpcb_set_result;
}

bool_t
rpcb_unset(rpcprog_t program, rpcvers_t version,
    const struct netconfig *nconf)
{
	(void)program;
	(void)version;
	if (nconf != NULL && nconf->nc_proto != NULL &&
	    strcmp(nconf->nc_proto, "udp") == 0)
		return mock_rpcb_unset_udp_result;
	if (nconf != NULL && nconf->nc_proto != NULL &&
	    strcmp(nconf->nc_proto, "tcp") == 0)
		return mock_rpcb_unset_tcp_result;
	return FALSE;
}

/* ===== pmap_clnt.c ===== */

/*	$NetBSD: pmap_clnt.c,v 1.16 2000/07/06 03:10:34 christos Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * pmap_clnt.c
 * Client interface to pmap rpc service.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

bool_t
ref_pmap_set(u_long program, u_long version, int protocol, int port)
{
	bool_t rslt;
	struct netbuf *na;
	struct netconfig *nconf;
	char buf[32];

	if ((protocol != IPPROTO_UDP) && (protocol != IPPROTO_TCP)) {
		return (FALSE);
	}
	nconf = __rpc_getconfip(protocol == IPPROTO_UDP ? "udp" : "tcp");
	if (nconf == NULL) {
		return (FALSE);
	}
	snprintf(buf, sizeof buf, "0.0.0.0.%d.%d", 
	    (((u_int32_t)port) >> 8) & 0xff, port & 0xff);
	na = uaddr2taddr(nconf, buf);
	if (na == NULL) {
		freenetconfigent(nconf);
		return (FALSE);
	}
	rslt = rpcb_set((rpcprog_t)program, (rpcvers_t)version, nconf, na);
	free(na);
	freenetconfigent(nconf);
	return (rslt);
}

/*
 * Remove the mapping between program, version and port.
 * Calls the pmap service remotely to do the un-mapping.
 */
bool_t
ref_pmap_unset(u_long program, u_long version)
{
	struct netconfig *nconf;
	bool_t udp_rslt = FALSE;
	bool_t tcp_rslt = FALSE;

	nconf = __rpc_getconfip("udp");
	if (nconf != NULL) {
		udp_rslt = rpcb_unset((rpcprog_t)program, (rpcvers_t)version,
		    nconf);
		freenetconfigent(nconf);
	}
	nconf = __rpc_getconfip("tcp");
	if (nconf != NULL) {
		tcp_rslt = rpcb_unset((rpcprog_t)program, (rpcvers_t)version,
		    nconf);
		freenetconfigent(nconf);
	}
	/*
	 * XXX: The call may still succeed even if only one of the
	 * calls succeeded.  This was the best that could be
	 * done for backward compatibility.
	 */
	return (tcp_rslt || udp_rslt);
}
