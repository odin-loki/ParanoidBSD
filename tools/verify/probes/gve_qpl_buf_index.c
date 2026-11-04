/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * sys/dev/gve/gve_tx_dqo.c: two functions index tx->dqo.qpl_bufs[] on a
 * local that only a loop body assigns, and both stores happen after the
 * loop rather than inside it.  A packet whose length is zero runs
 * neither loop.
 *
 *   cbmc -DOLD --unwind 6 --bounds-check --pointer-check \
 *       tools/verify/probes/gve_qpl_buf_index.c   -> 4 of 8 failed, FAILED
 *   cbmc       --unwind 6 --bounds-check --pointer-check \
 *       tools/verify/probes/gve_qpl_buf_index.c   -> 0 of 8, SUCCESSFUL
 *
 * Four, not two: each of the two written assertions is joined by the
 * array_bounds pair CBMC generates for the store it guards, which is
 * the point -- the assertion names the invariant and the bounds checks
 * say the store itself is out of the array.
 *
 * gve_tx_copy_mbuf_and_write_pkt_descs():
 *
 *     int32_t prev_buf = -1;
 *     int32_t buf;
 *     while (copy_offset < pkt_len) {
 *             buf = gve_tx_alloc_qpl_buf(tx);
 *             ...
 *             prev_buf = buf;
 *             pkt->num_qpl_bufs++;
 *     }
 *     tx->dqo.qpl_bufs[buf] = -1;
 *
 * pkt_len is mbuf->m_pkthdr.len.  Nothing between the caller's mbuf and
 * this function requires it to be positive: gve_xmit_dqo_qpl() counts
 * segments and credits, neither of which is a length test.  With
 * pkt_len 0 the loop body never runs, `buf' holds whatever the frame
 * held, and the last line WRITES through it.  prev_buf is the same
 * value on every path that entered the loop and -1 on the one that did
 * not, which is what the fix uses.
 *
 * gve_reap_qpl_bufs_dqo() is the same shape one function along:
 *
 *     for (i = 0; i < pkt->num_qpl_bufs; i++) {
 *             ...
 *             qpl_buf_tail = buf;
 *             buf = tx->dqo.qpl_bufs[buf];
 *     }
 *     buf = qpl_buf_tail;
 *     ...
 *     tx->dqo.qpl_bufs[buf] = old_head;
 *
 * and the packet that reaches it with num_qpl_bufs 0 is the packet the
 * first defect produces, so the two are one bug seen twice.  That path
 * also publishes pkt->qpl_buf_head -- still -1 -- as the head of the
 * per-ring free list, so the damage outlives the packet.
 *
 * Modelled rather than compiled: the driver's real frame needs a
 * bus_dma tag, a completion ring and a NIC.  The index arithmetic is
 * the whole of the defect and it is all here.  An uninitialised
 * automatic is nondet_int(), which is what it is.
 */

int nondet_int(void);

#define	NQPL_BUFS	16

int
main(void)
{
	int qpl_bufs[NQPL_BUFS];
	int pkt_len = nondet_int();
	int num_qpl_bufs = 0;
	int copy_offset = 0;
	int prev_buf = -1;
	int qpl_buf_tail;
	int alloc_next = 0;
	int buf;
	int i;

	__CPROVER_assume(pkt_len >= 0 && pkt_len <= 3);

	/* gve_tx_copy_mbuf_and_write_pkt_descs(): the writing path. */
	buf = nondet_int();		/* uninitialised automatic */
	while (copy_offset < pkt_len) {
		buf = alloc_next++;
		__CPROVER_assume(buf >= 0 && buf < NQPL_BUFS);
		copy_offset += 1;
		if (prev_buf != -1)
			qpl_bufs[prev_buf] = buf;
		prev_buf = buf;
		num_qpl_bufs++;
	}
#ifdef OLD
	__CPROVER_assert(buf >= 0 && buf < NQPL_BUFS,
	    "the list terminator subscripts qpl_bufs[] inside the array");
	qpl_bufs[buf] = -1;
#else
	if (prev_buf != -1) {
		__CPROVER_assert(prev_buf >= 0 && prev_buf < NQPL_BUFS,
		    "the list terminator subscripts qpl_bufs[] inside the array");
		qpl_bufs[prev_buf] = -1;
	}
#endif

	/* gve_reap_qpl_bufs_dqo(): the same shape, on the same packet. */
	qpl_buf_tail = nondet_int();	/* uninitialised automatic */
	buf = 0;
#ifndef OLD
	if (num_qpl_bufs == 0)
		return (0);
#endif
	for (i = 0; i < num_qpl_bufs; i++) {
		qpl_buf_tail = buf;
		__CPROVER_assume(buf >= 0 && buf < NQPL_BUFS);
		buf = qpl_bufs[buf];
		if (buf < 0)
			buf = 0;
	}
	buf = qpl_buf_tail;
	__CPROVER_assert(buf >= 0 && buf < NQPL_BUFS,
	    "the free-list publish subscripts qpl_bufs[] inside the array");

	return (0);
}
