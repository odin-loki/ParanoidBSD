/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/libsa/geli/gelidev.c: geli_dev_strategy() switches on
 * `rw & F_MASK' with arms for F_READ and F_WRITE and no default, then
 * returns rc - the uninitialised local it was declared as.
 *
 *   cbmc -DOLD tools/verify/probes/geli_strategy_default.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       tools/verify/probes/geli_strategy_default.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	int rc;
 *	...
 *	switch (rw & F_MASK) {
 *	case F_READ:
 *		...
 *	case F_WRITE:
 *		...
 *	}
 *   out:
 *	if (iobuf != buf)
 *		free(iobuf);
 *	return (rc);
 *
 * Zero is success to every caller of a dv_strategy hook, and success
 * here means "the buffer is filled" for a buffer nothing has written
 * - so the arm that does nothing can report a decrypted block that is
 * whatever the caller's buffer already held.  On a GELI-encrypted root
 * that buffer is the one the loader reads the kernel into.
 *
 * F_MASK is not two-valued.  stand.h:219-223 is F_READ 0x0001,
 * F_WRITE 0x0002, F_RAW 0x0004, F_NODEV 0x0008 - and F_MASK 0xFFFF,
 * so `rw & F_MASK' is not a two-way choice but sixteen bits, and
 * F_READ|F_WRITE alone already misses both arms.  The dv_strategy
 * contract does not say those cannot arrive; it says nothing at all.
 *
 * The assertion is that the value returned is one the function chose.
 */

#define	F_READ	0x0001
#define	F_WRITE	0x0002
#define	F_MASK	0xFFFF
#define	EINVAL	22

int nondet_int(void);

int
main(void)
{
	int rw = nondet_int();
	int rc = 0;
	int set = 0;

	switch (rw & F_MASK) {
	case F_READ:
		rc = 0;
		set = 1;
		break;
	case F_WRITE:
		rc = 0;
		set = 1;
		break;
#ifndef OLD
	default:
		rc = EINVAL;
		set = 1;
		break;
#endif
	}
	__CPROVER_assert(set == 1,
	    "the value returned is one the function chose");
	return (rc);
}
