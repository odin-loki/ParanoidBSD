/*
 * b0215 differential harness: pbsd.bin.dd.b0215 vs oracle.c
 *
 * clock_gettime(3) and clock_getres(3) are wrapped for deterministic
 * secs_elapsed/summary/progress tests.  progress(), terminate(), and
 * sigint_handler() cases that depend on fork-local or noreturn behaviour
 * run in forked children.
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

import pbsd.bin.dd.b0215;

namespace P = pbsd::bin_dd::b0215;
using STAT = P::STAT;

#define	C_NOXFER	0x0000000010000000ULL
#define	C_NOINFO	0x0000000020000000ULL
#define	C_PROGRESS	0x0000000040000000ULL

extern "C" {
typedef struct {
	uintmax_t	in_full;
	uintmax_t	in_part;
	uintmax_t	out_full;
	uintmax_t	out_part;
	uintmax_t	trunc;
	uintmax_t	swab;
	uintmax_t	bytes;
	struct timespec	start;
} CSTAT;

double ref_secs_elapsed(void);
void ref_summary(void);
void ref_progress(void);
void ref_siginfo_handler(int);
void ref_sigalarm_handler(int);
__attribute__((__noreturn__)) void ref_terminate(int);
void ref_sigint_handler(int);
void ref_prepare_io(void);
void ref_before_io(void);
void ref_after_io(void);

extern CSTAT st;
extern uint64_t ddflags;
extern volatile sig_atomic_t need_summary;
extern volatile sig_atomic_t need_progress;
extern volatile sig_atomic_t kill_signal;
extern sig_atomic_t in_io;
extern sig_atomic_t sigint_seen;
}

/* ----------------------------------------------------------- clock hooks  */

static struct timespec g_end;
static struct timespec g_res;
static int g_fail_gettime;
static int g_fail_getres;

extern "C" int __real_clock_gettime(clockid_t, struct timespec *);
extern "C" int __real_clock_getres(clockid_t, struct timespec *);

extern "C" int
__wrap_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	if (g_fail_gettime)
		return (-1);
	if (clk_id == CLOCK_MONOTONIC) {
		*tp = g_end;
		return (0);
	}
	return (__real_clock_gettime(clk_id, tp));
}

extern "C" int
__wrap_clock_getres(clockid_t clk_id, struct timespec *tp)
{
	if (g_fail_getres)
		return (-1);
	if (clk_id == CLOCK_MONOTONIC) {
		*tp = g_res;
		return (0);
	}
	return (__real_clock_getres(clk_id, tp));
}

static void
clock_set(const struct timespec *end, const struct timespec *res)
{
	g_end = *end;
	g_res = *res;
	g_fail_gettime = 0;
	g_fail_getres = 0;
}

/* ------------------------------------------------------------------ stats */

struct Stats {
	const char *name;
	unsigned long cases;
	unsigned long fails;
};

static Stats S_secs = { "secs_elapsed", 0, 0 };
static Stats S_summary = { "summary", 0, 0 };
static Stats S_progress = { "progress", 0, 0 };
static Stats S_siginfo = { "siginfo_handler", 0, 0 };
static Stats S_sigalarm = { "sigalarm_handler", 0, 0 };
static Stats S_terminate = { "terminate", 0, 0 };
static Stats S_sigint = { "sigint_handler", 0, 0 };
static Stats S_prepare = { "prepare_io", 0, 0 };
static Stats S_before = { "before_io", 0, 0 };
static Stats S_after = { "after_io", 0, 0 };

static const unsigned long MAX_REPORTED = 12;

/* ------------------------------------------------------------------- rng  */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s ? s : 0x9e3779b97f4a7c15ULL;
}

static uint64_t
rnd(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return (x);
}

static uint32_t
rnd_below(uint32_t n)
{
	return (n == 0 ? 0 : (uint32_t)(rnd() % n));
}

/* --------------------------------------------------------- stderr capture */

static int cap_fd = -1;
static char capA[16384];
static char capB[16384];

static int
capture_setup(void)
{
	char tmpl[] = "/tmp/pbsd-b0215-stderr-XXXXXX";
	int fd;

	fd = mkstemp(tmpl);
	if (fd < 0)
		return (-1);
	if (freopen(tmpl, "w+", stderr) == NULL) {
		close(fd);
		unlink(tmpl);
		return (-1);
	}
	unlink(tmpl);
	close(fd);
	setvbuf(stderr, NULL, _IONBF, 0);
	cap_fd = fileno(stderr);
	return (cap_fd < 0 ? -1 : 0);
}

static void
cap_begin(void)
{
	fflush(stderr);
	rewind(stderr);
}

