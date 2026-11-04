/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * lib/libc/db/hash's init_htab(), driven with HASHINFO.nelem and
 * HASHINFO.ffactor -- dbopen(3)'s own caller-supplied numbers.
 *
 *   cbmc -DOLD --unwind 40 --unwinding-assertions --bounds-check \
 *       --signed-overflow-check --conversion-check \
 *       tools/verify/probes/hash_init_htab_nelem.c   -> FAILED
 *   cbmc        --unwind 40 --unwinding-assertions --bounds-check \
 *       --signed-overflow-check --conversion-check \
 *       tools/verify/probes/hash_init_htab_nelem.c   -> SUCCESSFUL
 *
 * Three failures, all reached from the same two numbers:
 *
 *   - nelem is an int, so an info->nelem above INT_MAX arrives negative
 *     and `nelem - 1' is signed overflow on INT_MIN;
 *   - `l2' is log2 of the element count over the fill factor, so
 *     ffactor 1 with nelem INT_MAX gives 31, and `1 << 31' in an int is
 *     signed overflow;
 *   - spares[] holds NCACHED entries and the same l2 indexes [l2 + 1],
 *     which at 31 is one past the array -- over bitmaps[], whose
 *     entries are page numbers the table reads back.
 *
 * Nothing allocates between init_htab()'s entry and the store, so there
 * is no failure in between to stop it.
 */

#define NCACHED		32
#define DEF_FFACTOR	65536
#define INT_MAX		2147483647
#define MAX(a, b)	((a) < (b) ? (b) : (a))

typedef unsigned int u_int32_t;

unsigned int nondet_uint(void);

static u_int32_t
log2_(u_int32_t num)
{
	u_int32_t i, limit;

	limit = 1;
	for (i = 0; i < 32 && limit < num; limit = limit << 1, i++)
		;
	return (i);
}

int
main(void)
{
	u_int32_t info_nelem = nondet_uint();
	u_int32_t info_ffactor = nondet_uint();
	u_int32_t ffactor = DEF_FFACTOR;
	int spares[NCACHED];
	int nelem, l2, nbuckets;
	u_int32_t high_mask;

	__CPROVER_assume(info_nelem != 0);
	if (info_ffactor)
		ffactor = info_ffactor;

#ifndef OLD
	if (info_nelem > INT_MAX)
		return (1);			/* EINVAL */
#endif
	nelem = (int)info_nelem;		/* OLD: negative above INT_MAX */

	nelem = (nelem - 1) / ffactor + 1;	/* OLD: INT_MIN - 1 */

	l2 = (int)log2_((u_int32_t)MAX(nelem, 2));

#ifndef OLD
	if (l2 > NCACHED - 2)
		return (1);			/* EINVAL, before the shift */
#endif
	nbuckets = 1 << l2;			/* OLD: signed overflow at 31 */

	spares[l2] = l2 + 1;
	spares[l2 + 1] = l2 + 1;		/* OLD: one past the array */

	high_mask = ((u_int32_t)nbuckets << 1) - 1;

	__CPROVER_assert(l2 >= 1 && l2 <= NCACHED - 2,
	    "the splitpoint fits the spares table");
	__CPROVER_assert(nbuckets > 0 && high_mask >= (u_int32_t)nbuckets,
	    "the bucket count and its mask agree");
	return (0);
}
