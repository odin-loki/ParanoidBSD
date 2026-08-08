/*
 * harness.cpp -- differential test for PBSD batch b0097.
 *
 * wait6, sendto, clock_nanosleep and ppoll are libc interposition wrappers:
 * each loads a function pointer from __libc_interposing[] and tail-calls it
 * with the original argument list.  The harness installs instrumented mocks in
 * both the oracle (ref_*) and port tables, then compares the mock-visible
 * syscall tag, forwarded arguments, output-buffer writes and return value.
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

using port_wait6_fn = pid_t (*)(idtype_t, id_t, int *, int,
    struct __wrusage *, siginfo_t *);

static port_wait6_fn
port_wait6_ptr(void)
{
	return (reinterpret_cast<port_wait6_fn>(
	    reinterpret_cast<void *>(port::wait6)));
}

static pid_t
call_port_wait6(idtype_t idtype, id_t id, int *status, int options,
    struct __wrusage *ru, siginfo_t *infop)
{
	return (port_wait6_ptr()(idtype, id, status, options, ru, infop));
}

#define	GUARD			0x7f
#define	MSG_CAP			64
#define	MSG_GUARD_PAD		16
#define	ADDR_CAP		64
#define	ADDR_GUARD_PAD		16
#define	PFD_MAX			8
#define	PFD_GUARD_PAD		4
#define	TS_GUARD_PAD		8
#define	SIGSET_GUARD_PAD	8
#define	RU_GUARD_PAD		16
#define	INFO_GUARD_PAD		16
#define	STATUS_GUARD_PAD	16

#define	PBSD_TAG_NONE		0
#define	PBSD_TAG_WAIT6		1
#define	PBSD_TAG_SENDTO		2
#define	PBSD_TAG_CLOCK_NANOSLEEP 3
#define	PBSD_TAG_PPOLL		4

struct MockState {
	unsigned long long	ncalls;
	int			tag;
	idtype_t		idtype;
	id_t			id;
	int			options;
	int			s;
	size_t			len;
	int			flags;
	socklen_t		tolen;
	clockid_t		clock_id;
	nfds_t			nfds;
	unsigned char		msg_hash;
	unsigned char		addr_hash;
	unsigned char		rqtp_hash;
	unsigned char		timeout_hash;
	unsigned char		sigmask_hash;
	unsigned char		pfd_hash;
	int			status_out;
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
mock_enter(int tag)
{
	mock.ncalls++;
	mock.tag = tag;
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
	mock_enter(PBSD_TAG_WAIT6);
	mock.idtype = idtype;
	mock.id = id;
	mock.options = options;

	if (status != nullptr) {
		*status = (int)(0x7f00 ^ (unsigned)options);
		mock.status_out = *status;
	}
	if (ru != nullptr)
		fill_pattern((unsigned char *)ru, sizeof(struct __wrusage),
		    (unsigned char)(id & 0xff));
	if (infop != nullptr)
		fill_pattern((unsigned char *)infop, sizeof(siginfo_t),
		    (unsigned char)((id >> 8) & 0xff));

	return ((pid_t)mock.prog_ret);
}

static ssize_t
mock_sendto(int s, const void *msg, size_t len, int flags,
    const struct sockaddr *to, socklen_t tolen)
{
	mock_enter(PBSD_TAG_SENDTO);
	mock.s = s;
	mock.len = len;
	mock.flags = flags;
	mock.tolen = tolen;
	if (msg != nullptr && len > 0)
		mock.msg_hash = (unsigned char)hash_bytes(msg, len);
	if (to != nullptr && tolen > 0)
		mock.addr_hash = (unsigned char)hash_bytes(to, tolen);
	return ((ssize_t)mock.prog_ret);
}

static int
mock_clock_nanosleep(clockid_t clock_id, int flags,
    const struct timespec *rqtp, struct timespec *rmtp)
{
	mock_enter(PBSD_TAG_CLOCK_NANOSLEEP);
	mock.clock_id = clock_id;
	mock.flags = flags;
	if (rqtp != nullptr)
		mock.rqtp_hash = (unsigned char)hash_bytes(rqtp,
		    sizeof(struct timespec));
	if (rmtp != nullptr) {
		rmtp->tv_sec = (time_t)(clock_id ^ flags);
		rmtp->tv_nsec = (long)(flags ^ 0x55);
	}
	return ((int)mock.prog_ret);
}

static int
mock_ppoll(struct pollfd pfd[], nfds_t nfds, const struct timespec *timeout,
    const sigset_t *newsigmask)
{
	nfds_t i, n;

	mock_enter(PBSD_TAG_PPOLL);
	mock.nfds = nfds;
	if (timeout != nullptr)
		mock.timeout_hash = (unsigned char)hash_bytes(timeout,
		    sizeof(struct timespec));
	if (newsigmask != nullptr)
		mock.sigmask_hash = (unsigned char)hash_bytes(newsigmask,
		    sizeof(sigset_t));
	if (pfd != nullptr && nfds > 0) {
		n = nfds < PFD_MAX ? nfds : (nfds_t)PFD_MAX;
		mock.pfd_hash = (unsigned char)hash_bytes(pfd,
		    (size_t)n * sizeof(struct pollfd));
		for (i = 0; i < n; i++)
			pfd[i].revents = (short)(pfd[i].events ^ 0x7f);
	}
	return ((int)mock.prog_ret);
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

struct Snap {
	unsigned long long	ncalls;
	int			tag;
	idtype_t		idtype;
	id_t			id;
	int			options;
	int			s;
	size_t			len;
	int			flags;
	socklen_t		tolen;
	clockid_t		clock_id;
	nfds_t			nfds;
	unsigned char		msg_hash;
	unsigned char		addr_hash;
	unsigned char		rqtp_hash;
	unsigned char		timeout_hash;
	unsigned char		sigmask_hash;
	unsigned char		pfd_hash;
	int			status_out;
};

static Snap
take_snap(void)
{
	Snap s;

	s.ncalls = mock.ncalls;
	s.tag = mock.tag;
	s.idtype = mock.idtype;
	s.id = mock.id;
	s.options = mock.options;
	s.s = mock.s;
	s.len = mock.len;
	s.flags = mock.flags;
	s.tolen = mock.tolen;
	s.clock_id = mock.clock_id;
	s.nfds = mock.nfds;
	s.msg_hash = mock.msg_hash;
	s.addr_hash = mock.addr_hash;
	s.rqtp_hash = mock.rqtp_hash;
	s.timeout_hash = mock.timeout_hash;
	s.sigmask_hash = mock.sigmask_hash;
	s.pfd_hash = mock.pfd_hash;
	s.status_out = mock.status_out;
	return (s);
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

static bool
fail(int fn, const char *what, const char *detail)
{
	fn_fails[fn]++;
	if (fn_reported[fn] < MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-17s %-18s %s\n", fn_name[fn], what, detail);
	} else if (fn_reported[fn] == MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-17s ... further failures suppressed\n",
		    fn_name[fn]);
	}
	return (false);
}

static bool
cmp_snap(int fn, const Snap &a, const Snap &b, const char *ctx)
{
	if (a.ncalls != b.ncalls || a.tag != b.tag || a.idtype != b.idtype ||
	    a.id != b.id || a.options != b.options || a.s != b.s ||
	    a.len != b.len || a.flags != b.flags || a.tolen != b.tolen ||
	    a.clock_id != b.clock_id || a.nfds != b.nfds ||
	    a.msg_hash != b.msg_hash || a.addr_hash != b.addr_hash ||
	    a.rqtp_hash != b.rqtp_hash || a.timeout_hash != b.timeout_hash ||
	    a.sigmask_hash != b.sigmask_hash || a.pfd_hash != b.pfd_hash ||
	    a.status_out != b.status_out) {
		char msg[640];

		snprintf(msg, sizeof(msg),
		    "%s ref={nc=%llu tg=%d idt=%d id=%lld opt=%d s=%d "
		    "len=%zu fl=%d tolen=%u clk=%d nfds=%llu mh=%u ah=%u "
		    "rqh=%u th=%u sh=%u ph=%u st=%d} "
		    "port={nc=%llu tg=%d idt=%d id=%lld opt=%d s=%d "
		    "len=%zu fl=%d tolen=%u clk=%d nfds=%llu mh=%u ah=%u "
		    "rqh=%u th=%u sh=%u ph=%u st=%d}",
		    ctx, a.ncalls, a.tag, (int)a.idtype, (long long)a.id,
		    a.options, a.s, a.len, a.flags, (unsigned)a.tolen,
		    (int)a.clock_id, (unsigned long long)a.nfds, a.msg_hash,
		    a.addr_hash, a.rqtp_hash, a.timeout_hash, a.sigmask_hash,
		    a.pfd_hash, a.status_out, b.ncalls, b.tag, (int)b.idtype,
		    (long long)b.id, b.options, b.s, b.len, b.flags,
		    (unsigned)b.tolen, (int)b.clock_id,
		    (unsigned long long)b.nfds, b.msg_hash, b.addr_hash,
		    b.rqtp_hash, b.timeout_hash, b.sigmask_hash, b.pfd_hash,
		    b.status_out);
		fail(fn, "mock", msg);
		return (false);
	}
	return (true);
}

static bool
cmp_buf(int fn, const unsigned char *a, const unsigned char *b, size_t n,
    const char *ctx)
{
	for (size_t i = 0; i < n; i++) {
		if (a[i] != b[i]) {
			char msg[192];

			snprintf(msg, sizeof(msg),
			    "%s byte[%zu] ref=0x%02x port=0x%02x", ctx, i,
			    a[i], b[i]);
			fail(fn, "buffer", msg);
			return (false);
		}
	}
	return (true);
}

static void
fill_guard(unsigned char *buf, size_t n)
{
	memset(buf, GUARD, n);
}

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

static unsigned char
rnd_byte(void)
{
	return ((unsigned char)rng_u32());
}

static size_t
rnd_size(size_t max)
{
	if (max == 0)
		return (0);
	return ((size_t)rng_u32() % (max + 1));
}

static void
case_wait6(idtype_t idtype, id_t id, int *status, int options,
    struct __wrusage *ru, siginfo_t *infop, long long ret)
{
	unsigned char status_a[sizeof(int) + 2 * STATUS_GUARD_PAD];
	unsigned char status_b[sizeof(int) + 2 * STATUS_GUARD_PAD];
	unsigned char ru_a[sizeof(struct __wrusage) + 2 * RU_GUARD_PAD];
	unsigned char ru_b[sizeof(struct __wrusage) + 2 * RU_GUARD_PAD];
	unsigned char info_a[sizeof(siginfo_t) + 2 * INFO_GUARD_PAD];
	unsigned char info_b[sizeof(siginfo_t) + 2 * INFO_GUARD_PAD];
	int *psa, *psb;
	struct __wrusage *rua, *rub;
	siginfo_t *ia, *ib;
	Snap snap_a, snap_b;
	pid_t ra, rb;
	char ctx[256];

	fn_cases[FN_WAIT6]++;

	fill_guard(status_a, sizeof(status_a));
	fill_guard(status_b, sizeof(status_b));
	fill_guard(ru_a, sizeof(ru_a));
	fill_guard(ru_b, sizeof(ru_b));
	fill_guard(info_a, sizeof(info_a));
	fill_guard(info_b, sizeof(info_b));

	psa = status != nullptr ?
	    (int *)(status_a + STATUS_GUARD_PAD) : nullptr;
	psb = status != nullptr ?
	    (int *)(status_b + STATUS_GUARD_PAD) : nullptr;
	rua = ru != nullptr ?
	    (struct __wrusage *)(ru_a + RU_GUARD_PAD) : nullptr;
	rub = ru != nullptr ?
	    (struct __wrusage *)(ru_b + RU_GUARD_PAD) : nullptr;
	ia = infop != nullptr ?
	    (siginfo_t *)(info_a + INFO_GUARD_PAD) : nullptr;
	ib = infop != nullptr ?
	    (siginfo_t *)(info_b + INFO_GUARD_PAD) : nullptr;

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_wait6(idtype, id, psa, options, rua, ia);
	snap_a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = call_port_wait6(idtype, id, psb, options, rub, ib);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx),
	    "idtype=%d id=%lld opt=%d ret=%lld", (int)idtype,
	    (long long)id, options, ret);
	cmp_snap(FN_WAIT6, snap_a, snap_b, ctx);
	cmp_buf(FN_WAIT6, status_a, status_b, sizeof(status_a), ctx);
	cmp_buf(FN_WAIT6, ru_a, ru_b, sizeof(ru_a), ctx);
	cmp_buf(FN_WAIT6, info_a, info_b, sizeof(info_a), ctx);

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%lld port=%lld", ctx,
		    (long long)ra, (long long)rb);
		fail(FN_WAIT6, "return", msg);
	}
}

static void
case_sendto(int s, const void *msg, size_t len, int flags,
    const struct sockaddr *to, socklen_t tolen, long long ret)
{
	unsigned char msg_a[MSG_CAP + 2 * MSG_GUARD_PAD];
	unsigned char msg_b[MSG_CAP + 2 * MSG_GUARD_PAD];
	unsigned char addr_a[ADDR_CAP + 2 * ADDR_GUARD_PAD];
	unsigned char addr_b[ADDR_CAP + 2 * ADDR_GUARD_PAD];
	const void *ma, *mb;
	const struct sockaddr *ta, *tb;
	Snap snap_a, snap_b;
	ssize_t ra, rb;
	char ctx[256];
	size_t total_msg, total_addr, copy_len, copy_tolen;

	fn_cases[FN_SENDTO]++;

	total_msg = MSG_CAP + 2 * MSG_GUARD_PAD;
	total_addr = ADDR_CAP + 2 * ADDR_GUARD_PAD;
	fill_guard(msg_a, total_msg);
	fill_guard(msg_b, total_msg);
	fill_guard(addr_a, total_addr);
	fill_guard(addr_b, total_addr);

	ma = msg;
	mb = msg;
	if (msg != nullptr) {
		ma = msg_a + MSG_GUARD_PAD;
		mb = msg_b + MSG_GUARD_PAD;
		copy_len = len < MSG_CAP ? len : MSG_CAP;
		memcpy((void *)(msg_a + MSG_GUARD_PAD), msg, copy_len);
		memcpy((void *)(msg_b + MSG_GUARD_PAD), msg, copy_len);
	}

	ta = to;
	tb = to;
	if (to != nullptr) {
		ta = (const struct sockaddr *)(addr_a + ADDR_GUARD_PAD);
		tb = (const struct sockaddr *)(addr_b + ADDR_GUARD_PAD);
		copy_tolen = tolen < ADDR_CAP ? tolen : ADDR_CAP;
		memcpy((void *)(addr_a + ADDR_GUARD_PAD), to, copy_tolen);
		memcpy((void *)(addr_b + ADDR_GUARD_PAD), to, copy_tolen);
	}

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_sendto(s, ma, len, flags, ta, tolen);
	snap_a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::sendto(s, mb, len, flags, tb, tolen);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx), "s=%d len=%zu flags=%d tolen=%u ret=%lld",
	    s, len, flags, (unsigned)tolen, ret);
	cmp_snap(FN_SENDTO, snap_a, snap_b, ctx);
	cmp_buf(FN_SENDTO, msg_a, msg_b, total_msg, ctx);
	cmp_buf(FN_SENDTO, addr_a, addr_b, total_addr, ctx);

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%lld port=%lld", ctx,
		    (long long)ra, (long long)rb);
		fail(FN_SENDTO, "return", msg);
	}
}

static void
case_clock_nanosleep(clockid_t clock_id, int flags,
    const struct timespec *rqtp, struct timespec *rmtp, long long ret)
{
	unsigned char rq_a[sizeof(struct timespec) + 2 * TS_GUARD_PAD];
	unsigned char rq_b[sizeof(struct timespec) + 2 * TS_GUARD_PAD];
	unsigned char rm_a[sizeof(struct timespec) + 2 * TS_GUARD_PAD];
	unsigned char rm_b[sizeof(struct timespec) + 2 * TS_GUARD_PAD];
	const struct timespec *rqa, *rqb;
	struct timespec *rma, *rmb;
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_CLOCK_NANOSLEEP]++;

	fill_guard(rq_a, sizeof(rq_a));
	fill_guard(rq_b, sizeof(rq_b));
	fill_guard(rm_a, sizeof(rm_a));
	fill_guard(rm_b, sizeof(rm_b));

	rqa = rqtp != nullptr ?
	    (const struct timespec *)(rq_a + TS_GUARD_PAD) : nullptr;
	rqb = rqtp != nullptr ?
	    (const struct timespec *)(rq_b + TS_GUARD_PAD) : nullptr;
	rma = rmtp != nullptr ?
	    (struct timespec *)(rm_a + TS_GUARD_PAD) : nullptr;
	rmb = rmtp != nullptr ?
	    (struct timespec *)(rm_b + TS_GUARD_PAD) : nullptr;

	if (rqtp != nullptr)
		memcpy((void *)(rq_a + TS_GUARD_PAD), rqtp, sizeof(*rqtp));
	if (rqtp != nullptr)
		memcpy((void *)(rq_b + TS_GUARD_PAD), rqtp, sizeof(*rqtp));

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_clock_nanosleep(clock_id, flags, rqa, rma);
	snap_a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::clock_nanosleep(clock_id, flags, rqb, rmb);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx), "clk=%d flags=%d ret=%lld", (int)clock_id,
	    flags, ret);
	cmp_snap(FN_CLOCK_NANOSLEEP, snap_a, snap_b, ctx);
	cmp_buf(FN_CLOCK_NANOSLEEP, rq_a, rq_b, sizeof(rq_a), ctx);
	cmp_buf(FN_CLOCK_NANOSLEEP, rm_a, rm_b, sizeof(rm_a), ctx);

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_CLOCK_NANOSLEEP, "return", msg);
	}
}

static void
case_ppoll(struct pollfd pfd[], nfds_t nfds, const struct timespec *timeout,
    const sigset_t *newsigmask, long long ret)
{
	unsigned char pfd_a[(PFD_MAX + 2 * PFD_GUARD_PAD) * sizeof(struct pollfd)];
	unsigned char pfd_b[(PFD_MAX + 2 * PFD_GUARD_PAD) * sizeof(struct pollfd)];
	unsigned char ts_a[sizeof(struct timespec) + 2 * TS_GUARD_PAD];
	unsigned char ts_b[sizeof(struct timespec) + 2 * TS_GUARD_PAD];
	unsigned char mask_a[sizeof(sigset_t) + 2 * SIGSET_GUARD_PAD];
	unsigned char mask_b[sizeof(sigset_t) + 2 * SIGSET_GUARD_PAD];
	struct pollfd *pa, *pb;
	const struct timespec *ta, *tb;
	const sigset_t *ma, *mb;
	nfds_t copy_n;
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];
	size_t total_pfd;

	fn_cases[FN_PPOLL]++;

	total_pfd = (PFD_MAX + 2 * PFD_GUARD_PAD) * sizeof(struct pollfd);
	fill_guard(pfd_a, total_pfd);
	fill_guard(pfd_b, total_pfd);
	fill_guard(ts_a, sizeof(ts_a));
	fill_guard(ts_b, sizeof(ts_b));
	fill_guard(mask_a, sizeof(mask_a));
	fill_guard(mask_b, sizeof(mask_b));

	pa = pfd != nullptr ?
	    (struct pollfd *)(pfd_a + PFD_GUARD_PAD * sizeof(struct pollfd)) :
	    nullptr;
	pb = pfd != nullptr ?
	    (struct pollfd *)(pfd_b + PFD_GUARD_PAD * sizeof(struct pollfd)) :
	    nullptr;
	if (pfd != nullptr && nfds > 0) {
		copy_n = nfds < PFD_MAX ? nfds : (nfds_t)PFD_MAX;
		memcpy(pa, pfd, (size_t)copy_n * sizeof(struct pollfd));
		memcpy(pb, pfd, (size_t)copy_n * sizeof(struct pollfd));
	}

	ta = timeout != nullptr ?
	    (const struct timespec *)(ts_a + TS_GUARD_PAD) : nullptr;
	tb = timeout != nullptr ?
	    (const struct timespec *)(ts_b + TS_GUARD_PAD) : nullptr;
	ma = newsigmask != nullptr ?
	    (const sigset_t *)(mask_a + SIGSET_GUARD_PAD) : nullptr;
	mb = newsigmask != nullptr ?
	    (const sigset_t *)(mask_b + SIGSET_GUARD_PAD) : nullptr;
	if (timeout != nullptr)
		memcpy((void *)(ts_a + TS_GUARD_PAD), timeout, sizeof(*timeout));
	if (timeout != nullptr)
		memcpy((void *)(ts_b + TS_GUARD_PAD), timeout, sizeof(*timeout));
	if (newsigmask != nullptr) {
		memcpy((void *)(mask_a + SIGSET_GUARD_PAD), newsigmask,
		    sizeof(sigset_t));
		memcpy((void *)(mask_b + SIGSET_GUARD_PAD), newsigmask,
		    sizeof(sigset_t));
	}

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_ppoll(pa, nfds, ta, ma);
	snap_a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::ppoll(pb, nfds, tb, mb);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx), "nfds=%llu ret=%lld",
	    (unsigned long long)nfds, ret);
	cmp_snap(FN_PPOLL, snap_a, snap_b, ctx);
	cmp_buf(FN_PPOLL, pfd_a, pfd_b, total_pfd, ctx);
	cmp_buf(FN_PPOLL, ts_a, ts_b, sizeof(ts_a), ctx);
	cmp_buf(FN_PPOLL, mask_a, mask_b, sizeof(mask_a), ctx);

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_PPOLL, "return", msg);
	}
}

static void
test_wait6(void)
{
	static const idtype_t idtypes[] = {
		P_PID, P_PGID, P_ALL,
		(idtype_t)0, (idtype_t)0x7f, (idtype_t)0x80, (idtype_t)INT_MAX,
	};
	static const id_t ids[] = {
		(id_t)0, (id_t)1, (id_t)-1, (id_t)INT_MAX, (id_t)INT_MIN,
		(id_t)0x7f, (id_t)0x80,
	};
	static const int optss[] = {
		0, WNOHANG, WUNTRACED, WCONTINUED, INT_MIN, INT_MAX,
		0x7f, 0x80,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	int status;
	struct __wrusage ru;
	siginfo_t info;

	for (size_t t = 0; t < sizeof(idtypes) / sizeof(idtypes[0]); t++)
		for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); i++)
			for (size_t o = 0; o < sizeof(optss) / sizeof(optss[0]); o++)
				for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
					case_wait6(idtypes[t], ids[i], nullptr,
					    optss[o], nullptr, nullptr, rets[r]);
					case_wait6(idtypes[t], ids[i], &status,
					    optss[o], nullptr, nullptr, rets[r]);
					case_wait6(idtypes[t], ids[i], nullptr,
					    optss[o], &ru, nullptr, rets[r]);
					case_wait6(idtypes[t], ids[i], nullptr,
					    optss[o], nullptr, &info, rets[r]);
					case_wait6(idtypes[t], ids[i], &status,
					    optss[o], &ru, &info, rets[r]);
				}

	rng_seed(0x7761'6974ULL);
	for (int n = 0; n < 200000; n++) {
		idtype_t idtype = (idtype_t)rnd_int();
		id_t id = (id_t)rnd_int();
		int options = rnd_int();
		long long ret = rnd_ret();
		int use_status = (int)(rng_u32() & 1u);
		int use_ru = (int)(rng_u32() & 1u);
		int use_info = (int)(rng_u32() & 1u);

		case_wait6(idtype, id,
		    use_status ? &status : nullptr, options,
		    use_ru ? &ru : nullptr,
		    use_info ? &info : nullptr, ret);
	}
}

static void
test_sendto(void)
{
	unsigned char msg_bufs[4][MSG_CAP];
	unsigned char addr_bufs[4][ADDR_CAP];
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 2, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const size_t lens[] = {
		0, 1, 2, MSG_CAP, MSG_CAP + 1, (size_t)SSIZE_MAX,
	};
	static const int flagss[] = {
		0, MSG_DONTWAIT, MSG_OOB, INT_MIN, INT_MAX, 0x7f, 0x80,
	};
	static const socklen_t tolens[] = {
		0, 1, 2, sizeof(struct sockaddr_in), (socklen_t)ADDR_CAP,
		(socklen_t)(ADDR_CAP + 1),
	};
	static const long long rets[] = {
		-1, 0, 1, SSIZE_MAX, SSIZE_MIN, 0x7f, 0x80,
	};

	for (size_t b = 0; b < sizeof(msg_bufs) / sizeof(msg_bufs[0]); b++)
		for (size_t i = 0; i < sizeof(msg_bufs[0]); i++)
			msg_bufs[b][i] = (unsigned char)(0x80 + b * 11 + i);
	for (size_t b = 0; b < sizeof(addr_bufs) / sizeof(addr_bufs[0]); b++)
		for (size_t i = 0; i < sizeof(addr_bufs[0]); i++)
			addr_bufs[b][i] = (unsigned char)(b ^ (unsigned char)i);

	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t l = 0; l < sizeof(lens) / sizeof(lens[0]); l++)
			for (size_t f = 0; f < sizeof(flagss) / sizeof(flagss[0]); f++)
				for (size_t t = 0; t < sizeof(tolens) / sizeof(tolens[0]); t++)
					for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
						case_sendto(socks[s], nullptr,
						    lens[l], flagss[f], nullptr,
						    tolens[t], rets[r]);
						case_sendto(socks[s], msg_bufs[0],
						    lens[l], flagss[f],
						    (const struct sockaddr *)
						    addr_bufs[1], tolens[t],
						    rets[r]);
					}

	for (unsigned char b = 0; b < 0xff; b += 0x1f) {
		msg_bufs[0][0] = b;
		msg_bufs[0][1] = (unsigned char)(b ^ 0xff);
		addr_bufs[0][0] = (unsigned char)~b;
		case_sendto(3, msg_bufs[0], 2, (int)(int8_t)b,
		    (const struct sockaddr *)addr_bufs[0], 2,
		    (long long)(int8_t)b);
	}

	rng_seed(0x736e'6474ULL);
	for (int n = 0; n < 200000; n++) {
		unsigned char mb[MSG_CAP];
		unsigned char ab[ADDR_CAP];
		size_t len = rnd_size(MSG_CAP);
		socklen_t tolen = (socklen_t)rnd_size(ADDR_CAP);

		for (size_t i = 0; i < len; i++)
			mb[i] = (unsigned char)rnd_byte();
		for (size_t i = 0; i < tolen; i++)
			ab[i] = (unsigned char)rnd_byte();
		case_sendto(rnd_int(), len > 0 ? mb : nullptr, len, rnd_int(),
		    tolen > 0 ? (const struct sockaddr *)ab : nullptr, tolen,
		    rnd_ret());
	}
}

static void
test_clock_nanosleep(void)
{
	struct timespec rq, rm;
	static const clockid_t clocks[] = {
		CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID,
		(clockid_t)0, (clockid_t)-1, (clockid_t)INT_MAX,
		(clockid_t)0x7f, (clockid_t)0x80,
	};
	static const int flagss[] = {
		0, TIMER_ABSTIME, INT_MIN, INT_MAX, 0x7f, 0x80,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t c = 0; c < sizeof(clocks) / sizeof(clocks[0]); c++)
		for (size_t f = 0; f < sizeof(flagss) / sizeof(flagss[0]); f++)
			for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
				rq.tv_sec = 0;
				rq.tv_nsec = 0;
				case_clock_nanosleep(clocks[c], flagss[f],
				    nullptr, nullptr, rets[r]);
				case_clock_nanosleep(clocks[c], flagss[f],
				    &rq, nullptr, rets[r]);
				case_clock_nanosleep(clocks[c], flagss[f],
				    nullptr, &rm, rets[r]);
				case_clock_nanosleep(clocks[c], flagss[f],
				    &rq, &rm, rets[r]);
			}

	for (long sec = -1; sec <= 2; sec++)
		for (long nsec = 0; nsec <= 2; nsec++) {
			rq.tv_sec = sec;
			rq.tv_nsec = nsec;
			case_clock_nanosleep(CLOCK_REALTIME, 0, &rq, &rm,
			    (long long)(sec ^ nsec));
		}

	rng_seed(0x636c'6b6eULL);
	for (int n = 0; n < 200000; n++) {
		rq.tv_sec = (time_t)rnd_int();
		rq.tv_nsec = (long)rnd_int();
		case_clock_nanosleep((clockid_t)rnd_int(), rnd_int(),
		    (rng_u32() & 1u) ? &rq : nullptr,
		    (rng_u32() & 1u) ? &rm : nullptr, rnd_ret());
	}
}

static void
test_ppoll(void)
{
	struct pollfd pfds[PFD_MAX];
	struct timespec ts;
	sigset_t mask;
	static const nfds_t nfds_list[] = {
		0, 1, 2, (nfds_t)PFD_MAX, (nfds_t)(PFD_MAX + 1),
		(nfds_t)(PFD_MAX + 4), (nfds_t)INT_MAX,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t n = 0; n < sizeof(nfds_list) / sizeof(nfds_list[0]); n++)
		for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
			case_ppoll(nullptr, nfds_list[n], nullptr, nullptr,
			    rets[r]);
			case_ppoll(pfds, nfds_list[n], nullptr, nullptr,
			    rets[r]);
			case_ppoll(pfds, nfds_list[n], &ts, nullptr, rets[r]);
			case_ppoll(pfds, nfds_list[n], nullptr, &mask,
			    rets[r]);
			case_ppoll(pfds, nfds_list[n], &ts, &mask, rets[r]);
		}

	for (int i = 0; i < PFD_MAX; i++) {
		pfds[i].fd = i - 2;
		pfds[i].events = (short)(0x80 + i);
		pfds[i].revents = 0;
	}
	for (unsigned char b = 0; b < 0xff; b += 0x2f) {
		ts.tv_sec = (time_t)(int8_t)b;
		ts.tv_nsec = (long)(int8_t)(~b);
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);
		case_ppoll(pfds, (nfds_t)((b & 3) + 1), &ts, &mask,
		    (long long)(int8_t)b);
	}

	rng_seed(0x7070'6f6cULL);
	for (int n = 0; n < 200000; n++) {
		nfds_t nfds = (nfds_t)rnd_int();
		nfds_t use = nfds > PFD_MAX ? (nfds_t)PFD_MAX : nfds;

		for (nfds_t i = 0; i < use; i++) {
			pfds[i].fd = rnd_int();
			pfds[i].events = (short)rnd_int();
			pfds[i].revents = (short)rnd_int();
		}
		ts.tv_sec = (time_t)rnd_int();
		ts.tv_nsec = (long)rnd_int();
		sigemptyset(&mask);
		if ((rng_u32() & 3u) == 0)
			sigaddset(&mask, (int)(rng_u32() % 32));
		case_ppoll(nfds > 0 ? pfds : nullptr, nfds,
		    (rng_u32() & 1u) ? &ts : nullptr,
		    (rng_u32() & 1u) ? &mask : nullptr, rnd_ret());
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