static long
cap_end(char *out, size_t cap, size_t *got)
{
	off_t pos;
	size_t n;
	ssize_t r;

	*got = 0;
	out[0] = '\0';
	fflush(stderr);
	pos = ftello(stderr);
	if (pos < 0)
		return (-1);
	n = (size_t)pos;
	if (n > cap - 1)
		n = cap - 1;
	if (n > 0) {
		r = pread(cap_fd, out, n, 0);
		if (r < 0)
			return (-2);
		n = (size_t)r;
	}
	out[n] = '\0';
	*got = n;
	return ((long)pos);
}

/* ------------------------------------------------------------- sync state */

static void
copy_stat_fields(const STAT &src, STAT &dst)
{
	dst.in_full = src.in_full;
	dst.in_part = src.in_part;
	dst.out_full = src.out_full;
	dst.out_part = src.out_part;
	dst.trunc = src.trunc;
	dst.swab = src.swab;
	dst.bytes = src.bytes;
	dst.start.tv_sec = src.start.tv_sec;
	dst.start.tv_nsec = src.start.tv_nsec;
}

static void
copy_stat_fields(const STAT &src, CSTAT &dst)
{
	dst.in_full = src.in_full;
	dst.in_part = src.in_part;
	dst.out_full = src.out_full;
	dst.out_part = src.out_part;
	dst.trunc = src.trunc;
	dst.swab = src.swab;
	dst.bytes = src.bytes;
	dst.start.tv_sec = src.start.tv_sec;
	dst.start.tv_nsec = src.start.tv_nsec;
}

static void
set_port_state(const STAT &s, uint64_t flags, long ns, long np, long io,
    long seen)
{
	copy_stat_fields(s, P::st);
	P::ddflags = flags;
	P::need_summary = (sig_atomic_t)ns;
	P::need_progress = (sig_atomic_t)np;
	P::in_io = (sig_atomic_t)io;
	P::sigint_seen = (sig_atomic_t)seen;
	P::kill_signal = 0;
}

static void
set_ref_state(const STAT &s, uint64_t flags, long ns, long np, long io,
    long seen)
{
	copy_stat_fields(s, st);
	ddflags = flags;
	need_summary = (sig_atomic_t)ns;
	need_progress = (sig_atomic_t)np;
	::in_io = (sig_atomic_t)io;
	::sigint_seen = (sig_atomic_t)seen;
	kill_signal = 0;
}

static void
set_both_state(const STAT &s, uint64_t flags, long ns, long np, long io,
    long seen)
{
	set_port_state(s, flags, ns, np, io, seen);
	set_ref_state(s, flags, ns, np, io, seen);
}

static const STAT zero_stat = {};

static STAT
make_stat(uintmax_t in_full, uintmax_t in_part, uintmax_t out_full,
    uintmax_t out_part, uintmax_t trunc, uintmax_t swab, uintmax_t bytes,
    time_t ss, long sn)
{
	STAT s = zero_stat;

	s.in_full = in_full;
	s.in_part = in_part;
	s.out_full = out_full;
	s.out_part = out_part;
	s.trunc = trunc;
	s.swab = swab;
	s.bytes = bytes;
	s.start.tv_sec = ss;
	s.start.tv_nsec = sn;
	return (s);
}

/* ------------------------------------------------------- secs_elapsed     */

static void
check_secs(const struct timespec *end, const struct timespec *res,
    const STAT &s, const char *tag)
{
	double a, b;
	const char *why = NULL;

	clock_set(end, res);
	set_both_state(s, 0, 0, 0, 0, 0);
	S_secs.cases++;

	a = P::secs_elapsed();
	clock_set(end, res);
	set_both_state(s, 0, 0, 0, 0, 0);
	b = ref_secs_elapsed();

	if (memcmp(&a, &b, sizeof(a)) != 0)
		why = "return value";

	if (why == NULL)
		return;
	S_secs.fails++;
	if (S_secs.fails > MAX_REPORTED)
		return;
	printf("  FAIL secs_elapsed (%s): %s\n", tag, why);
	printf("    port = %.17g  ref = %.17g\n", a, b);
}

