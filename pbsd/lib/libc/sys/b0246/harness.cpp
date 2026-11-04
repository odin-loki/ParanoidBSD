/*
 * harness.cpp -- differential test for PBSD batch b0246.
 *
 * sigprocmask, sigaction, sigtimedwait and readv are libc interposition
 * wrappers: each loads a function pointer from __libc_interposing[] and
 * tail-calls it with the original argument list.  The harness installs
 * instrumented mocks in both the oracle (ref_*) and port tables, then
 * compares the mock-visible dispatch tag, every forwarded argument (scalars
 * and pointer offsets from each buffer base), content hashes of every buffer
 * the callee reads, the entire guard-filled buffers (including padding), and
 * the return value.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_sigprocmask,
	INTERPOS_sigaction,
	INTERPOS_sigtimedwait,
	INTERPOS_readv,
	INTERPOS_MAX
};

extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];

int ref_sigprocmask(int how, const sigset_t *set, sigset_t *oset);
int ref_sigaction(int sig, const struct sigaction *act,
    struct sigaction *oact);
int ref_sigtimedwait(const sigset_t * __restrict set,
    siginfo_t * __restrict info, const struct timespec * __restrict t);
ssize_t ref_readv(int fd, const struct iovec *iov, int iovcnt);

}

import pbsd.lib.libc.sys.b0246;

namespace port = pbsd::lib_libc_sys::b0246;

#define	GUARD		0x7f

#define	MASK_PAD	192
#define	MASK_TOTAL	(sizeof(sigset_t) + 2 * MASK_PAD)

#define	ACT_PAD		192
#define	ACT_TOTAL	(sizeof(struct sigaction) + 2 * ACT_PAD)

#define	INFO_PAD	192
#define	INFO_TOTAL	(sizeof(siginfo_t) + 2 * INFO_PAD)

#define	TS_PAD		64
#define	TS_TOTAL	(sizeof(struct timespec) + 2 * TS_PAD)

#define	IOV_TEST_MAX	8
#define	IOV_PAD		8
#define	IOV_ELEMS	(IOV_TEST_MAX + 2 * IOV_PAD)
#define	IOV_TOTAL	(IOV_ELEMS * sizeof(struct iovec))
#define	IOV_OFF		(IOV_PAD * sizeof(struct iovec))

#define	BASE_MAX	64
#define	BASE_PAD	32
#define	BASE_TOTAL	(BASE_MAX + 2 * BASE_PAD)
#define	BASE_OFF	BASE_PAD

#define	TAG_NONE		0
#define	TAG_SIGPROCMASK		1
#define	TAG_SIGACTION		2
#define	TAG_SIGTIMEDWAIT	3
#define	TAG_READV		4

#define	NSCALAR		4
#define	NPTR		10
#define	NHASH		10

#define	OFF_NULL	(-4242424242LL)

struct Snap {
	unsigned long long	ncalls;
	int			tag;
	long long		sc[NSCALAR];
	long long		po[NPTR];
	unsigned		hs[NHASH];
	long long		wr;
};

static Snap mock;
static long long mock_ret;

static const unsigned char *base_set_in;
static const unsigned char *base_set_out;
static const unsigned char *base_act_in;
static const unsigned char *base_act_out;
static const unsigned char *base_mask;
static const unsigned char *base_info;
static const unsigned char *base_ts;
static const unsigned char *base_iov;
static const unsigned char *base_bufs[IOV_TEST_MAX];

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

static void
fill_pattern(unsigned char *p, size_t n, unsigned char seed)
{
	for (size_t i = 0; i < n; i++)
		p[i] = (unsigned char)(seed + (unsigned char)i);
}

static int
mock_sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	mock_enter(TAG_SIGPROCMASK);
	mock.sc[0] = (long long)how;
	mock.po[0] = poff(set, base_set_in);
	mock.po[1] = poff(oset, base_set_out);

	if (set != nullptr)
		mock.hs[0] = hash_bytes(set, sizeof(sigset_t));
	if (oset != nullptr)
		fill_pattern((unsigned char *)oset, sizeof(sigset_t), 0xb5);
	return ((int)mock_ret);
}

static int
mock_sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
	mock_enter(TAG_SIGACTION);
	mock.sc[0] = (long long)sig;
	mock.po[0] = poff(act, base_act_in);
	mock.po[1] = poff(oact, base_act_out);

	if (act != nullptr)
		mock.hs[0] = hash_bytes(act, sizeof(struct sigaction));
	if (oact != nullptr)
		fill_pattern((unsigned char *)oact, sizeof(struct sigaction), 0xc5);
	return ((int)mock_ret);
}

static int
mock_sigtimedwait(const sigset_t * __restrict set,
    siginfo_t * __restrict info, const struct timespec * __restrict t)
{
	mock_enter(TAG_SIGTIMEDWAIT);
	mock.po[0] = poff(set, base_mask);
	mock.po[1] = poff(info, base_info);
	mock.po[2] = poff(t, base_ts);

	if (set != nullptr)
		mock.hs[0] = hash_bytes(set, sizeof(sigset_t));
	if (info != nullptr) {
		fill_pattern((unsigned char *)info, sizeof(siginfo_t), 0xa5);
		mock.wr = (long long)info->si_signo;
	}
	if (t != nullptr)
		mock.hs[1] = hash_bytes(t, sizeof(struct timespec));
	return ((int)mock_ret);
}

static ssize_t
mock_readv(int fd, const struct iovec *iov, int iovcnt)
{
	int n, i;
	unsigned h;

	mock_enter(TAG_READV);
	mock.sc[0] = (long long)fd;
	mock.sc[1] = (long long)iovcnt;
	mock.po[0] = poff(iov, base_iov);

	if (iov != nullptr && iovcnt > 0) {
		n = iovcnt < IOV_TEST_MAX ? iovcnt : IOV_TEST_MAX;
		h = 2166136261u;
		for (i = 0; i < n; i++) {
			h ^= (unsigned)iov[i].iov_len;
			h *= 16777619u;
			mock.po[i + 1] = poff(iov[i].iov_base, base_bufs[i]);
			if (iov[i].iov_base != nullptr && iov[i].iov_len > 0) {
				size_t len = iov[i].iov_len;

				if (len > BASE_MAX)
					len = BASE_MAX;
				mock.hs[i + 1] = hash_bytes(iov[i].iov_base, len);
			}
		}
		mock.hs[0] = h;
	}
	return ((ssize_t)mock_ret);
}

static void
install_mocks(interpos_func_t *table)
{
	table[INTERPOS_sigprocmask] = (interpos_func_t)mock_sigprocmask;
	table[INTERPOS_sigaction] = (interpos_func_t)mock_sigaction;
	table[INTERPOS_sigtimedwait] = (interpos_func_t)mock_sigtimedwait;
	table[INTERPOS_readv] = (interpos_func_t)mock_readv;
}

enum {
	FN_SIGPROCMASK,
	FN_SIGACTION,
	FN_SIGTIMEDWAIT,
	FN_READV,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"sigprocmask",
	"sigaction",
	"sigtimedwait",
	"readv",
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
	    "{nc=%llu tag=%d sc=[%lld,%lld,%lld,%lld] "
	    "po=[%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld] "
	    "hs=[%u,%u,%u,%u,%u,%u,%u,%u,%u,%u] wr=%lld}",
	    s.ncalls, s.tag, s.sc[0], s.sc[1], s.sc[2], s.sc[3], s.po[0],
	    s.po[1], s.po[2], s.po[3], s.po[4], s.po[5], s.po[6], s.po[7],
	    s.po[8], s.po[9], s.hs[0], s.hs[1], s.hs[2], s.hs[3], s.hs[4],
	    s.hs[5], s.hs[6], s.hs[7], s.hs[8], s.hs[9], s.wr);
}

static void
cmp_snap(int fn, const Snap &a, const Snap &b, const char *ctx)
{
	bool same = a.ncalls == b.ncalls && a.tag == b.tag && a.wr == b.wr;

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

static size_t
rnd_size(size_t max)
{
	return ((size_t)(rng_u32() % (uint32_t)(max + 1)));
}

static uint64_t
rnd_wide(void)
{
	switch (rng_u32() % 8u) {
	case 0:
		return (rng_u64());
	case 1:
		return ((uint64_t)UINT32_MAX);
	case 2:
		return ((uint64_t)INT32_MAX);
	case 3:
		return ((uint64_t)INT32_MAX + 1u);
	default:
		return (0);
	}
}

static void
sigset_fill(sigset_t *s, unsigned char seed)
{
	unsigned char *p = (unsigned char *)s;

	for (size_t i = 0; i < sizeof(sigset_t); i++)
		p[i] = (unsigned char)(seed + (unsigned char)i);
	sigemptyset(s);
	for (int sig = 1; sig < 32; sig += (int)(seed + 2)) {
		if ((sig + (int)seed) % 5 == 0)
			sigaddset(s, sig);
	}
}

static void
sigaction_fill(struct sigaction *sa, unsigned char seed)
{
	unsigned char *p = (unsigned char *)sa;

	for (size_t i = 0; i < sizeof(struct sigaction); i++)
		p[i] = (unsigned char)(seed + (unsigned char)i);
	memset(sa, 0, sizeof(*sa));
	sa->sa_handler = (void (*)(int))(uintptr_t)(0x1000 + seed);
	sa->sa_flags = (int)(seed << 8) | (seed ^ 0xff);
	sigemptyset(&sa->sa_mask);
	if (seed & 1)
		sigaddset(&sa->sa_mask, SIGINT);
}

static void
case_sigprocmask(int how, const sigset_t *set, bool use_oset, long long ret)
{
	unsigned char set_a[MASK_TOTAL], set_b[MASK_TOTAL];
	unsigned char oset_a[MASK_TOTAL], oset_b[MASK_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_SIGPROCMASK]++;

	memset(set_a, GUARD, sizeof(set_a));
	memset(set_b, GUARD, sizeof(set_b));
	memset(oset_a, GUARD, sizeof(oset_a));
	memset(oset_b, GUARD, sizeof(oset_b));

	if (set != nullptr) {
		memcpy(set_a + MASK_PAD, set, sizeof(sigset_t));
		memcpy(set_b + MASK_PAD, set, sizeof(sigset_t));
	}

	snprintf(ctx, sizeof(ctx), "how=%d set=%d oset=%d ret=%lld", how,
	    set != nullptr, (int)use_oset, ret);

	base_set_in = set_a;
	base_set_out = oset_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_sigprocmask(how, set != nullptr ?
	    (const sigset_t *)(set_a + MASK_PAD) : nullptr,
	    use_oset ? (sigset_t *)(oset_a + MASK_PAD) : nullptr);
	snap_a = mock;

	base_set_in = set_b;
	base_set_out = oset_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::sigprocmask(how, set != nullptr ?
	    (const sigset_t *)(set_b + MASK_PAD) : nullptr,
	    use_oset ? (sigset_t *)(oset_b + MASK_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_SIGPROCMASK, snap_a, snap_b, ctx);
	cmp_buf(FN_SIGPROCMASK, "set", set_a, set_b, sizeof(set_a), ctx);
	cmp_buf(FN_SIGPROCMASK, "oset", oset_a, oset_b, sizeof(oset_a), ctx);
	cmp_ret(FN_SIGPROCMASK, ra, rb, ctx);
}

static void
case_sigaction(int sig, const struct sigaction *act, bool use_oact,
    long long ret)
{
	unsigned char act_a[ACT_TOTAL], act_b[ACT_TOTAL];
	unsigned char oact_a[ACT_TOTAL], oact_b[ACT_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_SIGACTION]++;

	memset(act_a, GUARD, sizeof(act_a));
	memset(act_b, GUARD, sizeof(act_b));
	memset(oact_a, GUARD, sizeof(oact_a));
	memset(oact_b, GUARD, sizeof(oact_b));

	if (act != nullptr) {
		memcpy(act_a + ACT_PAD, act, sizeof(struct sigaction));
		memcpy(act_b + ACT_PAD, act, sizeof(struct sigaction));
	}

	snprintf(ctx, sizeof(ctx), "sig=%d act=%d oact=%d ret=%lld", sig,
	    act != nullptr, (int)use_oact, ret);

	base_act_in = act_a;
	base_act_out = oact_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_sigaction(sig, act != nullptr ?
	    (const struct sigaction *)(act_a + ACT_PAD) : nullptr,
	    use_oact ? (struct sigaction *)(oact_a + ACT_PAD) : nullptr);
	snap_a = mock;

	base_act_in = act_b;
	base_act_out = oact_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::sigaction(sig, act != nullptr ?
	    (const struct sigaction *)(act_b + ACT_PAD) : nullptr,
	    use_oact ? (struct sigaction *)(oact_b + ACT_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_SIGACTION, snap_a, snap_b, ctx);
	cmp_buf(FN_SIGACTION, "act", act_a, act_b, sizeof(act_a), ctx);
	cmp_buf(FN_SIGACTION, "oact", oact_a, oact_b, sizeof(oact_a), ctx);
	cmp_ret(FN_SIGACTION, ra, rb, ctx);
}

static void
case_sigtimedwait(const sigset_t *set, bool use_info, const struct timespec *ts,
    long long ret)
{
	unsigned char mask_a[MASK_TOTAL], mask_b[MASK_TOTAL];
	unsigned char info_a[INFO_TOTAL], info_b[INFO_TOTAL];
	unsigned char ts_a[TS_TOTAL], ts_b[TS_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_SIGTIMEDWAIT]++;

	memset(mask_a, GUARD, sizeof(mask_a));
	memset(mask_b, GUARD, sizeof(mask_b));
	memset(info_a, GUARD, sizeof(info_a));
	memset(info_b, GUARD, sizeof(info_b));
	memset(ts_a, GUARD, sizeof(ts_a));
	memset(ts_b, GUARD, sizeof(ts_b));

	if (set != nullptr) {
		memcpy(mask_a + MASK_PAD, set, sizeof(sigset_t));
		memcpy(mask_b + MASK_PAD, set, sizeof(sigset_t));
	}
	if (ts != nullptr) {
		memcpy(ts_a + TS_PAD, ts, sizeof(struct timespec));
		memcpy(ts_b + TS_PAD, ts, sizeof(struct timespec));
	}

	snprintf(ctx, sizeof(ctx), "set=%d info=%d ts=%d ret=%lld",
	    set != nullptr, (int)use_info, ts != nullptr, ret);

	base_mask = mask_a;
	base_info = info_a;
	base_ts = ts_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_sigtimedwait(set != nullptr ?
	    (const sigset_t *)(mask_a + MASK_PAD) : nullptr,
	    use_info ? (siginfo_t *)(info_a + INFO_PAD) : nullptr,
	    ts != nullptr ? (const struct timespec *)(ts_a + TS_PAD) : nullptr);
	snap_a = mock;

	base_mask = mask_b;
	base_info = info_b;
	base_ts = ts_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::sigtimedwait(set != nullptr ?
	    (const sigset_t *)(mask_b + MASK_PAD) : nullptr,
	    use_info ? (siginfo_t *)(info_b + INFO_PAD) : nullptr,
	    ts != nullptr ? (const struct timespec *)(ts_b + TS_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_SIGTIMEDWAIT, snap_a, snap_b, ctx);
	cmp_buf(FN_SIGTIMEDWAIT, "mask", mask_a, mask_b, sizeof(mask_a), ctx);
	cmp_buf(FN_SIGTIMEDWAIT, "info", info_a, info_b, sizeof(info_a), ctx);
	cmp_buf(FN_SIGTIMEDWAIT, "ts", ts_a, ts_b, sizeof(ts_a), ctx);
	cmp_ret(FN_SIGTIMEDWAIT, ra, rb, ctx);
}

static void
cmp_iov_bufs(int fn, const unsigned char *iov_a, const unsigned char *iov_b,
    const char *ctx)
{
	size_t i;

	cmp_buf(fn, "iov_pre", iov_a, iov_b, IOV_OFF, ctx);
	cmp_buf(fn, "iov_post", iov_a + IOV_OFF + IOV_TEST_MAX * sizeof(struct iovec),
	    iov_b + IOV_OFF + IOV_TEST_MAX * sizeof(struct iovec),
	    IOV_TOTAL - IOV_OFF - IOV_TEST_MAX * sizeof(struct iovec), ctx);

	for (i = 0; i < IOV_TEST_MAX; i++) {
		const struct iovec *va = (const struct iovec *)(iov_a + IOV_OFF);
		const struct iovec *vb = (const struct iovec *)(iov_b + IOV_OFF);

		if (va[i].iov_len != vb[i].iov_len) {
			char msg[1024];

			snprintf(msg, sizeof(msg),
			    "%s iov[%zu].iov_len ref=%zu port=%zu", ctx, i,
			    va[i].iov_len, vb[i].iov_len);
			fail(fn, "iov_len", msg);
		}
	}
}

static void
case_readv(int fd, const struct iovec *iov, int iovcnt, long long ret)
{
	unsigned char iov_a[IOV_TOTAL], iov_b[IOV_TOTAL];
	unsigned char bufs_a[IOV_TEST_MAX][BASE_TOTAL];
	unsigned char bufs_b[IOV_TEST_MAX][BASE_TOTAL];
	Snap snap_a, snap_b;
	ssize_t ra, rb;
	char ctx[256];
	int i;

	fn_cases[FN_READV]++;

	memset(iov_a, GUARD, sizeof(iov_a));
	memset(iov_b, GUARD, sizeof(iov_b));
	for (i = 0; i < IOV_TEST_MAX; i++) {
		memset(bufs_a[i], GUARD, sizeof(bufs_a[i]));
		memset(bufs_b[i], GUARD, sizeof(bufs_b[i]));
	}

	if (iov != nullptr && iovcnt > 0) {
		int n = iovcnt < IOV_TEST_MAX ? iovcnt : IOV_TEST_MAX;

		memcpy(iov_a + IOV_OFF, iov, (size_t)n * sizeof(struct iovec));
		memcpy(iov_b + IOV_OFF, iov, (size_t)n * sizeof(struct iovec));
		for (i = 0; i < n; i++) {
			if (iov[i].iov_base != nullptr) {
				size_t len = iov[i].iov_len;

				if (len > BASE_MAX)
					len = BASE_MAX;
				if (len > 0) {
					memcpy(bufs_a[i] + BASE_OFF, iov[i].iov_base,
					    len);
					memcpy(bufs_b[i] + BASE_OFF, iov[i].iov_base,
					    len);
				}
				((struct iovec *)(iov_a + IOV_OFF))[i].iov_base =
				    bufs_a[i] + BASE_OFF;
				((struct iovec *)(iov_b + IOV_OFF))[i].iov_base =
				    bufs_b[i] + BASE_OFF;
			}
		}
	}

	snprintf(ctx, sizeof(ctx), "fd=%d iov=%d iovcnt=%d ret=%lld", fd,
	    iov != nullptr, iovcnt, ret);

	base_iov = iov_a;
	for (i = 0; i < IOV_TEST_MAX; i++)
		base_bufs[i] = bufs_a[i];
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_readv(fd, iov != nullptr ?
	    (const struct iovec *)(iov_a + IOV_OFF) : nullptr, iovcnt);
	snap_a = mock;

	base_iov = iov_b;
	for (i = 0; i < IOV_TEST_MAX; i++)
		base_bufs[i] = bufs_b[i];
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::readv(fd, iov != nullptr ?
	    (const struct iovec *)(iov_b + IOV_OFF) : nullptr, iovcnt);
	snap_b = mock;

	cmp_snap(FN_READV, snap_a, snap_b, ctx);
	cmp_iov_bufs(FN_READV, iov_a, iov_b, ctx);
	for (i = 0; i < IOV_TEST_MAX; i++)
		cmp_buf(FN_READV, "base", bufs_a[i], bufs_b[i],
		    sizeof(bufs_a[i]), ctx);
	cmp_ret(FN_READV, (long long)ra, (long long)rb, ctx);
}

static void
test_sigprocmask(void)
{
	sigset_t mask;
	static const int hows[] = {
		INT_MIN, -1, 0, 1, 2, SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK,
		0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	for (size_t h = 0; h < sizeof(hows) / sizeof(hows[0]); h++)
		for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
			for (int m = 0; m < 4; m++)
				case_sigprocmask(hows[h],
				    (m & 1) != 0 ? &mask : nullptr,
				    (m & 2) != 0, rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		memset(&mask, (int)(char)b, sizeof(mask));
		case_sigprocmask((int)(int8_t)b, &mask, true,
		    (long long)(int8_t)b);
	}

	{
		sigset_t zero;

		memset(&zero, 0, sizeof(zero));
		case_sigprocmask(0, &zero, false, 0);
		case_sigprocmask(SIG_SETMASK, nullptr, true, -1);
	}

	rng_seed(0x73696770726f63ULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 3u);

		sigset_fill(&mask, (unsigned char)rng_u32());
		case_sigprocmask(rnd_int(), (m & 1) != 0 ? &mask : nullptr,
		    (m & 2) != 0, rnd_ret());
	}
}

static void
test_sigaction(void)
{
	struct sigaction act;
	static const int sigs[] = {
		INT_MIN, -1, 0, 1, SIGINT, SIGKILL, SIGTERM, 0x7e, 0x7f, 0x80,
		INT_MAX,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (size_t s = 0; s < sizeof(sigs) / sizeof(sigs[0]); s++)
		for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
			for (int m = 0; m < 4; m++) {
				sigaction_fill(&act, (unsigned char)(s + 1));
				case_sigaction(sigs[s], (m & 1) != 0 ? &act :
				    nullptr, (m & 2) != 0, rets[r]);
			}

	for (unsigned b = 0x00; b <= 0xff; b++) {
		memset(&act, (int)(char)b, sizeof(act));
		case_sigaction((int)(int8_t)b, &act, true, (long long)(int8_t)b);
	}

	{
		struct sigaction zero;

		memset(&zero, 0, sizeof(zero));
		case_sigaction(0, &zero, false, 0);
		case_sigaction(SIGINT, nullptr, true, 1);
	}

	rng_seed(0x73696761637469ULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 3u);

		sigaction_fill(&act, (unsigned char)rng_u32());
		case_sigaction(rnd_int(), (m & 1) != 0 ? &act : nullptr,
		    (m & 2) != 0, rnd_ret());
	}
}

static void
test_sigtimedwait(void)
{
	sigset_t mask;
	struct timespec ts;
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	ts.tv_sec = 0;
	ts.tv_nsec = 0;

	for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
		for (int m = 0; m < 8; m++)
			case_sigtimedwait((m & 1) != 0 ? &mask : nullptr,
			    (m & 2) != 0, (m & 4) != 0 ? &ts : nullptr,
			    rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		memset(&mask, (int)(char)b, sizeof(mask));
		ts.tv_sec = (time_t)(int8_t)b;
		ts.tv_nsec = (long)(int8_t)~b;
		case_sigtimedwait(&mask, true, &ts, (long long)(int8_t)b);
	}

	{
		sigset_t zero;
		struct timespec zts = { 0, 0 };

		memset(&zero, 0, sizeof(zero));
		case_sigtimedwait(&zero, true, &zts, 0);
		case_sigtimedwait(nullptr, false, nullptr, -1);
	}

	rng_seed(0x73696774696d65ULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 7u);

		memset(&mask, (int)(char)rng_u32(), sizeof(mask));
		ts.tv_sec = (time_t)(int64_t)rng_u64();
		ts.tv_nsec = (long)(int64_t)rng_u64();
		case_sigtimedwait((m & 1) != 0 ? &mask : nullptr,
		    (m & 2) != 0, (m & 4) != 0 ? &ts : nullptr, rnd_ret());
	}
}

static void
test_readv(void)
{
	struct iovec iov[IOV_TEST_MAX];
	unsigned char data[BASE_MAX];
	static const int fds[] = {
		INT_MIN, -1, 0, 1, 2, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const int iovcnts[] = {
		INT_MIN, -1, 0, 1, 2, IOV_TEST_MAX - 1, IOV_TEST_MAX,
		IOV_TEST_MAX + 1, IOV_TEST_MAX + 4, INT_MAX,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
		(long long)SSIZE_MAX, (long long)-SSIZE_MAX - 1,
	};

	for (int i = 0; i < BASE_MAX; i++)
		data[i] = (unsigned char)(0x80 + i);

	for (size_t f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
		for (size_t c = 0; c < sizeof(iovcnts) / sizeof(iovcnts[0]); c++)
			for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
				for (int m = 0; m < 2; m++) {
					for (int i = 0; i < IOV_TEST_MAX; i++) {
						iov[i].iov_base = data + (i % BASE_MAX);
						iov[i].iov_len = (size_t)(i + 1);
					}
					case_readv(fds[f], (m & 1) != 0 ? iov :
					    nullptr, iovcnts[c], rets[r]);
				}

	for (unsigned b = 0x00; b <= 0xff; b++) {
		for (int i = 0; i < IOV_TEST_MAX; i++) {
			iov[i].iov_base = data + (i % BASE_MAX);
			iov[i].iov_len = (size_t)((b + i) % (BASE_MAX + 1));
		}
		memset(data, (int)(char)b, sizeof(data));
		case_readv((int)(int8_t)b, iov, (int)(b % (IOV_TEST_MAX + 2u)),
		    (long long)(int8_t)b);
	}

	{
		struct iovec one = { data, 0 };

		case_readv(0, &one, 0, 0);
		one.iov_len = 1;
		case_readv(1, &one, 1, 1);
		case_readv(-1, nullptr, 0, -1);
	}

	rng_seed(0x7265616476ULL);
	for (int n = 0; n < 200000; n++) {
		int iovcnt = rnd_int();
		uint64_t wide = rnd_wide();
		int m = (int)(rng_u32() & 1u);

		for (int i = 0; i < IOV_TEST_MAX; i++) {
			size_t len = rnd_size(BASE_MAX);

			for (size_t j = 0; j < len; j++)
				data[j] = (unsigned char)rng_u32();
			iov[i].iov_base = (m != 0 && (rng_u32() & 3u) != 0) ?
			    data : nullptr;
			iov[i].iov_len = len;
		}
		if (wide != 0)
			iovcnt = (int)wide;
		case_readv(rnd_int(), (m != 0) ? iov : nullptr, iovcnt,
		    rnd_ret());
	}
}

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_sigprocmask();
	test_sigaction();
	test_sigtimedwait();
	test_readv();

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
