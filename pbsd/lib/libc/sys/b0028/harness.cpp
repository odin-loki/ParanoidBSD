/*
 * harness.cpp -- differential test for PBSD batch b0028.
 *
 * sendmsg, accept, connect and writev are libc interposition wrappers: each
 * loads a function pointer from __libc_interposing[] and tail-calls it with
 * the original argument list.  The harness installs instrumented mocks in
 * both the oracle (ref_*) and port tables, then compares the mock-visible
 * syscall tag, forwarded arguments and return value.  accept's mock writes
 * through caller-supplied buffers; those writes are compared in full.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <unistd.h>

#ifndef SSIZE_MIN
#define	SSIZE_MIN	((ssize_t)(-SSIZE_MAX - 1))
#endif

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_accept,
	INTERPOS_connect,
	INTERPOS_sendmsg,
	INTERPOS_writev,
	INTERPOS_MAX
};

extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];

ssize_t ref_sendmsg(int s, const struct msghdr *msg, int flags);
int ref_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int ref_connect(int s, const struct sockaddr *addr, socklen_t addrlen);
ssize_t ref_writev(int fd, const struct iovec *iov, int iovcnt);

}

import pbsd.lib.libc.sys.b0028;

namespace port = pbsd::lib_libc_sys::b0028;

/* ------------------------------------------------------------------ */
/* constants                                                          */
/* ------------------------------------------------------------------ */

#define	GUARD			0x7f
#define	ADDR_CAP		128
#define	ADDR_GUARD_PAD		32
#define	IOV_MAX_TEST		8
#define	IOV_DATA_CAP		64
#define	MSG_NAME_CAP		32
#define	MSG_CTRL_CAP		32

/* ------------------------------------------------------------------ */
/* instrumented mocks                                                 */
/* ------------------------------------------------------------------ */

#define	PBSD_TAG_NONE		0
#define	PBSD_TAG_ACCEPT		1
#define	PBSD_TAG_CONNECT	2
#define	PBSD_TAG_SENDMSG	3
#define	PBSD_TAG_WRITEV		4

struct MockState {
	unsigned long long	ncalls;
	int			tag;
	int			s;
	int			fd;
	int			flags;
	int			iovcnt;
	socklen_t		addrlen_in;
	socklen_t		addrlen_out;
	unsigned		msg_namelen;
	unsigned		msg_iovlen;
	unsigned		msg_controllen;
	unsigned char		addr_hash;
	unsigned char		iov_hash;
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

static ssize_t
mock_sendmsg(int s, const struct msghdr *msg, int flags)
{
	mock_enter(PBSD_TAG_SENDMSG);
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
		if (msg->msg_iov != nullptr && msg->msg_iovlen > 0)
			mock.iov_hash = (unsigned char)hash_bytes(msg->msg_iov,
			    msg->msg_iovlen * sizeof(struct iovec));
	}

	return ((ssize_t)mock.prog_ret);
}

static int
mock_accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	unsigned char pat;
	size_t fill;

	mock_enter(PBSD_TAG_ACCEPT);
	mock.s = s;

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

	pat = (unsigned char)((s ^ 0xa5) + (mock.addrlen_in & 0xff));
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
mock_connect(int s, const struct sockaddr *addr, socklen_t addrlen)
{
	mock_enter(PBSD_TAG_CONNECT);
	mock.s = s;
	mock.addrlen_in = addrlen;
	if (addr != nullptr && addrlen > 0)
		mock.addr_hash = (unsigned char)hash_bytes(addr, addrlen);
	return ((int)mock.prog_ret);
}

static ssize_t
mock_writev(int fd, const struct iovec *iov, int iovcnt)
{
	mock_enter(PBSD_TAG_WRITEV);
	mock.fd = fd;
	mock.iovcnt = iovcnt;
	if (iov != nullptr && iovcnt > 0) {
		int n = iovcnt < IOV_MAX_TEST ? iovcnt : IOV_MAX_TEST;
		mock.iov_hash = (unsigned char)hash_bytes(iov,
		    (size_t)n * sizeof(struct iovec));
	}
	return ((ssize_t)mock.prog_ret);
}

static void
install_mocks(interpos_func_t *table)
{
	table[INTERPOS_accept] = (interpos_func_t)mock_accept;
	table[INTERPOS_connect] = (interpos_func_t)mock_connect;
	table[INTERPOS_sendmsg] = (interpos_func_t)mock_sendmsg;
	table[INTERPOS_writev] = (interpos_func_t)mock_writev;
}

struct Snap {
	unsigned long long	ncalls;
	int			tag;
	int			s;
	int			fd;
	int			flags;
	int			iovcnt;
	socklen_t		addrlen_in;
	socklen_t		addrlen_out;
	unsigned		msg_namelen;
	unsigned		msg_iovlen;
	unsigned		msg_controllen;
	unsigned char		addr_hash;
	unsigned char		iov_hash;
};

