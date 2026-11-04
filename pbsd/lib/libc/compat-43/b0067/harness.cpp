/*
 * harness.cpp -- differential test for batch b0067.
 *
 * Compares pbsd::lib_libc_compat_43::b0067::setpgrp() against the
 * reference ref_setpgrp() from oracle.c.
 *
 * setpgrp(pid, pgid) is a pure passthrough to setpgid(2): it writes no
 * buffers and returns no pointers, so the buffer-guard / pointer-offset
 * comparisons required for string functions have no subject here.  What is
 * observable is the pair (return value, errno), and the *identity and order*
 * of the two arguments as they reach the kernel.  Both are compared exactly.
 *
 * Because setpgid(2) mutates process state when it succeeds, every case whose
 * pid argument designates the calling process (pid == 0 or pid == getpid())
 * is executed in a freshly forked child -- one child for the port, one child
 * for the oracle -- so both observe an identical pristine state and neither
 * can perturb the other or the test driver.  Cases whose pid cannot designate
 * the caller (and therefore always fail) are run in-process.
 *
 * Argument values are chosen so that both sides of every kernel-visible
 * boundary are exercised and so that perturbing either argument changes the
 * observable result:
 *   pgid <  0                      -> EINVAL   (checked before pid lookup)
 *   pgid >= 0, pid nonexistent     -> ESRCH
 *   pid  == self, pgid foreign     -> EPERM
 *   pid  == self, pgid 0 or self   -> success, 0
 * PID_MAX_LIMIT on Linux is 4194304, so any pid above that can never exist;
 * values straddling that limit, 0, +-1, and INT_MIN/INT_MAX are all covered.
 * Swapping the two arguments, or perturbing one by +-1, moves at least one
 * tested case across one of the boundaries above and is therefore detected.
 */

#include <cerrno>
#include <climits>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.lib.libc.compat_43.b0067;

namespace port = pbsd::lib_libc_compat_43::b0067;

extern "C" int ref_setpgrp(pid_t pid, pid_t pgid);

/* ------------------------------------------------------------------ */
/* bookkeeping                                                         */
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	long cases;
	long fails;
};

static Stat st_setpgrp = { "setpgrp", 0, 0 };

static long reported = 0;

static const int ERRNO_SENTINEL = 0x5eed;

struct Res {
	int ret;
	int err;
};

static inline bool same(const Res &a, const Res &b)
{
	return a.ret == b.ret && a.err == b.err;
}

static void report(const char *what, pid_t pid, pid_t pgid,
    const Res &p, const Res &r)
{
	if (reported++ < 20) {
		std::printf("  MISMATCH [%s] setpgrp(%ld, %ld): "
		    "port ret=%d errno=%d(%s) | oracle ret=%d errno=%d(%s)\n",
		    what, (long)pid, (long)pgid,
		    p.ret, p.err, std::strerror(p.err),
		    r.ret, r.err, std::strerror(r.err));
	}
}

/* ------------------------------------------------------------------ */
/* the two call wrappers                                               */
/* ------------------------------------------------------------------ */

static Res call_port(pid_t pid, pid_t pgid)
{
	Res out;
	errno = ERRNO_SENTINEL;
	out.ret = port::setpgrp(pid, pgid);
	out.err = errno;
	return out;
}

static Res call_ref(pid_t pid, pid_t pgid)
{
	Res out;
	errno = ERRNO_SENTINEL;
	out.ret = ref_setpgrp(pid, pgid);
	out.err = errno;
	return out;
}

/* ------------------------------------------------------------------ */
/* argument tokens (resolved at call site, so they mean the same thing */
/* inside an isolated child as they do in-process)                     */
/* ------------------------------------------------------------------ */

enum Tok {
	T_LIT = 0,	/* literal value			*/
	T_SELF,		/* getpid()				*/
	T_PARENT	/* getppid()				*/
};

struct Arg {
	Tok tok;
	pid_t lit;
};

static inline Arg lit(pid_t v)
{
	Arg a;
	a.tok = T_LIT;
	a.lit = v;
	return a;
}

static inline Arg self_arg(void)
{
	Arg a;
	a.tok = T_SELF;
	a.lit = 0;
	return a;
}

static inline Arg parent_arg(void)
{
	Arg a;
	a.tok = T_PARENT;
	a.lit = 0;
	return a;
}

static pid_t resolve(const Arg &a)
{
	switch (a.tok) {
	case T_SELF:
		return getpid();
	case T_PARENT:
		return getppid();
	default:
		return a.lit;
	}
}

/* true when the pid argument may designate the caller, i.e. when the call
   could succeed and mutate process state */
static bool mutating(const Arg &pid)
{
	if (pid.tok == T_SELF)
		return true;
	if (pid.tok == T_LIT && (pid.lit == 0 || pid.lit == getpid()))
		return true;
	return false;
}

