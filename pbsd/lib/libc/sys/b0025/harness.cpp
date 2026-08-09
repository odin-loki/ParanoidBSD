/*
 * harness.cpp -- differential test for PBSD batch b0025.
 *
 * fork, fsync, fdatasync and close are libc interposition wrappers: each
 * loads a function pointer from __libc_interposing[] and tail-calls it with
 * the original argument list.  The harness installs instrumented mocks in
 * both the oracle (ref_*) and port tables, then compares the mock-visible
 * syscall tag, forwarded fd (where applicable) and return value.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_close,
	INTERPOS_fdatasync,
	INTERPOS_fork,
	INTERPOS_fsync,
	INTERPOS_MAX
};

extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];

pid_t ref_fork(void);
int ref_fsync(int fd);
int ref_fdatasync(int fd);
int ref_close(int fd);

}

import pbsd.lib.libc.sys.b0025;

namespace port = pbsd::lib_libc_sys::b0025;

/* ------------------------------------------------------------------ */
/* instrumented mocks                                                 */
/* ------------------------------------------------------------------ */

#define	PBSD_TAG_NONE		0
#define	PBSD_TAG_CLOSE		1
#define	PBSD_TAG_FDATASYNC	2
#define	PBSD_TAG_FORK		3
#define	PBSD_TAG_FSYNC		4

struct MockState {
	unsigned long long	ncalls;
	int			tag;
	int			fd;
	long long		prog_ret;
};

static MockState mock;

static void
mock_reset(long long ret)
{
	memset(&mock, 0, sizeof(mock));
	mock.prog_ret = ret;
}

static void
mock_enter(int tag, int fd)
{
	mock.ncalls++;
	mock.tag = tag;
	mock.fd = fd;
}

static pid_t
mock_fork(void)
{
	mock_enter(PBSD_TAG_FORK, 0);
	return ((pid_t)mock.prog_ret);
}

static int
mock_fsync(int fd)
{
	mock_enter(PBSD_TAG_FSYNC, fd);
	return ((int)mock.prog_ret);
}

static int
mock_fdatasync(int fd)
{
	mock_enter(PBSD_TAG_FDATASYNC, fd);
	return ((int)mock.prog_ret);
}

static int
mock_close(int fd)
{
	mock_enter(PBSD_TAG_CLOSE, fd);
	return ((int)mock.prog_ret);
}

static void
install_mocks(interpos_func_t *table)
{
	table[INTERPOS_close] = (interpos_func_t)mock_close;
	table[INTERPOS_fdatasync] = (interpos_func_t)mock_fdatasync;
	table[INTERPOS_fork] = (interpos_func_t)mock_fork;
	table[INTERPOS_fsync] = (interpos_func_t)mock_fsync;
}

struct Snap {
	unsigned long long	ncalls;
	int			tag;
	int			fd;
};

static Snap
take_snap(void)
{
	Snap s;

	s.ncalls = mock.ncalls;
	s.tag = mock.tag;
	s.fd = mock.fd;
	return (s);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

enum {
	FN_FORK,
	FN_FSYNC,
	FN_FDATASYNC,
	FN_CLOSE,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"fork",
	"fsync",
	"fdatasync",
	"close",
};

static unsigned long long fn_cases[FN_COUNT];
static unsigned long long fn_fails[FN_COUNT];
static int fn_reported[FN_COUNT];

#define	MAX_REPORTS	8

static bool
fail(int fn, const char *what, const char *detail)
{
	fn_fails[fn]++;
	if (fn_reported[fn] < MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-9s %-18s %s\n", fn_name[fn], what, detail);
	} else if (fn_reported[fn] == MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-9s ... further failures suppressed\n",
		    fn_name[fn]);
	}
	return (false);
}

