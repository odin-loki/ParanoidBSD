/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * lib/libc/rpc: the netbuf uaddr2taddr() returns owns two allocations
 * and two of its four call sites freed one.
 *
 *   gcc -DOLD -fsanitize=address -g -O1 -o /tmp/p \
 *       tools/verify/probes/rpc_netbuf_free.c
 *   ASAN_OPTIONS=detect_leaks=1 /tmp/p
 *       -> SUMMARY: AddressSanitizer: 32 byte(s) leaked in 2 allocation(s).
 *   gcc       -fsanitize=address -g -O1 -o /tmp/p \
 *       tools/verify/probes/rpc_netbuf_free.c
 *   ASAN_OPTIONS=detect_leaks=1 /tmp/p
 *       -> clean, exit 0
 *
 * gcc rather than clang only because this container has libasan and not
 * clang's compiler-rt; the probe is plain C and either will build it.
 *
 * __rpc_uaddr2taddr_af() (rpc_generic.c) does
 *
 *	ret = (struct netbuf *)malloc(sizeof *ret);
 *	...
 *	sin = (struct sockaddr_in *)malloc(sizeof *sin);
 *	...
 *	ret->buf = sin;
 *
 * so the netbuf and the sockaddr are separate allocations and the
 * caller owns both.  __rpcb_findaddr_timed() in rpcb_clnt.c writes it
 * as `free(na->buf); free(na);'.  pmap_set() and rpc_broadcast_exp()
 * wrote `free(na)'.
 *
 * pmap_set()'s is once per call.  rpc_broadcast_exp()'s is inside the
 * loop over broadcast REPLIES, so its size is the network's to choose;
 * and there np can be NULL for a uaddr the transport cannot parse,
 * which the bare free() tolerated and `free(np->buf)' does not, so the
 * fix there carries a guard the other does not need.
 *
 * This models the ownership, not the parser: the property is that both
 * allocations are released, and a model of malloc is enough to state
 * it.  The real uaddr2taddr() is exercised by the tree's own RPC tests.
 */

#include <stdlib.h>
#include <string.h>

struct sockaddr_in_model {
	unsigned short	sin_family;
	unsigned short	sin_port;
	unsigned int	sin_addr;
	char		sin_zero[8];
};

struct netbuf_model {
	unsigned int	maxlen;
	unsigned int	len;
	void		*buf;
};

/* __rpc_uaddr2taddr_af(), reduced to its allocations. */
static struct netbuf_model *
uaddr2taddr_model(int parses)
{
	struct netbuf_model *ret;
	struct sockaddr_in_model *sin;

	if (!parses)
		return (NULL);		/* a uaddr this transport cannot read */
	ret = malloc(sizeof(*ret));
	if (ret == NULL)
		return (NULL);
	sin = malloc(sizeof(*sin));
	if (sin == NULL) {
		free(ret);
		return (NULL);
	}
	memset(sin, 0, sizeof(*sin));
	ret->buf = sin;
	ret->len = ret->maxlen = sizeof(*sin);
	return (ret);
}

int
main(void)
{
	struct netbuf_model *na, *np;

	/* pmap_set(): na is NULL-checked above the free, so no guard. */
	na = uaddr2taddr_model(1);
	if (na == NULL)
		return (1);
#ifndef OLD
	free(na->buf);
#endif
	free(na);

	/* rpc_broadcast_exp(): once per reply, and np may be NULL. */
	for (int i = 0; i < 2; i++) {
		np = uaddr2taddr_model(i == 0);
#ifndef OLD
		if (np != NULL)
			free(np->buf);
#endif
		free(np);
	}
	return (0);
}
