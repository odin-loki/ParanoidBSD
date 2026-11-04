/*
 * harness.cpp -- differential test for PBSD batch b0245.
 *
 * vadvise, pdfork, __sys_pipe and nanosleep are libc syscall wrappers.  The
 * harness installs instrumented mocks for the interposition table entries and
 * for __sys_pipe2, then compares mock-visible dispatch tags, every forwarded
 * argument (scalars and pointer offsets from each buffer base), content hashes
 * of every buffer the callee reads, the entire guard-filled buffers (including
 * padding), and the return value.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_accept,
	INTERPOS_accept4,
	INTERPOS_aio_suspend,
	INTERPOS_close,
	INTERPOS_connect,
	INTERPOS_fcntl,
	INTERPOS_fsync,
	INTERPOS_fork,
	INTERPOS_msync,
	INTERPOS_nanosleep,
	INTERPOS_openat,
	INTERPOS_poll,
	INTERPOS_pselect,
	INTERPOS_recvfrom,
	INTERPOS_recvmsg,
	INTERPOS_select,
	INTERPOS_sendmsg,
	INTERPOS_sendto,
	INTERPOS_setcontext,
	INTERPOS_sigaction,
	INTERPOS_sigprocmask,
	INTERPOS_sigsuspend,
	INTERPOS_sigwait,
	INTERPOS_sigtimedwait,
	INTERPOS_sigwaitinfo,
	INTERPOS_swapcontext,
	INTERPOS_system,
	INTERPOS_tcdrain,
	INTERPOS_read,
	INTERPOS_readv,
	INTERPOS_wait4,
	INTERPOS_write,
	INTERPOS_writev,
	INTERPOS__pthread_mutex_init_calloc_cb,
	INTERPOS_spinlock,
	INTERPOS_spinunlock,
	INTERPOS_kevent,
	INTERPOS_wait6,
	INTERPOS_ppoll,
	INTERPOS_map_stacks_exec,
	INTERPOS_fdatasync,
	INTERPOS_clock_nanosleep,
	INTERPOS__reserved0,
	INTERPOS_pdfork,
	INTERPOS_uexterr_gettext,
	INTERPOS_pdwait,
	INTERPOS_MAX
};

extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];

int ref_vadvise(int arg);
pid_t ref_pdfork(int *fdp, int flags);
int ref___sys_pipe(int fildes[2]);
int ref_nanosleep(const struct timespec *rqtp, struct timespec *rmtp);

}

import pbsd.lib.libc.sys.b0245;

namespace port = pbsd::lib_libc_sys::b0245;

#define	GUARD		0x7f

#define	FD_PAD		64
#define	FD_TOTAL	(2 * sizeof(int) + 2 * FD_PAD)

#define	TS_PAD		64
#define	TS_TOTAL	(sizeof(struct timespec) + 2 * TS_PAD)

#define	TAG_NONE		0
#define	TAG_PDFORK		1
#define	TAG_PIPE		2
#define	TAG_NANOSLEEP		3

#define	NSCALAR		4
#define	NPTR		4
#define	NHASH		2

#define	OFF_NULL	(-4242424242LL)

struct Snap {
	unsigned long long	ncalls;
	int			tag;
	long long		sc[NSCALAR];
	long long		po[NPTR];
	unsigned		hs[NHASH];
};

static Snap mock;
static long long mock_ret;

static const unsigned char *base_fdp;
static const unsigned char *base_fildes;
static const unsigned char *base_rqtp;
static const unsigned char *base_rmtp;

static void
mock_reset(long long ret)
{
	memset(&mock, 0, sizeof(mock));
	mock_ret = ret;
}

static void
mock_enter(int tag)
{
	mock.ncalls++;
	mock.tag = tag;
}

static long long
poff(const void *p, const unsigned char *base)
{
	if (p == nullptr)
		return (OFF_NULL);
	return ((long long)((const unsigned char *)p - base));
}

static unsigned
hash_bytes(const void *p, size_t n)
{
	const unsigned char *b = (const unsigned char *)p;
	unsigned h = 2166136261u;

	for (size_t i = 0; i < n; i++) {
		h ^= b[i];
		h *= 16777619u;
	}
	return (h);
}

static pid_t
mock_pdfork(int *fdp, int flags)
{
	mock_enter(TAG_PDFORK);
	mock.sc[0] = (long long)flags;
	mock.po[0] = poff(fdp, base_fdp);

	if (fdp != nullptr) {
		mock.hs[0] = hash_bytes(fdp, sizeof(int));
		*fdp = flags ^ 0x55;
	}
	return ((pid_t)mock_ret);
}

static int
mock_nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
	mock_enter(TAG_NANOSLEEP);
	mock.po[0] = poff(rqtp, base_rqtp);
	mock.po[1] = poff(rmtp, base_rmtp);

	if (rqtp != nullptr)
		mock.hs[0] = hash_bytes(rqtp, sizeof(struct timespec));
	if (rmtp != nullptr) {
		rmtp->tv_sec = rqtp != nullptr ? rqtp->tv_sec ^ 1 : 0;
		rmtp->tv_nsec = rqtp != nullptr ? rqtp->tv_nsec ^ 2 : 0;
		mock.hs[1] = hash_bytes(rmtp, sizeof(struct timespec));
	}
	return ((int)mock_ret);
}

extern "C" int
__sys_pipe2(int fildes[2], int flags)
{
	mock_enter(TAG_PIPE);
	mock.sc[0] = (long long)flags;
	mock.po[0] = poff(fildes, base_fildes);

	if (fildes != nullptr) {
		fildes[0] = flags + 1;
		fildes[1] = flags + 2;
	}
	return ((int)mock_ret);
}

static void
install_mocks(interpos_func_t *table)
{
	table[INTERPOS_pdfork] = (interpos_func_t)mock_pdfork;
	table[INTERPOS_nanosleep] = (interpos_func_t)mock_nanosleep;
}

enum {
	FN_VADVISE,
	FN_PDFORK,
	FN_SYS_PIPE,
	FN_NANOSLEEP,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"vadvise",
	"pdfork",
	"__sys_pipe",
	"nanosleep",
};

static unsigned long long fn_cases[FN_COUNT];
static unsigned long long fn_fails[FN_COUNT];
static int fn_reported[FN_COUNT];

#define	MAX_REPORTS	8

static void
fail(int fn, const char *what, const char *detail)
{
	fn_fails[fn]++;
	if (fn_reported[fn] < MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-17s %-8s %s\n", fn_name[fn], what, detail);
	} else if (fn_reported[fn] == MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-17s ... further failures suppressed\n",
		    fn_name[fn]);
	}
}

static void
snap_str(char *out, size_t n, const Snap &s)
{
	snprintf(out, n,
	    "{nc=%llu tag=%d sc=[%lld,%lld,%lld,%lld] po=[%lld,%lld,%lld,%lld] "
	    "hs=[%u,%u]}",
	    s.ncalls, s.tag, s.sc[0], s.sc[1], s.sc[2], s.sc[3], s.po[0],
	    s.po[1], s.po[2], s.po[3], s.hs[0], s.hs[1]);
}

static void
cmp_snap(int fn, const Snap &a, const Snap &b, const char *ctx)
{
	bool same = a.ncalls == b.ncalls && a.tag == b.tag;

	for (int i = 0; i < NSCALAR; i++)
		same = same && a.sc[i] == b.sc[i];
	for (int i = 0; i < NPTR; i++)
		same = same && a.po[i] == b.po[i];
	for (int i = 0; i < NHASH; i++)
		same = same && a.hs[i] == b.hs[i];
	if (same)
		return;

	char sa[384], sb[384], msg[1024];

	snap_str(sa, sizeof(sa), a);
	snap_str(sb, sizeof(sb), b);
	snprintf(msg, sizeof(msg), "%s ref=%s port=%s", ctx, sa, sb);
	fail(fn, "callee", msg);
}

static void
cmp_buf(int fn, const char *tag, const unsigned char *a,
    const unsigned char *b, size_t n, const char *ctx)
{
	for (size_t i = 0; i < n; i++) {
		if (a[i] != b[i]) {
			char msg[1024];

			snprintf(msg, sizeof(msg),
			    "%s %s byte[%zu] ref=0x%02x port=0x%02x", ctx,
			    tag, i, a[i], b[i]);
			fail(fn, "buffer", msg);
			return;
		}
	}
}

static void
cmp_ret(int fn, long long ra, long long rb, const char *ctx)
{
	if (ra != rb) {
		char msg[1024];

		snprintf(msg, sizeof(msg), "%s ref=%lld port=%lld", ctx, ra,
		    rb);
		fail(fn, "return", msg);
	}
}

static uint64_t rng_state;

static void
rng_seed(uint64_t seed)
{
	rng_state = seed;
}

static uint64_t
rng_u64(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return (rng_state);
}

static uint32_t
rng_u32(void)
{
	return ((uint32_t)(rng_u64() >> 11));
}

static int
rnd_int(void)
{
	return ((int)(int32_t)rng_u32());
}

static long long
rnd_ret(void)
{
	return ((long long)(int32_t)rng_u32());
}

static void
case_vadvise(int arg)
{
	int ra, rb;
	char ctx[128];

	fn_cases[FN_VADVISE]++;

	snprintf(ctx, sizeof(ctx), "arg=%d", arg);

	ra = ref_vadvise(arg);
	rb = port::vadvise(arg);
	cmp_ret(FN_VADVISE, ra, rb, ctx);
}

static void
case_pdfork(int *fdp, int flags, long long ret)
{
	unsigned char fdp_a[FD_TOTAL], fdp_b[FD_TOTAL];
	Snap snap_a, snap_b;
	pid_t ra, rb;
	char ctx[192];

	fn_cases[FN_PDFORK]++;

	memset(fdp_a, GUARD, sizeof(fdp_a));
	memset(fdp_b, GUARD, sizeof(fdp_b));

	if (fdp != nullptr) {
		memcpy(fdp_a + FD_PAD, fdp, sizeof(int));
		memcpy(fdp_b + FD_PAD, fdp, sizeof(int));
	}

	snprintf(ctx, sizeof(ctx), "fdp=%d flags=%d ret=%lld", fdp != nullptr,
	    flags, ret);

	base_fdp = fdp_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_pdfork(fdp != nullptr ? (int *)(fdp_a + FD_PAD) : nullptr,
	    flags);
	snap_a = mock;

	base_fdp = fdp_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::pdfork(fdp != nullptr ? (int *)(fdp_b + FD_PAD) : nullptr,
	    flags);
	snap_b = mock;

	cmp_snap(FN_PDFORK, snap_a, snap_b, ctx);
	cmp_buf(FN_PDFORK, "fdp", fdp_a, fdp_b, sizeof(fdp_a), ctx);
	cmp_ret(FN_PDFORK, (long long)ra, (long long)rb, ctx);
}

static void
case_sys_pipe(int *fildes, long long ret)
{
	unsigned char fds_a[FD_TOTAL], fds_b[FD_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[128];

	fn_cases[FN_SYS_PIPE]++;

	memset(fds_a, GUARD, sizeof(fds_a));
	memset(fds_b, GUARD, sizeof(fds_b));

	if (fildes != nullptr) {
		memcpy(fds_a + FD_PAD, fildes, 2 * sizeof(int));
		memcpy(fds_b + FD_PAD, fildes, 2 * sizeof(int));
	}

	snprintf(ctx, sizeof(ctx), "fildes=%d ret=%lld", fildes != nullptr,
	    ret);

	base_fildes = fds_a;
	mock_reset(ret);
	ra = ref___sys_pipe(fildes != nullptr ?
	    (int *)(fds_a + FD_PAD) : nullptr);
	snap_a = mock;

	base_fildes = fds_b;
	mock_reset(ret);
	rb = port::__sys_pipe(fildes != nullptr ?
	    (int *)(fds_b + FD_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_SYS_PIPE, snap_a, snap_b, ctx);
	cmp_buf(FN_SYS_PIPE, "fildes", fds_a, fds_b, sizeof(fds_a), ctx);
	cmp_ret(FN_SYS_PIPE, ra, rb, ctx);
}

static void
case_nanosleep(const struct timespec *rqtp, bool use_rmtp, long long ret)
{
	unsigned char rq_a[TS_TOTAL], rq_b[TS_TOTAL];
	unsigned char rm_a[TS_TOTAL], rm_b[TS_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[192];

	fn_cases[FN_NANOSLEEP]++;

	memset(rq_a, GUARD, sizeof(rq_a));
	memset(rq_b, GUARD, sizeof(rq_b));
	memset(rm_a, GUARD, sizeof(rm_a));
	memset(rm_b, GUARD, sizeof(rm_b));

	if (rqtp != nullptr) {
		memcpy(rq_a + TS_PAD, rqtp, sizeof(struct timespec));
		memcpy(rq_b + TS_PAD, rqtp, sizeof(struct timespec));
	}

	snprintf(ctx, sizeof(ctx), "rqtp=%d rmtp=%d ret=%lld",
	    rqtp != nullptr, (int)use_rmtp, ret);

	base_rqtp = rq_a;
	base_rmtp = rm_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_nanosleep(rqtp != nullptr ?
	    (const struct timespec *)(rq_a + TS_PAD) : nullptr,
	    use_rmtp ? (struct timespec *)(rm_a + TS_PAD) : nullptr);
	snap_a = mock;

	base_rqtp = rq_b;
	base_rmtp = rm_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::nanosleep(rqtp != nullptr ?
	    (const struct timespec *)(rq_b + TS_PAD) : nullptr,
	    use_rmtp ? (struct timespec *)(rm_b + TS_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_NANOSLEEP, snap_a, snap_b, ctx);
	cmp_buf(FN_NANOSLEEP, "rqtp", rq_a, rq_b, sizeof(rq_a), ctx);
	cmp_buf(FN_NANOSLEEP, "rmtp", rm_a, rm_b, sizeof(rm_a), ctx);
	cmp_ret(FN_NANOSLEEP, ra, rb, ctx);
}

static void
test_vadvise(void)
{
	static const int args[] = {
		0, 1, -1, 0x7e, 0x7f, 0x80, 0xff, INT_MIN, INT_MAX,
	};

	for (size_t i = 0; i < sizeof(args) / sizeof(args[0]); i++)
		case_vadvise(args[i]);

	for (unsigned b = 0x00; b <= 0xff; b++)
		case_vadvise((int)(int8_t)b);

	rng_seed(0x76616476697365ULL);
	for (int n = 0; n < 200000; n++)
		case_vadvise(rnd_int());
}

static void
test_pdfork(void)
{
	int fd = 42;
	static const int flags_list[] = {
		0, 1, -1, 0x7e, 0x7f, 0x80, INT_MIN, INT_MAX,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (size_t f = 0; f < sizeof(flags_list) / sizeof(flags_list[0]); f++)
		for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
			for (int m = 0; m < 2; m++)
				case_pdfork((m & 1) != 0 ? &fd : nullptr,
				    flags_list[f], rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		fd = (int)(int8_t)b;
		case_pdfork(&fd, (int)(int8_t)b, (long long)(int8_t)b);
	}

	{
		int zero = 0;

		case_pdfork(&zero, 0, 0);
		case_pdfork(nullptr, 0, -1);
	}

	rng_seed(0x7064666f726bULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 1u);

		fd = rnd_int();
		case_pdfork((m != 0) ? &fd : nullptr, rnd_int(), rnd_ret());
	}
}

static void
test_sys_pipe(void)
{
	int fds[2] = { 3, 4 };
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
		for (int m = 0; m < 2; m++)
			case_sys_pipe((m & 1) != 0 ? fds : nullptr, rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		fds[0] = (int)(int8_t)b;
		fds[1] = (int)(int8_t)~b;
		case_sys_pipe(fds, (long long)(int8_t)b);
	}

	{
		int zero[2] = { 0, 0 };

		case_sys_pipe(zero, 0);
		case_sys_pipe(nullptr, -1);
	}

	rng_seed(0x5f5f737973706970ULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 1u);

		fds[0] = rnd_int();
		fds[1] = rnd_int();
		case_sys_pipe((m != 0) ? fds : nullptr, rnd_ret());
	}
}

static void
test_nanosleep(void)
{
	struct timespec rq;
	static const long long secs[] = {
		0, 1, -1, 0x7e, 0x7f, 0x80, INT64_MAX, INT64_MIN,
	};
	static const long long nsecs[] = {
		0, 1, 999999999, 1000000000, -1, 0x7e, 0x7f, 0x80,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (size_t s = 0; s < sizeof(secs) / sizeof(secs[0]); s++)
		for (size_t ns = 0; ns < sizeof(nsecs) / sizeof(nsecs[0]); ns++)
			for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
				for (int m = 0; m < 4; m++) {
					rq.tv_sec = (time_t)secs[s];
					rq.tv_nsec = nsecs[ns];
					case_nanosleep((m & 1) != 0 ? &rq :
					    nullptr, (m & 2) != 0, rets[r]);
				}

	for (unsigned b = 0x00; b <= 0xff; b++) {
		rq.tv_sec = (time_t)(int8_t)b;
		rq.tv_nsec = (long)(int8_t)~b;
		case_nanosleep(&rq, true, (long long)(int8_t)b);
	}

	{
		struct timespec zero = { 0, 0 };

		case_nanosleep(&zero, true, 0);
		case_nanosleep(nullptr, false, -1);
	}

	rng_seed(0x6e616e6f736c6570ULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 3u);

		rq.tv_sec = (time_t)(int64_t)rng_u64();
		rq.tv_nsec = (long)(int64_t)rng_u64();
		case_nanosleep((m & 1) != 0 ? &rq : nullptr, (m & 2) != 0,
		    rnd_ret());
	}
}

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_vadvise();
	test_pdfork();
	test_sys_pipe();
	test_nanosleep();

	printf("\n%-17s %12s %12s\n", "function", "cases", "failures");
	printf("---------------------------------------------\n");
	for (int i = 0; i < FN_COUNT; i++) {
		printf("%-17s %12llu %12llu\n", fn_name[i], fn_cases[i],
		    fn_fails[i]);
		total_cases += fn_cases[i];
		total_fails += fn_fails[i];
	}
	printf("---------------------------------------------\n");
	printf("%-17s %12llu %12llu\n", "TOTAL", total_cases, total_fails);

	return (total_fails == 0 ? 0 : 1);
}