static void
test_secs_elapsed(void)
{
	static const struct {
		struct timespec end;
		struct timespec res;
		STAT st;
	} edge[] = {
		{ { 10, 0 }, { 0, 1 }, make_stat(0,0,0,0,0,0,0, 0, 0) },
		{ { 10, 0 }, { 0, 1 }, make_stat(0,0,0,0,0,0,0, 10, 0) },
		{ { 10, 500000000L }, { 0, 1 }, make_stat(0,0,0,0,0,0,0, 9, 0) },
		{ { 10, 0 }, { 0, 1 }, make_stat(0,0,0,0,0,0,0, 10, 1) },
		{ { 9, 999999999L }, { 0, 2 }, make_stat(0,0,0,0,0,0,0, 10, 0) },
		{ { 0, 0 }, { 1, 0 }, make_stat(0,0,0,0,0,0,0, 5, 0) },
		{ { 5, 0 }, { 2, 0 }, make_stat(0,0,0,0,0,0,0, 10, 0) },
		{ { 100, 0 }, { 0, 100000000L }, make_stat(0,0,0,0,0,0,0, 0, 0) },
		{ { 1, 1 }, { 0, 2 }, make_stat(0,0,0,0,0,0,0, 1, 2) },
		{ { 1, 0 }, { 0, 0 }, make_stat(0,0,0,0,0,0,0, 1, 0) },
		{ { 2000000000L, 0 }, { 0, 1 },
		    make_stat(1,2,3,4,5,6,7, 1999999999L, 500000000L) },
	};
	size_t i;
	long iter;

	for (i = 0; i < sizeof(edge) / sizeof(edge[0]); i++)
		check_secs(&edge[i].end, &edge[i].res, edge[i].st, "edge");

	rng_seed(0xb0215aULL);
	for (iter = 0; iter < 200000; iter++) {
		struct timespec end, res;
		STAT s;
		time_t ss;
		long sn;

		end.tv_sec = (time_t)(int64_t)(rnd() & 0x7fffffff);
		end.tv_nsec = (long)(rnd() % 1000000000L);
		res.tv_sec = (time_t)(rnd() % 4);
		res.tv_nsec = (long)(rnd() % 8);
		ss = (time_t)(int64_t)(rnd() & 0x7fffffff);
		sn = (long)(rnd() % 1000000000L);
		s = make_stat(rnd(), rnd(), rnd(), rnd(), rnd() % 3,
		    rnd() % 3, rnd(), ss, sn);
		check_secs(&end, &res, s, "random");
	}
}

/* ------------------------------------------------------- summary          */

static void
check_summary(const STAT &s, uint64_t flags, long pre_ns, const char *tag)
{
	long la, lb;
	size_t ga, gb;
	long va, vb;
	const char *why = NULL;
	struct timespec end = { 100, 0 };
	struct timespec res = { 0, 1 };

	clock_set(&end, &res);
	set_both_state(s, flags, pre_ns, 0, 0, 0);
	S_summary.cases++;

	cap_begin();
	P::summary();
	la = cap_end(capA, sizeof(capA), &ga);
	va = (long)P::need_summary;

	cap_begin();
	ref_summary();
	lb = cap_end(capB, sizeof(capB), &gb);
	vb = (long)need_summary;

	if (la < 0 || lb < 0)
		why = "stderr capture failed";
	else if (la != lb)
		why = "stderr byte count";
	else if (ga != gb || memcmp(capA, capB, ga) != 0)
		why = "stderr text";
	else if (va != vb)
		why = "need_summary after call";

	if (why == NULL)
		return;
	S_summary.fails++;
	if (S_summary.fails > MAX_REPORTED)
		return;
	printf("  FAIL summary (%s): %s\n", tag, why);
	printf("    flags=0x%llx pre_ns=%ld port_ns=%ld ref_ns=%ld\n",
	    (unsigned long long)flags, pre_ns, va, vb);
	printf("    port err = [%s]\n", capA);
	printf("    ref  err = [%s]\n", capB);
}

static void
test_summary(void)
{
	static const struct {
		STAT st;
		uint64_t flags;
		long pre_ns;
	} edge[] = {
		{ make_stat(0,0,0,0,0,0,0,0,0), C_NOINFO, 1 },
		{ make_stat(1,2,3,4,0,0,100,0,0), 0, 1 },
		{ make_stat(1,2,3,4,0,0,100,0,0), C_PROGRESS, 1 },
		{ make_stat(1,2,3,4,1,0,100,0,0), 0, 1 },
		{ make_stat(1,2,3,4,2,0,100,0,0), 0, 1 },
		{ make_stat(1,2,3,4,0,1,100,0,0), 0, 1 },
		{ make_stat(1,2,3,4,0,2,100,0,0), 0, 1 },
		{ make_stat(1,2,3,4,1,1,100,0,0), 0, 1 },
		{ make_stat(0,0,0,0,0,0,0,0,0), C_NOXFER, 1 },
		{ make_stat(0,0,0,0,0,0,0,0,0), C_NOXFER | C_PROGRESS, 1 },
		{ make_stat(UINTMAX_MAX,1,2,3,0,0,999,0,0), 0, 0 },
		{ make_stat(5,5,5,5,3,4,0,50,0), 0, 7 },
	};
	size_t i;
	long iter;

	for (i = 0; i < sizeof(edge) / sizeof(edge[0]); i++)
		check_summary(edge[i].st, edge[i].flags, edge[i].pre_ns, "edge");

	rng_seed(0xb0215bULL);
	for (iter = 0; iter < 200000; iter++) {
		STAT s;
		uint64_t flags = 0;

		s = make_stat(rnd(), rnd(), rnd(), rnd(), rnd() % 4,
		    rnd() % 4, rnd() % 1000000, (time_t)(rnd() % 1000),
		    (long)(rnd() % 1000000000L));
		if (rnd_below(5) == 0)
			flags |= C_NOINFO;
		if (rnd_below(4) == 0)
			flags |= C_PROGRESS;
		if (rnd_below(3) == 0)
			flags |= C_NOXFER;
		check_summary(s, flags, (long)(rnd_below(3)), "random");
	}
}

