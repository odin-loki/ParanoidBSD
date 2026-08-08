/*
 * harness.cpp -- differential test for PBSD batch b0096.
 *
 * recvmsg, accept4, pselect and aio_suspend are libc interposition wrappers:
 * each loads a function pointer from the interposing table and tail-calls it
 * with the original argument list.  The harness installs instrumented mocks in
 * both the oracle (ref_*) and port tables, then compares the mock-visible
 * syscall tag, every forwarded argument and return value.  For recvmsg and
 * accept4 the mock also writes through caller-supplied buffers; those buffers
 * are allocated with 0x7f guard bytes and compared in their entirety.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/aio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_recvmsg,
	INTERPOS_accept4,
	INTERPOS_pselect,
	INTERPOS_aio_suspend,
	INTERPOS_MAX
};

#define	__ssp_real_(fun)	fun
#define	__ssp_real(fun)		__ssp_real_(fun)
#define	ref___ssp_real_(fun)	ref_##fun
#define	ref___ssp_real(fun)	ref___ssp_real_(fun)

ssize_t ref___ssp_real(recvmsg)(int s, struct msghdr *msg, int flags);
int ref_accept4(int s, struct sockaddr *addr, socklen_t *addrlen, int flags);
int ref_pselect(int n, fd_set *rs, fd_set *ws, fd_set *es,
    const struct timespec *t, const sigset_t *s);
int ref_aio_suspend(const struct aiocb * const iocbs[], int niocb,
    const struct timespec *timeout);

void ref_set_interpos(int slot, interpos_func_t func);

}

import pbsd.lib.libc.sys.b0096;

namespace port = pbsd::lib_libc_sys::b0096;

#define	GUARD		0x7f
#define	BUF_TOTAL	512
#define	DATA_OFF	64
#define	DATA_CAP	(BUF_TOTAL - DATA_OFF)
#define	ADDR_TOTAL	256
#define	ADDR_OFF	32
#define	ADDR_CAP	(ADDR_TOTAL - ADDR_OFF)
#define	FDSET_BYTES	(sizeof(fd_set) + 64)
#define	DIGEST_MAX	64
#define	IOV_MAX		4

#define	PBSD_TAG_NONE		0
#define	PBSD_TAG_RECVMSG	1
#define	PBSD_TAG_ACCEPT4	2
#define	PBSD_TAG_PSELECT	3
#define	PBSD_TAG_AIO_SUSPEND	4

struct MockState {
	unsigned long long	ncalls;
	int			tag;
	int			s;
	int			flags;
	struct msghdr		*msg;
	int			msg_name_is_null;
	int			msg_iovlen;
	size_t			msg_iov_cap[IOV_MAX];
	int			n;
	fd_set			*rs;
	fd_set			*ws;
	fd_set			*es;
	const struct timespec	*t;
	const sigset_t		*sigmask;
	int			rs_is_null;
	int			ws_is_null;
	int			es_is_null;
	int			t_is_null;
	int			sigmask_is_null;
	const struct aiocb * const *iocbs;
	int			niocb;
	const struct timespec	*timeout;
	int			timeout_is_null;
	int			addr_is_null;
	int			addrlen_is_null;
	socklen_t		addrlen_out;
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

static ssize_t
mock_recvmsg(int s, struct msghdr *msg, int flags)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_RECVMSG;
	mock.s = s;
	mock.flags = flags;
	mock.msg = msg;

	if (msg != NULL) {
		mock.msg_name_is_null = (msg->msg_name == NULL);
		mock.msg_iovlen = (int)msg->msg_iovlen;
		if (msg->msg_iovlen > IOV_MAX)
			mock.msg_iovlen = IOV_MAX;
		for (int i = 0; i < mock.msg_iovlen; i++) {
			mock.msg_iov_cap[i] = msg->msg_iov[i].iov_len;
			mock_digest(msg->msg_iov[i].iov_base, msg->msg_iov[i].iov_len);
		}
	}

	ssize_t ret = (ssize_t)mock.prog_ret;

	if (ret > 0 && msg != NULL && msg->msg_iov != NULL) {
		size_t left = (size_t)ret;

		for (size_t i = 0; i < msg->msg_iovlen && left > 0; i++) {
			unsigned char *p = (unsigned char *)msg->msg_iov[i].iov_base;
			size_t cap = msg->msg_iov[i].iov_len;
			size_t n = left;

			if (n > cap)
				n = cap;
			for (size_t j = 0; j < n; j++)
				p[j] = (unsigned char)(0x80u |
				    ((s + (int)i + (int)j) & 0x7f));
			left -= n;
		}
	}
	if (ret > 0 && msg != NULL && msg->msg_name != NULL && msg->msg_namelen > 0) {
		unsigned char *p = (unsigned char *)msg->msg_name;
		size_t n = msg->msg_namelen;

		if (n > ADDR_CAP)
			n = ADDR_CAP;
		for (size_t j = 0; j < n; j++)
			p[j] = (unsigned char)(0xa0u | ((s + (int)j) & 0x0f));
	}
	return (ret);
}

static int
mock_accept4(int s, struct sockaddr *addr, socklen_t *addrlen, int flags)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_ACCEPT4;
	mock.s = s;
	mock.flags = flags;
	mock.addr_is_null = (addr == NULL);
	mock.addrlen_is_null = (addrlen == NULL);
	mock.addrlen_out = (socklen_t)(mock.prog_ret & 0xff);

	if (addrlen != NULL)
		*addrlen = mock.addrlen_out;
	if (addr != NULL && mock.addrlen_out > 0) {
		size_t n = mock.addrlen_out;

		if (n > ADDR_CAP)
			n = ADDR_CAP;
		unsigned char *p = (unsigned char *)addr;

		for (size_t j = 0; j < n; j++)
			p[j] = (unsigned char)(0xc0u | ((s + (int)j) & 0x1f));
	}
	return ((int)mock.prog_ret);
}

static int
mock_pselect(int n, fd_set *rs, fd_set *ws, fd_set *es,
    const struct timespec *t, const sigset_t *sigmask)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_PSELECT;
	mock.n = n;
	mock.rs = rs;
	mock.ws = ws;
	mock.es = es;
	mock.t = t;
	mock.sigmask = sigmask;
	mock.rs_is_null = (rs == NULL);
	mock.ws_is_null = (ws == NULL);
	mock.es_is_null = (es == NULL);
	mock.t_is_null = (t == NULL);
	mock.sigmask_is_null = (sigmask == NULL);

	if (rs != NULL)
		FD_SET(n & (FD_SETSIZE - 1), rs);
	if (ws != NULL && n > 0)
		FD_CLR((n - 1) & (FD_SETSIZE - 1), ws);
	if (es != NULL)
		FD_SET((n ^ 1) & (FD_SETSIZE - 1), es);

	return ((int)mock.prog_ret);
}

static int
mock_aio_suspend(const struct aiocb * const iocbs[], int niocb,
    const struct timespec *timeout)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_AIO_SUSPEND;
	mock.iocbs = iocbs;
	mock.niocb = niocb;
	mock.timeout = timeout;
	mock.timeout_is_null = (timeout == NULL);

	return ((int)mock.prog_ret);
}

static void
install_ref_mocks(void)
{
	ref_set_interpos(INTERPOS_recvmsg, (interpos_func_t)mock_recvmsg);
	ref_set_interpos(INTERPOS_accept4, (interpos_func_t)mock_accept4);
	ref_set_interpos(INTERPOS_pselect, (interpos_func_t)mock_pselect);
	ref_set_interpos(INTERPOS_aio_suspend,
	    (interpos_func_t)mock_aio_suspend);
}

static void
install_port_mocks(void)
{
	port::set_interpos(INTERPOS_recvmsg, (interpos_func_t)mock_recvmsg);
	port::set_interpos(INTERPOS_accept4, (interpos_func_t)mock_accept4);
	port::set_interpos(INTERPOS_pselect, (interpos_func_t)mock_pselect);
	port::set_interpos(INTERPOS_aio_suspend,
	    (interpos_func_t)mock_aio_suspend);
}

struct Snap {
	unsigned long long	ncalls;
	int			tag;
	int			s;
	int			flags;
	struct msghdr		*msg;
	int			msg_name_is_null;
	int			msg_iovlen;
	size_t			msg_iov_cap[IOV_MAX];
	int			n;
	fd_set			*rs;
	fd_set			*ws;
	fd_set			*es;
	const struct timespec	*t;
	const sigset_t		*sigmask;
	int			rs_is_null;
	int			ws_is_null;
	int			es_is_null;
	int			t_is_null;
	int			sigmask_is_null;
	const struct aiocb * const *iocbs;
	int			niocb;
	const struct timespec	*timeout;
	int			timeout_is_null;
	int			addr_is_null;
	int			addrlen_is_null;
	socklen_t		addrlen_out;
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
	s.s = mock.s;
	s.flags = mock.flags;
	s.msg = mock.msg;
	s.msg_name_is_null = mock.msg_name_is_null;
	s.msg_iovlen = mock.msg_iovlen;
	memcpy(s.msg_iov_cap, mock.msg_iov_cap, sizeof(s.msg_iov_cap));
	s.n = mock.n;
	s.rs = mock.rs;
	s.ws = mock.ws;
	s.es = mock.es;
	s.t = mock.t;
	s.sigmask = mock.sigmask;
	s.rs_is_null = mock.rs_is_null;
	s.ws_is_null = mock.ws_is_null;
	s.es_is_null = mock.es_is_null;
	s.t_is_null = mock.t_is_null;
	s.sigmask_is_null = mock.sigmask_is_null;
	s.iocbs = mock.iocbs;
	s.niocb = mock.niocb;
	s.timeout = mock.timeout;
	s.timeout_is_null = mock.timeout_is_null;
	s.addr_is_null = mock.addr_is_null;
	s.addrlen_is_null = mock.addrlen_is_null;
	s.addrlen_out = mock.addrlen_out;
	s.wdigest_len = mock.wdigest_len;
	memcpy(s.wdigest, mock.wdigest, DIGEST_MAX);
	return (s);
}

enum {
	FN_RECVMSG,
	FN_ACCEPT4,
	FN_PSELECT,
	FN_AIO_SUSPEND,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"recvmsg",
	"accept4",
	"pselect",
	"aio_suspend",
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
		printf("  FAIL %-12s %-18s %s\n", fn_name[fn], what, detail);
	} else if (fn_reported[fn] == MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-12s ... further failures suppressed\n",
		    fn_name[fn]);
	}
	return (false);
}

static bool
cmp_snap(int fn, const Snap &a, const Snap &b, const char *ctx)
{
	if (a.ncalls != b.ncalls || a.tag != b.tag || a.s != b.s ||
	    a.flags != b.flags || a.msg != b.msg ||
	    a.msg_name_is_null != b.msg_name_is_null ||
	    a.msg_iovlen != b.msg_iovlen ||
	    memcmp(a.msg_iov_cap, b.msg_iov_cap, sizeof(a.msg_iov_cap)) != 0 ||
	    a.n != b.n || a.rs != b.rs || a.ws != b.ws || a.es != b.es ||
	    a.t != b.t || a.sigmask != b.sigmask ||
	    a.rs_is_null != b.rs_is_null || a.ws_is_null != b.ws_is_null ||
	    a.es_is_null != b.es_is_null || a.t_is_null != b.t_is_null ||
	    a.sigmask_is_null != b.sigmask_is_null || a.iocbs != b.iocbs ||
	    a.niocb != b.niocb || a.timeout != b.timeout ||
	    a.timeout_is_null != b.timeout_is_null ||
	    a.addr_is_null != b.addr_is_null ||
	    a.addrlen_is_null != b.addrlen_is_null ||
	    a.addrlen_out != b.addrlen_out ||
	    a.wdigest_len != b.wdigest_len ||
	    memcmp(a.wdigest, b.wdigest, DIGEST_MAX) != 0) {
		char msg[640];

		snprintf(msg, sizeof(msg),
		    "%s ref={tag=%d s=%d fl=%d iov=%d n=%d niocb=%d} "
		    "port={tag=%d s=%d fl=%d iov=%d n=%d niocb=%d}", ctx,
		    a.tag, a.s, a.flags, a.msg_iovlen, a.n, a.niocb,
		    b.tag, b.s, b.flags, b.msg_iovlen, b.n, b.niocb);
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

static void
fill_addr(unsigned char *buf, size_t off, size_t len, unsigned seed)
{
	for (size_t i = 0; i < ADDR_TOTAL; i++)
		buf[i] = GUARD;
	for (size_t i = 0; i < len; i++)
		buf[off + i] = (unsigned char)((seed + (unsigned)i * 23u) & 0xffu);
}

static void
fill_fdset(unsigned char *buf)
{
	memset(buf, GUARD, FDSET_BYTES);
	memcpy(buf + 16, "\x00", 1);
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

static bool
case_recvmsg(int s, int flags, int niov, size_t iov_lens[IOV_MAX],
    int use_name, size_t namelen, long long ret)
{
	unsigned char iovbuf_a[IOV_MAX][BUF_TOTAL];
	unsigned char iovbuf_b[IOV_MAX][BUF_TOTAL];
	unsigned char name_a[ADDR_TOTAL], name_b[ADDR_TOTAL];
	struct iovec iova[IOV_MAX], iovb[IOV_MAX];
	struct msghdr ma, mb;
	Snap sa, sb;
	ssize_t ra, rb;
	char ctx[256];
	size_t n = (size_t)niov;

	if (n > IOV_MAX)
		n = IOV_MAX;

	fn_cases[FN_RECVMSG]++;

	memset(&ma, 0, sizeof(ma));
	memset(&mb, 0, sizeof(mb));

	for (size_t i = 0; i < n; i++) {
		size_t cap = iov_lens[i] & (BUF_TOTAL - 1);

		fill_buf(iovbuf_a[i], DATA_OFF, cap, (unsigned)(s ^ i));
		fill_buf(iovbuf_b[i], DATA_OFF, cap, (unsigned)(s ^ i));
		iova[i].iov_base = iovbuf_a[i] + DATA_OFF;
		iova[i].iov_len = cap;
		iovb[i].iov_base = iovbuf_b[i] + DATA_OFF;
		iovb[i].iov_len = cap;
	}
	ma.msg_iov = (n > 0) ? iova : NULL;
	ma.msg_iovlen = n;
	mb.msg_iov = (n > 0) ? iovb : NULL;
	mb.msg_iovlen = n;

	if (use_name) {
		fill_addr(name_a, ADDR_OFF, namelen, (unsigned)s);
		fill_addr(name_b, ADDR_OFF, namelen, (unsigned)s);
		ma.msg_name = name_a + ADDR_OFF;
		mb.msg_name = name_b + ADDR_OFF;
		ma.msg_namelen = namelen;
		mb.msg_namelen = namelen;
	}

	install_ref_mocks();
	mock_reset(ret);
	ra = ref___ssp_real(recvmsg)(s, &ma, flags);
	sa = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::__ssp_real(recvmsg)(s, &mb, flags);
	sb = take_snap();

	snprintf(ctx, sizeof(ctx), "s=%d fl=%d niov=%zu namelen=%zu ret=%lld",
	    s, flags, n, use_name ? namelen : 0, ret);
	cmp_snap(FN_RECVMSG, sa, sb, ctx);

	if (ra != rb) {
		char msg[288];

		snprintf(msg, sizeof(msg), "%s ref=%lld port=%lld", ctx,
		    (long long)ra, (long long)rb);
		fail(FN_RECVMSG, "return", msg);
		return (false);
	}
	for (size_t i = 0; i < n; i++) {
		if (memcmp(iovbuf_a[i], iovbuf_b[i], BUF_TOTAL) != 0) {
			fail(FN_RECVMSG, "iov_buf", ctx);
			return (false);
		}
	}
	if (use_name && memcmp(name_a, name_b, ADDR_TOTAL) != 0) {
		fail(FN_RECVMSG, "name_buf", ctx);
		return (false);
	}
	return (true);
}

static bool
case_accept4(int s, int flags, int use_addr, int use_addrlen, long long ret)
{
	unsigned char addr_a[ADDR_TOTAL], addr_b[ADDR_TOTAL];
	unsigned char len_a[sizeof(socklen_t) + 32];
	unsigned char len_b[sizeof(socklen_t) + 32];
	socklen_t *alen_a, *alen_b;
	struct sockaddr *sa_a, *sa_b;
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_ACCEPT4]++;

	fill_addr(addr_a, ADDR_OFF, ADDR_CAP, (unsigned)s);
	fill_addr(addr_b, ADDR_OFF, ADDR_CAP, (unsigned)s);
	memset(len_a, GUARD, sizeof(len_a));
	memset(len_b, GUARD, sizeof(len_b));

	sa_a = use_addr ? (struct sockaddr *)(addr_a + ADDR_OFF) : NULL;
	sa_b = use_addr ? (struct sockaddr *)(addr_b + ADDR_OFF) : NULL;
	alen_a = use_addrlen ? (socklen_t *)(len_a + 16) : NULL;
	alen_b = use_addrlen ? (socklen_t *)(len_b + 16) : NULL;

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_accept4(s, sa_a, alen_a, flags);
	snap_a = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::accept4(s, sa_b, alen_b, flags);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx), "s=%d fl=%d addr=%d alen=%d ret=%lld",
	    s, flags, use_addr, use_addrlen, ret);
	cmp_snap(FN_ACCEPT4, snap_a, snap_b, ctx);

	if (ra != rb) {
		char msg[288];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_ACCEPT4, "return", msg);
		return (false);
	}
	if (use_addr && memcmp(addr_a, addr_b, ADDR_TOTAL) != 0) {
		fail(FN_ACCEPT4, "addr_buf", ctx);
		return (false);
	}
	if (use_addrlen && memcmp(len_a, len_b, sizeof(len_a)) != 0) {
		fail(FN_ACCEPT4, "addrlen_buf", ctx);
		return (false);
	}
	return (true);
}

static bool
case_pselect(int n, int use_rs, int use_ws, int use_es, int use_t, int use_sig,
    long long ret)
{
	unsigned char rs_a[FDSET_BYTES], rs_b[FDSET_BYTES];
	unsigned char ws_a[FDSET_BYTES], ws_b[FDSET_BYTES];
	unsigned char es_a[FDSET_BYTES], es_b[FDSET_BYTES];
	unsigned char ts_a[sizeof(struct timespec) + 32];
	unsigned char ts_b[sizeof(struct timespec) + 32];
	unsigned char sig_a[sizeof(sigset_t) + 32];
	unsigned char sig_b[sizeof(sigset_t) + 32];
	fd_set *rsa, *rsb, *wsa, *wsb, *esa, *esb;
	const struct timespec *ta, *tb;
	const sigset_t *siga, *sigb;
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[320];

	fn_cases[FN_PSELECT]++;

	fill_fdset(rs_a);
	fill_fdset(rs_b);
	fill_fdset(ws_a);
	fill_fdset(ws_b);
	fill_fdset(es_a);
	fill_fdset(es_b);
	memset(ts_a, GUARD, sizeof(ts_a));
	memset(ts_b, GUARD, sizeof(ts_b));
	memset(sig_a, GUARD, sizeof(sig_a));
	memset(sig_b, GUARD, sizeof(sig_b));

	rsa = use_rs ? (fd_set *)(rs_a + 16) : NULL;
	rsb = use_rs ? (fd_set *)(rs_b + 16) : NULL;
	wsa = use_ws ? (fd_set *)(ws_a + 16) : NULL;
	wsb = use_ws ? (fd_set *)(ws_b + 16) : NULL;
	esa = use_es ? (fd_set *)(es_a + 16) : NULL;
	esb = use_es ? (fd_set *)(es_b + 16) : NULL;
	ta = use_t ? (const struct timespec *)(ts_a + 16) : NULL;
	tb = use_t ? (const struct timespec *)(ts_b + 16) : NULL;
	siga = use_sig ? (const sigset_t *)(sig_a + 16) : NULL;
	sigb = use_sig ? (const sigset_t *)(sig_b + 16) : NULL;

	if (use_t) {
		struct timespec *tp = (struct timespec *)(ts_a + 16);

		tp->tv_sec = n;
		tp->tv_nsec = 0;
		tp = (struct timespec *)(ts_b + 16);
		tp->tv_sec = n;
		tp->tv_nsec = 0;
	}

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_pselect(n, rsa, wsa, esa, ta, siga);
	snap_a = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::pselect(n, rsb, wsb, esb, tb, sigb);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx),
	    "n=%d rs=%d ws=%d es=%d t=%d sig=%d ret=%lld",
	    n, use_rs, use_ws, use_es, use_t, use_sig, ret);
	cmp_snap(FN_PSELECT, snap_a, snap_b, ctx);

	if (ra != rb) {
		char msg[320];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_PSELECT, "return", msg);
		return (false);
	}
	if (use_rs && memcmp(rs_a, rs_b, FDSET_BYTES) != 0) {
		fail(FN_PSELECT, "rs_buf", ctx);
		return (false);
	}
	if (use_ws && memcmp(ws_a, ws_b, FDSET_BYTES) != 0) {
		fail(FN_PSELECT, "ws_buf", ctx);
		return (false);
	}
	if (use_es && memcmp(es_a, es_b, FDSET_BYTES) != 0) {
		fail(FN_PSELECT, "es_buf", ctx);
		return (false);
	}
	return (true);
}

static struct aiocb aio_pool[IOV_MAX];

static bool
case_aio_suspend(int niocb, int use_timeout, long long ret)
{
	unsigned char ts_a[sizeof(struct timespec) + 32];
	unsigned char ts_b[sizeof(struct timespec) + 32];
	const struct aiocb *iocbs_a[IOV_MAX];
	const struct aiocb *iocbs_b[IOV_MAX];
	const struct timespec *ta, *tb;
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];
	int n = niocb;

	if (n < 0)
		n = 0;
	if (n > IOV_MAX)
		n = IOV_MAX;

	fn_cases[FN_AIO_SUSPEND]++;

	memset(ts_a, GUARD, sizeof(ts_a));
	memset(ts_b, GUARD, sizeof(ts_b));
	for (int i = 0; i < n; i++) {
		memset(&aio_pool[i], 0, sizeof(aio_pool[i]));
		iocbs_a[i] = &aio_pool[i];
		iocbs_b[i] = &aio_pool[i];
	}
	ta = use_timeout ? (const struct timespec *)(ts_a + 16) : NULL;
	tb = use_timeout ? (const struct timespec *)(ts_b + 16) : NULL;

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_aio_suspend(n > 0 ? iocbs_a : NULL, n, ta);
	snap_a = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::aio_suspend(n > 0 ? iocbs_b : NULL, n, tb);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx), "niocb=%d timeout=%d ret=%lld",
	    n, use_timeout, ret);
	cmp_snap(FN_AIO_SUSPEND, snap_a, snap_b, ctx);

	if (ra != rb) {
		char msg[288];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_AIO_SUSPEND, "return", msg);
		return (false);
	}
	return (true);
}

static void
test_recvmsg(void)
{
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 0x7f, 0x80, 0xff, INT_MAX,
	};
	static const int flags[] = {
		0, MSG_PEEK, MSG_DONTWAIT, -1, INT_MAX, 0x7f, 0x80,
	};
	static const size_t lens[] = {
		0, 1, 2, DATA_OFF - 1, DATA_OFF, DATA_CAP,
	};
	static const long long rets[] = {
		-1, 0, 1, 2, SSIZE_MAX, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t f = 0; f < sizeof(flags) / sizeof(flags[0]); f++)
			for (size_t niov = 0; niov <= IOV_MAX; niov++) {
				size_t iov_lens[IOV_MAX];

				for (size_t i = 0; i < IOV_MAX; i++)
					iov_lens[i] = lens[i % (sizeof(lens) /
					    sizeof(lens[0]))];
				for (size_t r = 0; r < sizeof(rets) /
				    sizeof(rets[0]); r++)
					case_recvmsg(socks[s], flags[f],
					    (int)niov, iov_lens, 0, 0,
					    rets[r]);
			}

	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t nl = 0; nl < sizeof(lens) / sizeof(lens[0]); nl++)
			for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]);
			    r++) {
				size_t iov_lens[IOV_MAX] = { DATA_CAP };

				case_recvmsg(socks[s], 0, 1, iov_lens, 1,
				    lens[nl], rets[r]);
			}

	case_recvmsg(0, 0, 0, NULL, 0, 0, -1);
	case_recvmsg(1, 0, 1, (size_t[IOV_MAX]){ (size_t)SIZE_MAX }, 0, 0,
	    SSIZE_MAX);

	rng_seed(0x72'65'63'76ULL);
	for (int i = 0; i < 200000; i++) {
		size_t iov_lens[IOV_MAX];
		int niov = rnd_int() & (IOV_MAX - 1);

		for (int j = 0; j < IOV_MAX; j++)
			iov_lens[j] = rnd_u32() & (BUF_TOTAL - 1);
		case_recvmsg(rnd_int(), rnd_int(), niov, iov_lens,
		    rnd_int() & 1, rnd_u32() & (ADDR_CAP - 1), rnd_ret());
	}
}

static void
test_accept4(void)
{
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 0x7f, 0x80, INT_MAX,
	};
	static const int flags[] = {
		0, SOCK_CLOEXEC, SOCK_NONBLOCK, -1, INT_MAX, 0x7f, 0x80,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80, 0xff,
	};

	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t f = 0; f < sizeof(flags) / sizeof(flags[0]); f++)
			for (int a = 0; a < 2; a++)
				for (int l = 0; l < 2; l++)
					for (size_t r = 0; r <
					    sizeof(rets) / sizeof(rets[0]); r++)
						case_accept4(socks[s], flags[f],
						    a, l, rets[r]);

	rng_seed(0x61'63'63'34ULL);
	for (int i = 0; i < 200000; i++)
		case_accept4(rnd_int(), rnd_int(), rnd_int() & 1,
		    rnd_int() & 1, rnd_ret());
}

static void
test_pselect(void)
{
	static const int ns[] = {
		INT_MIN, -1, 0, 1, 2, FD_SETSIZE - 1, FD_SETSIZE,
		0x7f, 0x80, INT_MAX,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t n = 0; n < sizeof(ns) / sizeof(ns[0]); n++)
		for (int rs = 0; rs < 2; rs++)
			for (int ws = 0; ws < 2; ws++)
				for (int es = 0; es < 2; es++)
					for (int t = 0; t < 2; t++)
						for (int sig = 0; sig < 2; sig++)
							for (size_t r = 0; r <
							    sizeof(rets) /
							    sizeof(rets[0]); r++)
								case_pselect(
								    ns[n], rs,
								    ws, es, t,
								    sig,
								    rets[r]);

	rng_seed(0x70'73'65'6cULL);
	for (int i = 0; i < 200000; i++)
		case_pselect(rnd_int(), rnd_int() & 1, rnd_int() & 1,
		    rnd_int() & 1, rnd_int() & 1, rnd_int() & 1, rnd_ret());
}

static void
test_aio_suspend(void)
{
	static const int counts[] = {
		0, 1, 2, IOV_MAX, -1, INT_MAX, 0x7f, 0x80,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t c = 0; c < sizeof(counts) / sizeof(counts[0]); c++)
		for (int to = 0; to < 2; to++)
			for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]);
			    r++)
				case_aio_suspend(counts[c], to, rets[r]);

	rng_seed(0x61'69'6f'73ULL);
	for (int i = 0; i < 200000; i++)
		case_aio_suspend(rnd_int(), rnd_int() & 1, rnd_ret());
}

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_recvmsg();
	test_accept4();
	test_pselect();
	test_aio_suspend();

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