static Snap
take_snap(void)
{
	Snap s;

	s.ncalls = mock.ncalls;
	s.tag = mock.tag;
	s.s = mock.s;
	s.fd = mock.fd;
	s.flags = mock.flags;
	s.iovcnt = mock.iovcnt;
	s.addrlen_in = mock.addrlen_in;
	s.addrlen_out = mock.addrlen_out;
	s.msg_namelen = mock.msg_namelen;
	s.msg_iovlen = mock.msg_iovlen;
	s.msg_controllen = mock.msg_controllen;
	s.addr_hash = mock.addr_hash;
	s.iov_hash = mock.iov_hash;
	return (s);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

enum {
	FN_SENDMSG,
	FN_ACCEPT,
	FN_CONNECT,
	FN_WRITEV,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"sendmsg",
	"accept",
	"connect",
	"writev",
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
	if (a.ncalls != b.ncalls || a.tag != b.tag || a.s != b.s ||
	    a.fd != b.fd || a.flags != b.flags || a.iovcnt != b.iovcnt ||
	    a.addrlen_in != b.addrlen_in || a.addrlen_out != b.addrlen_out ||
	    a.msg_namelen != b.msg_namelen || a.msg_iovlen != b.msg_iovlen ||
	    a.msg_controllen != b.msg_controllen || a.addr_hash != b.addr_hash ||
	    a.iov_hash != b.iov_hash) {
		char msg[512];

		snprintf(msg, sizeof(msg),
		    "%s ref={nc=%llu tg=%d s=%d fd=%d fl=%d iov=%d "
		    "alen_in=%u alen_out=%u nml=%u iovl=%u ctl=%u ah=%u ih=%u} "
		    "port={nc=%llu tg=%d s=%d fd=%d fl=%d iov=%d "
		    "alen_in=%u alen_out=%u nml=%u iovl=%u ctl=%u ah=%u ih=%u}",
		    ctx, a.ncalls, a.tag, a.s, a.fd, a.flags, a.iovcnt,
		    (unsigned)a.addrlen_in, (unsigned)a.addrlen_out,
		    a.msg_namelen, a.msg_iovlen, a.msg_controllen,
		    a.addr_hash, a.iov_hash, b.ncalls, b.tag, b.s, b.fd,
		    b.flags, b.iovcnt, (unsigned)b.addrlen_in,
		    (unsigned)b.addrlen_out, b.msg_namelen, b.msg_iovlen,
		    b.msg_controllen, b.addr_hash, b.iov_hash);
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

/* ------------------------------------------------------------------ */
/* per-function cases                                                 */
/* ------------------------------------------------------------------ */

static void
case_sendmsg(int s, const struct msghdr *msg, int flags, long long ret)
{
	Snap a, b;
	ssize_t ra, rb;
	char ctx[256];

	fn_cases[FN_SENDMSG]++;

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_sendmsg(s, msg, flags);
	a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::sendmsg(s, msg, flags);
	b = take_snap();

	snprintf(ctx, sizeof(ctx), "s=%d flags=%d ret=%lld", s, flags, ret);
	cmp_snap(FN_SENDMSG, a, b, ctx);

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%lld port=%lld", ctx,
		    (long long)ra, (long long)rb);
		fail(FN_SENDMSG, "return", msg);
	}
}

static void
case_accept(int s, struct sockaddr *addr, socklen_t *addrlen, long long ret)
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

	fn_cases[FN_ACCEPT]++;

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
	ra = ref_accept(s, sa, pa);
	snap_a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::accept(s, sb, pb);
	snap_b = take_snap();

	snprintf(ctx, sizeof(ctx), "s=%d addrlen=%u ret=%lld", s,
	    addrlen != nullptr ? (unsigned)*addrlen : 0u, ret);
	cmp_snap(FN_ACCEPT, snap_a, snap_b, ctx);
	cmp_buf(FN_ACCEPT, buf_a, buf_b, total, ctx);

	if (addrlen != nullptr && len_a != len_b) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s *addrlen ref=%u port=%u", ctx,
		    (unsigned)len_a, (unsigned)len_b);
		fail(FN_ACCEPT, "addrlen", msg);
	}

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_ACCEPT, "return", msg);
	}
}

static void
case_connect(int s, const struct sockaddr *addr, socklen_t addrlen,
    long long ret)
{
	Snap a, b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_CONNECT]++;

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_connect(s, addr, addrlen);
	a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::connect(s, addr, addrlen);
	b = take_snap();

	snprintf(ctx, sizeof(ctx), "s=%d addrlen=%u ret=%lld", s,
	    (unsigned)addrlen, ret);
	cmp_snap(FN_CONNECT, a, b, ctx);

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_CONNECT, "return", msg);
	}
}

