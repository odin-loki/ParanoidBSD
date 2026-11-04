/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * A model of sys/kern/kern_environment.c's init_dynamic_kenv_from()
 * cursor and the NULL store that follows it in init_dynamic_kenv().
 * KENV_SIZE is cut down so CBMC can unwind the loop; what is under test
 * is the relation between the guard and the array size, and that is the
 * same at 8 as at 512.
 *
 *   cbmc -DOLD_GUARD --unwind 16 --unwinding-assertions \
 *       tools/verify/probes/kenv_terminator.c        -> FAILURE
 *   cbmc          --unwind 16 --unwinding-assertions \
 *       tools/verify/probes/kenv_terminator.c        -> SUCCESSFUL
 *
 * Both runs agree that the STRING store is in bounds either way. The
 * one that goes out of bounds is the terminator, written by the caller
 * from the cursor this function hands back -- which is why reading only
 * the loop does not show it.
 */
#define KENV_SIZE 8
#define NSLOTS (KENV_SIZE + 1)

char *kenvp[NSLOTS];

#ifdef OLD_GUARD
#define TOO_MANY(i) ((i) > KENV_SIZE)
#else
#define TOO_MANY(i) ((i) >= KENV_SIZE)
#endif

static void
init_dynamic_kenv_from(int nstrings, int *curpos)
{
	int i, k;

	i = *curpos;
	for (k = 0; k < nstrings; k++) {
		if (TOO_MANY(i))
			continue;			/* goto sanitize */
		__CPROVER_assert(i >= 0 && i < NSLOTS,
		    "the string store is in bounds");
		kenvp[i++] = (char *)1;
	}
	*curpos = i;
}

int
main(void)
{
	int dynamic_envpos = 0;
	int n1 = nondet_int(), n2 = nondet_int();

	__CPROVER_assume(n1 >= 0 && n1 <= 12);
	__CPROVER_assume(n2 >= 0 && n2 <= 12);

	init_dynamic_kenv_from(n1, &dynamic_envpos);
	init_dynamic_kenv_from(n2, &dynamic_envpos);

	/* init_dynamic_kenv(): kenvp[dynamic_envpos] = NULL; */
	__CPROVER_assert(dynamic_envpos >= 0 && dynamic_envpos < NSLOTS,
	    "the NULL terminator is in bounds");
	kenvp[dynamic_envpos] = 0;
	return (0);
}
