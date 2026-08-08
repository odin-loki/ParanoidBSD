/*
 * harness.cpp -- differential test for PBSD batch b0097.
 *
 * wait6, sendto, clock_nanosleep and ppoll are libc interposition wrappers:
 * each one loads a function pointer out of __libc_interposing[] and calls it
 * with the original argument list, returning what it returns.  So the entire
 * observable behaviour is: which slot is read, and what arrives at the callee.
 *
 * The harness installs the same instrumented mocks in the oracle table
 * (ref___libc_interposing) and in the port table (port::__libc_interposing),
 * calls both wrappers with equivalent-but-separate buffers, and compares:
 *
 *	- the number of callee invocations and which slot was dispatched (tag);
 *	- every scalar argument as it arrived at the callee;
 *	- every pointer argument as an OFFSET from its own buffer base, so a
 *	  mutation such as "ru + 1" is caught even if nothing is written;
 *	- a content hash of every buffer the callee reads, so a mutation that
 *	  hands over different bytes is caught;
 *	- the two guard-filled buffers in their ENTIRETY, including the padding
 *	  before and after the nominal object, so any stray or shifted write is
 *	  caught;
 *	- the value returned to the caller.
 *
 * Guard padding on each side of every object is larger than the object, so a
 * mutant that shifts a pointer by one element still reads and writes inside
 * the buffer and is reported as a data difference rather than crashing.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>

#ifndef SSIZE_MIN
#define	SSIZE_MIN	((ssize_t)(-SSIZE_MAX - 1))
#endif

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_wait6,
	INTERPOS_sendto,
	INTERPOS_clock_nanosleep,
	INTERPOS_ppoll,
	INTERPOS_MAX
};

struct __wrusage {
	unsigned char	opaque[128];
};

extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];

pid_t ref_wait6(idtype_t idtype, id_t id, int *status, int options,
    struct __wrusage *ru, siginfo_t *infop);
ssize_t ref_sendto(int s, const void *msg, size_t len, int flags,
    const struct sockaddr *to, socklen_t tolen);
int ref_clock_nanosleep(clockid_t clock_id, int flags,
    const struct timespec *rqtp, struct timespec *rmtp);
int ref_ppoll(struct pollfd pfd[], nfds_t nfds,
    const struct timespec *timeout, const sigset_t *newsigmask);

}

import pbsd.lib.libc.sys.b0097;

namespace port = pbsd::lib_libc_sys::b0097;

/*
 * The port declares its own struct __wrusage, since libc's is not visible
 * outside the libc build.  It must be layout-identical to the oracle's for the
 * shared mock to see the same object through either table.
 */
static_assert(sizeof(port::__wrusage) == sizeof(struct __wrusage));
static_assert(alignof(port::__wrusage) == alignof(struct __wrusage));

#define	GUARD		0x7f

#define	MSG_CAP		64
#define	MSG_PAD		64
#define	MSG_TOTAL	(MSG_CAP + 2 * MSG_PAD)

#define	ADDR_CAP	64
#define	ADDR_PAD	64
#define	ADDR_TOTAL	(ADDR_CAP + 2 * ADDR_PAD)

#define	STATUS_PAD	64
#define	STATUS_TOTAL	(sizeof(int) + 2 * STATUS_PAD)

#define	RU_PAD		192
#define	RU_TOTAL	(sizeof(struct __wrusage) + 2 * RU_PAD)

#define	INFO_PAD	192
#define	INFO_TOTAL	(sizeof(siginfo_t) + 2 * INFO_PAD)

#define	TS_PAD		64
#define	TS_TOTAL	(sizeof(struct timespec) + 2 * TS_PAD)

#define	MASK_PAD	192
#define	MASK_TOTAL	(sizeof(sigset_t) + 2 * MASK_PAD)

#define	PFD_MAX		8
#define	PFD_PAD		8			/* in elements */
#define	PFD_ELEMS	(PFD_MAX + 2 * PFD_PAD)
#define	PFD_TOTAL	(PFD_ELEMS * sizeof(struct pollfd))
#define	PFD_OFF		(PFD_PAD * sizeof(struct pollfd))

#define	TAG_NONE		0
#define	TAG_WAIT6		1
#define	TAG_SENDTO		2
#define	TAG_CLOCK_NANOSLEEP	3
#define	TAG_PPOLL		4