/* ------------------------------------------------------- progress (fork)  */

struct PipeOut {
	int ok;
	long cap_len;
	size_t got;
	char buf[16384];
	long need_progress;
};

static int
run_progress_child(int use_port, const STAT &s, PipeOut *out)
{
	int pfd[2];
	pid_t pid;
	int st = 0;

	out->ok = 0;
	out->cap_len = -1;
	out->got = 0;
	out->buf[0] = '\0';
	out->need_progress = -1;

	if (pipe(pfd) != 0)
		return (-1);
	pid = fork();
	if (pid < 0) {
		close(pfd[0]);
		close(pfd[1]);
		return (-1);
	}
	if (pid == 0) {
		char tmpl[] = "/tmp/pbsd-b0215-p-stderr-XXXXXX";
		int fd;
		long len;
		size_t got;
		char local[16384];
		struct timespec end = { 50, 250000000L };
		struct timespec res = { 0, 1 };

		close(pfd[0]);
		fd = mkstemp(tmpl);
		if (fd < 0)
			_exit(90);
		if (dup2(fd, STDERR_FILENO) < 0)
			_exit(91);
		unlink(tmpl);
		close(fd);

		clock_set(&end, &res);
		if (use_port) {
			set_port_state(s, 0, 0, 1, 0, 0);
			P::progress();
			out->need_progress = (long)P::need_progress;
		} else {
			set_ref_state(s, 0, 0, 1, 0, 0);
			ref_progress();
			out->need_progress = (long)need_progress;
		}
		fflush(stderr);
		len = (long)lseek(STDERR_FILENO, 0, SEEK_END);
		got = 0;
		if (len > 0) {
			lseek(STDERR_FILENO, 0, SEEK_SET);
			if ((size_t)len >= sizeof(local))
				len = (long)sizeof(local) - 1;
			got = (size_t)read(STDERR_FILENO, local, (size_t)len);
		}
		local[got] = '\0';
		if (write(pfd[1], &len, sizeof(len)) != (ssize_t)sizeof(len) ||
		    write(pfd[1], &got, sizeof(got)) != (ssize_t)sizeof(got) ||
		    write(pfd[1], local, got) != (ssize_t)got ||
		    write(pfd[1], &out->need_progress,
		    sizeof(out->need_progress)) !=
		    (ssize_t)sizeof(out->need_progress))
			_exit(92);
		close(pfd[1]);
		_exit(0);
	}
	close(pfd[1]);
	if (read(pfd[0], &out->cap_len, sizeof(out->cap_len)) !=
	    (ssize_t)sizeof(out->cap_len) ||
	    read(pfd[0], &out->got, sizeof(out->got)) !=
	    (ssize_t)sizeof(out->got) ||
	    read(pfd[0], out->buf, out->got) != (ssize_t)out->got ||
	    read(pfd[0], &out->need_progress, sizeof(out->need_progress)) !=
	    (ssize_t)sizeof(out->need_progress)) {
		close(pfd[0]);
		kill(pid, SIGKILL);
		waitpid(pid, &st, 0);
		return (-1);
	}
	close(pfd[0]);
	if (waitpid(pid, &st, 0) < 0 || !WIFEXITED(st) || WEXITSTATUS(st) != 0)
		return (-1);
	out->ok = 1;
	return (0);
}

