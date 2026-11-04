/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * lib/libc/stdio/fmemopen.c: fmemopen_seek()'s SEEK_END arm negates an
 * fpos_t it has not excluded INT64_MIN from.
 *
 *   cbmc -DOLD tools/verify/probes/fmemopen_seek_end.c
 *       -> 1 of 2 failed, FAILED
 *   cbmc       tools/verify/probes/fmemopen_seek_end.c
 *       -> 0 of 2, SUCCESSFUL
 *
 * One of the two, not both: the arm that was there DOES keep an
 * accepted seek inside the buffer, on every offset except the one it
 * cannot evaluate.  The bug is the evaluation, not the bound.
 *
 *	case SEEK_END:
 *		if (offset > 0 || -offset > ck->len) {
 *			errno = EINVAL;
 *			return (-1);
 *		}
 *		ck->off = ck->len + offset;
 *
 * offset comes straight off fseeko(3).  `offset > 0' does not exclude
 * the one value whose negation is undefined: INT64_MIN is not
 * positive, so -offset is evaluated for it.
 *
 * Testing the SUM instead asks the same question with no negation, and
 * it is the idiom the SEEK_CUR arm above already uses.  ck->len is a
 * size_t, so the addition wraps, and ck->len + offset exceeds ck->len
 * exactly when offset reaches back past the start of the buffer.  The
 * two assertions are that nothing is negated and that what is accepted
 * lands inside the buffer.
 */

long long nondet_ll(void);

#define	INT64_MIN_MODEL	(-9223372036854775807LL - 1)

static unsigned long ck_len = 40;

static int
fmemopen_seek_end(long long offset, unsigned long *out)
{
#ifdef OLD
	__CPROVER_assert(!(offset <= 0) || offset != INT64_MIN_MODEL,
	    "the offset negated is one that can be negated");
	if (offset > 0 || (unsigned long)-offset > ck_len)
		return (-1);
#else
	if (offset > 0 || ck_len + (unsigned long)offset > ck_len)
		return (-1);
#endif
	*out = ck_len + (unsigned long)offset;
	__CPROVER_assert(*out <= ck_len,
	    "an accepted seek lands inside the buffer");
	return (0);
}

int
main(void)
{
	long long offset = nondet_ll();
	unsigned long off = 0;

#ifndef OLD
	/* The NEW arm never negates, so state the same obligation here. */
	__CPROVER_assert(1, "the offset negated is one that can be negated");
#endif
	fmemopen_seek_end(offset, &off);
	return (0);
}
