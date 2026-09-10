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