static void
check_progress(const STAT &s, const char *tag)
{
	PipeOut a, b;
	const char *why = NULL;

	S_progress.cases++;
	if (run_progress_child(1, s, &a) != 0 || run_progress_child(0, s, &b) != 0)
		why = "fork/pipe failed";
	else if (a.cap_len != b.cap_len)
		why = "stderr byte count";
	else if (a.got != b.got || memcmp(a.buf, b.buf, a.got) != 0)
		why = "stderr text";
	else if (a.need_progress != b.need_progress)
		why = "need_progress after call";

	if (why == NULL)
		return;
	S_progress.fails++;
	if (S_progress.fails > MAX_REPORTED)
		return;
	printf("  FAIL progress (%s): %s\n", tag, why);
	printf("    port err = [%s]\n", a.buf);
	printf("    ref  err = [%s]\n", b.buf);
}

static void
test_progress(void)
{
	static const STAT edge[] = {
		make_stat(0,0,0,0,0,0,0,0,0),
		make_stat(0,0,0,0,0,0,1,0,0),
		make_stat(0,0,0,0,0,0,999,0,0),
		make_stat(0,0,0,0,0,0,1000,0,0),
		make_stat(0,0,0,0,0,0,1023,0,0),
		make_stat(0,0,0,0,0,0,1024,0,0),
		make_stat(0,0,0,0,0,0,9999,0,0),
		make_stat(0,0,0,0,0,0,1000000,0,0),
		make_stat(0,0,0,0,0,0,1000000000ULL,0,0),
		make_stat(1,2,3,4,0,0,512000,10,0),
	};
	size_t i;
	long iter;
	STAT seq1, seq2;

	for (i = 0; i < sizeof(edge) / sizeof(edge[0]); i++)
		check_progress(edge[i], "edge");

	seq1 = make_stat(0,0,0,0,0,0,12345,0,0);
	seq2 = make_stat(0,0,0,0,0,0,54321,0,0);
	check_progress(seq1, "seq1");
	check_progress(seq2, "seq2");

	rng_seed(0xb0215cULL);
	for (iter = 0; iter < 200000; iter++) {
		STAT s = make_stat(0, 0, 0, 0, 0, 0, rnd() % 2000000000ULL,
		    (time_t)(rnd() % 100), (long)(rnd() % 1000000000L));
		check_progress(s, "random");
	}
}

/* ------------------------------------------------------- simple handlers  */

static void
check_siginfo(int signo, long pre, const char *tag)
{
	long va, vb;
	const char *why = NULL;
	STAT z = zero_stat;

	S_siginfo.cases++;
	set_both_state(z, 0, pre, 0, 0, 0);
	P::siginfo_handler(signo);
	va = (long)P::need_summary;
	set_both_state(z, 0, pre, 0, 0, 0);
	ref_siginfo_handler(signo);
	vb = (long)need_summary;

	if (va != vb)
		why = "need_summary";
	if (why == NULL)
		return;
	S_siginfo.fails++;
	if (S_siginfo.fails <= MAX_REPORTED)
		printf("  FAIL siginfo_handler (%s): %s (port=%ld ref=%ld)\n",
		    tag, why, va, vb);
}

static void
check_sigalarm(int signo, long pre, const char *tag)
{
	long va, vb;
	const char *why = NULL;
	STAT z = zero_stat;

	S_sigalarm.cases++;
	set_both_state(z, 0, 0, pre, 0, 0);
	P::sigalarm_handler(signo);
	va = (long)P::need_progress;
	set_both_state(z, 0, 0, pre, 0, 0);
	ref_sigalarm_handler(signo);
	vb = (long)need_progress;

	if (va != vb)
		why = "need_progress";
	if (why == NULL)
		return;
	S_sigalarm.fails++;
	if (S_sigalarm.fails <= MAX_REPORTED)
		printf("  FAIL sigalarm_handler (%s): %s (port=%ld ref=%ld)\n",
		    tag, why, va, vb);
}

static void
test_sig_handlers(void)
{
	static const int signos[] = { 0, 1, -1, 2, SIGINT, SIGUSR1, SIGALRM,
	    127, 128, 255, INT_MAX, INT_MIN };
	static const long pres[] = { 0, 1, -1, 2, 7, 127 };
	size_t i, j;
	long iter;

	for (i = 0; i < sizeof(signos) / sizeof(signos[0]); i++)
		for (j = 0; j < sizeof(pres) / sizeof(pres[0]); j++) {
			check_siginfo(signos[i], pres[j], "edge");
			check_sigalarm(signos[i], pres[j], "edge");
		}

	rng_seed(0xb0215dULL);
	for (iter = 0; iter < 200000; iter++) {
		int signo = (int)(int32_t)(uint32_t)rnd();
		long pre = (long)(int32_t)(uint32_t)rnd();

		check_siginfo(signo, pre, "random");
		check_sigalarm(signo, pre, "random");
	}
}

