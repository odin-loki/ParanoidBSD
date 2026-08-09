/*
 * Differential harness for batch b0258 (__dup3, __assert).
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef O_CLOFORK
#define O_CLOFORK 0x08000000
#endif
#ifndef F_DUP3FD
#define F_DUP3FD 24
#endif
#ifndef F_DUP3FD_SHIFT
#define F_DUP3FD_SHIFT 16
#endif
#ifndef FD_CLOFORK
#define FD_CLOFORK 4
#endif

import pbsd.lib.libc.gen.b0258;

namespace P = pbsd::lib_libc_gen::b0258;

extern "C" {
int ref___dup3(int oldfd, int newfd, int flags);
void ref___assert(const char *func, const char *file, int line,
    const char *failedexpr);
}

enum { F_DUP3, F_ASSERT, NFUNC };

static const char *const fname[NFUNC] = {
	"__dup3",
	"__assert",
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 16;
static int nreported;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-10s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0x9e3779b97f4a7c15ULL;

static std::uint64_t
nextrand(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static std::uint32_t
randu32(void)
{
	return (std::uint32_t)(nextrand() & 0xffffffffu);
}

static int
randint(void)
{
	return (int)randu32();
}

/* ------------------------------------------------------------------ */
/* abort mock                                                         */
/* ------------------------------------------------------------------ */

static int g_abort_called;

extern "C" void
__wrap_abort(void)
{
	g_abort_called++;
}

/* ------------------------------------------------------------------ */
/* _fcntl mock                                                        */
/* ------------------------------------------------------------------ */

struct FcntlCall {
	int	fd;
	int	cmd;
	int	arg;
};

static constexpr int FCNTL_LOG_MAX = 8;

static FcntlCall fcntl_log[FCNTL_LOG_MAX];
static int fcntl_log_n;
static int fcntl_fail;
static int fcntl_ret = 7;
static int fcntl_err = EBADF;

static void
fcntl_mock_reset(void)
{
	fcntl_log_n = 0;
	fcntl_fail = 0;
	fcntl_ret = 7;
	fcntl_err = EBADF;
}

static void
fcntl_mock_configure(int fail, int ret, int err)
{
	fcntl_fail = fail;
	fcntl_ret = ret;
	fcntl_err = err;
}

extern "C" int
_fcntl(int fd, int cmd, int arg)
{
	if (fcntl_log_n < FCNTL_LOG_MAX) {
		fcntl_log[fcntl_log_n].fd = fd;
		fcntl_log[fcntl_log_n].cmd = cmd;
		fcntl_log[fcntl_log_n].arg = arg;
		fcntl_log_n++;
	}
	if (fcntl_fail) {
		errno = fcntl_err;
		return (-1);
	}
	return (fcntl_ret);
}

/* ------------------------------------------------------------------ */
/* stderr capture                                                     */
/* ------------------------------------------------------------------ */

static constexpr std::size_t STDERR_CAP = 4096;

static char stderr_cap[STDERR_CAP];
static int stderr_saved_fd = -1;
static int stderr_pipe_rd = -1;
static int stderr_pipe_wr = -1;

static void
stderr_capture_begin(void)
{
	int p[2];

	if (pipe(p) != 0)
		std::abort();
	stderr_saved_fd = dup(STDERR_FILENO);
	if (stderr_saved_fd < 0)
		std::abort();
	std::fflush(stderr);
	if (dup2(p[1], STDERR_FILENO) < 0)
		std::abort();
	::close(p[1]);
	stderr_pipe_rd = p[0];
	stderr_pipe_wr = -1;
}

static void
stderr_capture_end(void)
{
	std::fflush(stderr);
	if (stderr_saved_fd >= 0) {
		dup2(stderr_saved_fd, STDERR_FILENO);
		::close(stderr_saved_fd);
		stderr_saved_fd = -1;
	}
	if (stderr_pipe_rd >= 0) {
		ssize_t n = 0;
		ssize_t total = 0;

		while (total < (ssize_t)sizeof stderr_cap - 1) {
			n = read(stderr_pipe_rd, stderr_cap + total,
			    sizeof stderr_cap - 1 - (size_t)total);
			if (n <= 0)
				break;
			total += n;
		}
		stderr_cap[total] = '\0';
		::close(stderr_pipe_rd);
		stderr_pipe_rd = -1;
	} else {
		stderr_cap[0] = '\0';
	}
}

