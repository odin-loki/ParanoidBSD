/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * lib/msun/src/e_rem_pio2.c and the two e_rem_pio2l.h copies: the
 * skip-zero-term loop walks down with no floor.
 *
 *   cbmc -DOLD tools/verify/probes/rem_pio2_nx_floor.c
 *       -> 2 of 2 failed, FAILED
 *   cbmc       tools/verify/probes/rem_pio2_nx_floor.c
 *       -> 0 of 2, SUCCESSFUL
 *
 *	nx = 3;
 *	while(tx[nx-1]==zero) nx--;
 *	n  =  __kernel_rem_pio2(tx,ty,e0,nx,1);
 *
 * With every term zero this reads tx[-1] on the way past, and then
 * hands __kernel_rem_pio2() an nx of 0.
 *
 * What keeps that from happening today is the exponent arithmetic
 * above it, not the loop: z is scaled into [2^23, 2^24), so
 * tx[0] = (double)(int32_t)z is at least 2^23 and the loop stops at
 * nx == 1 on its own.  That is a property of the CALLER'S exponent,
 * three architectures' worth of it -- the double, ld80 and ld128
 * copies each re-derive it -- and the loop should be true of its own
 * array instead.
 *
 * The two assertions are the two things that go wrong: the subscript
 * stays in range, and the count handed to the kernel routine is one
 * it can use.  The model drops the exponent invariant, because the
 * point is that the loop does not depend on it.
 */

int nondet_int(void);

#define	NTERMS	3

static int
model(const int *tx)
{
	int nx = NTERMS;

#ifdef OLD
	while (tx[nx - 1] == 0) {
		__CPROVER_assert(nx - 1 >= 0,
		    "the subscript stays inside tx[]");
		nx--;
	}
#else
	while (nx > 1 && tx[nx - 1] == 0) {
		__CPROVER_assert(nx - 1 >= 0,
		    "the subscript stays inside tx[]");
		nx--;
	}
#endif
	__CPROVER_assert(nx >= 1,
	    "__kernel_rem_pio2 is given at least one term");
	return (nx);
}

int
main(void)
{
	int tx[NTERMS];
	int i;

	for (i = 0; i < NTERMS; i++)
		tx[i] = nondet_int();

	model(tx);
	return (0);
}
