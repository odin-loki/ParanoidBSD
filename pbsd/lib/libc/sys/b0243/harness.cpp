/*
 * harness.cpp -- differential test for PBSD batch b0243.
 *
 * select, sigsuspend, sigwaitinfo and poll are libc interposition wrappers:
 * each loads a function pointer from __libc_interposing[] and tail-calls it
 * with the original argument list.  The harness installs instrumented mocks in
 * both the oracle (ref_*) and port tables, then compares the mock-visible
 * dispatch tag, every forwarded argument (scalars and pointer offsets from
 * each buffer base), content hashes of every buffer the callee reads, the
 * entire guard-filled buffers (including padding), and the return value.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/types.h>

#ifndef FD_SETSIZE
#define	FD_SETSIZE	1024
#endif

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_poll,
	INTERPOS_select,
	INTERPOS_sigsuspend,
	INTERPOS_sigwaitinfo,
	INTERPOS_MAX
};

extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];

int ref_select(int n, fd_set *rs, fd_set *ws, fd_set *es, struct timeval *t);
int ref_sigsuspend(const sigset_t *set);
int ref_sigwaitinfo(const sigset_t * __restrict set,
    siginfo_t * __restrict info);
int ref_poll(struct pollfd pfd[], nfds_t nfds, int timeout);

}

import pbsd.lib.libc.sys.b0243;

namespace port = pbsd::lib_libc_sys::b0243;

#define	GUARD		0x7f

#define	FDSET_PAD	192
#define	FDSET_TOTAL	(sizeof(fd_set) + 2 * FDSET_PAD)

#define	TV_PAD		64
#define	TV_TOTAL	(sizeof(struct timeval) + 2 * TV_PAD)

#define	MASK_PAD	192
#define	MASK_TOTAL	(sizeof(sigset_t) + 2 * MASK_PAD)

#define	INFO_PAD	192
#define	INFO_TOTAL	(sizeof(siginfo_t) + 2 * INFO_PAD)

#define	PFD_MAX		8
#define	PFD_PAD		8
#define	PFD_ELEMS	(PFD_MAX + 2 * PFD_PAD)
#define	PFD_TOTAL	(PFD_ELEMS * sizeof(struct pollfd))
#define	PFD_OFF		(PFD_PAD * sizeof(struct pollfd))

#define	TAG_NONE		0
#define	TAG_POLL		1
#define	TAG_SELECT		2
#define	TAG_SIGSUSPEND		3
#define	TAG_SIGWAITINFO		4

#define	NSCALAR		4
#define	NPTR		5
#define	NHASH		4

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

static const unsigned char *base_rs;
static const unsigned char *base_ws;
static const unsigned char *base_es;
static const unsigned char *base_tv;
static const unsigned char *base_mask;
static const unsigned char *base_info;
static const unsigned char *base_pfd;

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
mock_select(int n, fd_set *rs, fd_set *ws, fd_set *es, struct timeval *t)
{
	mock_enter(TAG_SELECT);
	mock.sc[0] = (long long)n;
	mock.po[0] = poff(rs, base_rs);
	mock.po[1] = poff(ws, base_ws);
	mock.po[2] = poff(es, base_es);
	mock.po[3] = poff(t, base_tv);

	if (rs != nullptr)
		mock.hs[0] = hash_bytes(rs, sizeof(fd_set));
	if (ws != nullptr)
		mock.hs[1] = hash_bytes(ws, sizeof(fd_set));
	if (es != nullptr)
		mock.hs[2] = hash_bytes(es, sizeof(fd_set));
	if (t != nullptr) {
		mock.hs[3] = hash_bytes(t, sizeof(struct timeval));
		t->tv_sec = (time_t)(n ^ 0x55);
		t->tv_usec = (suseconds_t)(n ^ 0xaa);
	}
	return ((int)mock_ret);
}

static int
mock_sigsuspend(const sigset_t *set)
{
	mock_enter(TAG_SIGSUSPEND);
	mock.po[0] = poff(set, base_mask);

	if (set != nullptr)
		mock.hs[0] = hash_bytes(set, sizeof(sigset_t));
	return ((int)mock_ret);
}

static int
mock_sigwaitinfo(const sigset_t * __restrict set,
    siginfo_t * __restrict info)
{
	mock_enter(TAG_SIGWAITINFO);
	mock.po[0] = poff(set, base_mask);
	mock.po[1] = poff(info, base_info);

	if (set != nullptr)
		mock.hs[0] = hash_bytes(set, sizeof(sigset_t));
	if (info != nullptr) {
		fill_pattern((unsigned char *)info, sizeof(siginfo_t), 0xa5);
		mock.wr = (long long)info->si_signo;
	}
	return ((int)mock_ret);
}

static int
mock_poll(struct pollfd pfd[], nfds_t nfds, int timeout)
{
	nfds_t i, n;

	mock_enter(TAG_POLL);
	mock.sc[0] = (long long)(unsigned long long)nfds;
	mock.sc[1] = (long long)timeout;
	mock.po[0] = poff(pfd, base_pfd);

	if (pfd != nullptr) {
		n = nfds < (nfds_t)PFD_MAX ? nfds : (nfds_t)PFD_MAX;
		mock.hs[0] = hash_bytes(pfd,
		    (size_t)n * sizeof(struct pollfd));
		for (i = 0; i < n; i++)
			pfd[i].revents = (short)(pfd[i].events ^ 0x7f);
	}
	return ((int)mock_ret);
}

static void
install_mocks(interpos_func_t *table)
{
	table[INTERPOS_poll] = (interpos_func_t)mock_poll;
	table[INTERPOS_select] = (interpos_func_t)mock_select;
	table[INTERPOS_sigsuspend] = (interpos_func_t)mock_sigsuspend;
	table[INTERPOS_sigwaitinfo] = (interpos_func_t)mock_sigwaitinfo;
}

enum {
	FN_SELECT,
	FN_SIGSUSPEND,
	FN_SIGWAITINFO,
	FN_POLL,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"select",
	"sigsuspend",
	"sigwaitinfo",
	"poll",
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
	    "{nc=%llu tag=%d sc=[%lld,%lld,%lld,%lld] po=[%lld,%lld,%lld,%lld,%lld] "
	    "hs=[%u,%u,%u,%u] wr=%lld}",
	    s.ncalls, s.tag, s.sc[0], s.sc[1], s.sc[2], s.sc[3], s.po[0],
	    s.po[1], s.po[2], s.po[3], s.po[4], s.hs[0], s.hs[1], s.hs[2],
	    s.hs[3], s.wr);
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
fdset_fill(fd_set *s, unsigned char seed)
{
	unsigned char *p = (unsigned char *)s;

	for (size_t i = 0; i < sizeof(fd_set); i++)
		p[i] = (unsigned char)(seed + (unsigned char)i);
	FD_ZERO(s);
	for (int fd = 0; fd < FD_SETSIZE; fd += (int)(seed + 3)) {
		if ((fd + (int)seed) % 7 == 0)
			FD_SET(fd, s);
	}
}

static void
case_select(int n, const fd_set *rs, const fd_set *ws, const fd_set *es,
    const struct timeval *tv, long long ret)
{
	unsigned char rsa[FDSET_TOTAL], rsb[FDSET_TOTAL];
	unsigned char wsa[FDSET_TOTAL], wsb[FDSET_TOTAL];
	unsigned char esa[FDSET_TOTAL], esb[FDSET_TOTAL];
	unsigned char tva[TV_TOTAL], tvb[TV_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_SELECT]++;

	memset(rsa, GUARD, sizeof(rsa));
	memset(rsb, GUARD, sizeof(rsb));
	memset(wsa, GUARD, sizeof(wsa));
	memset(wsb, GUARD, sizeof(wsb));
	memset(esa, GUARD, sizeof(esa));
	memset(esb, GUARD, sizeof(esb));
	memset(tva, GUARD, sizeof(tva));
	memset(tvb, GUARD, sizeof(tvb));

	if (rs != nullptr) {
		memcpy(rsa + FDSET_PAD, rs, sizeof(fd_set));
		memcpy(rsb + FDSET_PAD, rs, sizeof(fd_set));
	}
	if (ws != nullptr) {
		memcpy(wsa + FDSET_PAD, ws, sizeof(fd_set));
		memcpy(wsb + FDSET_PAD, ws, sizeof(fd_set));
	}
	if (es != nullptr) {
		memcpy(esa + FDSET_PAD, es, sizeof(fd_set));
		memcpy(esb + FDSET_PAD, es, sizeof(fd_set));
	}
	if (tv != nullptr) {
		memcpy(tva + TV_PAD, tv, sizeof(*tv));
		memcpy(tvb + TV_PAD, tv, sizeof(*tv));
	}

	snprintf(ctx, sizeof(ctx),
	    "n=%d rs=%d ws=%d es=%d tv=%d ret=%lld", n, rs != nullptr,
	    ws != nullptr, es != nullptr, tv != nullptr, ret);

	base_rs = rsa;
	base_ws = wsa;
	base_es = esa;
	base_tv = tva;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_select(n, rs != nullptr ? (fd_set *)(rsa + FDSET_PAD) :
	    nullptr, ws != nullptr ? (fd_set *)(wsa + FDSET_PAD) : nullptr,
	    es != nullptr ? (fd_set *)(esa + FDSET_PAD) : nullptr,
	    tv != nullptr ? (struct timeval *)(tva + TV_PAD) : nullptr);
	snap_a = mock;

	base_rs = rsb;
	base_ws = wsb;
	base_es = esb;
	base_tv = tvb;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::select(n, rs != nullptr ? (fd_set *)(rsb + FDSET_PAD) :
	    nullptr, ws != nullptr ? (fd_set *)(wsb + FDSET_PAD) : nullptr,
	    es != nullptr ? (fd_set *)(esb + FDSET_PAD) : nullptr,
	    tv != nullptr ? (struct timeval *)(tvb + TV_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_SELECT, snap_a, snap_b, ctx);
	cmp_buf(FN_SELECT, "rs", rsa, rsb, sizeof(rsa), ctx);
	cmp_buf(FN_SELECT, "ws", wsa, wsb, sizeof(wsa), ctx);
	cmp_buf(FN_SELECT, "es", esa, esb, sizeof(esa), ctx);
	cmp_buf(FN_SELECT, "tv", tva, tvb, sizeof(tva), ctx);
	cmp_ret(FN_SELECT, ra, rb, ctx);
}

static void
case_sigsuspend(const sigset_t *set, long long ret)
{
	unsigned char mask_a[MASK_TOTAL], mask_b[MASK_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[192];

	fn_cases[FN_SIGSUSPEND]++;

	memset(mask_a, GUARD, sizeof(mask_a));
	memset(mask_b, GUARD, sizeof(mask_b));

	if (set != nullptr) {
		memcpy(mask_a + MASK_PAD, set, sizeof(sigset_t));
		memcpy(mask_b + MASK_PAD, set, sizeof(sigset_t));
	}

	snprintf(ctx, sizeof(ctx), "set=%d ret=%lld", set != nullptr, ret);

	base_mask = mask_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_sigsuspend(set != nullptr ?
	    (const sigset_t *)(mask_a + MASK_PAD) : nullptr);
	snap_a = mock;

	base_mask = mask_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::sigsuspend(set != nullptr ?
	    (const sigset_t *)(mask_b + MASK_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_SIGSUSPEND, snap_a, snap_b, ctx);
	cmp_buf(FN_SIGSUSPEND, "mask", mask_a, mask_b, sizeof(mask_a), ctx);
	cmp_ret(FN_SIGSUSPEND, ra, rb, ctx);
}

static void
case_sigwaitinfo(const sigset_t *set, bool use_info, long long ret)
{
	unsigned char mask_a[MASK_TOTAL], mask_b[MASK_TOTAL];
	unsigned char info_a[INFO_TOTAL], info_b[INFO_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[192];

	fn_cases[FN_SIGWAITINFO]++;

	memset(mask_a, GUARD, sizeof(mask_a));
	memset(mask_b, GUARD, sizeof(mask_b));
	memset(info_a, GUARD, sizeof(info_a));
	memset(info_b, GUARD, sizeof(info_b));

	if (set != nullptr) {
		memcpy(mask_a + MASK_PAD, set, sizeof(sigset_t));
		memcpy(mask_b + MASK_PAD, set, sizeof(sigset_t));
	}

	snprintf(ctx, sizeof(ctx), "set=%d info=%d ret=%lld", set != nullptr,
	    (int)use_info, ret);

	base_mask = mask_a;
	base_info = info_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_sigwaitinfo(set != nullptr ?
	    (const sigset_t *)(mask_a + MASK_PAD) : nullptr,
	    use_info ? (siginfo_t *)(info_a + INFO_PAD) : nullptr);
	snap_a = mock;

	base_mask = mask_b;
	base_info = info_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::sigwaitinfo(set != nullptr ?
	    (const sigset_t *)(mask_b + MASK_PAD) : nullptr,
	    use_info ? (siginfo_t *)(info_b + INFO_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_SIGWAITINFO, snap_a, snap_b, ctx);
	cmp_buf(FN_SIGWAITINFO, "mask", mask_a, mask_b, sizeof(mask_a), ctx);
	cmp_buf(FN_SIGWAITINFO, "info", info_a, info_b, sizeof(info_a), ctx);
	cmp_ret(FN_SIGWAITINFO, ra, rb, ctx);
}

static void
case_poll(const struct pollfd *pfd, nfds_t nfds, int timeout, long long ret)
{
	unsigned char pfd_a[PFD_TOTAL], pfd_b[PFD_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[192];

	fn_cases[FN_POLL]++;

	memset(pfd_a, GUARD, sizeof(pfd_a));
	memset(pfd_b, GUARD, sizeof(pfd_b));

	if (pfd != nullptr) {
		size_t n = (nfds < (nfds_t)PFD_MAX ? (size_t)nfds :
		    (size_t)PFD_MAX) * sizeof(struct pollfd);

		memcpy(pfd_a + PFD_OFF, pfd, n);
		memcpy(pfd_b + PFD_OFF, pfd, n);
	}

	snprintf(ctx, sizeof(ctx), "pfd=%d nfds=%llu to=%d ret=%lld",
	    pfd != nullptr, (unsigned long long)nfds, timeout, ret);

	base_pfd = pfd_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_poll(pfd != nullptr ? (struct pollfd *)(pfd_a + PFD_OFF) :
	    nullptr, nfds, timeout);
	snap_a = mock;

	base_pfd = pfd_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::poll(pfd != nullptr ?
	    (struct pollfd *)(pfd_b + PFD_OFF) : nullptr, nfds, timeout);
	snap_b = mock;

	cmp_snap(FN_POLL, snap_a, snap_b, ctx);
	cmp_buf(FN_POLL, "pfd", pfd_a, pfd_b, sizeof(pfd_a), ctx);
	cmp_ret(FN_POLL, ra, rb, ctx);
}

static void
test_select(void)
{
	fd_set rs, ws, es;
	struct timeval tv;
	static const int ns[] = {
		INT_MIN, -1, 0, 1, 2, FD_SETSIZE - 1, FD_SETSIZE, FD_SETSIZE + 1,
		0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (size_t n = 0; n < sizeof(ns) / sizeof(ns[0]); n++)
		for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
			for (int m = 0; m < 16; m++) {
				fdset_fill(&rs, (unsigned char)(n + 1));
				fdset_fill(&ws, (unsigned char)(n + 2));
				fdset_fill(&es, (unsigned char)(n + 3));
				tv.tv_sec = (time_t)ns[n];
				tv.tv_usec = (suseconds_t)(ns[n] ^ 0x55);
				case_select(ns[n], (m & 1) != 0 ? &rs :
				    nullptr, (m & 2) != 0 ? &ws : nullptr,
				    (m & 4) != 0 ? &es : nullptr,
				    (m & 8) != 0 ? &tv : nullptr, rets[r]);
			}

	/* Every high-bit fd_set byte pattern. */
	for (unsigned b = 0x00; b <= 0xff; b++) {
		memset(&rs, (int)(char)b, sizeof(rs));
		memset(&ws, (int)(char)(b ^ 0xff), sizeof(ws));
		memset(&es, 0, sizeof(es));
		FD_ZERO(&es);
		if (b & 1)
			FD_SET((int)(b % FD_SETSIZE), &es);
		tv.tv_sec = (time_t)(int8_t)b;
		tv.tv_usec = (suseconds_t)(int8_t)~b;
		case_select((int)(int8_t)b, &rs, &ws, &es, &tv,
		    (long long)(int8_t)b);
	}

	/* NUL-heavy fd_sets. */
	{
		fd_set zero;

		memset(&zero, 0, sizeof(zero));
		case_select(0, &zero, &zero, &zero, nullptr, 0);
		case_select(1, &zero, nullptr, nullptr, nullptr, 1);
	}

	rng_seed(0x73656c656374ULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 15u);

		fdset_fill(&rs, (unsigned char)rng_u32());
		fdset_fill(&ws, (unsigned char)rng_u32());
		fdset_fill(&es, (unsigned char)rng_u32());
		tv.tv_sec = (time_t)(int64_t)rng_u64();
		tv.tv_usec = (suseconds_t)(int64_t)rng_u64();
		case_select(rnd_int(), (m & 1) != 0 ? &rs : nullptr,
		    (m & 2) != 0 ? &ws : nullptr, (m & 4) != 0 ? &es : nullptr,
		    (m & 8) != 0 ? &tv : nullptr, rnd_ret());
	}
}

