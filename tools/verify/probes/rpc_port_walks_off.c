/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/libsa/rpc.c: rpc_port starts at 0x400, is predecremented at
 * every call site and is never reset.
 *
 *   cbmc --signed-overflow-check --unwind 1100 --unwinding-assertions \
 *        -DOLD tools/verify/probes/rpc_port_walks_off.c
 *       -> the port is in the reserved range: FAILED
 *   cbmc --signed-overflow-check --unwind 1100 --unwinding-assertions \
 *        tools/verify/probes/rpc_port_walks_off.c
 *       -> 0 of N, SUCCESSFUL
 *
 *	int rpc_port = 0x400;	// predecrement
 *	...
 *	d->myport = htons(--rpc_port);		bootparam.c:155
 *	d->myport = htons(--rpc_port);		bootparam.c:269
 *	desc->myport = htons(--rpc_port);	nfs.c:504
 *
 * After 1023 calls rpc_port is 1; after 1024 it is 0; after that it is
 * negative, and htons() of a negative int is a source port the server
 * will not answer.  The decrement is undefined at INT_MIN.
 *
 * A netboot reaches 1024 RPC calls by retrying: every readlink, every
 * lookup and every read of every module is a call.
 *
 * The assertion is that the port stays in the privileged range the
 * predecrement was reaching for, 1..1023.
 */

static int rpc_port = 0x400;

static int
nextport(void)
{
#ifdef OLD
	return (--rpc_port);
#else
	if (--rpc_port < 1)
		rpc_port = 0x400 - 1;
	return (rpc_port);
#endif
}

int
main(void)
{
	int i, p;

	for (i = 0; i < 1030; i++) {
		p = nextport();
		__CPROVER_assert(p >= 1 && p <= 1023,
		    "the port is in the reserved range");
	}
	return (0);
}