/* ------------------------------------------------------- sigint_handler   */

struct SigintOut {
	int ok;
	long sigint_seen;
	int terminated;
	int term_signo;
};

static int
run_sigint_child(int use_port, long in_io_v, long seen_v, SigintOut *out)
{
	pid_t pid;
	int st = 0;
	int pfd[2];
	STAT z = zero_stat;

	out->ok = 0;
	out->sigint_seen = -1;
	out->terminated = 0;
	out->term_signo = -1;

	if (pipe(pfd) != 0)
		return (-1);
	pid = fork();
	if (pid < 0) {
		close(pfd[0]);
		close(pfd[1]);
		return (-1);
	}
	if (pid == 0) {
		struct sigaction sa;

		close(pfd[0]);
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGINT, &sa, NULL);

		set_both_state(z, 0, 0, 0, in_io_v, seen_v);
		if (use_port) {
			P::in_io = (sig_atomic_t)in_io_v;
			P::sigint_seen = (sig_atomic_t)seen_v;
			P::sigint_handler(SIGINT);
			if (!in_io_v)
				out->sigint_seen = (long)P::sigint_seen;
		} else {
			::in_io = (sig_atomic_t)in_io_v;
			::sigint_seen = (sig_atomic_t)seen_v;
			ref_sigint_handler(SIGINT);
			if (!in_io_v)
				out->sigint_seen = (long)::sigint_seen;
		}
		write(pfd[1], out, sizeof(*out));
		close(pfd[1]);
		_exit(0);
	}
	close(pfd[1]);
	if (read(pfd[0], out, sizeof(*out)) != (ssize_t)sizeof(*out)) {
		close(pfd[0]);
		kill(pid, SIGKILL);
		waitpid(pid, &st, 0);
		return (-1);
	}
	close(pfd[0]);
	if (waitpid(pid, &st, 0) < 0)
		return (-1);
	if (in_io_v) {
		if (WIFSIGNALED(st) && WTERMSIG(st) == SIGINT) {
			out->terminated = 1;
			out->term_signo = SIGINT;
			out->ok = 1;
			return (0);
		}
		return (-1);
	}
	if (!WIFEXITED(st) || WEXITSTATUS(st) != 0)
		return (-1);
	out->ok = 1;
	return (0);
}

static void
check_sigint(long in_io_v, long seen_v, const char *tag)
{
	SigintOut a, b;
	const char *why = NULL;

	S_sigint.cases++;
	if (run_sigint_child(1, in_io_v, seen_v, &a) != 0 ||
	    run_sigint_child(0, in_io_v, seen_v, &b) != 0)
		why = "fork failed";
	else if (a.terminated != b.terminated)
		why = "terminated mismatch";
	else if (a.terminated) {
		if (a.term_signo != b.term_signo)
			why = "terminating signal";
	} else if (a.sigint_seen != b.sigint_seen)
		why = "sigint_seen";

	if (why == NULL)
		return;
	S_sigint.fails++;
	if (S_sigint.fails <= MAX_REPORTED)
		printf("  FAIL sigint_handler (%s): %s in_io=%ld seen=%ld\n",
		    tag, why, in_io_v, seen_v);
}

static void
test_sigint_handler(void)
{
	long iter;

	check_sigint(0, 0, "edge");
	check_sigint(0, 1, "edge");
	check_sigint(1, 0, "edge");

	rng_seed(0xb0215eULL);
	for (iter = 0; iter < 200000; iter++) {
		long io = (long)(rnd_below(2));
		long seen = (long)(rnd_below(2));
		check_sigint(io, seen, "random");
	}
}

/* ------------------------------------------------------- before/after io  */

static int
run_before_after_child(int use_port, int use_before, long seen_v,
    long *io_out)
{
	pid_t pid;
	int st = 0;
	STAT z = zero_stat;

	pid = fork();
	if (pid < 0)
		return (-1);
	if (pid == 0) {
		struct sigaction sa;

		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGINT, &sa, NULL);

		set_both_state(z, 0, 0, 0, 0, seen_v);
		if (use_port) {
			if (use_before)
				P::before_io();
			else
				P::after_io();
			*io_out = (long)P::in_io;
		} else {
			if (use_before)
				ref_before_io();
			else
				ref_after_io();
			*io_out = (long)::in_io;
		}
		_exit(0);
	}
	if (waitpid(pid, &st, 0) < 0)
		return (-1);
	if (seen_v) {
		if (WIFSIGNALED(st) && WTERMSIG(st) == SIGINT)
			return (1);
		return (-1);
	}
	if (!WIFEXITED(st) || WEXITSTATUS(st) != 0)
		return (-1);
	return (0);
}

