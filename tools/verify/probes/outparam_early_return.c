/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * Two functions whose contract is an out-parameter, and whose early
 * return skips it.  Different subsystems, one shape.
 *
 *   cbmc -DOLD --unwind 4 tools/verify/probes/outparam_early_return.c
 *       -> 2 of 2 failed, FAILED
 *   cbmc       --unwind 4 tools/verify/probes/outparam_early_return.c
 *       -> 0 of 2, SUCCESSFUL
 *
 * (1) lib/libc/amd64/string/amd64_archlevel.c, env_archlevel().  Its
 *     own comment says
 *
 *         *force is set to 1 if the architecture level is valid and
 *         begins with a ! and to 0 otherwise.
 *
 *     and two of its three returns do that.  The third,
 *
 *         if (environ == NULL)
 *                 return (X86_64_UNDEFINED);
 *
 *     does not.  archlevel() reads it unconditionally:
 *
 *         wantlevel = env_archlevel(&force);
 *         if (!force) {
 *                 hwlevel = supported_archlevel(...);
 *                 if (wantlevel == X86_64_UNDEFINED || wantlevel > hwlevel)
 *                         wantlevel = hwlevel;
 *         }
 *
 *     so garbage that is not zero skips supported_archlevel() and
 *     leaves wantlevel at X86_64_UNDEFINED, which is -1.  That is
 *     stored into amd64_archlevel and handed to
 *
 *         for (level = archlevel(...); level >= 0; level--)
 *                 if (funcs[level] != 0)
 *                         return ...;
 *         __builtin_trap();
 *
 *     which runs zero iterations and traps.  Assigning environ = NULL
 *     to scrub the environment is an ordinary thing for a daemon to
 *     do, and this is libc's dispatch for every SIMD string function.
 *
 * (2) sys/dev/e1000/e1000_phy.c, e1000_phy_has_link_generic().  Its
 *     `@success: pointer to whether polling was successful or not' is
 *     the whole answer, and
 *
 *         if (!hw->phy.ops.read_reg)
 *                 return E1000_SUCCESS;
 *
 *     returns the success code without writing it.  All 24 call sites
 *     pass the address of an uninitialised `bool link' and read it the
 *     moment the return is E1000_SUCCESS.
 *
 * The invariant is the same sentence for both: when the function
 * returns, the out-parameter has been written.
 */

int nondet_int(void);

static int force_written;
static int success_written;

static int
env_archlevel(int *force)
{
	int environ_is_null = nondet_int();
	int found = nondet_int();

	if (environ_is_null) {
#ifndef OLD
		*force = 0;
		force_written = 1;
#endif
		return (-1);			/* X86_64_UNDEFINED */
	}
	if (found) {
		*force = nondet_int() ? 1 : 0;	/* match_archlevel() */
		force_written = 1;
		return (0);
	}
	*force = 0;
	force_written = 1;
	return (-1);
}

static int
phy_has_link(int have_read_reg, int *success)
{
	if (!have_read_reg) {
#ifndef OLD
		*success = 0;
		success_written = 1;
#endif
		return (0);			/* E1000_SUCCESS */
	}
	*success = nondet_int() ? 1 : 0;
	success_written = 1;
	return (nondet_int());
}

int
main(void)
{
	int force, success, ret;

	force_written = 0;
	(void)env_archlevel(&force);
	__CPROVER_assert(force_written,
	    "env_archlevel wrote *force on every return");

	success_written = 0;
	ret = phy_has_link(nondet_int(), &success);
	if (ret == 0)
		__CPROVER_assert(success_written,
		    "phy_has_link wrote *success before returning success");

	return (0);
}