static void
case_writev(int fd, const struct iovec *iov, int iovcnt, long long ret)
{
	Snap a, b;
	ssize_t ra, rb;
	char ctx[256];

	fn_cases[FN_WRITEV]++;

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_writev(fd, iov, iovcnt);
	a = take_snap();

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::writev(fd, iov, iovcnt);
	b = take_snap();

	snprintf(ctx, sizeof(ctx), "fd=%d iovcnt=%d ret=%lld", fd, iovcnt, ret);
	cmp_snap(FN_WRITEV, a, b, ctx);

	if (ra != rb) {
		char msg[224];

		snprintf(msg, sizeof(msg), "%s ref=%lld port=%lld", ctx,
		    (long long)ra, (long long)rb);
		fail(FN_WRITEV, "return", msg);
	}
}

/* ------------------------------------------------------------------ */
/* test drivers                                                       */
/* ------------------------------------------------------------------ */

static void
test_sendmsg(void)
{
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

	unsigned char name_buf[MSG_NAME_CAP];
	unsigned char ctrl_buf[MSG_CTRL_CAP];
	unsigned char data_bufs[IOV_MAX_TEST][IOV_DATA_CAP];
	struct iovec iovs[IOV_MAX_TEST];
	struct msghdr msg;
	size_t ni, ci, di, ii;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = name_buf;
	msg.msg_control = ctrl_buf;
	msg.msg_iov = iovs;

	/* edge: null msg */
	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t f = 0; f < sizeof(flagss) / sizeof(flagss[0]); f++)
			for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
				case_sendmsg(socks[s], nullptr, flagss[f],
				    rets[r]);

	/* edge: empty msghdr */
	msg.msg_namelen = 0;
	msg.msg_iovlen = 0;
	msg.msg_controllen = 0;
	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		case_sendmsg(socks[s], &msg, 0, 0);

	/* edge: single-byte name, single iovec, NUL and high-bit payloads */
	for (unsigned char b = 0; b < 0xff; b += 0x3f) {
		name_buf[0] = (char)b;
		data_bufs[0][0] = (char)(b ^ 0x55);
		iovs[0].iov_base = data_bufs[0];
		iovs[0].iov_len = 1;
		msg.msg_namelen = 1;
		msg.msg_iovlen = 1;
		msg.msg_controllen = 0;
		case_sendmsg(3, &msg, (int)b, (long long)(int8_t)b);
	}

	/* edge: boundary iov counts and lengths */
	for (int cnt = 0; cnt <= IOV_MAX_TEST + 2; cnt++) {
		for (size_t i = 0; i < (size_t)cnt && i < IOV_MAX_TEST; i++) {
			data_bufs[i][0] = (unsigned char)(0x80 + i);
			iovs[i].iov_base = data_bufs[i];
			iovs[i].iov_len = (size_t)(i + 1);
		}
		msg.msg_namelen = 4;
		memcpy(name_buf, "\0\1\x7f\xff", 4);
		msg.msg_iovlen = (size_t)cnt;
		msg.msg_controllen = MSG_CTRL_CAP;
		memset(ctrl_buf, 0xcc, MSG_CTRL_CAP);
		case_sendmsg(7, &msg, cnt, (long long)cnt);
	}

	rng_seed(0x736e'646dULL);
	for (int i = 0; i < 200000; i++) {
		int s = rnd_int();
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
		case_sendmsg(s, &msg, flags, ret);
	}
}

static void
test_accept(void)
{
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 2, 0x7e, 0x7f, 0x80, 0xfe, INT_MAX,
	};
	static const socklen_t lens[] = {
		0, 1, 2, 16, (socklen_t)ADDR_CAP,
		(socklen_t)(ADDR_CAP + 1), (socklen_t)(ADDR_CAP + 16),
		(socklen_t)256,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	struct sockaddr_storage ss;
	socklen_t len;

	/* edge: null addr, null addrlen */
	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
			case_accept(socks[s], nullptr, nullptr, rets[r]);
			case_accept(socks[s], (struct sockaddr *)&ss, nullptr,
			    rets[r]);
			case_accept(socks[s], nullptr, &len, rets[r]);
		}

	/* edge: boundary addrlen values */
	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t l = 0; l < sizeof(lens) / sizeof(lens[0]); l++)
			for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
				len = lens[l];
				case_accept(socks[s], (struct sockaddr *)&ss,
				    &len, rets[r]);
			}

	rng_seed(0x6163'6370ULL);
	for (int i = 0; i < 200000; i++) {
		int s = rnd_int();
		socklen_t l = (socklen_t)rnd_int();
		long long ret = rnd_ret();

		case_accept(s, (struct sockaddr *)&ss, &l, ret);
		if ((rng_u32() & 3u) == 0)
			case_accept(s, nullptr, &l, ret);
		if ((rng_u32() & 7u) == 0)
			case_accept(s, (struct sockaddr *)&ss, nullptr, ret);
		if ((rng_u32() & 15u) == 0)
			case_accept(s, nullptr, nullptr, ret);
	}
}