static void
check_before_after(int use_before, long seen_v, const char *tag)
{
	long io_a = -1, io_b = -1;
	int ra, rb;
	const char *why = NULL;

	if (use_before)
		S_before.cases++;
	else
		S_after.cases++;

	ra = run_before_after_child(1, use_before, seen_v, &io_a);
	rb = run_before_after_child(0, use_before, seen_v, &io_b);

	if (ra < 0 || rb < 0)
		why = "fork/wait failed";
	else if (seen_v) {
		if (ra != 1 || rb != 1)
			why = "expected SIGINT termination";
	} else if (io_a != io_b)
		why = "in_io after call";
	else if (use_before && io_a != 1)
		why = "before_io did not set in_io";
	else if (!use_before && io_a != 0)
		why = "after_io did not clear in_io";

	if (why == NULL)
		return;
	if (use_before) {
		S_before.fails++;
		if (S_before.fails <= MAX_REPORTED)
			printf("  FAIL before_io (%s): %s seen=%ld\n", tag, why,
			    seen_v);
	} else {
		S_after.fails++;
		if (S_after.fails <= MAX_REPORTED)
			printf("  FAIL after_io (%s): %s seen=%ld\n", tag, why,
			    seen_v);
	}
}

static void
test_before_after(void)
{
	long iter;

	check_before_after(1, 0, "edge");
	check_before_after(1, 1, "edge");
	check_before_after(0, 0, "edge");
	check_before_after(0, 1, "edge");

	rng_seed(0xb0215fULL);
	for (iter = 0; iter < 200000; iter++) {
		long seen = (long)(rnd_below(2));
		check_before_after(1, seen, "random");
		check_before_after(0, seen, "random");
	}
}

/* ------------------------------------------------------- prepare_io       */

struct PrepareOut {
	int ok;
	int flags;
	long seen_after;
};

static int
run_prepare_child(int use_port, PrepareOut *out)
{
	pid_t pid;
	int st = 0;
	int pfd[2];
	struct sigaction sa;
	STAT z = zero_stat;

	out->ok = 0;
	out->flags = -1;
	out->seen_after = -1;

	if (pipe(pfd) != 0)
		return (-1);
	pid = fork();
	if (pid < 0) {
		close(pfd[0]);
		close(pfd[1]);
		return (-1);
	}
	if (pid == 0) {
		close(pfd[0]);
		set_both_state(z, 0, 0, 0, 0, 0);
		if (use_port)
			P::prepare_io();
		else
			ref_prepare_io();
		if (sigaction(SIGINT, NULL, &sa) != 0)
			_exit(93);
		out->flags = sa.sa_flags;
		if (use_port) {
			P::in_io = 0;
			P::sigint_seen = 0;
			sa.sa_handler(SIGINT);
			out->seen_after = (long)P::sigint_seen;
		} else {
			::in_io = 0;
			::sigint_seen = 0;
			sa.sa_handler(SIGINT);
			out->seen_after = (long)::sigint_seen;
		}
		out->ok = 1;
		write(pfd[1], out, sizeof(*out));
		close(pfd[1]);
		_exit(0);
	}
	close(pfd[1]);
	if (read(pfd[0], out, sizeof(*out)) != (ssize_t)sizeof(*out)) {
		close(pfd[0]);
		kill(pid, SIGKILL);
		waitpid(pid, &st, 0);
		return (-1);
	}
	close(pfd[0]);
	if (waitpid(pid, &st, 0) < 0 || !WIFEXITED(st) || WEXITSTATUS(st) != 0)
		return (-1);
	return (0);
}

static void
check_prepare(const char *tag)
{
	PrepareOut a, b;
	const char *why = NULL;

	S_prepare.cases++;
	if (run_prepare_child(1, &a) != 0 || run_prepare_child(0, &b) != 0)
		why = "fork failed";
	else if (a.flags != b.flags)
		why = "sigaction flags";
	else if (a.seen_after != b.seen_after)
		why = "installed handler behaviour";

	if (why == NULL)
		return;
	S_prepare.fails++;
	if (S_prepare.fails <= MAX_REPORTED)
		printf("  FAIL prepare_io (%s): %s\n", tag, why);
}

static void
test_prepare_io(void)
{
	long iter;

	check_prepare("edge");
	rng_seed(0xb021510ULL);
	for (iter = 0; iter < 200000; iter++)
		check_prepare("random");
}

/* ------------------------------------------------------- terminate        */

struct TermOut {
	int ok;
	long cap_len;
	size_t got;
	char buf[16384];
	int signo;
};

