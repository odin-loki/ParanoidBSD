/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/uboot/devicename.c: uboot_parsedev()'s DEVT_NET arm reads *cp
 * whether or not the strtol() that sets it ran.
 *
 *   cbmc -DOLD tools/verify/probes/uboot_parsedev_cp.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       tools/verify/probes/uboot_parsedev_cp.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	char *cp;
 *	const char *np;
 *	...
 *	np = (devspec + strlen(dv->dv_name));
 *	...
 *	case DEVT_NET:
 *		unit = 0;
 *		if (*np && (*np != ':')) {
 *			unit = strtol(np, &cp, 0);
 *			...
 *		}
 *		if (*cp && (*cp != ':')) {
 *
 * `net' and `net:' both leave np pointing at '\0' or ':', so the
 * strtol is skipped and cp is still the uninitialised local it was
 * declared as when the next line dereferences it.  A device name is
 * something a person types at the loader prompt and something
 * loader.conf sets, so both spellings arrive.
 *
 * The assertion is that cp is only read after it has been written.
 */

int nondet_int(void);

static int cp_set;

static void
read_cp(void)
{
	__CPROVER_assert(cp_set == 1, "cp is read only after it is written");
}

int
main(void)
{
	int have_unit = nondet_int();	/* *np && *np != ':' */

	cp_set = 0;
#ifndef OLD
	/* cp = __DECONST(char *, np); - where strtol would leave it. */
	cp_set = 1;
#endif
	if (have_unit) {
		/* strtol(np, &cp, 0) */
		cp_set = 1;
	}
	read_cp();
	return (0);
}