/* ------------------------------------------------------------------ */
/* __dup3 helpers                                                     */
/* ------------------------------------------------------------------ */

struct Dup3Obs {
	int	ret;
	int	err;
	int	fcntl_n;
	FcntlCall log[FCNTL_LOG_MAX];
};

static Dup3Obs
run_ref_dup3(int oldfd, int newfd, int flags)
{
	Dup3Obs obs{};

	errno = 0;
	obs.ret = ref___dup3(oldfd, newfd, flags);
	obs.err = errno;
	obs.fcntl_n = fcntl_log_n;
	std::memcpy(obs.log, fcntl_log,
	    sizeof(FcntlCall) * (size_t)fcntl_log_n);
	return (obs);
}

static Dup3Obs
run_port_dup3(int oldfd, int newfd, int flags)
{
	Dup3Obs obs{};

	errno = 0;
	obs.ret = P::__dup3(oldfd, newfd, flags);
	obs.err = errno;
	obs.fcntl_n = fcntl_log_n;
	std::memcpy(obs.log, fcntl_log,
	    sizeof(FcntlCall) * (size_t)fcntl_log_n);
	return (obs);
}

static int
dup3_obs_equal(const Dup3Obs &a, const Dup3Obs &b)
{
	if (a.ret != b.ret || a.err != b.err || a.fcntl_n != b.fcntl_n)
		return (0);
	for (int i = 0; i < a.fcntl_n; i++) {
		if (a.log[i].fd != b.log[i].fd ||
		    a.log[i].cmd != b.log[i].cmd ||
		    a.log[i].arg != b.log[i].arg)
			return (0);
	}
	return (1);
}

static void
check_dup3(int f, const char *ctx, int oldfd, int newfd, int flags,
    int fail, int ret, int err)
{
	char detail[256];
	Dup3Obs ref_obs;
	Dup3Obs port_obs;

	ncases[f]++;
	fcntl_mock_reset();
	fcntl_mock_configure(fail, ret, err);
	ref_obs = run_ref_dup3(oldfd, newfd, flags);

	fcntl_mock_reset();
	fcntl_mock_configure(fail, ret, err);
	port_obs = run_port_dup3(oldfd, newfd, flags);

	if (!dup3_obs_equal(ref_obs, port_obs)) {
		std::snprintf(detail, sizeof detail,
		    "old=%d new=%d flags=0x%x ref={ret=%d err=%d n=%d} "
		    "port={ret=%d err=%d n=%d}",
		    oldfd, newfd, flags, ref_obs.ret, ref_obs.err,
		    ref_obs.fcntl_n, port_obs.ret, port_obs.err,
		    port_obs.fcntl_n);
		report(f, ctx, detail);
	}
}

static void
fill_hibyte_str(char *buf, std::size_t bufsz, std::uint32_t pat, int len)
{
	std::size_t i;

	if (len < 0)
		len = 0;
	if ((std::size_t)len >= bufsz)
		len = (int)bufsz - 1;
	for (i = 0; i < (std::size_t)len; i++)
		buf[i] = (char)(0x80u | ((pat + (std::uint32_t)i) & 0x7fu));
	buf[len] = '\0';
}

/* ------------------------------------------------------------------ */
/* __assert helpers                                                   */
/* ------------------------------------------------------------------ */

struct AssertObs {
	int	abort_called;
	char	msg[STDERR_CAP];
};