static bool
cmp_snap(int fn, const Snap &a, const Snap &b, const char *ctx)
{
	if (a.ncalls != b.ncalls || a.tag != b.tag || a.fd != b.fd) {
		char msg[320];

		snprintf(msg, sizeof(msg),
		    "%s ref={ncalls=%llu tag=%d fd=%d} "
		    "port={ncalls=%llu tag=%d fd=%d}", ctx,
		    a.ncalls, a.tag, a.fd, b.ncalls, b.tag, b.fd);
		fail(fn, "mock", msg);
		return (false);
	}
	return (true);
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static uint64_t rng_state;

static void
rng_seed(uint64_t seed)
{
	rng_state = seed;
}

static uint32_t
rng_u32(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return ((uint32_t)rng_state);
}

static int
rnd_int(void)
{
	uint32_t u = rng_u32();

	return ((int)(u ^ (u >> 1)));
}

static long long
rnd_ret(void)
{
	uint32_t u = rng_u32();

	return ((long long)(int32_t)u);
}

/* ------------------------------------------------------------------ */
/* per-function cases                                                 */
/* ------------------------------------------------------------------ */

static void
case_fork(long long ret)
{
	Snap a, b;
	pid_t ra, rb;
	char ctx[128];

	fn_cases[FN_FORK]++;

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_fork();
	a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::fork();
	b = take_snap();

	snprintf(ctx, sizeof(ctx), "ret=%lld", ret);
	cmp_snap(FN_FORK, a, b, ctx);

	if (ra != rb) {
		char msg[192];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, (int)ra,
		    (int)rb);
		fail(FN_FORK, "return", msg);
	}
}

static void
case_fd(int fn, int fd, long long ret)
{
	Snap a, b;
	int ra, rb;
	char ctx[160];

	fn_cases[fn]++;

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	switch (fn) {
	case FN_FSYNC:
		ra = ref_fsync(fd);
		break;
	case FN_FDATASYNC:
		ra = ref_fdatasync(fd);
		break;
	case FN_CLOSE:
		ra = ref_close(fd);
		break;
	default:
		return;
	}
	a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	switch (fn) {
	case FN_FSYNC:
		rb = port::fsync(fd);
		break;
	case FN_FDATASYNC:
		rb = port::fdatasync(fd);
		break;
	case FN_CLOSE:
		rb = port::close(fd);
		break;
	default:
		return;
	}
	b = take_snap();

	snprintf(ctx, sizeof(ctx), "fd=%d ret=%lld", fd, ret);
	cmp_snap(fn, a, b, ctx);

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(fn, "return", msg);
	}
}

static void
test_fork(void)
{
	static const long long rets[] = {
		-1, 0, 1, 2, INT_MIN, INT_MAX,
		(long long)INT_MIN + 1, (long long)INT_MAX - 1,
		0x7f, 0x80, 0xff, -2, 32767, -32768,
	};

	for (size_t i = 0; i < sizeof(rets) / sizeof(rets[0]); i++)
		case_fork(rets[i]);

	rng_seed(0x66'6f'72'6bULL);
	for (int i = 0; i < 200000; i++)
		case_fork(rnd_ret());
}

static void
test_fd_syscalls(int fn)
{
	static const int fds[] = {
		INT_MIN, INT_MIN + 1, -2, -1, 0, 1, 2, 3,
		0x7e, 0x7f, 0x80, 0xfe, 0xff,
		INT_MAX - 1, INT_MAX,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
		for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
			case_fd(fn, fds[f], rets[r]);

	rng_seed(0x6664'0000ULL + (uint64_t)fn);
	for (int i = 0; i < 200000; i++)
		case_fd(fn, rnd_int(), rnd_ret());
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_fork();
	test_fd_syscalls(FN_FSYNC);
	test_fd_syscalls(FN_FDATASYNC);
	test_fd_syscalls(FN_CLOSE);

	printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	printf("--------------------------------------\n");
	for (int i = 0; i < FN_COUNT; i++) {
		printf("%-12s %12llu %12llu\n", fn_name[i], fn_cases[i],
		    fn_fails[i]);
		total_cases += fn_cases[i];
		total_fails += fn_fails[i];
	}
	printf("--------------------------------------\n");
	printf("%-12s %12llu %12llu\n", "TOTAL", total_cases, total_fails);

	return (total_fails == 0 ? 0 : 1);
}
