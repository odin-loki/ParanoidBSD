/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/libsa/smbios.c: smbios_getstring() walks idx-1 NUL-terminated
 * strings forward from a structure with no bound, and the caller
 * setenv()s what it lands on.
 *
 *   cbmc -DOLD --unwind 12 tools/verify/probes/smbios_string_walk.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       --unwind 12 tools/verify/probes/smbios_string_walk.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	idx = SMBIOS_GET8(addr, offset);
 *	if (idx != 0) {
 *		cp = SMBIOS_GETSTR(addr);
 *		for (i = 1; i < idx; i++)
 *			cp += strlen(cp) + 1;
 *		return cp;
 *	}
 *
 * idx is one byte of the structure, so 1 to 255, and SMBIOS_GETSTR()
 * is `addr + SMBIOS_GET8(addr, 0x01)' - the structure's own length
 * byte.  Both come off the SMBIOS table, which is firmware data the
 * loader maps and reads.  A structure naming string 255 walks that
 * many terminators past the region the entry point declared, and
 * smbios_parse_table() hands the result to smbios_setenv().
 *
 * The consequence is not a crash.  It is adjacent physical memory
 * published as smbios.bios.vendor, smbios.system.product and friends
 * - loader variables the kernel inherits, and kenv(1) hands to any
 * user.
 *
 * The same file's smbios_parse_table() scans for a double NUL with no
 * bound either, and smbios_find_struct() - which DOES bound its copy
 * - tests the read before the bound because && is left to right.
 *
 * The string section is modelled as the offsets of its successive
 * terminators, strictly increasing and placed wherever the firmware
 * put them - including past the length the entry point declared,
 * which is what having no bound permits. A first attempt modelled it
 * as a byte array scanned in a while loop, and BOTH sides passed:
 * without --unwinding-assertions CBMC truncates an unbounded scan and
 * the failing path simply stops existing. A probe that cannot fail on
 * the old code is not evidence of anything.
 *
 * The assertion is that the walk stays inside the table the entry
 * point declared.
 */

unsigned nondet_uint(void);

int
main(void)
{
	unsigned tablen;	/* smbios.length */
	unsigned t[8];		/* offsets of successive string terminators */
	unsigned idx, off, k;

	tablen = nondet_uint();
	__CPROVER_assume(tablen > 0 && tablen <= 32);

	/*
	 * The table's string section: terminators at strictly
	 * increasing offsets, placed wherever the firmware put them -
	 * including past the length the entry point declared, which is
	 * the case with no bound to stop it.
	 */
	t[0] = nondet_uint();
	__CPROVER_assume(t[0] <= 63);
	for (k = 1; k < 8; k++) {
		t[k] = nondet_uint();
		__CPROVER_assume(t[k] > t[k - 1] && t[k] <= 63);
	}

	idx = nondet_uint();		/* SMBIOS_GET8(addr, offset) */
	__CPROVER_assume(idx >= 1 && idx <= 8);

	/* The walk lands just past the (idx - 1)th terminator. */
#ifndef OLD
	if (idx >= 2 && t[idx - 2] >= tablen)
		return (0);
#endif
	off = (idx == 1) ? 0 : t[idx - 2] + 1;

	__CPROVER_assert(off <= tablen,
	    "the walk stays inside the table the entry point declared");
	return (0);
}
