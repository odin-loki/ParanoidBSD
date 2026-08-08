/*
 * harness.cpp -- differential test for PBSD batch b0249.
 *
 * setcontext, recvfrom, kevent and open are libc interposition wrappers:
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
#include <stdarg.h>
#include <ucontext.h>
#include <sys/fcntl.h>

#if defined(__has_include)
#if __has_include(<sys/event.h>)
#include <sys/event.h>
#else
#define	B0249_NEED_EVENT_STUB
#endif
#else
#define	B0249_NEED_EVENT_STUB
#endif

#if defined(B0249_NEED_EVENT_STUB)
struct kevent {
	uintptr_t ident;
	short filter;
	unsigned short flags;
	unsigned int fflags;
	intptr_t data;
	void *udata;
};
#define	EVFILT_READ	(-1)
#endif
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>

#ifndef SSIZE_MIN
#define	SSIZE_MIN	((ssize_t)(-SSIZE_MAX - 1))
#endif

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_setcontext,
	INTERPOS_recvfrom,
	INTERPOS_kevent,
	INTERPOS_openat,
	INTERPOS_MAX
};

extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];

int ref_setcontext(const ucontext_t *uc);
ssize_t ref_recvfrom(int s, void *buf, size_t len, int flags,
    struct sockaddr *from, socklen_t *fromlen);
int ref_kevent(int kq, const struct kevent *changelist, int nchanges,
    struct kevent *eventlist, int nevents, const struct timespec *timeout);
int ref_open(const char *path, int flags, ...);

}

import pbsd.lib.libc.sys.b0249;

namespace port = pbsd::lib_libc_sys::b0249;

#define	GUARD		0x7f

#define	UC_PAD		192
#define	UC_TOTAL	(sizeof(ucontext_t) + 2 * UC_PAD)

#define	BUF_CAP		64
#define	BUF_PAD		64
#define	BUF_TOTAL	(BUF_CAP + 2 * BUF_PAD)

#define	ADDR_CAP	64
#define	ADDR_PAD	64
#define	ADDR_TOTAL	(ADDR_CAP + 2 * ADDR_PAD)

#define	FLEN_PAD	64
#define	FLEN_TOTAL	(sizeof(socklen_t) + 2 * FLEN_PAD)

#define	KEV_MAX		8
#define	KEV_PAD		8
#define	KEV_ELEMS	(KEV_MAX + 2 * KEV_PAD)
#define	KEV_TOTAL	(KEV_ELEMS * sizeof(struct kevent))
#define	KEV_OFF		(KEV_PAD * sizeof(struct kevent))

#define	TS_PAD		64
#define	TS_TOTAL	(sizeof(struct timespec) + 2 * TS_PAD)

#define	PATH_CAP	64
#define	PATH_PAD	64
#define	PATH_TOTAL	(PATH_CAP + 2 * PATH_PAD)
#define	PATH_OFF	PATH_PAD

#define	TAG_NONE		0
#define	TAG_SETCONTEXT		1
#define	TAG_RECVFROM		2
#define	TAG_KEVENT		3
#define	TAG_OPENAT		4

#define	NSCALAR		6
#define	NPTR		6
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

static const unsigned char *base_uc;
static const unsigned char *base_buf;
static const unsigned char *base_addr;
static const unsigned char *base_flen;
static const unsigned char *base_ch;
static const unsigned char *base_ev;
static const unsigned char *base_ts;
static const unsigned char *base_path;

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
mock_setcontext(const ucontext_t *uc)
{
	mock_enter(TAG_SETCONTEXT);
	mock.po[0] = poff(uc, base_uc);

	if (uc != nullptr)
		mock.hs[0] = hash_bytes(uc, sizeof(ucontext_t));
	return ((int)mock_ret);
}

static ssize_t
mock_recvfrom(int s, void *buf, size_t len, int flags,
    struct sockaddr *from, socklen_t *fromlen)
{
	mock_enter(TAG_RECVFROM);
	mock.sc[0] = (long long)s;
	mock.sc[1] = (long long)(unsigned long long)len;
	mock.sc[2] = (long long)flags;
	mock.po[0] = poff(buf, base_buf);
	mock.po[1] = poff(from, base_addr);
	mock.po[2] = poff(fromlen, base_flen);

	if (buf != nullptr)
		mock.hs[0] = hash_bytes(buf,
		    len < BUF_CAP ? len : (size_t)BUF_CAP);
	if (from != nullptr)
		mock.hs[1] = hash_bytes(from, sizeof(struct sockaddr));
	if (fromlen != nullptr) {
		mock.hs[2] = hash_bytes(fromlen, sizeof(socklen_t));
		*fromlen = (socklen_t)(s ^ flags);
		mock.wr = (long long)*fromlen;
	}
	if (buf != nullptr && len > 0) {
		size_t n = len < BUF_CAP ? len : (size_t)BUF_CAP;

		fill_pattern((unsigned char *)buf, n, (unsigned char)(s & 0xff));
	}
	return ((ssize_t)mock_ret);
}

static int
mock_kevent(int kq, const struct kevent *changelist, int nchanges,
    struct kevent *eventlist, int nevents, const struct timespec *timeout)
{
	int i, n;

	mock_enter(TAG_KEVENT);
	mock.sc[0] = (long long)kq;
	mock.sc[1] = (long long)nchanges;
	mock.sc[2] = (long long)nevents;
	mock.po[0] = poff(changelist, base_ch);
	mock.po[1] = poff(eventlist, base_ev);
	mock.po[2] = poff(timeout, base_ts);

	if (changelist != nullptr && nchanges > 0) {
		n = nchanges < KEV_MAX ? nchanges : KEV_MAX;
		mock.hs[0] = hash_bytes(changelist,
		    (size_t)n * sizeof(struct kevent));
	}
	if (eventlist != nullptr && nevents > 0) {
		n = nevents < KEV_MAX ? nevents : KEV_MAX;
		for (i = 0; i < n; i++)
			eventlist[i].flags = (unsigned short)(kq ^ i);
		mock.hs[1] = hash_bytes(eventlist,
		    (size_t)n * sizeof(struct kevent));
	}
	if (timeout != nullptr)
		mock.hs[2] = hash_bytes(timeout, sizeof(struct timespec));
	return ((int)mock_ret);
}

static int
mock_openat(int fd, const char *path, int flags, int mode)
{
	size_t n;

	mock_enter(TAG_OPENAT);
	mock.sc[0] = (long long)fd;
	mock.sc[1] = (long long)flags;
	mock.sc[2] = (long long)mode;
	mock.po[0] = poff(path, base_path);

	if (path != nullptr) {
		n = strnlen(path, PATH_CAP);
		mock.hs[0] = hash_bytes(path, n + 1);
	}
	return ((int)mock_ret);
}

static void
install_mocks(interpos_func_t *table)
{
	table[INTERPOS_setcontext] = (interpos_func_t)mock_setcontext;
	table[INTERPOS_recvfrom] = (interpos_func_t)mock_recvfrom;
	table[INTERPOS_kevent] = (interpos_func_t)mock_kevent;
	table[INTERPOS_openat] = (interpos_func_t)mock_openat;
}

enum {
	FN_SETCONTEXT,
	FN_RECVFROM,
	FN_KEVENT,
	FN_OPEN,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"setcontext",
	"recvfrom",
	"kevent",
	"open",
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
	    "{nc=%llu tag=%d sc=[%lld,%lld,%lld,%lld,%lld,%lld] "
	    "po=[%lld,%lld,%lld,%lld,%lld,%lld] "
	    "hs=[%u,%u,%u,%u] wr=%lld}",
	    s.ncalls, s.tag, s.sc[0], s.sc[1], s.sc[2], s.sc[3], s.sc[4],
	    s.sc[5], s.po[0], s.po[1], s.po[2], s.po[3], s.po[4], s.po[5],
	    s.hs[0], s.hs[1], s.hs[2], s.hs[3], s.wr);
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

	char sa[512], sb[512], msg[1200];

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
path_copy(unsigned char *dst, const char *src, size_t cap)
{
	size_t n;

	memset(dst, 0, cap);
	if (src == nullptr)
		return;
	n = strlen(src);
	if (n >= cap)
		n = cap - 1;
	memcpy(dst, src, n);
	dst[n] = '\0';
}

static void
path_fill(unsigned char *dst, size_t cap, unsigned char seed)
{
	size_t i;

	for (i = 0; i + 1 < cap; i++)
		dst[i] = (unsigned char)(seed + (unsigned char)i);
	dst[cap - 1] = '\0';
}

static int
call_ref_open(const char *path, int flags, int mode)
{
	if ((flags & O_CREAT) != 0)
		return (ref_open(path, flags, mode));
	return (ref_open(path, flags));
}

static int
call_port_open(const char *path, int flags, int mode)
{
	if ((flags & O_CREAT) != 0)
		return (port::open(path, flags, mode));
	return (port::open(path, flags));
}

static void
case_setcontext(const ucontext_t *uc, long long ret)
{
	unsigned char uca[UC_TOTAL], ucb[UC_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[192];

	fn_cases[FN_SETCONTEXT]++;

	memset(uca, GUARD, sizeof(uca));
	memset(ucb, GUARD, sizeof(ucb));

	if (uc != nullptr) {
		memcpy(uca + UC_PAD, uc, sizeof(ucontext_t));
		memcpy(ucb + UC_PAD, uc, sizeof(ucontext_t));
	}

	snprintf(ctx, sizeof(ctx), "uc=%d ret=%lld", uc != nullptr, ret);

	base_uc = uca;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_setcontext(uc != nullptr ?
	    (const ucontext_t *)(uca + UC_PAD) : nullptr);
	snap_a = mock;

	base_uc = ucb;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::setcontext(uc != nullptr ?
	    (const ucontext_t *)(ucb + UC_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_SETCONTEXT, snap_a, snap_b, ctx);
	cmp_buf(FN_SETCONTEXT, "uc", uca, ucb, sizeof(uca), ctx);
	cmp_ret(FN_SETCONTEXT, ra, rb, ctx);
}

static void
case_recvfrom(int s, const void *msg, size_t len, int flags,
    const struct sockaddr *from, const socklen_t *fromlen, long long ret)
{
	unsigned char bufa[BUF_TOTAL], bufb[BUF_TOTAL];
	unsigned char addra[ADDR_TOTAL], addrb[ADDR_TOTAL];
	unsigned char flena[FLEN_TOTAL], flenb[FLEN_TOTAL];
	Snap snap_a, snap_b;
	ssize_t ra, rb;
	char ctx[256];

	fn_cases[FN_RECVFROM]++;

	memset(bufa, GUARD, sizeof(bufa));
	memset(bufb, GUARD, sizeof(bufb));
	memset(addra, GUARD, sizeof(addra));
	memset(addrb, GUARD, sizeof(addrb));
	memset(flena, GUARD, sizeof(flena));
	memset(flenb, GUARD, sizeof(flenb));

	if (msg != nullptr) {
		size_t n = len < BUF_CAP ? len : (size_t)BUF_CAP;

		memcpy(bufa + BUF_PAD, msg, n);
		memcpy(bufb + BUF_PAD, msg, n);
	}
	if (from != nullptr) {
		memcpy(addra + ADDR_PAD, from, sizeof(struct sockaddr));
		memcpy(addrb + ADDR_PAD, from, sizeof(struct sockaddr));
	}
	if (fromlen != nullptr) {
		memcpy(flena + FLEN_PAD, fromlen, sizeof(socklen_t));
		memcpy(flenb + FLEN_PAD, fromlen, sizeof(socklen_t));
	}

	snprintf(ctx, sizeof(ctx),
	    "s=%d len=%zu fl=%d buf=%d from=%d flen=%d ret=%lld", s,
	    (size_t)len, flags, msg != nullptr, from != nullptr,
	    fromlen != nullptr, ret);

	base_buf = bufa;
	base_addr = addra;
	base_flen = flena;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_recvfrom(s, msg != nullptr ? (void *)(bufa + BUF_PAD) :
	    nullptr, len, flags, from != nullptr ?
	    (struct sockaddr *)(addra + ADDR_PAD) : nullptr,
	    fromlen != nullptr ? (socklen_t *)(flena + FLEN_PAD) : nullptr);
	snap_a = mock;

	base_buf = bufb;
	base_addr = addrb;
	base_flen = flenb;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::__ssp_real(recvfrom)(s,
	    msg != nullptr ? (void *)(bufb + BUF_PAD) : nullptr, len, flags,
	    from != nullptr ? (struct sockaddr *)(addrb + ADDR_PAD) : nullptr,
	    fromlen != nullptr ? (socklen_t *)(flenb + FLEN_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_RECVFROM, snap_a, snap_b, ctx);
	cmp_buf(FN_RECVFROM, "buf", bufa, bufb, sizeof(bufa), ctx);
	cmp_buf(FN_RECVFROM, "addr", addra, addrb, sizeof(addra), ctx);
	cmp_buf(FN_RECVFROM, "flen", flena, flenb, sizeof(flena), ctx);
	cmp_ret(FN_RECVFROM, ra, rb, ctx);
}

static void
case_kevent(int kq, const struct kevent *ch, int nchanges,
    const struct kevent *ev, int nevents, const struct timespec *ts,
    long long ret)
{
	unsigned char cha[KEV_TOTAL], chb[KEV_TOTAL];
	unsigned char eva[KEV_TOTAL], evb[KEV_TOTAL];
	unsigned char tsa[TS_TOTAL], tsb[TS_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_KEVENT]++;

	memset(cha, GUARD, sizeof(cha));
	memset(chb, GUARD, sizeof(chb));
	memset(eva, GUARD, sizeof(eva));
	memset(evb, GUARD, sizeof(evb));
	memset(tsa, GUARD, sizeof(tsa));
	memset(tsb, GUARD, sizeof(tsb));

	if (ch != nullptr) {
		size_t n = (size_t)(nchanges < KEV_MAX ? nchanges : KEV_MAX) *
		    sizeof(struct kevent);

		memcpy(cha + KEV_OFF, ch, n);
		memcpy(chb + KEV_OFF, ch, n);
	}
	if (ev != nullptr) {
		size_t n = (size_t)(nevents < KEV_MAX ? nevents : KEV_MAX) *
		    sizeof(struct kevent);

		memcpy(eva + KEV_OFF, ev, n);
		memcpy(evb + KEV_OFF, ev, n);
	}
	if (ts != nullptr) {
		memcpy(tsa + TS_PAD, ts, sizeof(struct timespec));
		memcpy(tsb + TS_PAD, ts, sizeof(struct timespec));
	}

	snprintf(ctx, sizeof(ctx),
	    "kq=%d nch=%d nev=%d ch=%d ev=%d ts=%d ret=%lld", kq, nchanges,
	    nevents, ch != nullptr, ev != nullptr, ts != nullptr, ret);

	base_ch = cha;
	base_ev = eva;
	base_ts = tsa;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_kevent(kq, ch != nullptr ?
	    (const struct kevent *)(cha + KEV_OFF) : nullptr, nchanges,
	    ev != nullptr ? (struct kevent *)(eva + KEV_OFF) : nullptr,
	    nevents, ts != nullptr ?
	    (const struct timespec *)(tsa + TS_PAD) : nullptr);
	snap_a = mock;

	base_ch = chb;
	base_ev = evb;
	base_ts = tsb;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::kevent(kq, ch != nullptr ?
	    (const struct kevent *)(chb + KEV_OFF) : nullptr, nchanges,
	    ev != nullptr ? (struct kevent *)(evb + KEV_OFF) : nullptr,
	    nevents, ts != nullptr ?
	    (const struct timespec *)(tsb + TS_PAD) : nullptr);
	snap_b = mock;

	cmp_snap(FN_KEVENT, snap_a, snap_b, ctx);
	cmp_buf(FN_KEVENT, "ch", cha, chb, sizeof(cha), ctx);
	cmp_buf(FN_KEVENT, "ev", eva, evb, sizeof(eva), ctx);
	cmp_buf(FN_KEVENT, "ts", tsa, tsb, sizeof(tsa), ctx);
	cmp_ret(FN_KEVENT, ra, rb, ctx);
}

static void
case_open(const char *path, int flags, int mode, long long ret)
{
	unsigned char patha[PATH_TOTAL], pathb[PATH_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_OPEN]++;

	memset(patha, GUARD, sizeof(patha));
	memset(pathb, GUARD, sizeof(pathb));

	if (path != nullptr) {
		path_copy(patha + PATH_OFF, path, PATH_CAP);
		path_copy(pathb + PATH_OFF, path, PATH_CAP);
	}

	snprintf(ctx, sizeof(ctx),
	    "path=%d flags=0x%x mode=0x%x creat=%d ret=%lld", path != nullptr,
	    flags, mode, (flags & O_CREAT) != 0, ret);

	base_path = patha;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = call_ref_open(path != nullptr ?
	    (const char *)(patha + PATH_OFF) : nullptr, flags, mode);
	snap_a = mock;

	base_path = pathb;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = call_port_open(path != nullptr ?
	    (const char *)(pathb + PATH_OFF) : nullptr, flags, mode);
	snap_b = mock;

	cmp_snap(FN_OPEN, snap_a, snap_b, ctx);
	cmp_buf(FN_OPEN, "path", patha, pathb, sizeof(patha), ctx);
	cmp_ret(FN_OPEN, ra, rb, ctx);
}

static void
test_setcontext(void)
{
	ucontext_t uc;
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	memset(&uc, 0, sizeof(uc));

	for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
		case_setcontext(nullptr, rets[r]);
		case_setcontext(&uc, rets[r]);
	}

	for (unsigned b = 0x00; b <= 0xff; b++) {
		memset(&uc, (int)(char)b, sizeof(uc));
		case_setcontext(&uc, (long long)(int8_t)b);
	}

	{
		ucontext_t zero;

		memset(&zero, 0, sizeof(zero));
		case_setcontext(&zero, 0);
	}

	rng_seed(0x736574637478ULL);
	for (int n = 0; n < 200000; n++) {
		memset(&uc, (int)(char)rng_u32(), sizeof(uc));
		case_setcontext((rng_u32() & 7u) != 0 ? &uc : nullptr, rnd_ret());
	}
}

static void
test_recvfrom(void)
{
	unsigned char msg[BUF_CAP];
	struct sockaddr_in sin;
	socklen_t flen;
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const size_t lens[] = {
		0, 1, 2, BUF_CAP - 1, BUF_CAP, BUF_CAP + 1, SIZE_MAX,
	};
	static const int flags_list[] = {
		INT_MIN, -1, 0, 1, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const long long rets[] = {
		0, 1, -1, SSIZE_MAX, SSIZE_MIN, 0x7f, 0x80,
	};

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(80);
	flen = sizeof(sin);

	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t l = 0; l < sizeof(lens) / sizeof(lens[0]); l++)
			for (size_t f = 0;
			    f < sizeof(flags_list) / sizeof(flags_list[0]); f++)
				for (size_t r = 0;
				    r < sizeof(rets) / sizeof(rets[0]); r++)
					for (int m = 0; m < 8; m++) {
						fill_pattern(msg, sizeof(msg),
						    (unsigned char)(s + f));
						case_recvfrom(socks[s], (m & 1) ?
						    msg : nullptr, lens[l],
						    flags_list[f], (m & 2) ?
						    (struct sockaddr *)&sin :
						    nullptr, (m & 4) ? &flen :
						    nullptr, rets[r]);
					}

	for (unsigned b = 0x00; b <= 0xff; b++) {
		memset(msg, (int)(char)b, sizeof(msg));
		sin.sin_addr.s_addr = htonl((uint32_t)b << 24);
		flen = (socklen_t)b;
		case_recvfrom((int)(int8_t)b, msg, (size_t)(b % 16u),
		    (int)(int8_t)~b, (struct sockaddr *)&sin, &flen,
		    (long long)(int8_t)b);
	}

	{
		unsigned char one = 0;

		case_recvfrom(0, &one, 0, 0, nullptr, nullptr, 0);
		case_recvfrom(1, &one, 1, -1, nullptr, nullptr, 1);
	}

	rng_seed(0x726563766672ULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 7u);
		size_t len = rnd_size(BUF_CAP);
		uint64_t wide = rnd_wide();

		fill_pattern(msg, sizeof(msg), (unsigned char)rng_u32());
		flen = (socklen_t)rng_u32();
		if (wide != 0)
			len = (size_t)wide;
		case_recvfrom(rnd_int(), (m & 1) != 0 ? msg : nullptr, len,
		    rnd_int(), (m & 2) != 0 ? (struct sockaddr *)&sin :
		    nullptr, (m & 4) != 0 ? &flen : nullptr, rnd_ret());
	}
}

static void
test_kevent(void)
{
	struct kevent ch[KEV_MAX], ev[KEV_MAX];
	struct timespec ts;
	static const int kqs[] = {
		INT_MIN, -1, 0, 1, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const int nchanges_list[] = {
		INT_MIN, -1, 0, 1, 2, KEV_MAX - 1, KEV_MAX, KEV_MAX + 1,
	};
	static const int nevents_list[] = {
		INT_MIN, -1, 0, 1, 2, KEV_MAX - 1, KEV_MAX, KEV_MAX + 1,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (int i = 0; i < KEV_MAX; i++) {
		ch[i].ident = (uintptr_t)(i + 1);
		ch[i].filter = (short)(EVFILT_READ + i);
		ch[i].flags = (unsigned short)(0x80 + i);
		ch[i].fflags = (unsigned int)(i * 3);
		ch[i].data = (intptr_t)(i - 4);
		ch[i].udata = (void *)(intptr_t)(i ^ 0x55);
		ev[i] = ch[i];
	}
	ts.tv_sec = 1;
	ts.tv_nsec = 2;

	for (size_t k = 0; k < sizeof(kqs) / sizeof(kqs[0]); k++)
		for (size_t nc = 0;
		    nc < sizeof(nchanges_list) / sizeof(nchanges_list[0]); nc++)
			for (size_t ne = 0;
			    ne < sizeof(nevents_list) / sizeof(nevents_list[0]);
			    ne++)
				for (size_t r = 0;
				    r < sizeof(rets) / sizeof(rets[0]); r++)
					for (int m = 0; m < 8; m++)
						case_kevent(kqs[k], (m & 1) ?
						    ch : nullptr,
						    nchanges_list[nc],
						    (m & 2) ? ev : nullptr,
						    nevents_list[ne],
						    (m & 4) ? &ts : nullptr,
						    rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		for (int i = 0; i < KEV_MAX; i++) {
			memset(&ch[i], (int)(char)b, sizeof(ch[i]));
			memset(&ev[i], (int)(char)(b ^ 0xff), sizeof(ev[i]));
		}
		ts.tv_sec = (time_t)(int8_t)b;
		ts.tv_nsec = (long)(int8_t)~b;
		case_kevent((int)(int8_t)b, ch, (int)(b % (KEV_MAX + 2u)), ev,
		    (int)(b % (KEV_MAX + 1u)), &ts, (long long)(int8_t)b);
	}

	{
		struct kevent one = {};

		case_kevent(0, &one, 0, &one, 0, nullptr, 0);
		case_kevent(1, nullptr, 0, nullptr, 0, &ts, -1);
	}

	rng_seed(0x6b6576656e74ULL);
	for (int n = 0; n < 200000; n++) {
		int m = (int)(rng_u32() & 7u);
		int nch = rnd_int();
		int nev = rnd_int();

		for (int i = 0; i < KEV_MAX; i++) {
			ch[i].ident = (uintptr_t)rng_u64();
			ch[i].filter = (short)rnd_int();
			ch[i].flags = (unsigned short)rng_u32();
			ch[i].fflags = rng_u32();
			ch[i].data = (intptr_t)rng_u64();
			ev[i] = ch[i];
		}
		ts.tv_sec = (time_t)(int64_t)rng_u64();
		ts.tv_nsec = (long)(int64_t)rng_u64();
		case_kevent(rnd_int(), (m & 1) != 0 ? ch : nullptr, nch,
		    (m & 2) != 0 ? ev : nullptr, nev, (m & 4) != 0 ? &ts :
		    nullptr, rnd_ret());
	}
}

static void
test_open(void)
{
	char path[PATH_CAP];
	static const int flags_list[] = {
		0, O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_CREAT | O_RDWR,
		O_CREAT | O_EXCL, O_TRUNC, O_APPEND, O_CREAT | O_TRUNC,
		0x7e, 0x7f, 0x80, INT_MAX, INT_MIN,
	};
	static const int modes[] = {
		0, 1, 0644, 0755, 0x7e, 0x7f, 0x80, INT_MAX, INT_MIN,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};
	static const char *const paths[] = {
		"",
		"a",
		"/",
		"/tmp/x",
		"\0",
		"abc\0def",
	};

	for (size_t p = 0; p < sizeof(paths) / sizeof(paths[0]); p++)
		for (size_t f = 0; f < sizeof(flags_list) / sizeof(flags_list[0]);
		    f++)
			for (size_t mo = 0; mo < sizeof(modes) / sizeof(modes[0]);
			    mo++)
				for (size_t r = 0;
				    r < sizeof(rets) / sizeof(rets[0]); r++)
					for (int m = 0; m < 2; m++)
						case_open((m & 1) != 0 ?
						    paths[p] : nullptr,
						    flags_list[f], modes[mo],
						    rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		path_fill((unsigned char *)path, sizeof(path),
		    (unsigned char)b);
		case_open(path, (b & 1) != 0 ? (O_CREAT | O_RDWR) : O_RDONLY,
		    (int)(int8_t)b, (long long)(int8_t)b);
	}

	{
		case_open("", 0, 0, 0);
		case_open("x", O_CREAT, 0644, 1);
		case_open("y", O_RDONLY, 0, -1);
		case_open(nullptr, O_CREAT | O_EXCL, 0777, 2);
	}

	rng_seed(0x6f70656e00ULL);
	for (int n = 0; n < 200000; n++) {
		int flags = rnd_int();
		int mode = rnd_int();
		int m = (int)(rng_u32() & 1u);

		path_fill((unsigned char *)path, sizeof(path),
		    (unsigned char)rng_u32());
		case_open((m != 0) ? path : nullptr, flags, mode, rnd_ret());
	}
}

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_setcontext();
	test_recvfrom();
	test_kevent();
	test_open();

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