static void
test_connect(void)
{
	unsigned char addr_bufs[4][32];
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 2, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const socklen_t lens[] = {
		0, 1, 2, 16, sizeof(struct sockaddr_in),
		sizeof(struct sockaddr_un), (socklen_t)32,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t b = 0; b < sizeof(addr_bufs) / sizeof(addr_bufs[0]); b++)
		for (size_t i = 0; i < sizeof(addr_bufs[0]); i++)
			addr_bufs[b][i] = (unsigned char)(0x80 + b * 17 + i);

	/* edge: null addr */
	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t l = 0; l < sizeof(lens) / sizeof(lens[0]); l++)
			for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
				case_connect(socks[s], nullptr, lens[l],
				    rets[r]);

	/* edge: NUL-heavy and high-bit sockaddr payloads */
	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t b = 0; b < sizeof(addr_bufs) / sizeof(addr_bufs[0]); b++)
			for (size_t l = 0; l < sizeof(lens) / sizeof(lens[0]); l++)
				case_connect(socks[s],
				    (const struct sockaddr *)addr_bufs[b],
				    lens[l], (long long)(int)s);

	rng_seed(0x636f'6e6eULL);
	for (int i = 0; i < 200000; i++) {
		unsigned char ab[32];
		size_t n = rnd_size(sizeof(ab));

		for (size_t j = 0; j < n; j++)
			ab[j] = (unsigned char)rnd_byte();
		case_connect(rnd_int(), (const struct sockaddr *)ab,
		    (socklen_t)n, rnd_ret());
		if ((rng_u32() & 7u) == 0)
			case_connect(rnd_int(), nullptr, (socklen_t)n,
			    rnd_ret());
	}
}

static void
test_writev(void)
{
	unsigned char data_bufs[IOV_MAX_TEST + 2][IOV_DATA_CAP];
	struct iovec iovs[IOV_MAX_TEST + 2];
	static const int fds[] = {
		INT_MIN, -1, 0, 1, 2, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const int cnts[] = {
		0, 1, 2, IOV_MAX_TEST, IOV_MAX_TEST + 1, IOV_MAX_TEST + 2,
		INT_MIN, -1, INT_MAX,
	};
	static const long long rets[] = {
		-1, 0, 1, SSIZE_MAX, (long long)SSIZE_MIN, 0x7f, 0x80,
	};

	/* edge: null iov with various counts */
	for (size_t f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
		for (size_t c = 0; c < sizeof(cnts) / sizeof(cnts[0]); c++)
			for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
				case_writev(fds[f], nullptr, cnts[c], rets[r]);

	/* edge: empty, single-char, NUL-heavy, high-bit iovecs */
	for (int cnt = 1; cnt <= IOV_MAX_TEST; cnt++) {
		for (int i = 0; i < cnt; i++) {
			data_bufs[i][0] = (unsigned char)(0x80 + i);
			iovs[i].iov_base = data_bufs[i];
			iovs[i].iov_len = (size_t)((i & 1) ? 0 : 1);
		}
		case_writev(3, iovs, cnt, (long long)cnt);
	}

	for (unsigned char b = 0; b < 0xff; b += 0x1f) {
		data_bufs[0][0] = b;
		data_bufs[0][1] = (unsigned char)(b ^ 0xff);
		iovs[0].iov_base = data_bufs[0];
		iovs[0].iov_len = 2;
		case_writev(5, iovs, 1, (long long)(int8_t)b);
	}

	rng_seed(0x7772'7465ULL);
	for (int i = 0; i < 200000; i++) {
		int fd = rnd_int();
		int cnt = rnd_int();
		int n = cnt < 0 ? 0 : (cnt > IOV_MAX_TEST ? IOV_MAX_TEST : cnt);

		for (int k = 0; k < n; k++) {
			size_t len = rnd_size(IOV_DATA_CAP);
			for (size_t j = 0; j < len; j++)
				data_bufs[k][j] = (unsigned char)rnd_byte();
			iovs[k].iov_base = data_bufs[k];
			iovs[k].iov_len = len;
		}
		case_writev(fd, n > 0 ? iovs : nullptr, cnt, rnd_ret());
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_sendmsg();
	test_accept();
	test_connect();
	test_writev();

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
