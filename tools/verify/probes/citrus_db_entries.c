/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * lib/libc/iconv/citrus_db.c's one bound on dhx_num_entries, and the
 * two scalings that trust it.  The number is read out of a .db file the
 * library maps for iconv(3).
 *
 *   cbmc -DOLD --unwind 4 --signed-overflow-check --conversion-check \
 *       tools/verify/probes/citrus_db_entries.c      -> FAILED
 *   cbmc        --unwind 4 --signed-overflow-check --conversion-check \
 *       tools/verify/probes/citrus_db_entries.c      -> SUCCESSFUL
 *
 * _citrus_db_open() rejected a file whose entry table does not fit:
 *
 *	if (be32toh(dhx->dhx_num_entries)*_CITRUS_DB_ENTRY_SIZE >
 *	    _memstream_remainder(&ms))
 *		return (EFTYPE);
 *
 * be32toh() is uint32_t and _CITRUS_DB_ENTRY_SIZE is a plain 24, so the
 * product is uint32_t arithmetic and wraps.  178956971 * 24 is
 * 4294967304, which is 8 once it has wrapped -- under the remainder of
 * any file at all.  Dividing instead is the same test with nothing to
 * wrap.
 *
 * _citrus_db_get_entry() then scales an int index by the same constant,
 * in int, so an index the accepted num_entries permits is signed
 * overflow before the seek that would have rejected the offset.
 */

#define ENTRY_SIZE	24

typedef unsigned int uint32_t;
typedef unsigned long size_t_;

uint32_t nondet_u32(void);
size_t_ nondet_size(void);
int nondet_int(void);

int
main(void)
{
	uint32_t num_entries = nondet_u32();
	size_t_ remainder = nondet_size();
	size_t_ offset;
	int idx;

	/* a mapping is not bigger than the address space it lives in */
	__CPROVER_assume(remainder <= 0x40000000UL);

#ifdef OLD
	if (num_entries * ENTRY_SIZE > remainder)
		return (1);			/* EFTYPE */
#else
	if (num_entries > remainder / ENTRY_SIZE)
		return (1);			/* EFTYPE */
#endif

	/*
	 * The table was accepted, so every entry it claims must lie
	 * inside it.  That is the whole content of the check.
	 */
	__CPROVER_assert((size_t_)num_entries * ENTRY_SIZE <= remainder,
	    "an accepted entry table fits the mapping");

	idx = nondet_int();
	if (idx < 0 || (uint32_t)idx >= num_entries)
		return (1);			/* EINVAL */

#ifdef OLD
	offset = idx * ENTRY_SIZE;		/* int arithmetic */
#else
	offset = (size_t_)idx * ENTRY_SIZE;
#endif
	__CPROVER_assert(offset <= remainder,
	    "the entry the index names is inside the table");
	return (0);
}
