/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * sys/dev/syscons/syscons.c: sc_bell() computes the 8254 divisor as
 * 1193182 / pitch and hands it to sysbeep(), which passes it on to
 * timer_spkr_setfreq() -- and that divides BY it, unguarded.
 *
 *   cbmc -DOLD tools/verify/probes/sc_bell_divisor.c
 *       -> 3 of 3 failed, FAILED
 *   cbmc       tools/verify/probes/sc_bell_divisor.c
 *       -> 0 of 3, SUCCESSFUL
 *
 * The chain, all three pieces in the tree:
 *
 *     sc_bell():            sysbeep(1193182 / pitch, ...)
 *     sysbeep():            timer_spkr_setfreq(pitch)   -- no test
 *     timer_spkr_setfreq(): freq = i8254_freq / freq;   -- no test
 *
 * sc_bell() tested `pitch != 0', which is the wrong end.  A pitch
 * ABOVE 1193182 makes the quotient zero, and a negative pitch makes
 * the doubling on the non-current screen overflow.  Two of the three
 * ways to reach sc_bell() bound the pitch already: KDMKTONE masks its
 * argument with 0xffff (syscons.c), and scteken packs pitch and
 * duration into one word so TP_SETBELLPD_PITCH recovers only 16 bits
 * (sys/teken/teken_subr_compat.h).  The cons25 emulator does not --
 * scterm-sc.c assigns `scp->bell_pitch = tcp->param[0]' straight from
 * an escape parameter, on a kernel built with SC_DFLT_TERM="sc"
 * (documented in sys/x86/conf/NOTES).  Anything that can write to that
 * console picks the divisor.
 *
 * The three assertions are the three things the chain needs: the
 * doubling does not overflow, the quotient is not zero, and what
 * reaches the division is positive.
 */

int nondet_int(void);

#define	I8254_FREQ	1193182
#define	PIT_BASE	1193182

static int last_divisor;

/* sys/x86/isa/clock.c */
static void
timer_spkr_setfreq(int freq)
{
#ifndef OLD
	if (freq <= 0)
		return;
#endif
	__CPROVER_assert(freq > 0,
	    "timer_spkr_setfreq divides by a positive frequency");
	last_divisor = I8254_FREQ / freq;
}

/* sys/kern/kern_cons.c -- sysbeep() forwards without testing */
static void
sysbeep(int pitch)
{
	timer_spkr_setfreq(pitch);
}

/* sys/dev/syscons/syscons.c */
static void
sc_bell(int pitch, int duration, int on_current_screen)
{
#ifdef OLD
	if (duration != 0 && pitch != 0) {
		if (!on_current_screen) {
			__CPROVER_assert(pitch <= 2147483647 / 2,
			    "the doubling does not overflow");
			pitch *= 2;
		}
		__CPROVER_assert(PIT_BASE / pitch != 0,
		    "the 8254 divisor sysbeep divides by is not zero");
		sysbeep(PIT_BASE / pitch);
	}
#else
	if (duration != 0 && pitch > 0) {
		if (!on_current_screen && pitch < PIT_BASE) {
			__CPROVER_assert(pitch <= 2147483647 / 2,
			    "the doubling does not overflow");
			pitch *= 2;
		}
		if (pitch > PIT_BASE)
			pitch = PIT_BASE;
		__CPROVER_assert(PIT_BASE / pitch != 0,
		    "the 8254 divisor sysbeep divides by is not zero");
		sysbeep(PIT_BASE / pitch);
	}
#endif
}

int
main(void)
{
	/*
	 * scterm-sc.c's `scp->bell_pitch = tcp->param[0]' is an int off
	 * an escape sequence, so the only thing known about it is its
	 * type.
	 */
	int pitch = nondet_int();
	int duration = nondet_int();
	int on_current = nondet_int();

	__CPROVER_assume(duration != 0);
	sc_bell(pitch, duration, on_current != 0);
	return (0);
}