static int
run_terminate_child(int use_port, int signo, const STAT &s, TermOut *out)
{
	pid_t pid;
	int st = 0;
	int pfd[2];

	out->ok = 0;
	out->cap_len = -1;
	out->got = 0;
	out->buf[0] = '\0';
	out->signo = signo;

	if (pipe(pfd) != 0)
		return (-1);
	pid = fork();
	if (pid < 0) {
		close(pfd[0]);
		close(pfd[1]);
		return (-1);
	}
	if (pid == 0) {
		struct timespec end = { 20, 0 };
		struct timespec res = { 0, 1 };

		close(pfd[0]);

		clock_set(&end, &res);
		set_both_state(s, 0, 1, 0, 0, 0);
		if (use_port)
			P::terminate(signo);
		else
			ref_terminate(signo);
		_exit(95);
	}
	close(pfd[1]);
	if (waitpid(pid, &st, 0) < 0)
		return (-1);
	if (!(WIFSIGNALED(st) && WTERMSIG(st) == signo))
		return (-1);
	out->ok = 1;
	return (0);
}

static void
check_terminate(const STAT &s, int signo, const char *tag)
{
	TermOut a, b;
	const char *why = NULL;

	S_terminate.cases++;
	if (run_terminate_child(1, signo, s, &a) != 0 ||
	    run_terminate_child(0, signo, s, &b) != 0)
		why = "fork/signal outcome";

	if (why == NULL)
		return;
	S_terminate.fails++;
	if (S_terminate.fails <= MAX_REPORTED)
		printf("  FAIL terminate (%s): %s signo=%d\n", tag, why, signo);
}

static void
test_terminate(void)
{
	static const int signos[] = { SIGINT, SIGUSR1, SIGTERM };
	static const STAT edge[] = {
		make_stat(0,0,0,0,0,0,0,0,0),
		make_stat(1,2,3,4,0,0,500,0,0),
		make_stat(0,0,0,0,1,2,1000,0,0),
	};
	size_t i, j;
	long iter;

	for (i = 0; i < sizeof(edge) / sizeof(edge[0]); i++)
		for (j = 0; j < sizeof(signos) / sizeof(signos[0]); j++)
			check_terminate(edge[i], signos[j], "edge");

	rng_seed(0xb021511ULL);
	for (iter = 0; iter < 200000; iter++) {
		STAT s = make_stat(rnd() % 10, rnd() % 10, rnd() % 10,
		    rnd() % 10, rnd() % 3, rnd() % 3, rnd() % 10000,
		    (time_t)(rnd() % 100), (long)(rnd() % 1000000000L));
		int signo = (int[]){ SIGINT, SIGUSR1, SIGTERM }[rnd_below(3)];
		check_terminate(s, signo, "random");
	}
}

/* ------------------------------------------------------------------ main  */

static void
row(const Stats &s)
{
	printf("  %-20s %10lu %10lu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "ok" : "FAILED");
}

int
main(void)
{
	unsigned long total_cases, total_fails;

	printf("b0215 dd misc: pbsd.bin.dd.b0215 vs oracle.c\n");

	test_secs_elapsed();

	if (capture_setup() != 0) {
		printf("harness: cannot redirect stderr: %s\n", strerror(errno));
		return (1);
	}

	test_summary();
	test_progress();
	test_sig_handlers();
	test_sigint_handler();
	test_before_after();
	test_prepare_io();
	test_terminate();

	total_cases = S_secs.cases + S_summary.cases + S_progress.cases +
	    S_siginfo.cases + S_sigalarm.cases + S_terminate.cases +
	    S_sigint.cases + S_prepare.cases + S_before.cases + S_after.cases;
	total_fails = S_secs.fails + S_summary.fails + S_progress.fails +
	    S_siginfo.fails + S_sigalarm.fails + S_terminate.fails +
	    S_sigint.fails + S_prepare.fails + S_before.fails + S_after.fails;

	printf("\n  %-20s %10s %10s\n", "function", "cases", "failures");
	printf("  %-20s %10s %10s\n", "--------------------", "----------",
	    "----------");
	row(S_secs);
	row(S_summary);
	row(S_progress);
	row(S_siginfo);
	row(S_sigalarm);
	row(S_terminate);
	row(S_sigint);
	row(S_prepare);
	row(S_before);
	row(S_after);
	printf("  %-20s %10s %10s\n", "--------------------", "----------",
	    "----------");
	printf("  %-20s %10lu %10lu   %s\n", "TOTAL", total_cases, total_fails,
	    total_fails == 0 ? "PASS" : "FAIL");

	return (total_fails == 0 ? 0 : 1);
}
