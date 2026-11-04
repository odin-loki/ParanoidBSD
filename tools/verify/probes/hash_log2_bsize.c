/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * lib/libc/db/hash's __log2() and the `1 << BSHIFT' in __hash_open()
 * that consumes it, driven with HASHINFO.bsize -- dbopen(3)'s own
 * caller-supplied number.
 *
 *   cbmc -DOLD --unwind 40 --unwinding-assertions --signed-overflow-check \
 *       tools/verify/probes/hash_log2_bsize.c        -> FAILED
 *   cbmc        --unwind 40 --unwinding-assertions --signed-overflow-check \
 *       tools/verify/probes/hash_log2_bsize.c        -> SUCCESSFUL
 *
 * The old loop has no termination condition of its own: `limit' is
 * u_int32_t, so for bsize above 2^31 the shift wraps it to 0 on the
 * thirty-second round and `limit < num' is true forever. CBMC reports
 * that as the unwinding assertion. The shift is the second half: 1 is
 * an int, so `1 << 31' -- __log2's answer for a bsize just over 2^30 --
 * is signed overflow, and the old code tested the shifted value
 * AFTERWARDS.
 */

#define MAX_BSIZE 32768

typedef unsigned int u_int32_t;

unsigned int nondet_uint(void);

static u_int32_t
log2_(u_int32_t num)
{
	u_int32_t i, limit;

	limit = 1;
#ifdef OLD
	for (i = 0; limit < num; limit = limit << 1, i++)
		;
#else
	for (i = 0; i < 32 && limit < num; limit = limit << 1, i++)
		;
#endif
	return (i);
}

int
main(void)
{
	u_int32_t bsize = nondet_uint();
	int bshift, bsz;

	__CPROVER_assume(bsize != 0);

	bshift = (int)log2_(bsize);

#ifdef OLD
	bsz = 1 << bshift;			/* signed overflow at 31 */
	if (bsz > MAX_BSIZE)
		return (1);			/* EINVAL, too late */
#else
	if (bshift >= 32 || (1U << bshift) > MAX_BSIZE)
		return (1);			/* EINVAL, before the shift */
	bsz = 1 << bshift;
#endif
	__CPROVER_assert(bsz > 0 && bsz <= MAX_BSIZE,
	    "the bucket size is a sane power of two");
	return (0);
}