/* ------------------------------------------------------------------ */
/* isolated execution: one fork per call, so the port call and the      */
/* oracle call each start from an identical, pristine process state     */
/* ------------------------------------------------------------------ */

static bool run_isolated(bool use_port, const Arg &pid, const Arg &pgid,
    Res *out, pid_t *seen_pid, pid_t *seen_pgid)
{
	int fds[2];

	if (pipe(fds) != 0)
		return false;

	pid_t kid = fork();
	if (kid < 0) {
		close(fds[0]);
		close(fds[1]);
		return false;
	}
	if (kid == 0) {
		close(fds[0]);
		pid_t a = resolve(pid);
		pid_t b = resolve(pgid);
		Res r = use_port ? call_port(a, b) : call_ref(a, b);
		long msg[4];
		msg[0] = r.ret;
		msg[1] = r.err;
		msg[2] = (long)a;
		msg[3] = (long)b;
		ssize_t w = write(fds[1], msg, sizeof msg);
		_exit(w == (ssize_t)sizeof msg ? 0 : 1);
	}

	close(fds[1]);
	long msg[4];
	ssize_t n = 0;
	char *p = (char *)msg;
	while (n < (ssize_t)sizeof msg) {
		ssize_t k = read(fds[0], p + n, sizeof msg - (size_t)n);
		if (k <= 0)
			break;
		n += k;
	}
	close(fds[0]);
	int status = 0;
	while (waitpid(kid, &status, 0) < 0 && errno == EINTR)
		;
	if (n != (ssize_t)sizeof msg)
		return false;
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return false;
	out->ret = (int)msg[0];
	out->err = (int)msg[1];
	*seen_pid = (pid_t)msg[2];
	*seen_pgid = (pid_t)msg[3];
	return true;
}

/* ------------------------------------------------------------------ */
/* one case                                                            */
/* ------------------------------------------------------------------ */

static void do_case(const Arg &pid, const Arg &pgid, bool oracle_first)
{
	st_setpgrp.cases++;

	if (mutating(pid)) {
		Res p, r;
		pid_t pa = 0, pb = 0, ra = 0, rb = 0;
		bool okp, okr;

		if (oracle_first) {
			okr = run_isolated(false, pid, pgid, &r, &ra, &rb);
			okp = run_isolated(true, pid, pgid, &p, &pa, &pb);
		} else {
			okp = run_isolated(true, pid, pgid, &p, &pa, &pb);
			okr = run_isolated(false, pid, pgid, &r, &ra, &rb);
		}
		if (!okp || !okr) {
			st_setpgrp.fails++;
			if (reported++ < 20)
				std::printf("  MISMATCH [isolated] harness "
				    "could not run case (fork/pipe failed)\n");
			return;
		}
		/* the two children have different pids, so a T_SELF argument
		   legitimately differs; only compare the observable result */
		if (!same(p, r)) {
			st_setpgrp.fails++;
			report("isolated", ra, rb, p, r);
		}
		return;
	}

	pid_t a = resolve(pid);
	pid_t b = resolve(pgid);
	Res p, r;

	if (oracle_first) {
		r = call_ref(a, b);
		p = call_port(a, b);
	} else {
		p = call_port(a, b);
		r = call_ref(a, b);
	}
	if (!same(p, r)) {
		st_setpgrp.fails++;
		report("direct", a, b, p, r);
	}
}

/* ------------------------------------------------------------------ */
/* fixed-seed PRNG (splitmix64) -- reproducible independent of libc     */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0x0067706273640067ULL;

static inline std::uint64_t next_u64(void)
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static inline std::uint64_t next_below(std::uint64_t n)
{
	return next_u64() % n;
}

/* PID_MAX_LIMIT on Linux; a pid above this can never exist */
static const std::int64_t PID_CEILING = 4194304;

static const int edge_values[] = {
	INT_MIN, INT_MIN + 1, -2147483000, -8388609, -8388608,
	-4194305, -4194304, -4194303, -1000000, -100, -3, -2, -1,
	0, 1, 2, 3, 4, 5, 100, 1000, 65535, 65536,
	4194302, 4194303, 4194304, 4194305, 4194306, 8388607, 8388608,
	100000000, INT_MAX - 1, INT_MAX
};
static const int n_edge = (int)(sizeof edge_values / sizeof edge_values[0]);

/* pid values that cannot designate the caller: chosen so the in-process
   call is guaranteed to fail and therefore cannot mutate state */
