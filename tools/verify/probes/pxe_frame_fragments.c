/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/i386/libi386/pxe.c: pxe_netif_receive_isr() sizes its receive
 * buffer from the FIRST UNDI buffer's FrameLength and then appends
 * every following BufferLength to it, testing the total only AFTER
 * the copy.
 *
 *   cbmc -DOLD --unwind 6 tools/verify/probes/pxe_frame_fragments.c
 *       -> 1 of 2 failed, FAILED
 *   cbmc       --unwind 6 tools/verify/probes/pxe_frame_fragments.c
 *       -> 0 of 2, SUCCESSFUL
 *
 * The second assertion - that the running total does not wrap - holds
 * either way here, because this probe bounds each BufferLength at
 * 2048 to keep the unwinding small.  It is stated so that a later
 * widening of that bound cannot make the overflow look fixed when it
 * has only changed shape.
 *
 *	if (buf == NULL) {
 *		size = isr->FrameLength;
 *		buf = malloc(size + ETHER_ALIGN);
 *		if (buf == NULL)
 *			return (ENOMEM);
 *		ptr = buf + ETHER_ALIGN;
 *	}
 *	...
 *	bcopy(PTOV(frame), ptr, isr->BufferLength);
 *	ptr += isr->BufferLength;
 *	rsize += isr->BufferLength;
 *	if (rsize >= size) {
 *		data_pending = true;
 *		break;
 *	}
 *
 * FrameLength and BufferLength are separate fields of the
 * t_PXENV_UNDI_ISR the PXE firmware fills in on each
 * PXENV_UNDI_ISR_IN_GET_NEXT, so nothing makes the fragments add up
 * to the length the first one declared.  The very first BufferLength
 * can already exceed it, and the `rsize >= size' test cannot help
 * because it runs after the bytes are in.
 *
 * This is the loader's PXE receive path: it is how a diskless machine
 * gets its DHCP reply, its TFTP blocks and its NFS traffic, and it
 * runs before the kernel and therefore before every HardenedBSD
 * mitigation.
 *
 * The assertion is that no byte is written past the end of buf.
 */

#define	ETHER_ALIGN	2
#define	ENXIO		6

unsigned nondet_uint(void);

int
main(void)
{
	unsigned size = 0, rsize = 0, cap = 0;
	unsigned buflen;
	int first = 1;
	int i;

	for (i = 0; i < 4; i++) {
		buflen = nondet_uint();
		__CPROVER_assume(buflen <= 2048);
		if (first) {
			size = nondet_uint();
			__CPROVER_assume(size <= 2048);
			cap = size;	/* buf holds size bytes past ETHER_ALIGN */
			first = 0;
		}
#ifndef OLD
		if (buflen > size - rsize)
			return (ENXIO);
#endif
		__CPROVER_assert(rsize + buflen <= cap,
		    "the fragment fits the buffer the first frame length bought");
		__CPROVER_assert(rsize + buflen >= rsize,
		    "the running total does not wrap");
		rsize += buflen;
		if (rsize >= size)
			break;
	}
	return (0);
}