static AssertObs
run_ref_assert(const char *func, const char *file, int line,
    const char *failedexpr)
{
	AssertObs obs{};

	g_abort_called = 0;
	stderr_capture_begin();
	ref___assert(func, file, line, failedexpr);
	stderr_capture_end();
	obs.abort_called = g_abort_called;
	std::strncpy(obs.msg, stderr_cap, sizeof obs.msg - 1);
	obs.msg[sizeof obs.msg - 1] = '\0';
	return (obs);
}

static AssertObs
run_port_assert(const char *func, const char *file, int line,
    const char *failedexpr)
{
	AssertObs obs{};

	g_abort_called = 0;
	stderr_capture_begin();
	P::__assert(func, file, line, failedexpr);
	stderr_capture_end();
	obs.abort_called = g_abort_called;
	std::strncpy(obs.msg, stderr_cap, sizeof obs.msg - 1);
	obs.msg[sizeof obs.msg - 1] = '\0';
	return (obs);
}

static int
assert_obs_equal(const AssertObs &a, const AssertObs &b)
{
	return (a.abort_called == b.abort_called &&
	    std::strcmp(a.msg, b.msg) == 0);
}

static void
check_assert(int f, const char *ctx, const char *func, const char *file,
    int line, const char *failedexpr)
{
	char detail[256];
	AssertObs ref_obs;
	AssertObs port_obs;

	ncases[f]++;
	ref_obs = run_ref_assert(func, file, line, failedexpr);
	port_obs = run_port_assert(func, file, line, failedexpr);
	if (!assert_obs_equal(ref_obs, port_obs)) {
		std::snprintf(detail, sizeof detail,
		    "ref_abort=%d port_abort=%d msgs differ",
		    ref_obs.abort_called, port_obs.abort_called);
		report(f, ctx, detail);
	}
}

static void
test_dup3_hand(void)
{
	const int f = F_DUP3;
	static const int badflag = 0x00010000;

	check_dup3(f, "same_fd_0", 0, 0, 0, 0, 7, 0);
	check_dup3(f, "same_fd_3", 3, 3, O_CLOEXEC, 0, 7, 0);
	check_dup3(f, "same_fd_neg", -1, -1, O_CLOFORK, 0, 7, 0);
	check_dup3(f, "same_fd_max", INT_MAX, INT_MAX, 0, 0, 7, 0);

	check_dup3(f, "bad_flag_bit", 3, 4, badflag, 0, 7, 0);
	check_dup3(f, "bad_flag_mixed", 5, 6, O_CLOEXEC | badflag, 0, 7, 0);
	check_dup3(f, "bad_flag_clofork", 1, 2, O_CLOFORK | badflag, 0, 7, 0);
	check_dup3(f, "bad_flag_all", 9, 10,
	    O_CLOEXEC | O_CLOFORK | badflag, 0, 7, 0);

	check_dup3(f, "valid_zero", 4, 8, 0, 0, 11, 0);
	check_dup3(f, "valid_cloexec", 4, 8, O_CLOEXEC, 0, 12, 0);
	check_dup3(f, "valid_clofork", 4, 8, O_CLOFORK, 0, 13, 0);
	check_dup3(f, "valid_both", 4, 8, O_CLOEXEC | O_CLOFORK, 0, 14, 0);

	check_dup3(f, "fcntl_fail", 2, 5, O_CLOEXEC, 1, -1, EBADF);
	check_dup3(f, "fcntl_fail_clofork", 2, 5, O_CLOFORK, 1, -1, EINVAL);
	check_dup3(f, "fcntl_fail_both", 2, 5, O_CLOEXEC | O_CLOFORK, 1, -1,
	    EINTR);

	check_dup3(f, "old_neg", -2, 3, 0, 0, 20, 0);
	check_dup3(f, "new_neg", 3, -2, O_CLOEXEC, 0, 21, 0);
	check_dup3(f, "both_neg", -3, -4, O_CLOFORK, 0, 22, 0);
}