#define	NSCALAR	4
#define	NPTR	3
#define	NHASH	3

/* Sentinel offset recorded for a null pointer argument. */
#define	OFF_NULL	(-4242424242LL)

/*
 * What the callee saw, in a form that can be compared between the two runs
 * without ever comparing a raw address.
 */
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

/* Buffer bases the mock measures incoming pointers against. */
static const unsigned char *base_status;
static const unsigned char *base_ru;
static const unsigned char *base_info;
static const unsigned char *base_msg;
static const unsigned char *base_addr;
static const unsigned char *base_rqtp;
static const unsigned char *base_rmtp;
static const unsigned char *base_pfd;
static const unsigned char *base_ts;
static const unsigned char *base_mask;

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

static pid_t
mock_wait6(idtype_t idtype, id_t id, int *status, int options,
    struct __wrusage *ru, siginfo_t *infop)
{
	mock_enter(TAG_WAIT6);
	mock.sc[0] = (long long)idtype;
	mock.sc[1] = (long long)id;
	mock.sc[2] = (long long)options;
	mock.po[0] = poff(status, base_status);
	mock.po[1] = poff(ru, base_ru);
	mock.po[2] = poff(infop, base_info);

	if (status != nullptr) {
		*status = (int)(0x7f00 ^ (unsigned)options);
		mock.wr = *status;
	}
	if (ru != nullptr)
		fill_pattern((unsigned char *)ru, sizeof(struct __wrusage),
		    (unsigned char)(id & 0xff));
	if (infop != nullptr)
		fill_pattern((unsigned char *)infop, sizeof(siginfo_t),
		    (unsigned char)((id >> 8) & 0xff));

	return ((pid_t)mock_ret);
}

static ssize_t
mock_sendto(int s, const void *msg, size_t len, int flags,
    const struct sockaddr *to, socklen_t tolen)
{
	mock_enter(TAG_SENDTO);
	mock.sc[0] = (long long)s;
	mock.sc[1] = (long long)(unsigned long long)len;
	mock.sc[2] = (long long)flags;
	mock.sc[3] = (long long)(unsigned long long)tolen;
	mock.po[0] = poff(msg, base_msg);
	mock.po[1] = poff(to, base_addr);

	if (msg != nullptr)
		mock.hs[0] = hash_bytes(msg,
		    len < MSG_CAP ? len : (size_t)MSG_CAP);
	if (to != nullptr)
		mock.hs[1] = hash_bytes(to,
		    tolen < ADDR_CAP ? (size_t)tolen : (size_t)ADDR_CAP);

	return ((ssize_t)mock_ret);
}

static int
mock_clock_nanosleep(clockid_t clock_id, int flags,
    const struct timespec *rqtp, struct timespec *rmtp)
{
	mock_enter(TAG_CLOCK_NANOSLEEP);
	mock.sc[0] = (long long)clock_id;
	mock.sc[1] = (long long)flags;
	mock.po[0] = poff(rqtp, base_rqtp);
	mock.po[1] = poff(rmtp, base_rmtp);

	if (rqtp != nullptr)
		mock.hs[0] = hash_bytes(rqtp, sizeof(struct timespec));
	if (rmtp != nullptr) {
		rmtp->tv_sec = (time_t)(clock_id ^ flags);
		rmtp->tv_nsec = (long)(flags ^ 0x55);
	}
	return ((int)mock_ret);
}

static int
mock_ppoll(struct pollfd pfd[], nfds_t nfds, const struct timespec *timeout,
    const sigset_t *newsigmask)
{
	nfds_t i, n;

	mock_enter(TAG_PPOLL);
	mock.sc[0] = (long long)(unsigned long long)nfds;
	mock.po[0] = poff(pfd, base_pfd);
	mock.po[1] = poff(timeout, base_ts);
	mock.po[2] = poff(newsigmask, base_mask);

	if (timeout != nullptr)
		mock.hs[1] = hash_bytes(timeout, sizeof(struct timespec));
	if (newsigmask != nullptr)
		mock.hs[2] = hash_bytes(newsigmask, sizeof(sigset_t));
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
	table[INTERPOS_wait6] = (interpos_func_t)mock_wait6;
	table[INTERPOS_sendto] = (interpos_func_t)mock_sendto;
	table[INTERPOS_clock_nanosleep] =
	    (interpos_func_t)mock_clock_nanosleep;
	table[INTERPOS_ppoll] = (interpos_func_t)mock_ppoll;
}

