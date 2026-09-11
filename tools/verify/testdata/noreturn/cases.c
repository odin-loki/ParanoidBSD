/* Fixtures for noreturn_check.py.  Each function is named for what the
 * lint should conclude about it. */

#include <stdlib.h>

/* REPORT: ends in exit(), no return anywhere, no attribute. */
static void
report_plain(void)
{
	printf("bye\n");
	exit(1);
}

/* REPORT: ends in a function this file already concluded never returns. */
static void
report_chained(void)
{
	cleanup();
	report_plain();
}

/* QUIET: has a return, so it does come back -- ifconfig's mapfreq()
 * shape, which an earlier version of the lint called noreturn and would
 * have marked wrongly. */
static void
quiet_has_return(int n)
{
	int i;

	for (i = 0; i < n; i++)
		if (table[i] == n)
			return;
	errx(1, "not found");
}

/* QUIET: the exit is inside an if, so the function falls out of the
 * bottom -- set80211()'s shape, which cost eighty false conclusions
 * through the propagation before top-level statements were tracked. */
static void
quiet_conditional(int fd)
{
	if (ioctl(fd, 0) < 0)
		err(1, "ioctl");
}

/* QUIET: already declared, in the spelling that a word-boundary match
 * on "noreturn" misses. */
static void quiet_declared(void) __attribute__((__noreturn__));
static void
quiet_declared(void)
{
	exit(2);
}

/* QUIET: __dead2, the spelling the tree actually uses. */
static void quiet_dead2(void) __dead2;
static void
quiet_dead2(void)
{
	exit(3);
}

/* QUIET: main() is called by the runtime and returns to it. */
int
main(void)
{
	exit(0);
}

/* QUIET: ends in a block, not a call. */
static void
quiet_ends_in_block(int n)
{
	if (n) {
		exit(1);
	}
}

/* REPORT: return type and name on one line -- chat(1)'s style, which a
 * column-0-name-only pattern misses entirely. */
void oneline_report(int status)
{
	cleanup();
	exit(status);
}

/* QUIET: the same style, already declared. */
void oneline_quiet(int status) __dead2;
void oneline_quiet(int status)
{
	exit(status);
}

/* REPORT: a preprocessor line between the last statement and the brace,
 * which is how chat(1)'s terminate() is written. */
void report_after_endif(int status)
{
	do_cleanup();
#ifdef SOMETHING
	if (thing)
		other();
#endif

	exit(status);
}

/* REPORT: a varargs wrapper that cleans up after the call that does not
 * return -- efivar(8)'s rep_err().  va_end() is unreachable, so the last
 * statement that decides anything is verr(). */
void report_va_end(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(eval, fmt, ap);
	va_end(ap);
}

/* QUIET: the same shape, but the call before va_end() does return. */
void quiet_va_end(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarn(fmt, ap);
	va_end(ap);
}

/*
 * A SEED name this file DEFINES, whose body can return.  sbin/restore
 * has exactly this: its own panic() that prints and comes back unless
 * the user says to abort.  Trusting the name made badentry() -- which
 * ends in it -- the highest-scoring --guards candidate in the tree, and
 * putting __dead2 on it would have been a lie told to the optimiser.
 */
int yflag;
void
panic(const char *fmt)
{
	fprintf(stderr, "%s", fmt);
	if (yflag)
		return;
	done(1);
}

void
quiet_ends_in_local_panic(void)
{
	fprintf(stderr, "bad entry\n");
	panic("flags");
}

/*
 * ...and the same shape where the local definition really does exit.
 * fsck_ffs, fsck and at all define panic() like this, so dropping the
 * seed must not cost them: propagation puts it straight back.
 */
void
exiting_panic(const char *fmt)
{
	fprintf(stderr, "%s", fmt);
	exit(8);
}

void
report_ends_in_local_exiting_panic(void)
{
	fprintf(stderr, "bad entry\n");
	exiting_panic("flags");
}

/*
 * rtld's abort(): raise() then a builtin that cannot come back.  Without
 * __builtin_trap in SEED, dropping the local name took __assert() with
 * it -- a true positive lost to a false-positive fix.
 */
void
local_abort(void)
{
	raise(SIGABRT);
	__builtin_trap();
}

void
report_ends_in_trapping_abort(void)
{
	fprintf(stderr, "assertion failed\n");
	local_abort();
}
