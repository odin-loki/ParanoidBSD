/*
 * harness.cpp -- differential test for PBSD batch b0097.
 *
 * wait6, sendto, clock_nanosleep and ppoll are libc interposition wrappers:
 * each loads a function pointer from the interposing table and tail-calls it
 * with the original argument list.  The harness installs instrumented mocks in
 * both the oracle (ref_*) and port tables, then compares the mock-visible
 * syscall tag, every forwarded argument and return value.  Functions that write
 * through caller-supplied buffers use 0x7f guard bytes and are compared in full.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_wait6,
	INTERPOS_sendto,
	INTERPOS_clock_nanosleep,
	INTERPOS_ppoll,
	INTERPOS_MAX
};

struct __wrusage;

pid_t ref_wait6(idtype_t idtype, id_t id, int *status, int options,
    struct __wrusage *ru, siginfo_t *infop);
ssize_t ref_sendto(int s, const void *msg, size_t len, int flags,
    const struct sockaddr *to, socklen_t tolen);
int ref_clock_nanosleep(clockid_t clock_id, int flags,
    const struct timespec *rqtp, struct timespec *rmtp);
int ref_ppoll(struct pollfd pfd[], nfds_t nfds,
    const struct timespec *timeout, const sigset_t *newsigmask);

void ref_set_interpos(int slot, interpos_func_t func);

}

import pbsd.lib.libc.sys.b0097;

namespace port = pbsd::lib_libc_sys::b0097;

#define	GUARD		0x7f
#define	BUF_TOTAL	512
#define	DATA_OFF	64
#define	DATA_CAP	(BUF_TOTAL - DATA_OFF)
#define	DIGEST_MAX	64
#define	PFD_MAX		16

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
	int			status_is_null;
	int			ru_is_null;
	int			infop_is_null;
	int			status_out;
	int			s;
	size_t			len;
	int			flags;
	socklen_t		tolen;
	const void		*to_ptr;
	clockid_t		clock_id;
	int			cn_flags;
	int			rqtp_is_null;
	int			rmtp_is_null;
	nfds_t			nfds;
	int			timeout_is_null;
	int			sigmask_is_null;
	unsigned char		wdigest[DIGEST_MAX];
	size_t			wdigest_len;
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
mock_digest(const void *buf, size_t nbytes)
{
	size_t n = nbytes;

	if (n > DIGEST_MAX)
		n = DIGEST_MAX;
	mock.wdigest_len = n;
	if (n > 0 && buf != NULL)
		memcpy(mock.wdigest, buf, n);
}

static pid_t
mock_wait6(idtype_t idtype, id_t id, int *status, int options,
    struct __wrusage *ru, siginfo_t *infop)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_WAIT6;
	mock.idtype = idtype;
	mock.id = id;
	mock.options = options;
	mock.status_is_null = (status == NULL);
	mock.ru_is_null = (ru == NULL);
	mock.infop_is_null = (infop == NULL);
	mock.status_out = (int)(mock.prog_ret ^ 0x5a5a5a5a);

	if (status != NULL)
		*status = mock.status_out;
	if (ru != NULL)
		memset(ru, 0x42, 64);
	if (infop != NULL)
		memset(infop, 0x33, sizeof(*infop));

	return ((pid_t)mock.prog_ret);
}

static ssize_t
mock_sendto(int s, const void *msg, size_t len, int flags,
    const struct sockaddr *to, socklen_t tolen)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_SENDTO;
	mock.s = s;
	mock.len = len;
	mock.flags = flags;
	mock.tolen = tolen;
	mock.to_ptr = to;
	mock_digest(msg, len);
	return ((ssize_t)mock.prog_ret);
}

static int
mock_clock_nanosleep(clockid_t clock_id, int flags,
    const struct timespec *rqtp, struct timespec *rmtp)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_CLOCK_NANOSLEEP;
	mock.clock_id = clock_id;
	mock.cn_flags = flags;
	mock.rqtp_is_null = (rqtp == NULL);
	mock.rmtp_is_null = (rmtp == NULL);
	if (rqtp != NULL)
		mock_digest(rqtp, sizeof(*rqtp));
	if (rmtp != NULL) {
		rmtp->tv_sec = (time_t)(mock.prog_ret ^ 0x1111);
		rmtp->tv_nsec = (long)(mock.prog_ret ^ 0x2222);
	}
	return ((int)mock.prog_ret);
}

static int
mock_ppoll(struct pollfd pfd[], nfds_t nfds,
    const struct timespec *timeout, const sigset_t *newsigmask)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_PPOLL;
	mock.nfds = nfds;
	mock.timeout_is_null = (timeout == NULL);
	mock.sigmask_is_null = (newsigmask == NULL);
	if (timeout != NULL)
		mock_digest(timeout, sizeof(*timeout));
	if (newsigmask != NULL)
		mock_digest(newsigmask, sizeof(*newsigmask));

	if (pfd != NULL) {
		for (nfds_t i = 0; i < nfds && i < PFD_MAX; i++)
			pfd[i].revents = (short)((mock.prog_ret + (long long)i) & 0xffff);
	}
	return ((int)mock.prog_ret);
}

static void
install_ref_mocks(void)
{
	ref_set_interpos(INTERPOS_wait6, (interpos_func_t)mock_wait6);
	ref_set_interpos(INTERPOS_sendto, (interpos_func_t)mock_sendto);
	ref_set_interpos(INTERPOS_clock_nanosleep,
	    (interpos_func_t)mock_clock_nanosleep);
	ref_set_interpos(INTERPOS_ppoll, (interpos_func_t)mock_ppoll);
}

static void
install_port_mocks(void)
{
	port::set_interpos(INTERPOS_wait6, (interpos_func_t)mock_wait6);
	port::set_interpos(INTERPOS_sendto, (interpos_func_t)mock_sendto);
	port::set_interpos(INTERPOS_clock_nanosleep,
	    (interpos_func_t)mock_clock_nanosleep);
	port::set_interpos(INTERPOS_ppoll, (interpos_func_t)mock_ppoll);
}

struct Snap {
	unsigned long long	ncalls;
	int			tag;
	idtype_t		idtype;
	id_t			id;
	int			options;
	int			status_is_null;
	int			ru_is_null;
	int			infop_is_null;
	int			s;
	size_t			len;
	int			flags;
	socklen_t		tolen;
	const void		*to_ptr;
	clockid_t		clock_id;
	int			cn_flags;
	int			rqtp_is_null;
	int			rmtp_is_null;
	nfds_t			nfds;
	int			timeout_is_null;
	int			sigmask_is_null;
	unsigned char		wdigest[DIGEST_MAX];
	size_t			wdigest_len;
};

static Snap
take_snap(void)
{
	Snap s;

	memset(&s, 0, sizeof(s));
	s.ncalls = mock.ncalls;
	s.tag = mock.tag;
	s.idtype = mock.idtype;
	s.id = mock.id;
	s.options = mock.options;
	s.status_is_null = mock.status_is_null;
	s.ru_is_null = mock.ru_is_null;
	s.infop_is_null = mock.infop_is_null;
	s.s = mock.s;
	s.len = mock.len;
	s.flags = mock.flags;
	s.tolen = mock.tolen;
	s.to_ptr = mock.to_ptr;
	s.clock_id = mock.clock_id;
	s.cn_flags = mock.cn_flags;
	s.rqtp_is_null = mock.rqtp_is_null;
	s.rmtp_is_null = mock.rmtp_is_null;
	s.nfds = mock.nfds;
	s.timeout_is_null = mock.timeout_is_null;
	s.sigmask_is_null = mock.sigmask_is_null;
	s.wdigest_len = mock.wdigest_len;
	memcpy(s.wdigest, mock.wdigest, DIGEST_MAX);
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
	    a.id != b.id || a.options != b.options ||
	    a.status_is_null != b.status_is_null ||
	    a.ru_is_null != b.ru_is_null ||
	    a.infop_is_null != b.infop_is_null || a.s != b.s ||
	    a.len != b.len || a.flags != b.flags || a.tolen != b.tolen ||
	    a.to_ptr != b.to_ptr || a.clock_id != b.clock_id ||
	    a.cn_flags != b.cn_flags || a.rqtp_is_null != b.rqtp_is_null ||
	    a.rmtp_is_null != b.rmtp_is_null || a.nfds != b.nfds ||
	    a.timeout_is_null != b.timeout_is_null ||
	    a.sigmask_is_null != b.sigmask_is_null ||
	    a.wdigest_len != b.wdigest_len ||
	    memcmp(a.wdigest, b.wdigest, DIGEST_MAX) != 0) {
		char msg[640];

		snprintf(msg, sizeof(msg),
		    "%s ref={tag=%d idt=%d id=%llu opt=%d s=%d len=%zu fl=%d "
		    "clk=%d cnfl=%d nfds=%llu wdl=%zu} "
		    "port={tag=%d idt=%d id=%llu opt=%d s=%d len=%zu fl=%d "
		    "clk=%d cnfl=%d nfds=%llu wdl=%zu}", ctx,
		    a.tag, (int)a.idtype, (unsigned long long)a.id, a.options,
		    a.s, a.len, a.flags, (int)a.clock_id, a.cn_flags,
		    (unsigned long long)a.nfds, a.wdigest_len,
		    b.tag, (int)b.idtype, (unsigned long long)b.id, b.options,
		    b.s, b.len, b.flags, (int)b.clock_id, b.cn_flags,
		    (unsigned long long)b.nfds, b.wdigest_len);
		fail(fn, "mock", msg);
		return (false);
	}
	return (true);
}

static void
fill_buf(unsigned char *buf, size_t off, size_t len, unsigned seed)
{
	for (size_t i = 0; i < BUF_TOTAL; i++)
		buf[i] = GUARD;
	for (size_t i = 0; i < len; i++) {
		unsigned v = (seed + (unsigned)i * 17u) & 0xffu;

		if ((i & 3u) == 0u)
			v = 0;
		buf[off + i] = (unsigned char)v;
	}
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

static unsigned
rnd_u32(void)
{
	return (rng_u32());
}

static long long
rnd_ret(void)
{
	uint32_t u = rng_u32();

	return ((long long)(int32_t)u);
}

static size_t
rnd_size(void)
{
	return ((size_t)rng_u32());
}

static bool
case_wait6(idtype_t idtype, id_t id, int options, int use_status, int use_ru,
    int use_infop, long long ret)
{
	unsigned char st_a[sizeof(int) + 32], st_b[sizeof(int) + 32];
	unsigned char ru_buf_a[128], ru_buf_b[128];
	unsigned char info_buf_a[sizeof(siginfo_t) + 32];
	unsigned char info_buf_b[sizeof(siginfo_t) + 32];
	int *status_a, *status_b;
	struct __wrusage *ru_a, *ru_b;
	siginfo_t *info_a, *info_b;
	Snap sa, sb;
	pid_t ra, rb;
	char ctx[320];

	fn_cases[FN_WAIT6]++;

	memset(st_a, GUARD, sizeof(st_a));
	memset(st_b, GUARD, sizeof(st_b));
	memset(ru_buf_a, GUARD, sizeof(ru_buf_a));
	memset(ru_buf_b, GUARD, sizeof(ru_buf_b));
	memset(info_buf_a, GUARD, sizeof(info_buf_a));
	memset(info_buf_b, GUARD, sizeof(info_buf_b));

	status_a = use_status ? (int *)(st_a + 16) : NULL;
	status_b = use_status ? (int *)(st_b + 16) : NULL;
	ru_a = use_ru ? (struct __wrusage *)(ru_buf_a + 16) : NULL;
	ru_b = use_ru ? (struct __wrusage *)(ru_buf_b + 16) : NULL;
	info_a = use_infop ? (siginfo_t *)(info_buf_a + 16) : NULL;
	info_b = use_infop ? (siginfo_t *)(info_buf_b + 16) : NULL;

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_wait6(idtype, id, status_a, options, ru_a, info_a);
	sa = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::wait6(idtype, id, status_b, options, ru_b, info_b);
	sb = take_snap();

	snprintf(ctx, sizeof(ctx),
	    "idt=%d id=%llu opt=%d st=%d ru=%d info=%d ret=%lld",
	    (int)idtype, (unsigned long long)id, options, use_status, use_ru,
	    use_infop, ret);
	cmp_snap(FN_WAIT6, sa, sb, ctx);

	if (ra != rb) {
		char msg[384];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, (int)ra,
		    (int)rb);
		fail(FN_WAIT6, "return", msg);
		return (false);
	}
	if (use_status && memcmp(st_a, st_b, sizeof(st_a)) != 0) {
		fail(FN_WAIT6, "status_buf", ctx);
		return (false);
	}
	if (use_ru && memcmp(ru_buf_a, ru_buf_b, sizeof(ru_buf_a)) != 0) {
		fail(FN_WAIT6, "ru_buf", ctx);
		return (false);
	}
	if (use_infop && memcmp(info_buf_a, info_buf_b, sizeof(info_buf_a)) != 0) {
		fail(FN_WAIT6, "info_buf", ctx);
		return (false);
	}
	return (true);
}

static struct sockaddr_in sock_storage;

static bool
case_sendto(int s, size_t len, int flags, int use_to, socklen_t tolen,
    long long ret, unsigned pat_seed)
{
	unsigned char buf_a[BUF_TOTAL], buf_b[BUF_TOTAL];
	unsigned char to_a[sizeof(struct sockaddr_in) + 32];
	unsigned char to_b[sizeof(struct sockaddr_in) + 32];
	struct sockaddr *to_ref, *to_port;
	Snap sa, sb;
	ssize_t ra, rb;
	char ctx[320];

	fn_cases[FN_SENDTO]++;

	fill_buf(buf_a, DATA_OFF, BUF_TOTAL - DATA_OFF, pat_seed);
	fill_buf(buf_b, DATA_OFF, BUF_TOTAL - DATA_OFF, pat_seed);
	memset(to_a, GUARD, sizeof(to_a));
	memset(to_b, GUARD, sizeof(to_b));
	memcpy(to_a + 16, &sock_storage, sizeof(sock_storage));
	memcpy(to_b + 16, &sock_storage, sizeof(sock_storage));
	to_ref = use_to ? (struct sockaddr *)(to_a + 16) : NULL;
	to_port = use_to ? (struct sockaddr *)(to_b + 16) : NULL;

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_sendto(s, buf_a + DATA_OFF, len, flags, to_ref, tolen);
	sa = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::sendto(s, buf_b + DATA_OFF, len, flags, to_port, tolen);
	sb = take_snap();

	snprintf(ctx, sizeof(ctx),
	    "s=%d len=%zu fl=%d to=%d tolen=%u ret=%lld seed=%u", s, len,
	    flags, use_to, (unsigned)tolen, ret, pat_seed);
	cmp_snap(FN_SENDTO, sa, sb, ctx);

	if (ra != rb) {
		char msg[384];

		snprintf(msg, sizeof(msg), "%s ref=%lld port=%lld", ctx,
		    (long long)ra, (long long)rb);
		fail(FN_SENDTO, "return", msg);
		return (false);
	}
	if (memcmp(buf_a, buf_b, BUF_TOTAL) != 0) {
		fail(FN_SENDTO, "msg_buf", ctx);
		return (false);
	}
	if (use_to && memcmp(to_a, to_b, sizeof(to_a)) != 0) {
		fail(FN_SENDTO, "to_buf", ctx);
		return (false);
	}
	return (true);
}

static bool
case_clock_nanosleep(clockid_t clock_id, int flags, int use_rqtp, int use_rmtp,
    long long ret, unsigned pat_seed)
{
	unsigned char rq_a[sizeof(struct timespec) + 32];
	unsigned char rq_b[sizeof(struct timespec) + 32];
	unsigned char rm_a[sizeof(struct timespec) + 32];
	unsigned char rm_b[sizeof(struct timespec) + 32];
	const struct timespec *rq_ref, *rq_port;
	struct timespec *rm_ref, *rm_port;
	Snap sa, sb;
	int ra, rb;
	char ctx[320];

	fn_cases[FN_CLOCK_NANOSLEEP]++;

	memset(rq_a, GUARD, sizeof(rq_a));
	memset(rq_b, GUARD, sizeof(rq_b));
	memset(rm_a, GUARD, sizeof(rm_a));
	memset(rm_b, GUARD, sizeof(rm_b));
	if (use_rqtp) {
		struct timespec *rq = (struct timespec *)(rq_a + 16);

		rq->tv_sec = (time_t)(pat_seed ^ 0x1234);
		rq->tv_nsec = (long)(pat_seed ^ 0x5678);
		rq = (struct timespec *)(rq_b + 16);
		rq->tv_sec = (time_t)(pat_seed ^ 0x1234);
		rq->tv_nsec = (long)(pat_seed ^ 0x5678);
	}
	rq_ref = use_rqtp ? (const struct timespec *)(rq_a + 16) : NULL;
	rq_port = use_rqtp ? (const struct timespec *)(rq_b + 16) : NULL;
	rm_ref = use_rmtp ? (struct timespec *)(rm_a + 16) : NULL;
	rm_port = use_rmtp ? (struct timespec *)(rm_b + 16) : NULL;

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_clock_nanosleep(clock_id, flags, rq_ref, rm_ref);
	sa = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::clock_nanosleep(clock_id, flags, rq_port, rm_port);
	sb = take_snap();

	snprintf(ctx, sizeof(ctx),
	    "clk=%d fl=%d rq=%d rm=%d ret=%lld seed=%u", (int)clock_id, flags,
	    use_rqtp, use_rmtp, ret, pat_seed);
	cmp_snap(FN_CLOCK_NANOSLEEP, sa, sb, ctx);

	if (ra != rb) {
		char msg[384];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_CLOCK_NANOSLEEP, "return", msg);
		return (false);
	}
	if (use_rmtp && memcmp(rm_a, rm_b, sizeof(rm_a)) != 0) {
		fail(FN_CLOCK_NANOSLEEP, "rmtp_buf", ctx);
		return (false);
	}
	return (true);
}

static bool
case_ppoll(nfds_t nfds, int use_timeout, int use_sigmask, long long ret,
    unsigned pat_seed)
{
	unsigned char pfd_a[(PFD_MAX + 2) * sizeof(struct pollfd)];
	unsigned char pfd_b[(PFD_MAX + 2) * sizeof(struct pollfd)];
	unsigned char ts_a[sizeof(struct timespec) + 32];
	unsigned char ts_b[sizeof(struct timespec) + 32];
	unsigned char sig_a[sizeof(sigset_t) + 32];
	unsigned char sig_b[sizeof(sigset_t) + 32];
	struct pollfd *pf_ref, *pf_port;
	const struct timespec *ts_ref, *ts_port;
	const sigset_t *sig_ref, *sig_port;
	Snap sa, sb;
	int ra, rb;
	char ctx[320];

	fn_cases[FN_PPOLL]++;

	memset(pfd_a, GUARD, sizeof(pfd_a));
	memset(pfd_b, GUARD, sizeof(pfd_b));
	memset(ts_a, GUARD, sizeof(ts_a));
	memset(ts_b, GUARD, sizeof(ts_b));
	memset(sig_a, GUARD, sizeof(sig_a));
	memset(sig_b, GUARD, sizeof(sig_b));

	pf_ref = (struct pollfd *)(pfd_a + sizeof(struct pollfd));
	pf_port = (struct pollfd *)(pfd_b + sizeof(struct pollfd));
	for (nfds_t i = 0; i < nfds && i < PFD_MAX; i++) {
		pf_ref[i].fd = (int)(pat_seed + i);
		pf_ref[i].events = (short)((pat_seed >> (i & 3u)) & 0xffff);
		pf_ref[i].revents = 0;
		pf_port[i].fd = pf_ref[i].fd;
		pf_port[i].events = pf_ref[i].events;
		pf_port[i].revents = 0;
	}
	if (use_timeout) {
		struct timespec *ts = (struct timespec *)(ts_a + 16);

		ts->tv_sec = (time_t)(pat_seed ^ 0xabcd);
		ts->tv_nsec = (long)(pat_seed ^ 0xef01);
		ts = (struct timespec *)(ts_b + 16);
		ts->tv_sec = (time_t)(pat_seed ^ 0xabcd);
		ts->tv_nsec = (long)(pat_seed ^ 0xef01);
	}
	ts_ref = use_timeout ? (const struct timespec *)(ts_a + 16) : NULL;
	ts_port = use_timeout ? (const struct timespec *)(ts_b + 16) : NULL;
	sig_ref = use_sigmask ? (const sigset_t *)(sig_a + 16) : NULL;
	sig_port = use_sigmask ? (const sigset_t *)(sig_b + 16) : NULL;
	if (use_sigmask) {
		memset((void *)(sig_a + 16), (int)(pat_seed & 0xff),
		    sizeof(sigset_t));
		memset((void *)(sig_b + 16), (int)(pat_seed & 0xff),
		    sizeof(sigset_t));
	}

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_ppoll(pf_ref, nfds, ts_ref, sig_ref);
	sa = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::ppoll(pf_port, nfds, ts_port, sig_port);
	sb = take_snap();

	snprintf(ctx, sizeof(ctx),
	    "nfds=%llu to=%d sig=%d ret=%lld seed=%u",
	    (unsigned long long)nfds, use_timeout, use_sigmask, ret, pat_seed);
	cmp_snap(FN_PPOLL, sa, sb, ctx);

	if (ra != rb) {
		char msg[384];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_PPOLL, "return", msg);
		return (false);
	}
	if (memcmp(pfd_a, pfd_b, sizeof(pfd_a)) != 0) {
		fail(FN_PPOLL, "pfd_buf", ctx);
		return (false);
	}
	if (use_timeout && memcmp(ts_a, ts_b, sizeof(ts_a)) != 0) {
		fail(FN_PPOLL, "timeout_buf", ctx);
		return (false);
	}
	if (use_sigmask && memcmp(sig_a, sig_b, sizeof(sig_a)) != 0) {
		fail(FN_PPOLL, "sigmask_buf", ctx);
		return (false);
	}
	return (true);
}

static void
test_wait6(void)
{
	static const idtype_t idtypes[] = {
		P_ALL, P_PID, P_PGID, (idtype_t)0, (idtype_t)1,
		(idtype_t)0x7f, (idtype_t)0x80, (idtype_t)INT_MAX,
	};
	static const id_t ids[] = {
		(id_t)0, (id_t)1, (id_t)INT_MAX, (id_t)UINT_MAX,
	};
	static const int opts[] = {
		0, WNOHANG, WUNTRACED, -1, INT_MAX, 0x7f, 0x80,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t t = 0; t < sizeof(idtypes) / sizeof(idtypes[0]); t++)
		for (size_t i = 0; i < sizeof(ids) / sizeof(ids[0]); i++)
			for (size_t o = 0; o < sizeof(opts) / sizeof(opts[0]); o++)
				for (int st = 0; st < 2; st++)
					for (int ru = 0; ru < 2; ru++)
						for (int info = 0; info < 2; info++)
							for (size_t r = 0;
							    r < sizeof(rets) /
							    sizeof(rets[0]); r++)
								case_wait6(
								    idtypes[t],
								    ids[i],
								    opts[o], st,
								    ru, info,
								    rets[r]);

	rng_seed(0x77'61'69'74'36ULL);
	for (int i = 0; i < 200000; i++)
		case_wait6((idtype_t)rnd_int(), (id_t)rnd_u32(), rnd_int(),
		    rnd_int() & 1, rnd_int() & 1, rnd_int() & 1, rnd_ret());
}

static void
test_sendto(void)
{
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 0x7f, 0x80, 0xff, INT_MAX,
	};
	static const size_t lens[] = {
		0, 1, 2, DATA_OFF, DATA_CAP,
	};
	static const int flags[] = {
		0, MSG_OOB, MSG_DONTWAIT, -1, INT_MAX, 0x7f, 0x80,
	};
	static const socklen_t tolen[] = {
		0, sizeof(struct sockaddr_in), (socklen_t)INT_MAX,
	};
	static const unsigned seeds[] = {
		0, 1, 0x7f, 0x80, 0xff, 0xdeadbeef,
	};
	static const long long rets[] = {
		-1, 0, 1, SSIZE_MAX, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	memset(&sock_storage, 0, sizeof(sock_storage));
	sock_storage.sin_family = AF_INET;
	sock_storage.sin_port = 0x1234;

	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t l = 0; l < sizeof(lens) / sizeof(lens[0]); l++)
			for (size_t f = 0; f < sizeof(flags) / sizeof(flags[0]); f++)
				for (int to = 0; to < 2; to++)
					for (size_t tl = 0;
					    tl < sizeof(tolen) / sizeof(tolen[0]);
					    tl++)
						for (size_t sd = 0;
						    sd < sizeof(seeds) /
						    sizeof(seeds[0]); sd++)
							for (size_t r = 0;
							    r < sizeof(rets) /
							    sizeof(rets[0]); r++)
								case_sendto(
								    socks[s],
								    lens[l],
								    flags[f], to,
								    tolen[tl],
								    rets[r],
								    seeds[sd]);

	case_sendto(0, (size_t)SIZE_MAX, 0, 1,
	    sizeof(struct sockaddr_in), -1, 0x80);
	case_sendto(1, (size_t)SIZE_MAX, MSG_OOB, 0, 0, 0, 0xff);

	rng_seed(0x73'65'6e'64ULL);
	for (int i = 0; i < 200000; i++)
		case_sendto(rnd_int(), rnd_size() & (BUF_TOTAL - 1), rnd_int(),
		    rnd_int() & 1, (socklen_t)rnd_u32(), rnd_ret(), rnd_u32());
}

static void
test_clock_nanosleep(void)
{
	static const clockid_t clocks[] = {
		CLOCK_REALTIME, CLOCK_MONOTONIC, (clockid_t)-1,
		(clockid_t)INT_MAX, (clockid_t)0x7f, (clockid_t)0x80,
	};
	static const int flags[] = {
		0, TIMER_ABSTIME, -1, INT_MAX, 0x7f, 0x80,
	};
	static const unsigned seeds[] = {
		0, 1, 0x7f, 0x80, 0xff, 0xcafebabe,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t c = 0; c < sizeof(clocks) / sizeof(clocks[0]); c++)
		for (size_t f = 0; f < sizeof(flags) / sizeof(flags[0]); f++)
			for (int rq = 0; rq < 2; rq++)
				for (int rm = 0; rm < 2; rm++)
					for (size_t s = 0;
					    s < sizeof(seeds) / sizeof(seeds[0]);
					    s++)
						for (size_t r = 0;
						    r < sizeof(rets) /
						    sizeof(rets[0]); r++)
							case_clock_nanosleep(
							    clocks[c], flags[f],
							    rq, rm, rets[r],
							    seeds[s]);

	rng_seed(0x63'6c'6f'63'6bULL);
	for (int i = 0; i < 200000; i++)
		case_clock_nanosleep((clockid_t)rnd_int(), rnd_int(),
		    rnd_int() & 1, rnd_int() & 1, rnd_ret(), rnd_u32());
}

static void
test_ppoll(void)
{
	static const nfds_t nfds_vals[] = {
		0, 1, 2, PFD_MAX - 1, PFD_MAX, PFD_MAX + 1, (nfds_t)INT_MAX,
	};
	static const unsigned seeds[] = {
		0, 1, 0x7f, 0x80, 0xff, 0xfeedface,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t n = 0; n < sizeof(nfds_vals) / sizeof(nfds_vals[0]); n++)
		for (int to = 0; to < 2; to++)
			for (int sig = 0; sig < 2; sig++)
				for (size_t s = 0;
				    s < sizeof(seeds) / sizeof(seeds[0]); s++)
					for (size_t r = 0;
					    r < sizeof(rets) / sizeof(rets[0]);
					    r++)
						case_ppoll(nfds_vals[n], to, sig,
						    rets[r], seeds[s]);

	rng_seed(0x70'70'6f'6c'6cULL);
	for (int i = 0; i < 200000; i++)
		case_ppoll((nfds_t)(rnd_u32() & 0x1f), rnd_int() & 1,
		    rnd_int() & 1, rnd_ret(), rnd_u32());
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