enum {
	FN_WAIT6,
	FN_SENDTO,
	FN_CLOCK_NANOSLEEP,
	FN_PPOLL,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"wait6",
	"sendto",
	"clock_nanosleep",
	"ppoll",
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
	    "{nc=%llu tag=%d sc=[%lld,%lld,%lld,%lld] po=[%lld,%lld,%lld] "
	    "hs=[%u,%u,%u] wr=%lld}",
	    s.ncalls, s.tag, s.sc[0], s.sc[1], s.sc[2], s.sc[3], s.po[0],
	    s.po[1], s.po[2], s.hs[0], s.hs[1], s.hs[2], s.wr);
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

	char sa[320], sb[320], msg[1024];

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

/*
 * A length that is usually small enough to exercise the buffer, but sometimes
 * enormous, so that a mutant which narrowed a size_t/socklen_t/nfds_t on the
 * way to the callee is visible.
 */
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
case_wait6(idtype_t idtype, id_t id, bool use_status, int options,
    bool use_ru, bool use_info, long long ret)
{
	unsigned char status_a[STATUS_TOTAL], status_b[STATUS_TOTAL];
	unsigned char ru_a[RU_TOTAL], ru_b[RU_TOTAL];
	unsigned char info_a[INFO_TOTAL], info_b[INFO_TOTAL];
	Snap snap_a, snap_b;
	pid_t ra, rb;
	char ctx[192];

	fn_cases[FN_WAIT6]++;

	memset(status_a, GUARD, sizeof(status_a));
	memset(status_b, GUARD, sizeof(status_b));
	memset(ru_a, GUARD, sizeof(ru_a));
	memset(ru_b, GUARD, sizeof(ru_b));
	memset(info_a, GUARD, sizeof(info_a));
	memset(info_b, GUARD, sizeof(info_b));

	snprintf(ctx, sizeof(ctx), "idtype=%d id=%lld opt=%d s=%d r=%d i=%d "
	    "ret=%lld", (int)idtype, (long long)id, options, (int)use_status,
	    (int)use_ru, (int)use_info, ret);

	base_status = status_a;
	base_ru = ru_a;
	base_info = info_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_wait6(idtype, id,
	    use_status ? (int *)(status_a + STATUS_PAD) : nullptr, options,
	    use_ru ? (struct __wrusage *)(ru_a + RU_PAD) : nullptr,
	    use_info ? (siginfo_t *)(info_a + INFO_PAD) : nullptr);
	snap_a = mock;

	base_status = status_b;
	base_ru = ru_b;
	base_info = info_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::wait6(idtype, id,
	    use_status ? (int *)(status_b + STATUS_PAD) : nullptr, options,
	    use_ru ? (port::__wrusage *)(ru_b + RU_PAD) : nullptr,
	    use_info ? (siginfo_t *)(info_b + INFO_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_WAIT6, snap_a, snap_b, ctx);
	cmp_buf(FN_WAIT6, "status", status_a, status_b, sizeof(status_a), ctx);
	cmp_buf(FN_WAIT6, "ru", ru_a, ru_b, sizeof(ru_a), ctx);
	cmp_buf(FN_WAIT6, "infop", info_a, info_b, sizeof(info_a), ctx);
	cmp_ret(FN_WAIT6, (long long)ra, (long long)rb, ctx);
}

static void
case_sendto(int s, const unsigned char *msg, size_t len, int flags,
    const unsigned char *to, socklen_t tolen, long long ret)
{
	unsigned char msg_a[MSG_TOTAL], msg_b[MSG_TOTAL];
	unsigned char addr_a[ADDR_TOTAL], addr_b[ADDR_TOTAL];
	Snap snap_a, snap_b;
	ssize_t ra, rb;
	char ctx[192];

	fn_cases[FN_SENDTO]++;

	memset(msg_a, GUARD, sizeof(msg_a));
	memset(msg_b, GUARD, sizeof(msg_b));
	memset(addr_a, GUARD, sizeof(addr_a));
	memset(addr_b, GUARD, sizeof(addr_b));

	if (msg != nullptr) {
		size_t n = len < MSG_CAP ? len : (size_t)MSG_CAP;

		memcpy(msg_a + MSG_PAD, msg, n);
		memcpy(msg_b + MSG_PAD, msg, n);
	}
	if (to != nullptr) {
		size_t n = tolen < ADDR_CAP ? (size_t)tolen :
		    (size_t)ADDR_CAP;

		memcpy(addr_a + ADDR_PAD, to, n);
		memcpy(addr_b + ADDR_PAD, to, n);
	}

	snprintf(ctx, sizeof(ctx),
	    "s=%d msg=%d len=%zu flags=%d to=%d tolen=%u ret=%lld", s,
	    msg != nullptr, len, flags, to != nullptr, (unsigned)tolen, ret);

	base_msg = msg_a;
	base_addr = addr_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_sendto(s, msg != nullptr ? msg_a + MSG_PAD : nullptr, len,
	    flags,
	    to != nullptr ? (const struct sockaddr *)(addr_a + ADDR_PAD) :
	    nullptr, tolen);
	snap_a = mock;

	base_msg = msg_b;
	base_addr = addr_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::sendto(s, msg != nullptr ? msg_b + MSG_PAD : nullptr, len,
	    flags,
	    to != nullptr ? (const struct sockaddr *)(addr_b + ADDR_PAD) :
	    nullptr, tolen);
	snap_b = mock;

	cmp_snap(FN_SENDTO, snap_a, snap_b, ctx);
	cmp_buf(FN_SENDTO, "msg", msg_a, msg_b, sizeof(msg_a), ctx);
	cmp_buf(FN_SENDTO, "to", addr_a, addr_b, sizeof(addr_a), ctx);
	cmp_ret(FN_SENDTO, (long long)ra, (long long)rb, ctx);
}

static void
case_clock_nanosleep(clockid_t clock_id, int flags,
    const struct timespec *rqtp, bool use_rmtp, long long ret)
{
	unsigned char rq_a[TS_TOTAL], rq_b[TS_TOTAL];
	unsigned char rm_a[TS_TOTAL], rm_b[TS_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[192];

	fn_cases[FN_CLOCK_NANOSLEEP]++;

	memset(rq_a, GUARD, sizeof(rq_a));
	memset(rq_b, GUARD, sizeof(rq_b));
	memset(rm_a, GUARD, sizeof(rm_a));
	memset(rm_b, GUARD, sizeof(rm_b));

	if (rqtp != nullptr) {
		memcpy(rq_a + TS_PAD, rqtp, sizeof(*rqtp));
		memcpy(rq_b + TS_PAD, rqtp, sizeof(*rqtp));
	}

	snprintf(ctx, sizeof(ctx),
	    "clk=%d flags=%d rq=%d rm=%d ret=%lld", (int)clock_id, flags,
	    rqtp != nullptr, (int)use_rmtp, ret);

	base_rqtp = rq_a;
	base_rmtp = rm_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_clock_nanosleep(clock_id, flags,
	    rqtp != nullptr ? (const struct timespec *)(rq_a + TS_PAD) :
	    nullptr,
	    use_rmtp ? (struct timespec *)(rm_a + TS_PAD) : nullptr);
	snap_a = mock;

	base_rqtp = rq_b;
	base_rmtp = rm_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::clock_nanosleep(clock_id, flags,
	    rqtp != nullptr ? (const struct timespec *)(rq_b + TS_PAD) :
	    nullptr,
	    use_rmtp ? (struct timespec *)(rm_b + TS_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_CLOCK_NANOSLEEP, snap_a, snap_b, ctx);
	cmp_buf(FN_CLOCK_NANOSLEEP, "rqtp", rq_a, rq_b, sizeof(rq_a), ctx);
	cmp_buf(FN_CLOCK_NANOSLEEP, "rmtp", rm_a, rm_b, sizeof(rm_a), ctx);
	cmp_ret(FN_CLOCK_NANOSLEEP, ra, rb, ctx);
}

static void
case_ppoll(const struct pollfd *pfd, nfds_t nfds,
    const struct timespec *timeout, const sigset_t *newsigmask,
    long long ret)
{
	unsigned char pfd_a[PFD_TOTAL], pfd_b[PFD_TOTAL];
	unsigned char ts_a[TS_TOTAL], ts_b[TS_TOTAL];
	unsigned char mask_a[MASK_TOTAL], mask_b[MASK_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[192];

	fn_cases[FN_PPOLL]++;

	memset(pfd_a, GUARD, sizeof(pfd_a));
	memset(pfd_b, GUARD, sizeof(pfd_b));
	memset(ts_a, GUARD, sizeof(ts_a));
	memset(ts_b, GUARD, sizeof(ts_b));
	memset(mask_a, GUARD, sizeof(mask_a));
	memset(mask_b, GUARD, sizeof(mask_b));

	if (pfd != nullptr) {
		size_t n = (nfds < (nfds_t)PFD_MAX ? (size_t)nfds :
		    (size_t)PFD_MAX) * sizeof(struct pollfd);

		memcpy(pfd_a + PFD_OFF, pfd, n);
		memcpy(pfd_b + PFD_OFF, pfd, n);
	}
	if (timeout != nullptr) {
		memcpy(ts_a + TS_PAD, timeout, sizeof(*timeout));
		memcpy(ts_b + TS_PAD, timeout, sizeof(*timeout));
	}
	if (newsigmask != nullptr) {
		memcpy(mask_a + MASK_PAD, newsigmask, sizeof(sigset_t));
		memcpy(mask_b + MASK_PAD, newsigmask, sizeof(sigset_t));
	}

	snprintf(ctx, sizeof(ctx), "pfd=%d nfds=%llu to=%d mask=%d ret=%lld",
	    pfd != nullptr, (unsigned long long)nfds, timeout != nullptr,
	    newsigmask != nullptr, ret);

	base_pfd = pfd_a;
	base_ts = ts_a;
	base_mask = mask_a;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_ppoll(pfd != nullptr ? (struct pollfd *)(pfd_a + PFD_OFF) :
	    nullptr, nfds,
	    timeout != nullptr ? (const struct timespec *)(ts_a + TS_PAD) :
	    nullptr,
	    newsigmask != nullptr ? (const sigset_t *)(mask_a + MASK_PAD) :
	    nullptr);
	snap_a = mock;

	base_pfd = pfd_b;
	base_ts = ts_b;
	base_mask = mask_b;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::ppoll(pfd != nullptr ? (struct pollfd *)(pfd_b + PFD_OFF) :
	    nullptr, nfds,
	    timeout != nullptr ? (const struct timespec *)(ts_b + TS_PAD) :
	    nullptr,
	    newsigmask != nullptr ? (const sigset_t *)(mask_b + MASK_PAD) :
	    nullptr);
	snap_b = mock;

	cmp_snap(FN_PPOLL, snap_a, snap_b, ctx);
	cmp_buf(FN_PPOLL, "pfd", pfd_a, pfd_b, sizeof(pfd_a), ctx);
	cmp_buf(FN_PPOLL, "timeout", ts_a, ts_b, sizeof(ts_a), ctx);
	cmp_buf(FN_PPOLL, "sigmask", mask_a, mask_b, sizeof(mask_a), ctx);
	cmp_ret(FN_PPOLL, ra, rb, ctx);
}

static void
test_wait6(void)
{
	static const idtype_t idtypes[] = {
		P_ALL, P_PID, P_PGID, (idtype_t)0, (idtype_t)0x7f,
		(idtype_t)0x80, (idtype_t)INT_MAX,
	};
	static const id_t ids[] = {
		(id_t)0, (id_t)1, (id_t)-1, (id_t)0x7f, (id_t)0x80,
		(id_t)INT_MAX, (id_t)INT_MIN, (id_t)UINT_MAX,
	};
	static const int optss[] = {
		0, 1, -1, WNOHANG, WUNTRACED, WCONTINUED, 0x7f, 0x80,
		INT_MIN, INT_MAX,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (size_t t = 0; t < sizeof(idtypes) / sizeof(idtypes[0]); t++)
		for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); i++)
			for (size_t o = 0;
			    o < sizeof(optss) / sizeof(optss[0]); o++)
				for (size_t r = 0;
				    r < sizeof(rets) / sizeof(rets[0]); r++)
					for (int m = 0; m < 8; m++)
						case_wait6(idtypes[t], ids[i],
						    (m & 1) != 0, optss[o],
						    (m & 2) != 0,
						    (m & 4) != 0, rets[r]);

	rng_seed(0x7761697436ULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 7u);

		case_wait6((idtype_t)rnd_int(), (id_t)rng_u32(), (m & 1) != 0,
		    rnd_int(), (m & 2) != 0, (m & 4) != 0, rnd_ret());
	}
}

static void
test_sendto(void)
{
	unsigned char mbuf[MSG_CAP], abuf[ADDR_CAP];
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 2, 3, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const size_t lens[] = {
		0, 1, 2, MSG_CAP - 1, MSG_CAP, MSG_CAP + 1,
		(size_t)INT_MAX, (size_t)INT_MAX + 1, (size_t)SSIZE_MAX,
		(size_t)-1,
	};
	static const int flagss[] = {
		0, 1, -1, MSG_OOB, MSG_DONTWAIT, 0x7f, 0x80, INT_MIN,
		INT_MAX,
	};
	static const socklen_t tolens[] = {
		0, 1, 2, sizeof(struct sockaddr_in), ADDR_CAP - 1, ADDR_CAP,
		ADDR_CAP + 1, (socklen_t)INT_MAX, (socklen_t)-1,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, SSIZE_MAX, SSIZE_MIN,
	};

	for (size_t i = 0; i < sizeof(mbuf); i++)
		mbuf[i] = (unsigned char)(0x80 + i);
	for (size_t i = 0; i < sizeof(abuf); i++)
		abuf[i] = (unsigned char)(0xff - i);

	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t l = 0; l < sizeof(lens) / sizeof(lens[0]); l++)
			for (size_t f = 0;
			    f < sizeof(flagss) / sizeof(flagss[0]); f++)
				for (size_t t = 0;
				    t < sizeof(tolens) / sizeof(tolens[0]); t++)
					for (size_t r = 0;
					    r < sizeof(rets) / sizeof(rets[0]);
					    r++) {
						case_sendto(socks[s], nullptr,
						    lens[l], flagss[f],
						    nullptr, tolens[t],
						    rets[r]);
						case_sendto(socks[s], mbuf,
						    lens[l], flagss[f], abuf,
						    tolens[t], rets[r]);
					}

	/* Every high-bit payload byte, at every one-byte offset. */
	for (unsigned b = 0x00; b <= 0xff; b++) {
		unsigned char one[1] = { (unsigned char)b };
		unsigned char addr1[1] = { (unsigned char)(b ^ 0xff) };

		case_sendto((int)(int8_t)b, one, 1, (int)(int8_t)b, addr1, 1,
		    (long long)(int8_t)b);
		case_sendto(3, one, 1, 4, addr1, 1, (long long)b);
	}

	/* NUL-heavy payloads. */
	{
		unsigned char zeros[MSG_CAP];

		memset(zeros, 0, sizeof(zeros));
		for (size_t l = 0; l <= MSG_CAP; l++)
			case_sendto(5, zeros, l, 6, zeros,
			    (socklen_t)l, (long long)l);
	}

	rng_seed(0x73656e64746fULL);
	for (int n = 0; n < 200000; n++) {
		size_t len = rnd_size(MSG_CAP);
		socklen_t tolen = (socklen_t)rnd_size(ADDR_CAP);
		uint64_t wide = rnd_wide();

		for (size_t i = 0; i < len; i++)
			mbuf[i] = (unsigned char)rng_u32();
		for (size_t i = 0; i < tolen; i++)
			abuf[i] = (unsigned char)rng_u32();
		if (wide != 0)
			len = (size_t)wide;
		case_sendto(rnd_int(), (rng_u32() & 7u) != 0 ? mbuf : nullptr,
		    len, rnd_int(),
		    (rng_u32() & 7u) != 0 ? abuf : nullptr, tolen, rnd_ret());
	}
}

static void
test_clock_nanosleep(void)
{
	struct timespec rq;
	static const clockid_t clocks[] = {
		CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID,
		(clockid_t)0, (clockid_t)1, (clockid_t)-1, (clockid_t)0x7f,
		(clockid_t)0x80, (clockid_t)INT_MAX, (clockid_t)INT_MIN,
	};
	static const int flagss[] = {
		0, 1, -1, TIMER_ABSTIME, 0x7f, 0x80, INT_MIN, INT_MAX,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (size_t c = 0; c < sizeof(clocks) / sizeof(clocks[0]); c++)
		for (size_t f = 0; f < sizeof(flagss) / sizeof(flagss[0]); f++)
			for (size_t r = 0;
			    r < sizeof(rets) / sizeof(rets[0]); r++)
				for (int m = 0; m < 4; m++) {
					rq.tv_sec = (time_t)(c + 1);
					rq.tv_nsec = (long)(f + 1);
					case_clock_nanosleep(clocks[c],
					    flagss[f],
					    (m & 1) != 0 ? &rq : nullptr,
					    (m & 2) != 0, rets[r]);
				}

	static const long secs[] = {
		LONG_MIN, -1, 0, 1, 0x7f, 0x80, LONG_MAX,
	};
	static const long nsecs[] = {
		LONG_MIN, -1, 0, 1, 999999998, 999999999, 1000000000,
		LONG_MAX,
	};

	for (size_t s = 0; s < sizeof(secs) / sizeof(secs[0]); s++)
		for (size_t n = 0; n < sizeof(nsecs) / sizeof(nsecs[0]); n++) {
			rq.tv_sec = (time_t)secs[s];
			rq.tv_nsec = nsecs[n];
			case_clock_nanosleep(CLOCK_REALTIME, 0, &rq, true,
			    (long long)(secs[s] ^ nsecs[n]));
		}

	rng_seed(0x636c6f636bULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 3u);

		rq.tv_sec = (time_t)(int64_t)rng_u64();
		rq.tv_nsec = (long)(int64_t)rng_u64();
		case_clock_nanosleep((clockid_t)rnd_int(), rnd_int(),
		    (m & 1) != 0 ? &rq : nullptr, (m & 2) != 0, rnd_ret());
	}
}

static void
test_ppoll(void)
{
	struct pollfd pfds[PFD_MAX];
	struct timespec ts;
	sigset_t mask;
	static const nfds_t nfds_list[] = {
		0, 1, 2, PFD_MAX - 1, PFD_MAX, PFD_MAX + 1, PFD_MAX + 4,
		(nfds_t)INT_MAX, (nfds_t)UINT_MAX, (nfds_t)-1,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (int i = 0; i < PFD_MAX; i++) {
		pfds[i].fd = i - 2;
		pfds[i].events = (short)(0x80 + i);
		pfds[i].revents = (short)-1;
	}
	ts.tv_sec = 1;
	ts.tv_nsec = 2;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);

	for (size_t n = 0; n < sizeof(nfds_list) / sizeof(nfds_list[0]); n++)
		for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
			for (int m = 0; m < 8; m++)
				case_ppoll((m & 1) != 0 ? pfds : nullptr,
				    nfds_list[n],
				    (m & 2) != 0 ? &ts : nullptr,
				    (m & 4) != 0 ? &mask : nullptr, rets[r]);

	/* Every high-bit events pattern, one descriptor at a time. */
	for (unsigned b = 0x00; b <= 0xff; b++) {
		for (int i = 0; i < PFD_MAX; i++) {
			pfds[i].fd = (int)(int8_t)b - i;
			pfds[i].events = (short)((b << 8) | (b ^ 0xff));
			pfds[i].revents = 0;
		}
		ts.tv_sec = (time_t)(int8_t)b;
		ts.tv_nsec = (long)(int8_t)~b;
		sigemptyset(&mask);
		sigaddset(&mask, 1 + (int)(b % 30u));
		case_ppoll(pfds, (nfds_t)(b % (PFD_MAX + 2u)), &ts, &mask,
		    (long long)(int8_t)b);
	}

	rng_seed(0x70706f6c6cULL);
	for (int n = 0; n < 200000; n++) {
		nfds_t nfds = (nfds_t)rnd_size(PFD_MAX);
		uint64_t wide = rnd_wide();
		int m = (int)(rng_u32() & 7u);

		for (int i = 0; i < PFD_MAX; i++) {
			pfds[i].fd = rnd_int();
			pfds[i].events = (short)rnd_int();
			pfds[i].revents = (short)rnd_int();
		}
		ts.tv_sec = (time_t)(int64_t)rng_u64();
		ts.tv_nsec = (long)(int64_t)rng_u64();
		sigemptyset(&mask);
		if ((rng_u32() & 3u) == 0)
			sigaddset(&mask, 1 + (int)(rng_u32() % 30u));
		if (wide != 0)
			nfds = (nfds_t)wide;
		case_ppoll((m & 1) != 0 ? pfds : nullptr, nfds,
		    (m & 2) != 0 ? &ts : nullptr,
		    (m & 4) != 0 ? &mask : nullptr, rnd_ret());
	}
}

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_wait6();
	test_sendto();
	test_clock_nanosleep();
	test_ppoll();

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