static void
test_sigsuspend(void)
{
	sigset_t mask;
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
		case_sigsuspend(nullptr, rets[r]);
		case_sigsuspend(&mask, rets[r]);
	}

	for (unsigned b = 0x00; b <= 0xff; b++) {
		memset(&mask, (int)(char)b, sizeof(mask));
		case_sigsuspend(&mask, (long long)(int8_t)b);
	}

	{
		sigset_t zero;

		memset(&zero, 0, sizeof(zero));
		case_sigsuspend(&zero, 0);
	}

	rng_seed(0x736967737573ULL);
	for (int n = 0; n < 200000; n++) {
		memset(&mask, (int)(char)rng_u32(), sizeof(mask));
		if ((rng_u32() & 3u) == 0)
			sigemptyset(&mask);
		case_sigsuspend((rng_u32() & 7u) != 0 ? &mask : nullptr,
		    rnd_ret());
	}
}

static void
test_sigwaitinfo(void)
{
	sigset_t mask;
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);

	for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
		for (int m = 0; m < 4; m++)
			case_sigwaitinfo((m & 1) != 0 ? &mask : nullptr,
			    (m & 2) != 0, rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		memset(&mask, (int)(char)b, sizeof(mask));
		case_sigwaitinfo(&mask, true, (long long)(int8_t)b);
	}

	{
		sigset_t zero;

		memset(&zero, 0, sizeof(zero));
		case_sigwaitinfo(&zero, true, 0);
		case_sigwaitinfo(nullptr, false, -1);
	}

	rng_seed(0x73696777616974ULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 3u);

		memset(&mask, (int)(char)rng_u32(), sizeof(mask));
		case_sigwaitinfo((m & 1) != 0 ? &mask : nullptr,
		    (m & 2) != 0, rnd_ret());
	}
}

