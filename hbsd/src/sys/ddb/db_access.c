/*-
 * SPDX-License-Identifier: MIT-CMU
 *
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date:	7/90
 */

#include <sys/param.h>
#include <sys/kdb.h>
#include <sys/endian.h>

#include <ddb/ddb.h>
#include <ddb/db_access.h>

/*
 * Access unaligned data items on aligned (longword)
 * boundaries.
 */

static unsigned db_extend[] = {	/* table for sign-extending */
	0,
	0xFFFFFF80U,
	0xFFFF8000U,
	0xFF800000U
};

db_expr_t
db_get_value(db_addr_t addr, int size, bool is_signed)
{
	char		data[sizeof(uint64_t)];
	db_expr_t	value;
	uint64_t	uvalue;
	int		i;

	if (db_read_bytes(addr, size, data) != 0) {
		db_printf("*** error reading from address %llx ***\n",
		    (long long)addr);
		kdb_reenter();
	}

	/*
	 * PBSD: accumulate unsigned.  db_expr_t is SIGNED -- `long' on
	 * amd64 and arm64, `int' on i386 and arm -- and this loop shifted
	 * it left eight bits per byte read, so the last byte of a
	 * full-width read moves a set bit into the sign bit.  That is
	 * undefined, and on the 32-bit targets it is not an exotic case:
	 * it happens on every `x/x' of a word whose top bit is set.
	 *
	 * uint64_t is the width of `data' and shifts into its top bit are
	 * defined.  The narrowing back to db_expr_t is
	 * implementation-defined rather than undefined, and is two's
	 * complement on every target this tree builds -- which is the bit
	 * pattern the old expression was already producing, by a route
	 * the standard does not define.
	 *
	 * The db_extend[] sign extension below is unaffected: it only runs
	 * for size < 4, where value is small and positive either way.
	 */
	uvalue = 0;
#if _BYTE_ORDER == _BIG_ENDIAN
	for (i = 0; i < size; i++)
#else	/* _LITTLE_ENDIAN */
	for (i = size - 1; i >= 0; i--)
#endif
	{
	    uvalue = (uvalue << 8) | (uint64_t)(data[i] & 0xFF);
	}
	value = (db_expr_t)uvalue;

	if (size < 4) {
	    if (is_signed && (value & db_extend[size]) != 0)
		value |= db_extend[size];
	}
	return (value);
}

void
db_put_value(db_addr_t addr, int size, db_expr_t value)
{
	char		data[sizeof(int)];
	int		i;

#if _BYTE_ORDER == _BIG_ENDIAN
	for (i = size - 1; i >= 0; i--)
#else	/* _LITTLE_ENDIAN */
	for (i = 0; i < size; i++)
#endif
	{
	    data[i] = value & 0xFF;
	    value >>= 8;
	}

	if (db_write_bytes(addr, size, data) != 0) {
		db_printf("*** error writing to address %llx ***\n",
		    (long long)addr);
		kdb_reenter();
	}
}