static void
test_dup3_random(void)
{
	const int f = F_DUP3;
	char ctx[64];

	for (int i = 0; i < 200000; i++) {
		int oldfd = randint();
		int newfd = randint();
		int flags = (int)randu32();
		int fail = (int)(randu32() & 1u);
		int ret = randint();
		int err = (int)(randu32() % 64u);

		if (err == 0)
			err = EINVAL;
		std::snprintf(ctx, sizeof ctx, "rand_%d", i);
		check_dup3(f, ctx, oldfd, newfd, flags, fail, ret, err);
	}
}

static void
test_assert_hand(void)
{
	const int f = F_ASSERT;
	char expr[128];
	char file[128];
	char func[128];

	check_assert(f, "null_func", NULL, "file.c", 1, "x == 0");
	check_assert(f, "with_func", "foo", "bar.c", 42, "ptr != NULL");

	check_assert(f, "empty_expr", "fn", "f.c", 0, "");
	check_assert(f, "empty_file", "fn", "", 0, "e");
	check_assert(f, "empty_func", "", "f.c", 0, "e");

	check_assert(f, "line_zero", "fn", "f.c", 0, "bad");
	check_assert(f, "line_neg", "fn", "f.c", -1, "bad");
	check_assert(f, "line_max", "fn", "f.c", INT_MAX, "bad");
	check_assert(f, "line_min", "fn", "f.c", INT_MIN, "bad");

	fill_hibyte_str(expr, sizeof expr, 0xa5u, 1);
	check_assert(f, "single_hibyte_expr", "fn", "f.c", 9, expr);

	fill_hibyte_str(expr, sizeof expr, 0xc3u, 32);
	fill_hibyte_str(file, sizeof file, 0x5bu, 24);
	fill_hibyte_str(func, sizeof func, 0x91u, 16);
	check_assert(f, "hibyte_all", func, file, 77, expr);

	fill_hibyte_str(expr, sizeof expr, 0xffu, 80);
	check_assert(f, "long_hibyte_expr", NULL, "z.c", 3, expr);

	check_assert(f, "nul_heavy_expr", "fn", "f.c", 2,
	    "a\0b\0c");
	check_assert(f, "nul_heavy_file", "fn", "a\0b.c", 2, "e");
}

static void
test_assert_random(void)
{
	const int f = F_ASSERT;
	char ctx[64];
	char expr[96];
	char file[96];
	char funcbuf[64];
	const char *func;
	static const char *const files[] = {
		"", "x.c", "path/deep/file.c", "a\0hidden"
	};
	static const char *const exprs[] = {
		"", "0", "len > 0", "buf[i] == 0x7f", "p != NULL", "x\0y"
	};

	for (int i = 0; i < 200000; i++) {
		int line = randint();
		int use_null_func = (int)(randu32() & 1u);
		int fl = (int)(randu32() % 4u);
		int el = (int)(randu32() % 6u);
		int hi = (int)(randu32() & 1u);

		if (use_null_func)
			func = NULL;
		else if (hi) {
			fill_hibyte_str(funcbuf, sizeof funcbuf, randu32(),
			    (int)(randu32() % 20u) + 1);
			func = funcbuf;
		} else {
			func = "random_fn";
		}

		if (hi)
			fill_hibyte_str(file, sizeof file, randu32(),
			    (int)(randu32() % 40u) + 1);
		else
			std::strncpy(file, files[fl], sizeof file - 1);
		file[sizeof file - 1] = '\0';

		if (hi)
			fill_hibyte_str(expr, sizeof expr, randu32(),
			    (int)(randu32() % 60u) + 1);
		else
			std::strncpy(expr, exprs[el], sizeof expr - 1);
		expr[sizeof expr - 1] = '\0';

		std::snprintf(ctx, sizeof ctx, "rand_%d", i);
		check_assert(f, ctx, func, file, line, expr);
	}
}

int
main(void)
{
	test_dup3_hand();
	test_dup3_random();
	test_assert_hand();
	test_assert_random();

	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NFUNC; i++)
		std::printf("%-12s %12llu %12llu\n", fname[i],
		    ncases[i], nfails[i]);

	for (int i = 0; i < NFUNC; i++) {
		if (nfails[i] != 0)
			return (1);
	}
	return (0);
}
