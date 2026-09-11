/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * lib/libc/nls/msgcat.c: catgets(3)'s binary search over a .cat file
 * that load_msgcat() mmap()s and, before this, checked only for a size
 * of four bytes and a magic number.
 *
 *   cbmc -DOLD --unwind 12 --unwinding-assertions --bounds-check \
 *       --signed-overflow-check tools/verify/probes/msgcat_bounds.c
 *                                                    -> FAILED
 *   cbmc        --unwind 12 --unwinding-assertions --bounds-check \
 *       --signed-overflow-check tools/verify/probes/msgcat_bounds.c
 *                                                    -> SUCCESSFUL
 *
 * Two properties, on a mapping whose header fields are nondeterministic
 * because the file chooses them:
 *
 *   - `i = (l + u) / 2' overflows.  u is `ntohl(cat_hdr->__nsets) - 1',
 *     so the file picks it; a negative i then subscripts before the
 *     mapping.
 *   - the set table itself is never bounded, so even a non-negative i
 *     reads past the end.
 *
 * The fix is the pair: valid_msgcat() bounds nsets by what the mapping
 * can hold, and the midpoint is written so that it would not need it to.
 */

#define NSETS_MAX	4		/* what this small model can hold */

typedef unsigned int u_int32_t;

int nondet_int(void);

struct set_hdr {
	int setno;
	int nmsgs;
	int index;
};

static struct set_hdr sets[NSETS_MAX];

int
main(void)
{
	int nsets = nondet_int();
	int set_id = nondet_int();
	int i, l, u, r;

	for (i = 0; i < NSETS_MAX; i++) {
		sets[i].setno = nondet_int();
		sets[i].nmsgs = nondet_int();
		sets[i].index = nondet_int();
	}

#ifndef OLD
	/* valid_msgcat(): the count the file gives must fit the mapping. */
	if (nsets < 0 || nsets > NSETS_MAX)
		return (1);
#endif

	l = 0;
	u = nsets - 1;
	while (l <= u) {
#ifdef OLD
		i = (l + u) / 2;
#else
		i = l + (u - l) / 2;
#endif
		__CPROVER_assert(i >= 0 && i < NSETS_MAX,
		    "the set index is inside the mapping");
#ifdef OLD
		r = set_id - sets[i].setno;
		if (r == 0)
			return (0);
		else if (r < 0)
			u = i - 1;
		else
			l = i + 1;
#else
		/*
		 * and the difference of two int32_t -- one the caller's
		 * set_id, one the file's setno -- where only the sign is
		 * read.  There is nothing to subtract for.
		 */
		r = sets[i].setno;
		if (set_id == r)
			return (0);
		else if (set_id < r)
			u = i - 1;
		else
			l = i + 1;
#endif
	}
	return (1);
}
