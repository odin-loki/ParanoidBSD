/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/libsa/rpc.c: rpc_getport() guards on `cc < sizeof(*res)' with
 * cc an ssize_t, so the comparison is UNSIGNED and rpc_call()'s -1
 * reads as SIZE_MAX.
 *
 *   cbmc -DOLD tools/verify/probes/rpc_getport_signed.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       tools/verify/probes/rpc_getport_signed.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	ssize_t cc;
 *	...
 *	cc = rpc_call(d, PMAPPROG, PMAPVERS, PMAPPROC_GETPORT,
 *	    args, sizeof(*args), (void **)&res, &pkt);
 *	if (cc < sizeof(*res)) {
 *		...
 *		return (-1);
 *	}
 *	port = (int)ntohl(res->port);
 *
 * The usual arithmetic conversions promote cc to size_t, so -1 becomes
 * 0xffffffffffffffff and `-1 < 4' is false: the failure arm is never
 * taken.  rpc_call() returns -1 from eleven places and writes neither
 * *rdata nor *pkt on any of them - rpc_getport() itself is called from
 * its first line - so `res' is still the uninitialised local it was
 * declared as when res->port is read.
 *
 * ether.c:70, ether.c:98 and udp.c:119 all write `n == -1 || n <
 * sizeof(...)' for the same shape.  This was the one site in
 * stand/libsa that left the first half out.
 *
 * A diskless boot reaches it: rpc_call() fails whenever the portmapper
 * does not answer, answers short, or answers with a rejection, and any
 * host on the boot network can arrange all three.
 *
 * The assertion is that res is only dereferenced after rpc_call() has
 * set it.
 */

typedef long ssize_t;
typedef unsigned long size_t;

struct res { unsigned int port; };

int nondet_int(void);

/* rpc_call(): -1 on failure, leaving *rdata untouched. */
static ssize_t
rpc_call(struct res **rdata, int *set)
{
	if (nondet_int()) {
		*rdata = (struct res *)0x1000;
		*set = 1;
		return ((ssize_t)sizeof(struct res));
	}
	return (-1);
}

int
main(void)
{
	struct res *res;
	ssize_t cc;
	int set = 0;

	cc = rpc_call(&res, &set);
#ifdef OLD
	if (cc < sizeof(*res))
		return (-1);
#else
	if (cc < 0 || (size_t)cc < sizeof(*res))
		return (-1);
#endif
	__CPROVER_assert(set == 1,
	    "res is dereferenced only after rpc_call() has set it");
	return (0);
}
