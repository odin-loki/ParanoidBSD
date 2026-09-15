/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * lib/libc/iconv/citrus_pivot_factory.c: dump_db() left `ret' at 0 on
 * both of its malloc failures, and 0 is what the caller reads as
 * success.
 *
 *   cbmc -DOLD --unwind 6 tools/verify/probes/citrus_dump_db_ret.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       --unwind 6 tools/verify/probes/citrus_dump_db_ret.c
 *       -> 0 of 1, SUCCESSFUL
 *
 * The real caller is
 *
 *     ret = dump_db(&sh, &r);
 *     ...
 *     if (ret)
 *             return (ret);
 *     if (fwrite(_region_head(&r), _region_size(&r), 1, out) != 1)
 *
 * so the invariant is one line: when dump_db() returns 0, *r has been
 * written.  `initialised' below stands for "_region_init(r, ...) ran",
 * which is the only thing that writes it.
 *
 * The sibling file, citrus_lookup_factory.c, returns errno in the same
 * place and has always been correct.  That difference is the whole
 * defect, and it is why the -DOLD arm models only the pivot version.
 */

int nondet_int(void);

static int r_initialised;

static int
dump_db(int entries, int malloc_fails_at)
{
	int ret;
	int i;

	ret = nondet_int();		/* _db_factory_create() */
	if (ret)
		return (ret);
	/* ret is 0 from here on unless something sets it. */

	for (i = 0; i < entries; i++) {
		if (i == malloc_fails_at) {	/* per-entry malloc */
#ifndef OLD
			ret = 1;		/* errno; never 0 */
#endif
			goto quit;
		}
		ret = nondet_int();		/* _db_factory_serialize() */
		if (ret)
			goto quit;
		ret = nondet_int();		/* _db_factory_add_by_s() */
		if (ret)
			goto quit;
	}

	if (malloc_fails_at == entries) {	/* the malloc for *r */
#ifndef OLD
		ret = 1;
#endif
		goto quit;
	}
	r_initialised = 1;			/* _region_init(r, ...) */
	ret = nondet_int();			/* _db_factory_serialize() */

quit:
	return (ret);
}

int
main(void)
{
	int entries = nondet_int();
	int fails_at = nondet_int();
	int ret;

	__CPROVER_assume(entries >= 0 && entries <= 3);
	__CPROVER_assume(fails_at >= -1 && fails_at <= entries);

	r_initialised = 0;
	ret = dump_db(entries, fails_at);
	if (ret != 0)
		return (0);			/* the caller returns here */

	/* The caller now does fwrite(_region_head(&r), _region_size(&r)). */
	__CPROVER_assert(r_initialised,
	    "dump_db returned 0, so *r has been written");
	return (0);
}
