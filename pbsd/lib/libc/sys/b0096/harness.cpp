/*
 * harness.cpp -- differential test for PBSD batch b0096.
 *
 * recvmsg, accept4, pselect and aio_suspend are libc interposition wrappers:
 * each loads a function pointer from __libc_interposing[] and tail-calls it
 * with the original argument list.  The harness installs instrumented mocks in
 * both the oracle (ref_*) and port tables, then compares the mock-visible
 * syscall tag, forwarded arguments and return value.  accept4's mock writes
 * through caller-supplied sockaddr buffers; recvmsg's mock writes through iovec
 * payloads; pselect's mock writes through fd_set buffers.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#if defined(__linux__)
#include <aio.h>
#else
#include <sys/aio.h>
#endif

#ifndef SSIZE_MIN
#define	SSIZE_MIN	((ssize_t)(-SSIZE_MAX - 1))
#endif

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_accept4,
	INTERPOS_aio_suspend,
	INTERPOS_pselect,
	INTERPOS_recvmsg,
	INTERPOS_MAX
};

extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];

ssize_t ref_recvmsg(int s, struct msghdr *msg, int flags);
int ref_accept4(int s, struct sockaddr *addr, socklen_t *addrlen, int flags);
int ref_pselect(int n, fd_set *rs, fd_set *ws, fd_set *es,
    const struct timespec *t, const sigset_t *s);
int ref_aio_suspend(const struct aiocb * const iocbs[], int niocb,
    const struct timespec *timeout);

}

import pbsd.lib.libc.sys.b0096;

namespace port = pbsd::lib_libc_sys::b0096;

#define	GUARD			0x7f
#define	ADDR_CAP		128
#define	ADDR_GUARD_PAD		32
#define	IOV_MAX_TEST		8
#define	IOV_DATA_CAP		64
#define	MSG_NAME_CAP		32
#define	MSG_CTRL_CAP		32
#define	FDSET_CAP		(sizeof(fd_set))
#define	FDSET_GUARD_PAD		16
#define	AIOCBS_MAX		16

#define	PBSD_TAG_RECVMSG	1
#define	PBSD_TAG_ACCEPT4	2
#define	PBSD_TAG_PSELECT	3
#define	PBSD_TAG_AIO_SUSPEND	4

struct MockState {
	unsigned long long	ncalls;
	int			tag;
	int			s;
	int			n;
	int			flags;
	int			niocb;
	socklen_t		addrlen_in;
	socklen_t		addrlen_out;
	unsigned		msg_namelen;
	unsigned		msg_iovlen;
	unsigned		msg_controllen;
	unsigned char		addr_hash;
	unsigned char		iov_hash;
	unsigned char		fd_hash;
	unsigned char		sig_hash;
	long long		timeout_sec;
	long long		timeout_nsec;
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
fill_fdset(fd_set *set, int n, unsigned char pat)
{
	unsigned char *p;
	size_t i, lim;

	if (set == nullptr)
		return;
	p = (unsigned char *)set;
	lim = FDSET_CAP;
	if (n > 0 && (size_t)n < lim)
		lim = (size_t)n;
	for (i = 0; i < lim; i++)
		p[i] = (unsigned char)(pat + (unsigned char)i);
}

static ssize_t
mock_recvmsg(int s, struct msghdr *msg, int flags)
{
	size_t k, j, lim;

	mock_enter(PBSD_TAG_RECVMSG);
	mock.s = s;
	mock.flags = flags;
	mock.msg_namelen = 0;
	mock.msg_iovlen = 0;
	mock.msg_controllen = 0;

	if (msg != nullptr) {
		mock.msg_namelen = msg->msg_namelen;
		mock.msg_iovlen = msg->msg_iovlen;
		mock.msg_controllen = msg->msg_controllen;
		if (msg->msg_name != nullptr && msg->msg_namelen > 0)
			mock.addr_hash = (unsigned char)hash_bytes(msg->msg_name,
			    msg->msg_namelen);
		if (msg->msg_iov != nullptr && msg->msg_iovlen > 0) {
			size_t niov = msg->msg_iovlen;
			unsigned h = 2166136261u;

			if (niov > IOV_MAX_TEST)
				niov = IOV_MAX_TEST;
			for (k = 0; k < niov; k++) {
				size_t len = msg->msg_iov[k].iov_len;

				h ^= hash_bytes(&len, sizeof(len));
				if (msg->msg_iov[k].iov_base != nullptr && len > 0) {
					if (len > IOV_DATA_CAP)
						len = IOV_DATA_CAP;
					h ^= hash_bytes(msg->msg_iov[k].iov_base, len);
				}
			}
			mock.iov_hash = (unsigned char)h;
			for (k = 0; k < niov; k++) {
				if (msg->msg_iov[k].iov_base == nullptr)
					continue;
				lim = msg->msg_iov[k].iov_len;
				if (lim > IOV_DATA_CAP)
					lim = IOV_DATA_CAP;
				for (j = 0; j < lim; j++) {
					((unsigned char *)msg->msg_iov[k].iov_base)[j] =
					    (unsigned char)((s ^ flags ^ (int)k) +
					    (unsigned char)j);
				}
			}
		}
	}

	return ((ssize_t)mock.prog_ret);
}

static int
mock_accept4(int s, struct sockaddr *addr, socklen_t *addrlen, int flags)
{
	unsigned char pat;
	size_t fill;

	mock_enter(PBSD_TAG_ACCEPT4);
	mock.s = s;
	mock.flags = flags;

	if (addrlen != nullptr) {
		mock.addrlen_in = *addrlen;
		mock.addrlen_out = (*addrlen > ADDR_CAP) ?
		    (socklen_t)ADDR_CAP : *addrlen;
		if (mock.addrlen_out == 0 && *addrlen > 0)
			mock.addrlen_out = 1;
	} else {
		mock.addrlen_in = 0;
		mock.addrlen_out = 0;
	}

	pat = (unsigned char)((s ^ flags ^ 0xa5) + (mock.addrlen_in & 0xff));
	fill = (size_t)mock.addrlen_out;
	if (addr != nullptr && fill > 0) {
		unsigned char *p = (unsigned char *)addr;
		for (size_t i = 0; i < fill; i++)
			p[i] = (unsigned char)(pat + (unsigned char)i);
	}

	if (addrlen != nullptr)
		*addrlen = mock.addrlen_out;

	return ((int)mock.prog_ret);
}

static int
mock_pselect(int n, fd_set *rs, fd_set *ws, fd_set *es,
    const struct timespec *t, const sigset_t *s)
{
	mock_enter(PBSD_TAG_PSELECT);
	mock.n = n;
	mock.timeout_sec = 0;
	mock.timeout_nsec = 0;
	if (t != nullptr) {
		mock.timeout_sec = (long long)t->tv_sec;
		mock.timeout_nsec = (long long)t->tv_nsec;
	}
	if (s != nullptr)
		mock.sig_hash = (unsigned char)hash_bytes(s, sizeof(sigset_t));
	if (rs != nullptr) {
		mock.fd_hash ^= (unsigned char)hash_bytes(rs, FDSET_CAP);
		fill_fdset(rs, n, (unsigned char)(0x80 ^ n));
	}
	if (ws != nullptr)
		fill_fdset(ws, n, (unsigned char)(0x90 ^ n));
	if (es != nullptr)
		fill_fdset(es, n, (unsigned char)(0xa0 ^ n));

	return ((int)mock.prog_ret);
}

static int
mock_aio_suspend(const struct aiocb * const iocbs[], int niocb,
    const struct timespec *timeout)
{
	mock_enter(PBSD_TAG_AIO_SUSPEND);
	mock.niocb = niocb;
	mock.timeout_sec = 0;
	mock.timeout_nsec = 0;
	if (timeout != nullptr) {
		mock.timeout_sec = (long long)timeout->tv_sec;
		mock.timeout_nsec = (long long)timeout->tv_nsec;
	}
	if (iocbs != nullptr && niocb > 0) {
		int n = niocb < AIOCBS_MAX ? niocb : AIOCBS_MAX;
		mock.iov_hash = (unsigned char)hash_bytes(iocbs,
		    (size_t)n * sizeof(const struct aiocb *));
	}
	return ((int)mock.prog_ret);
}

static void
install_mocks(interpos_func_t *table)
{
	table[INTERPOS_recvmsg] = (interpos_func_t)mock_recvmsg;
	table[INTERPOS_accept4] = (interpos_func_t)mock_accept4;
	table[INTERPOS_pselect] = (interpos_func_t)mock_pselect;
	table[INTERPOS_aio_suspend] = (interpos_func_t)mock_aio_suspend;
}

struct Snap {
	unsigned long long	ncalls;
	int			tag;
	int			s;
	int			n;
	int			flags;
	int			niocb;
	socklen_t		addrlen_in;
	socklen_t		addrlen_out;
	unsigned		msg_namelen;
	unsigned		msg_iovlen;
	unsigned		msg_controllen;
	unsigned char		addr_hash;
	unsigned char		iov_hash;
	unsigned char		fd_hash;
	unsigned char		sig_hash;
	long long		timeout_sec;
	long long		timeout_nsec;
};

static Snap
take_snap(void)
{
	Snap s;

	s.ncalls = mock.ncalls;
	s.tag = mock.tag;
	s.s = mock.s;
	s.n = mock.n;
	s.flags = mock.flags;
	s.niocb = mock.niocb;
	s.addrlen_in = mock.addrlen_in;
	s.addrlen_out = mock.addrlen_out;
	s.msg_namelen = mock.msg_namelen;
	s.msg_iovlen = mock.msg_iovlen;
	s.msg_controllen = mock.msg_controllen;
	s.addr_hash = mock.addr_hash;
	s.iov_hash = mock.iov_hash;
	s.fd_hash = mock.fd_hash;
	s.sig_hash = mock.sig_hash;
	s.timeout_sec = mock.timeout_sec;
	s.timeout_nsec = mock.timeout_nsec;
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
	    a.n != b.n || a.flags != b.flags || a.niocb != b.niocb ||
	    a.addrlen_in != b.addrlen_in || a.addrlen_out != b.addrlen_out ||
	    a.msg_namelen != b.msg_namelen || a.msg_iovlen != b.msg_iovlen ||
	    a.msg_controllen != b.msg_controllen || a.addr_hash != b.addr_hash ||
	    a.iov_hash != b.iov_hash || a.fd_hash != b.fd_hash ||
	    a.sig_hash != b.sig_hash || a.timeout_sec != b.timeout_sec ||
	    a.timeout_nsec != b.timeout_nsec) {
		char msg[640];

		snprintf(msg, sizeof(msg),
		    "%s ref={nc=%llu tg=%d s=%d n=%d fl=%d niocb=%d "
		    "alen_in=%u alen_out=%u nml=%u iovl=%u ctl=%u "
		    "ah=%u ih=%u fh=%u sh=%u ts=%lld tn=%lld} "
		    "port={nc=%llu tg=%d s=%d n=%d fl=%d niocb=%d "
		    "alen_in=%u alen_out=%u nml=%u iovl=%u ctl=%u "
		    "ah=%u ih=%u fh=%u sh=%u ts=%lld tn=%lld}",
		    ctx, a.ncalls, a.tag, a.s, a.n, a.flags, a.niocb,
		    (unsigned)a.addrlen_in, (unsigned)a.addrlen_out,
		    a.msg_namelen, a.msg_iovlen, a.msg_controllen,
		    a.addr_hash, a.iov_hash, a.fd_hash, a.sig_hash,
		    a.timeout_sec, a.timeout_nsec, b.ncalls, b.tag, b.s,
		    b.n, b.flags, b.niocb, (unsigned)b.addrlen_in,
		    (unsigned)b.addrlen_out, b.msg_namelen, b.msg_iovlen,
		    b.msg_controllen, b.addr_hash, b.iov_hash, b.fd_hash,
		    b.sig_hash, b.timeout_sec, b.timeout_nsec);
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
case_recvmsg(int s, struct msghdr *msg, int flags, long long ret)
{
	unsigned char data_bufs_a[IOV_MAX_TEST][IOV_DATA_CAP + 2 * 16];
	unsigned char data_bufs_b[IOV_MAX_TEST][IOV_DATA_CAP + 2 * 16];
	struct iovec iovs_a[IOV_MAX_TEST];
	struct iovec iovs_b[IOV_MAX_TEST];
	struct msghdr msg_a, msg_b;
	Snap a, b;
	ssize_t ra, rb;
	char ctx[256];
	size_t k, total;

	fn_cases[FN_RECVMSG]++;

	memset(&msg_a, 0, sizeof(msg_a));
	memset(&msg_b, 0, sizeof(msg_b));

	if (msg != nullptr) {
		msg_a = *msg;
		msg_b = *msg;
		if (msg->msg_iov != nullptr && msg->msg_iovlen > 0) {
			for (k = 0; k < msg->msg_iovlen && k < IOV_MAX_TEST; k++) {
				total = IOV_DATA_CAP + 2 * 16;
				fill_guard(data_bufs_a[k], total);
				fill_guard(data_bufs_b[k], total);
				if (msg->msg_iov[k].iov_len > 0) {
					size_t cp = msg->msg_iov[k].iov_len;
					if (cp > IOV_DATA_CAP)
						cp = IOV_DATA_CAP;
					if (msg->msg_iov[k].iov_base != nullptr)
						memcpy(data_bufs_a[k] + 16,
						    msg->msg_iov[k].iov_base, cp);
					memcpy(data_bufs_b[k] + 16,
					    data_bufs_a[k] + 16, cp);
				}
				iovs_a[k].iov_base = data_bufs_a[k] + 16;
				iovs_a[k].iov_len = msg->msg_iov[k].iov_len;
				iovs_b[k].iov_base = data_bufs_b[k] + 16;
				iovs_b[k].iov_len = msg->msg_iov[k].iov_len;
			}
			msg_a.msg_iov = iovs_a;
			msg_b.msg_iov = iovs_b;
		}
	}

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_recvmsg(s, msg != nullptr ? &msg_a : nullptr, flags);
	a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::recvmsg(s, msg != nullptr ? &msg_b : nullptr, flags);
	b = take_snap();

	snprintf(ctx, sizeof(ctx), "s=%d flags=%d ret=%lld", s, flags, ret);
	cmp_snap(FN_RECVMSG, a, b, ctx);

	if (msg != nullptr && msg->msg_iov != nullptr) {
		for (k = 0; k < msg->msg_iovlen && k < IOV_MAX_TEST; k++) {
			total = IOV_DATA_CAP + 2 * 16;
			snprintf(ctx, sizeof(ctx), "iov[%zu]", k);
			cmp_buf(FN_RECVMSG, data_bufs_a[k], data_bufs_b[k],
			    total, ctx);
		}
	}

	if (ra != rb) {
		char msgbuf[224];

		snprintf(msgbuf, sizeof(msgbuf), "%s ref=%lld port=%lld", ctx,
		    (long long)ra, (long long)rb);
		fail(FN_RECVMSG, "return", msgbuf);
	}
}

static void
case_accept4(int s, struct sockaddr *addr, socklen_t *addrlen, int flags,
    long long ret)
{
	unsigned char buf_a[ADDR_CAP + 2 * ADDR_GUARD_PAD];
	unsigned char buf_b[ADDR_CAP + 2 * ADDR_GUARD_PAD];
	socklen_t len_a, len_b;
	socklen_t *pa, *pb;
	struct sockaddr *sa, *sb;
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];
	size_t total;

	fn_cases[FN_ACCEPT4]++;

	total = ADDR_CAP + 2 * ADDR_GUARD_PAD;
	fill_guard(buf_a, total);
	fill_guard(buf_b, total);

	if (addr != nullptr) {
		sa = (struct sockaddr *)(buf_a + ADDR_GUARD_PAD);
		sb = (struct sockaddr *)(buf_b + ADDR_GUARD_PAD);
	} else {
		sa = nullptr;
		sb = nullptr;
	}

	if (addrlen != nullptr) {
		len_a = *addrlen;
		len_b = *addrlen;
		pa = &len_a;
		pb = &len_b;
	} else {
		len_a = 0;
		len_b = 0;
		pa = nullptr;
		pb = nullptr;
	}

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_accept4(s, sa, pa, flags);
	snap_a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::accept4(s, sb, pb, flags);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx), "s=%d flags=%d addrlen=%u ret=%lld", s,
	    flags, addrlen != nullptr ? (unsigned)*addrlen : 0u, ret);
	cmp_snap(FN_ACCEPT4, snap_a, snap_b, ctx);
	cmp_buf(FN_ACCEPT4, buf_a, buf_b, total, ctx);

	if (addrlen != nullptr && len_a != len_b) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s *addrlen ref=%u port=%u", ctx,
		    (unsigned)len_a, (unsigned)len_b);
		fail(FN_ACCEPT4, "addrlen", msg);
	}

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_ACCEPT4, "return", msg);
	}
}

static void
case_pselect(int n, fd_set *rs, fd_set *ws, fd_set *es,
    const struct timespec *t, const sigset_t *s, long long ret)
{
	unsigned char rs_a[FDSET_CAP + 2 * FDSET_GUARD_PAD];
	unsigned char rs_b[FDSET_CAP + 2 * FDSET_GUARD_PAD];
	unsigned char ws_a[FDSET_CAP + 2 * FDSET_GUARD_PAD];
	unsigned char ws_b[FDSET_CAP + 2 * FDSET_GUARD_PAD];
	unsigned char es_a[FDSET_CAP + 2 * FDSET_GUARD_PAD];
	unsigned char es_b[FDSET_CAP + 2 * FDSET_GUARD_PAD];
	unsigned char sig_a[sizeof(sigset_t) + 2 * FDSET_GUARD_PAD];
	unsigned char sig_b[sizeof(sigset_t) + 2 * FDSET_GUARD_PAD];
	struct timespec ts_a, ts_b;
	const struct timespec *ta, *tb;
	fd_set *rsa, *rsb, *wsa, *wsb, *esa, *esb;
	const sigset_t *sa, *sb;
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];
	size_t total;

	fn_cases[FN_PSELECT]++;

	total = FDSET_CAP + 2 * FDSET_GUARD_PAD;
	fill_guard(rs_a, total);
	fill_guard(rs_b, total);
	fill_guard(ws_a, total);
	fill_guard(ws_b, total);
	fill_guard(es_a, total);
	fill_guard(es_b, total);
	fill_guard(sig_a, sizeof(sig_a));
	fill_guard(sig_b, sizeof(sig_b));

	rsa = rs != nullptr ? (fd_set *)(rs_a + FDSET_GUARD_PAD) : nullptr;
	rsb = rs != nullptr ? (fd_set *)(rs_b + FDSET_GUARD_PAD) : nullptr;
	wsa = ws != nullptr ? (fd_set *)(ws_a + FDSET_GUARD_PAD) : nullptr;
	wsb = ws != nullptr ? (fd_set *)(ws_b + FDSET_GUARD_PAD) : nullptr;
	esa = es != nullptr ? (fd_set *)(es_a + FDSET_GUARD_PAD) : nullptr;
	esb = es != nullptr ? (fd_set *)(es_b + FDSET_GUARD_PAD) : nullptr;

	if (t != nullptr) {
		ts_a = *t;
		ts_b = *t;
		ta = &ts_a;
		tb = &ts_b;
	} else {
		ta = nullptr;
		tb = nullptr;
	}

	if (s != nullptr) {
		memcpy(sig_a + FDSET_GUARD_PAD, s, sizeof(sigset_t));
		memcpy(sig_b + FDSET_GUARD_PAD, s, sizeof(sigset_t));
		sa = (const sigset_t *)(sig_a + FDSET_GUARD_PAD);
		sb = (const sigset_t *)(sig_b + FDSET_GUARD_PAD);
	} else {
		sa = nullptr;
		sb = nullptr;
	}

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_pselect(n, rsa, wsa, esa, ta, sa);
	snap_a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::pselect(n, rsb, wsb, esb, tb, sb);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx), "n=%d ret=%lld", n, ret);
	cmp_snap(FN_PSELECT, snap_a, snap_b, ctx);

	if (rs != nullptr)
		cmp_buf(FN_PSELECT, rs_a, rs_b, total, "rs");
	if (ws != nullptr)
		cmp_buf(FN_PSELECT, ws_a, ws_b, total, "ws");
	if (es != nullptr)
		cmp_buf(FN_PSELECT, es_a, es_b, total, "es");
	if (s != nullptr)
		cmp_buf(FN_PSELECT, sig_a, sig_b, sizeof(sig_a), "sig");

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_PSELECT, "return", msg);
	}
}

static void
case_aio_suspend(const struct aiocb * const iocbs[], int niocb,
    const struct timespec *timeout, long long ret)
{
	struct timespec ts_a, ts_b;
	const struct timespec *ta, *tb;
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_AIO_SUSPEND]++;

	if (timeout != nullptr) {
		ts_a = *timeout;
		ts_b = *timeout;
		ta = &ts_a;
		tb = &ts_b;
	} else {
		ta = nullptr;
		tb = nullptr;
	}

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_aio_suspend(iocbs, niocb, ta);
	snap_a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::aio_suspend(iocbs, niocb, tb);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx), "niocb=%d ret=%lld", niocb, ret);
	cmp_snap(FN_AIO_SUSPEND, snap_a, snap_b, ctx);

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_AIO_SUSPEND, "return", msg);
	}
}

static void
test_recvmsg(void)
{
	unsigned char name_buf[MSG_NAME_CAP];
	unsigned char ctrl_buf[MSG_CTRL_CAP];
	unsigned char data_bufs[IOV_MAX_TEST][IOV_DATA_CAP];
	struct iovec iovs[IOV_MAX_TEST];
	struct msghdr msg;
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 2, 0x7e, 0x7f, 0x80, 0xfe, INT_MAX,
	};
	static const int flagss[] = {
		0, MSG_DONTWAIT, MSG_OOB, MSG_EOR, INT_MIN, INT_MAX,
		0x7f, 0x80, 0xff,
	};
	static const long long rets[] = {
		-1, 0, 1, SSIZE_MAX, (long long)SSIZE_MAX - 1,
		(long long)SSIZE_MIN, (long long)SSIZE_MIN + 1,
		0x7f, 0x80,
	};
	size_t ni, ci, di, ii, s, f, r;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = name_buf;
	msg.msg_control = ctrl_buf;
	msg.msg_iov = iovs;

	for (s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (f = 0; f < sizeof(flagss) / sizeof(flagss[0]); f++)
			for (r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
				case_recvmsg(socks[s], nullptr, flagss[f],
				    rets[r]);

	msg.msg_namelen = 0;
	msg.msg_iovlen = 0;
	msg.msg_controllen = 0;
	for (s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		case_recvmsg(socks[s], &msg, 0, 0);

	for (unsigned char b = 0; b < 0xff; b += 0x3f) {
		name_buf[0] = (char)b;
		data_bufs[0][0] = (char)(b ^ 0x55);
		iovs[0].iov_base = data_bufs[0];
		iovs[0].iov_len = 1;
		msg.msg_namelen = 1;
		msg.msg_iovlen = 1;
		msg.msg_controllen = 0;
		case_recvmsg(3, &msg, (int)b, (long long)(int8_t)b);
	}

	for (int cnt = 0; cnt <= IOV_MAX_TEST + 2; cnt++) {
		for (ii = 0; ii < (size_t)cnt && ii < IOV_MAX_TEST; ii++) {
			data_bufs[ii][0] = (unsigned char)(0x80 + ii);
			iovs[ii].iov_base = data_bufs[ii];
			iovs[ii].iov_len = (size_t)(ii + 1);
		}
		msg.msg_namelen = 4;
		memcpy(name_buf, "\0\1\x7f\xff", 4);
		msg.msg_iovlen = (size_t)cnt;
		msg.msg_controllen = MSG_CTRL_CAP;
		memset(ctrl_buf, 0xcc, MSG_CTRL_CAP);
		case_recvmsg(7, &msg, cnt, (long long)cnt);
	}

	rng_seed(0x7263'766dULL);
	for (int i = 0; i < 200000; i++) {
		int sock = rnd_int();
		int flags = rnd_int();
		long long ret = rnd_ret();

		ni = rnd_size(MSG_NAME_CAP);
		ci = rnd_size(MSG_CTRL_CAP);
		ii = rnd_size(IOV_MAX_TEST);
		for (size_t k = 0; k < ni; k++)
			name_buf[k] = (unsigned char)rnd_byte();
		for (size_t k = 0; k < ci; k++)
			ctrl_buf[k] = (unsigned char)rnd_byte();
		for (size_t k = 0; k < ii; k++) {
			di = rnd_size(IOV_DATA_CAP);
			for (size_t j = 0; j < di; j++)
				data_bufs[k][j] = (unsigned char)rnd_byte();
			iovs[k].iov_base = data_bufs[k];
			iovs[k].iov_len = di;
		}
		msg.msg_namelen = ni;
		msg.msg_iovlen = ii;
		msg.msg_controllen = ci;
		case_recvmsg(sock, &msg, flags, ret);
	}
}

static void
test_accept4(void)
{
	struct sockaddr_storage ss;
	socklen_t len;
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 2, 0x7e, 0x7f, 0x80, 0xfe, INT_MAX,
	};
	static const int flagss[] = {
		0, SOCK_CLOEXEC, SOCK_NONBLOCK, INT_MIN, INT_MAX,
		0x7f, 0x80, 0xff,
	};
	static const socklen_t lens[] = {
		0, 1, 2, 16, (socklen_t)ADDR_CAP,
		(socklen_t)(ADDR_CAP + 1), (socklen_t)(ADDR_CAP + 16),
		(socklen_t)256,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};
	size_t s, f, l, r;

	for (s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (f = 0; f < sizeof(flagss) / sizeof(flagss[0]); f++)
			for (r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
				case_accept4(socks[s], nullptr, nullptr,
				    flagss[f], rets[r]);
				case_accept4(socks[s], (struct sockaddr *)&ss,
				    nullptr, flagss[f], rets[r]);
				case_accept4(socks[s], nullptr, &len,
				    flagss[f], rets[r]);
			}

	for (s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (f = 0; f < sizeof(flagss) / sizeof(flagss[0]); f++)
			for (l = 0; l < sizeof(lens) / sizeof(lens[0]); l++)
				for (r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
					len = lens[l];
					case_accept4(socks[s],
					    (struct sockaddr *)&ss, &len,
					    flagss[f], rets[r]);
				}

	rng_seed(0x6163'6374ULL);
	for (int i = 0; i < 200000; i++) {
		int sock = rnd_int();
		int flags = rnd_int();
		socklen_t l = (socklen_t)rnd_int();
		long long ret = rnd_ret();

		case_accept4(sock, (struct sockaddr *)&ss, &l, flags, ret);
		if ((rng_u32() & 3u) == 0)
			case_accept4(sock, nullptr, &l, flags, ret);
		if ((rng_u32() & 7u) == 0)
			case_accept4(sock, (struct sockaddr *)&ss, nullptr,
			    flags, ret);
		if ((rng_u32() & 15u) == 0)
			case_accept4(sock, nullptr, nullptr, flags, ret);
	}
}

static void
test_pselect(void)
{
	fd_set rs, ws, es;
	sigset_t sig;
	struct timespec ts;
	static const int ns[] = {
		INT_MIN, -1, 0, 1, 2, FD_SETSIZE - 1, FD_SETSIZE,
		FD_SETSIZE + 1, INT_MAX, 0x7f, 0x80,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};
	size_t n, r;

	FD_ZERO(&rs);
	FD_ZERO(&ws);
	FD_ZERO(&es);
	sigemptyset(&sig);
	ts.tv_sec = 0;
	ts.tv_nsec = 0;

	for (n = 0; n < sizeof(ns) / sizeof(ns[0]); n++)
		for (r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
			case_pselect(ns[n], nullptr, nullptr, nullptr, nullptr,
			    nullptr, rets[r]);
			case_pselect(ns[n], &rs, nullptr, nullptr, nullptr,
			    nullptr, rets[r]);
			case_pselect(ns[n], nullptr, &ws, nullptr, nullptr,
			    nullptr, rets[r]);
			case_pselect(ns[n], nullptr, nullptr, &es, nullptr,
			    nullptr, rets[r]);
			case_pselect(ns[n], &rs, &ws, &es, &ts, &sig,
			    rets[r]);
		}

	for (int fd = 0; fd < 32; fd++) {
		FD_ZERO(&rs);
		FD_SET(fd, &rs);
		ts.tv_sec = fd;
		ts.tv_nsec = (long)(fd * 1000);
		sigemptyset(&sig);
		sigaddset(&sig, fd % NSIG);
		case_pselect(fd + 1, &rs, &ws, &es, &ts, &sig,
		    (long long)fd);
	}

	for (unsigned char b = 0; b < 0xff; b += 0x1f) {
		unsigned char *p = (unsigned char *)&rs;
		for (size_t i = 0; i < FDSET_CAP; i++)
			p[i] = b;
		ts.tv_sec = (time_t)(int8_t)b;
		ts.tv_nsec = (long)b * 1000000L;
		case_pselect((int)(int8_t)b, &rs, &ws, &es, &ts, &sig,
		    (long long)(int8_t)b);
	}

	rng_seed(0x7073'656cULL);
	for (int i = 0; i < 200000; i++) {
		int nfds = rnd_int();
		long long ret = rnd_ret();
		unsigned char *rp = (unsigned char *)&rs;
		unsigned char *wp = (unsigned char *)&ws;
		unsigned char *ep = (unsigned char *)&es;

		for (size_t j = 0; j < FDSET_CAP; j++) {
			rp[j] = (unsigned char)rnd_byte();
			wp[j] = (unsigned char)rnd_byte();
			ep[j] = (unsigned char)rnd_byte();
		}
		ts.tv_sec = (time_t)rnd_ret();
		ts.tv_nsec = (long)rnd_ret();
		sigemptyset(&sig);
		if ((rng_u32() & 1u) != 0)
			sigaddset(&sig, (int)((unsigned)rnd_int() % (unsigned)NSIG));

		switch (rng_u32() % 8u) {
		case 0:
			case_pselect(nfds, &rs, &ws, &es, &ts, &sig, ret);
			break;
		case 1:
			case_pselect(nfds, &rs, nullptr, nullptr, &ts, nullptr,
			    ret);
			break;
		case 2:
			case_pselect(nfds, nullptr, &ws, nullptr, nullptr,
			    &sig, ret);
			break;
		case 3:
			case_pselect(nfds, nullptr, nullptr, &es, &ts, &sig,
			    ret);
			break;
		case 4:
			case_pselect(nfds, &rs, &ws, nullptr, nullptr, nullptr,
			    ret);
			break;
		case 5:
			case_pselect(nfds, nullptr, &ws, &es, &ts, &sig, ret);
			break;
		case 6:
			case_pselect(nfds, &rs, nullptr, &es, nullptr, &sig,
			    ret);
			break;
		default:
			case_pselect(nfds, nullptr, nullptr, nullptr, nullptr,
			    nullptr, ret);
			break;
		}
	}
}

static void
test_aio_suspend(void)
{
	struct aiocb cb_bufs[AIOCBS_MAX];
	const struct aiocb *iocb_ptrs[AIOCBS_MAX];
	struct timespec ts;
	static const int counts[] = {
		0, 1, 2, AIOCBS_MAX, AIOCBS_MAX + 1, AIOCBS_MAX + 4,
		INT_MIN, -1, INT_MAX,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};
	size_t c, r;

	memset(cb_bufs, 0, sizeof(cb_bufs));
	for (size_t i = 0; i < AIOCBS_MAX; i++)
		iocb_ptrs[i] = &cb_bufs[i];

	ts.tv_sec = 0;
	ts.tv_nsec = 0;

	for (c = 0; c < sizeof(counts) / sizeof(counts[0]); c++)
		for (r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
			case_aio_suspend(nullptr, counts[c], nullptr, rets[r]);
			case_aio_suspend(iocb_ptrs, counts[c], nullptr,
			    rets[r]);
			case_aio_suspend(iocb_ptrs, counts[c], &ts, rets[r]);
		}

	for (unsigned char b = 0; b < 0xff; b += 0x2f) {
		cb_bufs[0].aio_fildes = (int)(int8_t)b;
		cb_bufs[0].aio_nbytes = (size_t)b;
		ts.tv_sec = (time_t)(int8_t)b;
		ts.tv_nsec = (long)b;
		case_aio_suspend(iocb_ptrs, 1, &ts, (long long)(int8_t)b);
	}

	rng_seed(0x6169'6f73ULL);
	for (int i = 0; i < 200000; i++) {
		int niocb = rnd_int();
		long long ret = rnd_ret();
		int n = niocb < 0 ? 0 :
		    (niocb > AIOCBS_MAX ? AIOCBS_MAX : niocb);

		for (int k = 0; k < n; k++) {
			cb_bufs[k].aio_fildes = rnd_int();
			cb_bufs[k].aio_nbytes = (size_t)rnd_int();
			cb_bufs[k].aio_offset = (off_t)rnd_ret();
			iocb_ptrs[k] = &cb_bufs[k];
		}
		ts.tv_sec = (time_t)rnd_ret();
		ts.tv_nsec = (long)rnd_ret();

		if ((rng_u32() & 3u) == 0)
			case_aio_suspend(n > 0 ? iocb_ptrs : nullptr, niocb,
			    &ts, ret);
		else if ((rng_u32() & 7u) == 0)
			case_aio_suspend(n > 0 ? iocb_ptrs : nullptr, niocb,
			    nullptr, ret);
		else
			case_aio_suspend(n > 0 ? iocb_ptrs : nullptr, niocb,
			    (rng_u32() & 1u) ? &ts : nullptr, ret);
	}
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
