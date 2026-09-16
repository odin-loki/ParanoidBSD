/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * stand/ficl/loader.c: ficlCcall() fills int p[10] from the Forth
 * stack, with the count taken off that stack and no bound against 10.
 *
 *   cbmc -DOLD --unwind 34 tools/verify/probes/ficl_ccall_nparam.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       --unwind 34 tools/verify/probes/ficl_ccall_nparam.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	int result, p[10];
 *	int nparam, i;
 *	...
 *	nparam = stackPopINT(pVM->pStack);
 *	for (i = 0; i < nparam; i++)
 *		p[i] = stackPopINT(pVM->pStack);
 *	result = func(p[0], p[1], ... p[9]);
 *
 * `ccall' is a trusted word - it calls an arbitrary C function by
 * address - so this is not a privilege boundary.  An array is still
 * the size it is, and the ten arguments are passed however few were
 * popped, so a `2 ccall' also handed the callee eight stack words.
 *
 * The assertion is that every subscript written is one p[] has.
 */

int nondet_int(void);

int
main(void)
{
	int p[10];
	int nparam, i;

	nparam = nondet_int();
	__CPROVER_assume(nparam >= -2 && nparam <= 32);

#ifndef OLD
	if (nparam < 0 || nparam > (int)(sizeof(p) / sizeof(p[0])))
		return (1);
#endif
	for (i = 0; i < nparam; i++) {
		__CPROVER_assert(i >= 0 && i < 10,
		    "every subscript written is one p[] has");
		p[i] = nondet_int();
	}
	return (0);
}
