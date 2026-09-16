/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * lib/libc/powerpc/gen/makecontext.c and powerpc64's copy: the NULL
 * test shares a condition whose body writes through the pointer.
 *
 *   cbmc -DOLD tools/verify/probes/makecontext_null_ucp.c
 *       -> 1 of 1 failed, FAILED
 *   cbmc       tools/verify/probes/makecontext_null_ucp.c
 *       -> 0 of 1, SUCCESSFUL
 *
 *	if ((ucp == NULL) || (argc < 0)
 *	    || (ucp->uc_stack.ss_sp == NULL)
 *	    || (ucp->uc_stack.ss_size < MINSIGSTKSZ)) {
 *		ucp->uc_mcontext.mc_len = 0;	-- invalidate context
 *		return;
 *	}
 *
 * Invalidating the context IS a store, at ucp->uc_mcontext.mc_len, so
 * the one argument the check exists to reject is the one it writes
 * through.  The condition itself is fine -- || short-circuits, so the
 * ss_sp read never happens on the NULL arm; it is the body that is
 * shared between "ucp is not usable" and "ucp is not there".
 *
 * Four of the seven architectures already separate them.  amd64 and
 * i386 use `if (ucp == NULL) return; else if (<stack is bad>) {
 * invalidate; }'; aarch64 and riscv return on NULL before touching
 * anything.  arm had no check at all, which this probe OLD arm also
 * covers: with no test, every argument reaches the store.
 */

int nondet_int(void);

struct stack { void *ss_sp; unsigned long ss_size; };
struct mcontext { int mc_len; };
struct ucontext { struct stack uc_stack; struct mcontext uc_mcontext; };

#define	MINSIGSTKSZ	2048

static struct stack good_stack;
static char a_stack[4096];
static struct ucontext a_context;

static void
__makecontext(struct ucontext *ucp, int argc)
{
#ifdef OLD
	if ((ucp == 0) || (argc < 0)
	    || (ucp->uc_stack.ss_sp == 0)
	    || (ucp->uc_stack.ss_size < MINSIGSTKSZ)) {
		__CPROVER_assert(ucp != 0,
		    "the context invalidated is one that exists");
		ucp->uc_mcontext.mc_len = 0;
		return;
	}
#else
	if (ucp == 0)
		return;
	if ((argc < 0)
	    || (ucp->uc_stack.ss_sp == 0)
	    || (ucp->uc_stack.ss_size < MINSIGSTKSZ)) {
		__CPROVER_assert(ucp != 0,
		    "the context invalidated is one that exists");
		ucp->uc_mcontext.mc_len = 0;
		return;
	}
#endif
	ucp->uc_mcontext.mc_len = sizeof(struct mcontext);
}

int
main(void)
{
	good_stack.ss_sp = a_stack;
	good_stack.ss_size = sizeof(a_stack);
	a_context.uc_stack = good_stack;

	/* Every argument makecontext(3) can be given. */
	__makecontext(nondet_int() ? &a_context : 0, nondet_int());
	return (0);
}
