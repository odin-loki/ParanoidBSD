/*
 * Differential harness for PBSD batch b0195 (bin/chmod/chmod.c).
 *
 * Each ported function is driven with identical input alongside the
 * untouched C oracle in oracle.c, and every observable is compared:
 * the file-scope `siginfo' flag, the exact bytes written to stderr
 * (captured into a guard-filled buffer and compared in full, tail
 * included) and the child process termination status.
 */

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdint>

import pbsd.bin.chmod.b0195;

namespace port = pbsd::bin_chmod::b0195;

extern "C" {
void ref_siginfo_handler_call(int sig);
void ref_usage_call(void);
sig_atomic_t ref_siginfo_get(void);
void ref_siginfo_set(sig_atomic_t v);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                         */
/* ------------------------------------------------------------------ */

struct Stats {
	const char *name;
	unsigned long cases;
	unsigned long failures;
	unsigned long reported;
};

static Stats st_siginfo_handler = { "siginfo_handler", 0, 0, 0 };
static Stats st_usage = { "usage", 0, 0, 0 };

static const unsigned long REPORT_LIMIT = 12;

static void fail(Stats &s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

static void
fail(Stats &s, const char *fmt, ...)
{
	va_list ap;

	s.failures++;
	if (s.reported >= REPORT_LIMIT)
		return;
	s.reported++;
	fputs("  FAIL [", stdout);
	fputs(s.name, stdout);
	fputs("] ", stdout);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fputc('\n', stdout);
}

/* Deterministic fixed-seed xorshift64* generator. */
static uint64_t rng_state;

static void
rng_seed(uint64_t seed)
{

	rng_state = seed ? seed : 0x9e3779b97f4a7c15ULL;
}

static uint64_t
rng_next(void)
{
	uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return (x * 0x2545f4914f6cdd1dULL);
}

static uint32_t
rng_u32(void)
{

	return ((uint32_t)(rng_next() >> 32));
}

/* ------------------------------------------------------------------ */
/* siginfo_handler                                                     */
/* ------------------------------------------------------------------ */

/*
 * Both implementations start from an identical flag value, are handed
 * the identical signal number, and the resulting flag is compared.  The
 * pre-state is varied across 0, 1 and arbitrary bit patterns so that a
 * port storing any other constant -- or storing nothing at all --
 * diverges from the oracle for at least one pre-state.
 */
static void
check_siginfo_handler(sig_atomic_t pre, int sig, const char *what)
{
	sig_atomic_t got_port, got_ref;
	sig_atomic_t seen_port, seen_ref;

	st_siginfo_handler.cases++;

	port::siginfo_set(pre);
	ref_siginfo_set(pre);

	seen_port = port::siginfo_get();
	seen_ref = ref_siginfo_get();
	if (seen_port != seen_ref || seen_port != pre) {
		fail(st_siginfo_handler,
		    "%s: pre-state readback pre=%ld port=%ld ref=%ld",
		    what, (long)pre, (long)seen_port, (long)seen_ref);
		return;
	}

	port::siginfo_handler(sig);
	ref_siginfo_handler_call(sig);

	got_port = port::siginfo_get();
	got_ref = ref_siginfo_get();
	if (got_port != got_ref) {
		fail(st_siginfo_handler,
		    "%s: pre=%ld sig=%d post port=%ld ref=%ld",
		    what, (long)pre, sig, (long)got_port, (long)got_ref);
		return;
	}

	/* Redelivery must leave both flags in the same state. */
	port::siginfo_handler(sig);
	ref_siginfo_handler_call(sig);
	got_port = port::siginfo_get();
	got_ref = ref_siginfo_get();
	if (got_port != got_ref) {
		fail(st_siginfo_handler,
		    "%s: pre=%ld sig=%d second post port=%ld ref=%ld",
		    what, (long)pre, sig, (long)got_port, (long)got_ref);
		return;
	}

	/*
	 * Cross-check: after the handler has run, driving the oracle from
	 * the port's resulting value (and vice versa) must be a fixed
	 * point for both.
	 */
	port::siginfo_set(got_ref);
	ref_siginfo_set(got_port);
	port::siginfo_handler(sig);
	ref_siginfo_handler_call(sig);
	if (port::siginfo_get() != ref_siginfo_get())
		fail(st_siginfo_handler,
		    "%s: pre=%ld sig=%d crossed port=%ld ref=%ld",
		    what, (long)pre, sig, (long)port::siginfo_get(),
		    (long)ref_siginfo_get());
}

static void
test_siginfo_handler(void)
{
	static const sig_atomic_t pres[] = {
		0, 1, -1, 2, -2, 3, 0x7f, -0x7f, 0x80, -0x80, 0xff, 0x100,
		(sig_atomic_t)INT_MAX, (sig_atomic_t)INT_MIN,
		(sig_atomic_t)(INT_MAX - 1), (sig_atomic_t)(INT_MIN + 1),
		(sig_atomic_t)0x7ffffffe, (sig_atomic_t)0x80000000u,
		(sig_atomic_t)0xffffffffu,
	};
	static const int sigs[] = {
		0, 1, 2, 9, 15, 27, 28, 29, 30, 31, 32, 63, 64, 65,
		-1, -2, INT_MAX, INT_MIN, 0x7f, 0x80, 0xff, 0x100,
	};
	char buf[128];
	size_t i, j;
	unsigned long iter;

	for (i = 0; i < sizeof(pres) / sizeof(pres[0]); i++)
		for (j = 0; j < sizeof(sigs) / sizeof(sigs[0]); j++) {
			snprintf(buf, sizeof(buf),
			    "edge pre[%zu]=%ld sig[%zu]=%d", i,
			    (long)pres[i], j, sigs[j]);
			check_siginfo_handler(pres[i], sigs[j], buf);
		}

	rng_seed(0xb0195c0ffee1234ULL);
	for (iter = 0; iter < 250000UL; iter++) {
		sig_atomic_t pre;
		int sig;

		switch (rng_u32() % 4u) {
		case 0:
			pre = (sig_atomic_t)0;
			break;
		case 1:
			pre = (sig_atomic_t)1;
			break;
		case 2:
			pre = (sig_atomic_t)(int32_t)rng_u32();
			break;
		default:
			pre = (sig_atomic_t)(int)(rng_u32() % 256u) - 128;
			break;
		}
		switch (rng_u32() % 3u) {
		case 0:
			sig = (int)(rng_u32() % 65u);
			break;
		case 1:
			sig = (int)(int32_t)rng_u32();
			break;
		default:
			sig = (int)(rng_u32() % 256u) - 128;
			break;
		}
		snprintf(buf, sizeof(buf), "random iter=%lu", iter);
		check_siginfo_handler(pre, sig, buf);
	}
}

/* ------------------------------------------------------------------ */
/* usage                                                               */
/* ------------------------------------------------------------------ */

#define	CAP_SIZE	512	/* full guarded capture buffer	*/
#define	CAP_WINDOW	256	/* nominal write window		*/

struct Capture {
	unsigned char buf[CAP_SIZE];
	long nread;
	int spawn_error;
	int wait_error;
	int status;
};

/*
 * Run fn() in a forked child with stderr redirected into a pipe.  The
 * capture buffer is pre-filled with guard byte 0x7f and only its first
 * CAP_WINDOW bytes are ever written into, so comparing all CAP_SIZE
 * bytes also proves neither side disturbed the tail.
 */
static void
run_in_child(void (*fn)(void), Capture *cap)
{
	int fds[2];
	pid_t pid, w;
	ssize_t n;
	size_t off;

	memset(cap->buf, 0x7f, sizeof(cap->buf));
	cap->nread = 0;
	cap->spawn_error = 0;
	cap->wait_error = 0;
	cap->status = -1;

	if (pipe(fds) == -1) {
		cap->spawn_error = errno;
		return;
	}
	fflush(NULL);
	pid = fork();
	if (pid == -1) {
		cap->spawn_error = errno;
		close(fds[0]);
		close(fds[1]);
		return;
	}
	if (pid == 0) {
		close(fds[0]);
		if (dup2(fds[1], STDERR_FILENO) == -1)
			_exit(121);
		close(fds[1]);
		fn();
		_exit(120);	/* fn() is __dead2; returning is a bug */
	}
	close(fds[1]);

	off = 0;
	for (;;) {
		if (off >= CAP_WINDOW) {
			char sink[256];

			n = read(fds[0], sink, sizeof(sink));
			if (n == -1 && errno == EINTR)
				continue;
			if (n <= 0)
				break;
			cap->nread += (long)n;
			continue;
		}
		n = read(fds[0], cap->buf + off, CAP_WINDOW - off);
		if (n == -1) {
			if (errno == EINTR)
				continue;
			break;
		}
		if (n == 0)
			break;
		off += (size_t)n;
		cap->nread += (long)n;
	}
	close(fds[0]);

	do {
		w = waitpid(pid, &cap->status, 0);
	} while (w == -1 && errno == EINTR);
	if (w == -1)
		cap->wait_error = errno;
}

static void
describe_status(int status, char *out, size_t outsz)
{

	if (WIFEXITED(status))
		snprintf(out, outsz, "exited(%d)", WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		snprintf(out, outsz, "signaled(%d)", WTERMSIG(status));
	else
		snprintf(out, outsz, "raw(0x%x)", (unsigned)status);
}

static void
check_usage_once(const char *what)
{
	Capture cp, cr;
	char sp[64], sr[64];
	size_t k;

	st_usage.cases++;

	run_in_child(port::usage, &cp);
	run_in_child(ref_usage_call, &cr);

	if (cp.spawn_error != 0 || cr.spawn_error != 0) {
		fail(st_usage, "%s: spawn failed port=%d ref=%d", what,
		    cp.spawn_error, cr.spawn_error);
		return;
	}
	if (cp.wait_error != 0 || cr.wait_error != 0) {
		fail(st_usage, "%s: wait failed port=%d ref=%d", what,
		    cp.wait_error, cr.wait_error);
		return;
	}
	if (cp.nread != cr.nread) {
		fail(st_usage, "%s: stderr length port=%ld ref=%ld", what,
		    cp.nread, cr.nread);
		return;
	}
	/* Entire buffer, guard tail included. */
	for (k = 0; k < CAP_SIZE; k++)
		if (cp.buf[k] != cr.buf[k]) {
			fail(st_usage,
			    "%s: stderr byte %zu port=0x%02x ref=0x%02x",
			    what, k, cp.buf[k], cr.buf[k]);
			return;
		}
	if (cp.status != cr.status) {
		describe_status(cp.status, sp, sizeof(sp));
		describe_status(cr.status, sr, sizeof(sr));
		fail(st_usage, "%s: exit status port=%s ref=%s", what, sp,
		    sr);
		return;
	}
	if (!WIFEXITED(cp.status) || WEXITSTATUS(cp.status) == 120 ||
	    WEXITSTATUS(cp.status) == 121) {
		describe_status(cp.status, sp, sizeof(sp));
		fail(st_usage, "%s: child did not terminate via exit(): %s",
		    what, sp);
	}
}

static void
test_usage(void)
{
	char buf[64];
	unsigned long iter;

	/*
	 * usage() reads no argument and no program state, so its input
	 * space is a single point.  The sweep therefore varies the
	 * ambient state a port could accidentally depend on -- the
	 * siginfo flag, errno, and dirty stdio buffers -- and checks the
	 * captured stderr and exit status after every run.  Each case
	 * forks twice, which is what bounds the iteration count here.
	 */
	check_usage_once("edge first-call");

	fputs("", stderr);
	fputs("", stdout);
	check_usage_once("edge dirty-stdio");

	port::siginfo_set(1);
	ref_siginfo_set(1);
	check_usage_once("edge siginfo-set");

	port::siginfo_set(0);
	ref_siginfo_set(0);
	check_usage_once("edge siginfo-clear");

	port::siginfo_set((sig_atomic_t)INT_MIN);
	ref_siginfo_set((sig_atomic_t)INT_MIN);
	check_usage_once("edge siginfo-intmin");

	errno = ERANGE;
	check_usage_once("edge errno-dirty");
	errno = 0;
	check_usage_once("edge errno-clear");

	rng_seed(0x0195057a6e99ULL);
	for (iter = 0; iter < 20000UL; iter++) {
		sig_atomic_t v = (sig_atomic_t)(int32_t)rng_u32();

		port::siginfo_set(v);
		ref_siginfo_set(v);
		errno = (int)(rng_u32() % 64u);
		snprintf(buf, sizeof(buf), "random iter=%lu", iter);
		check_usage_once(buf);
	}
	errno = 0;
}

/* ------------------------------------------------------------------ */

static void
print_row(const Stats &s)
{

	printf("  %-20s %12lu %12lu  %s\n", s.name, s.cases, s.failures,
	    s.failures == 0 ? "ok" : "FAILED");
}

int
main(void)
{
	unsigned long total_fail;

	setvbuf(stdout, NULL, _IOLBF, 0);
	printf("pbsd b0195 bin/chmod -- differential harness\n\n");

	test_siginfo_handler();
	test_usage();

	printf("\n  %-20s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	printf("  %-20s %12s %12s  %s\n", "--------------------",
	    "------------", "------------", "------");
	print_row(st_siginfo_handler);
	print_row(st_usage);

	total_fail = st_siginfo_handler.failures + st_usage.failures;
	printf("\n  total cases   : %lu\n",
	    st_siginfo_handler.cases + st_usage.cases);
	printf("  total failures: %lu\n", total_fail);

	if (total_fail != 0) {
		printf("\nRESULT: FAIL\n");
		return (1);
	}
	printf("\nRESULT: PASS\n");
	return (0);
}