static void
test_poll(void)
{
	struct pollfd pfds[PFD_MAX];
	static const nfds_t nfds_list[] = {
		0, 1, 2, PFD_MAX - 1, PFD_MAX, PFD_MAX + 1, PFD_MAX + 4,
		(nfds_t)INT_MAX, (nfds_t)UINT_MAX, (nfds_t)-1,
	};
	static const int timeouts[] = {
		INT_MIN, -1, 0, 1, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (int i = 0; i < PFD_MAX; i++) {
		pfds[i].fd = i - 2;
		pfds[i].events = (short)(0x80 + i);
		pfds[i].revents = (short)-1;
	}

	for (size_t n = 0; n < sizeof(nfds_list) / sizeof(nfds_list[0]); n++)
		for (size_t t = 0; t < sizeof(timeouts) / sizeof(timeouts[0]); t++)
			for (size_t r = 0;
			    r < sizeof(rets) / sizeof(rets[0]); r++)
				for (int m = 0; m < 2; m++)
					case_poll((m & 1) != 0 ? pfds : nullptr,
					    nfds_list[n], timeouts[t], rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		for (int i = 0; i < PFD_MAX; i++) {
			pfds[i].fd = (int)(int8_t)b - i;
			pfds[i].events = (short)((b << 8) | (b ^ 0xff));
			pfds[i].revents = 0;
		}
		case_poll(pfds, (nfds_t)(b % (PFD_MAX + 2u)), (int)(int8_t)b,
		    (long long)(int8_t)b);
	}

	{
		struct pollfd one = { 0, 0, 0 };

		case_poll(&one, 0, 0, 0);
		case_poll(&one, 1, -1, 1);
	}

	rng_seed(0x706f6c6cULL);
	for (int n = 0; n < 200000; n++) {
		nfds_t nfds = (nfds_t)rnd_size(PFD_MAX);
		uint64_t wide = rnd_wide();
		int m = (int)(rng_u32() & 1u);

		for (int i = 0; i < PFD_MAX; i++) {
			pfds[i].fd = rnd_int();
			pfds[i].events = (short)rnd_int();
			pfds[i].revents = (short)rnd_int();
		}
		if (wide != 0)
			nfds = (nfds_t)wide;
		case_poll((m != 0) ? pfds : nullptr, nfds, rnd_int(), rnd_ret());
	}
}

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_select();
	test_sigsuspend();
	test_sigwaitinfo();
	test_poll();

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