static pid_t rand_pid_nonself(pid_t self, pid_t parent)
{
	pid_t v;

	switch ((unsigned)next_below(5)) {
	case 0:
	case 1:
		/* negative: [INT_MIN, -1] */
		if (next_below(64) == 0)
			v = INT_MIN;
		else
			v = -(pid_t)(1 + next_below(2147483647ULL));
		break;
	case 2:
	case 3:
		/* above PID_MAX_LIMIT: guaranteed nonexistent */
		v = (pid_t)(PID_CEILING + 1 +
		    (std::int64_t)next_below(2147483647ULL - PID_CEILING));
		break;
	default: {
		static const int pool[] = {
			1, 2, 3, 4, 5, -1, -2, INT_MIN, INT_MAX, INT_MAX - 1,
			4194303, 4194304, 4194305, 8388608, -4194304, -4194305
		};
		if (next_below(4) == 0)
			v = parent;
		else
			v = pool[next_below(sizeof pool / sizeof pool[0])];
		break; }
	}
	if (v == 0 || v == self)
		v = (pid_t)(PID_CEILING + 1);
	return v;
}

static pid_t rand_pgid(void)
{
	switch ((unsigned)next_below(7)) {
	case 0:
		return 0;
	case 1:
		return -1;
	case 2:
		return -(pid_t)(1 + next_below(2147483647ULL));
	case 3:
		return (pid_t)next_below((std::uint64_t)PID_CEILING + 1);
	case 4:
		return (pid_t)(PID_CEILING + 1 +
		    (std::int64_t)next_below(2147483647ULL - PID_CEILING));
	case 5:
		return (pid_t)(std::int32_t)(std::uint32_t)next_u64();
	default:
		return edge_values[next_below((std::uint64_t)n_edge)];
	}
}

/* ------------------------------------------------------------------ */

int main(void)
{
	/* a mutated port could move us out of the foreground process group;
	   never let that stop the test driver */
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	const pid_t self = getpid();
	const pid_t parent = getppid();

	std::printf("b0067 differential test: setpgrp(pid_t, pid_t)\n");
	std::printf("  self=%ld parent=%ld pid_max_limit=%ld\n",
	    (long)self, (long)parent, (long)PID_CEILING);
	std::printf("  buffer-guard / pointer-offset checks: not applicable "
	    "(no buffer or pointer results in this batch)\n");

	/* ---- hand-written edge cases: full cross product of the boundary
	   value list, plus the self/parent tokens in both slots ---- */
	bool flip = false;
	for (int i = 0; i < n_edge; i++) {
		for (int j = 0; j < n_edge; j++) {
			do_case(lit(edge_values[i]), lit(edge_values[j]), flip);
			flip = !flip;
		}
	}
	for (int i = 0; i < n_edge; i++) {
		do_case(self_arg(), lit(edge_values[i]), flip);
		flip = !flip;
		do_case(lit(edge_values[i]), self_arg(), flip);
		flip = !flip;
		do_case(parent_arg(), lit(edge_values[i]), flip);
		flip = !flip;
		do_case(lit(edge_values[i]), parent_arg(), flip);
		flip = !flip;
	}
	do_case(self_arg(), self_arg(), false);
	do_case(self_arg(), parent_arg(), true);
	do_case(parent_arg(), self_arg(), false);
	do_case(parent_arg(), parent_arg(), true);
	do_case(lit(0), lit(0), false);
	do_case(self_arg(), lit(0), true);
	do_case(lit(0), self_arg(), false);
	do_case(lit(0), parent_arg(), true);
	do_case(parent_arg(), lit(0), false);

	const long edge_cases = st_setpgrp.cases;
	const long edge_fails = st_setpgrp.fails;

	/* ---- fixed-seed randomised sweep ---- */
	const long ITER = 250000;
	for (long k = 0; k < ITER; k++) {
		pid_t a = rand_pid_nonself(self, parent);
		pid_t b = rand_pgid();
		do_case(lit(a), lit(b), (k & 1) != 0);
	}

	const long rand_cases = st_setpgrp.cases - edge_cases;
	const long rand_fails = st_setpgrp.fails - edge_fails;

	std::printf("\n");
	std::printf("%-12s %12s %12s %12s %12s %10s\n",
	    "function", "edge cases", "edge fails", "rand cases",
	    "rand fails", "result");
	std::printf("%-12s %12ld %12ld %12ld %12ld %10s\n",
	    st_setpgrp.name, edge_cases, edge_fails, rand_cases, rand_fails,
	    st_setpgrp.fails == 0 ? "PASS" : "FAIL");
	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-12s %12ld %12ld\n", st_setpgrp.name, st_setpgrp.cases,
	    st_setpgrp.fails);

	long total_cases = st_setpgrp.cases;
	long total_fails = st_setpgrp.fails;
	std::printf("%-12s %12ld %12ld\n", "TOTAL", total_cases, total_fails);
	std::printf("\n%s\n", total_fails == 0 ? "ALL CASES MATCHED" :
	    "DIVERGENCE DETECTED");

	return total_fails == 0 ? 0 : 1;
}
