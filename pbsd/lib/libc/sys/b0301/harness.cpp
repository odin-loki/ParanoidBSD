/*
 * harness.cpp -- differential test for PBSD batch b0301.
 *
 * swapcontext, fcntl, __wait and send are libc interposition wrappers: each
 * loads a function pointer from __libc_interposing[] and tail-calls it with the
 * original argument list.  The harness installs instrumented mocks in both the
 * oracle (ref_*) and port tables, then compares the mock-visible dispatch tag,
 * every forwarded argument (scalars and pointer offsets from each buffer
 * base), content hashes of every buffer the callee reads, the entire
 * guard-filled buffers (including padding), and the return value.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ucontext.h>
#include <sys/wait.h>

#ifndef SSIZE_MIN
#define	SSIZE_MIN	((ssize_t)(-SSIZE_MAX - 1))
#endif

#ifndef WAIT_ANY
#define	WAIT_ANY	(-1)
#endif

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_swapcontext,
	INTERPOS_fcntl,
	INTERPOS_wait4,
	INTERPOS_sendto,
	INTERPOS_MAX
};

extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];

int ref_swapcontext(ucontext_t *oucp, const ucontext_t *ucp);
int ref_fcntl(int fd, int cmd, ...);
pid_t ref___wait(int *istat);
ssize_t ref_send(int s, const void *msg, size_t len, int flags);

}

import pbsd.lib.libc.sys.b0301;

namespace port = pbsd::lib_libc_sys::b0301;

#define	GUARD		0x7f

#define	CTX_PAD		64
#define	CTX_TOTAL	(sizeof(ucontext_t) + 2 * CTX_PAD)
#define	CTX_OFF		CTX_PAD

#define	BUF_CAP		64
#define	BUF_PAD		64
#define	BUF_TOTAL	(BUF_CAP + 2 * BUF_PAD)
#define	BUF_OFF		BUF_PAD

#define	ISTAT_PAD	64
#define	ISTAT_TOTAL	(sizeof(int) + 2 * ISTAT_PAD)
#define	ISTAT_OFF	ISTAT_PAD

#define	TAG_NONE		0
#define	TAG_SWAPCONTEXT		1
#define	TAG_FCNTL		2
#define	TAG_WAIT4		3
#define	TAG_SENDTO		4

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

static const unsigned char *base_oucp;
static const unsigned char *base_ucp;
static const unsigned char *base_buf;
static const unsigned char *base_istat;

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
mock_swapcontext(ucontext_t *oucp, const ucontext_t *ucp)
{
	mock_enter(TAG_SWAPCONTEXT);
	mock.po[0] = poff(oucp, base_oucp);
	mock.po[1] = poff(ucp, base_ucp);

	if (oucp != nullptr)
		mock.hs[0] = hash_bytes(oucp, sizeof(ucontext_t));
	if (ucp != nullptr)
		mock.hs[1] = hash_bytes(ucp, sizeof(ucontext_t));
	return ((int)mock_ret);
}

static int
mock_fcntl(int fd, int cmd, long arg)
{
	mock_enter(TAG_FCNTL);
	mock.sc[0] = (long long)fd;
	mock.sc[1] = (long long)cmd;
	mock.sc[2] = (long long)arg;
	return ((int)mock_ret);
}

static pid_t
mock_wait4(pid_t pid, int *istat, int options, struct rusage *rup)
{
	mock_enter(TAG_WAIT4);
	mock.sc[0] = (long long)pid;
	mock.sc[1] = (long long)options;
	mock.po[0] = poff(istat, base_istat);
	mock.po[1] = poff(rup, nullptr);

	if (istat != nullptr) {
		mock.hs[0] = hash_bytes(istat, sizeof(int));
		*istat = (int)(pid ^ options);
		mock.wr = (long long)*istat;
	}
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
	mock.sc[3] = (long long)tolen;
	mock.po[0] = poff(msg, base_buf);
	mock.po[1] = poff(to, nullptr);

	if (msg != nullptr)
		mock.hs[0] = hash_bytes(msg, len < BUF_CAP ? len : (size_t)BUF_CAP);
	if (msg != nullptr && len > 0) {
		size_t n = len < BUF_CAP ? len : (size_t)BUF_CAP;

		fill_pattern((unsigned char *)msg, n, (unsigned char)(s & 0xff));
	}
	return ((ssize_t)mock_ret);
}

static void
install_mocks(interpos_func_t *table)
{
	table[INTERPOS_swapcontext] = (interpos_func_t)mock_swapcontext;
	table[INTERPOS_fcntl] = (interpos_func_t)mock_fcntl;
	table[INTERPOS_wait4] = (interpos_func_t)mock_wait4;
	table[INTERPOS_sendto] = (interpos_func_t)mock_sendto;
}

enum {
	FN_SWAPCONTEXT,
	FN_FCNTL,
	FN_WAIT,
	FN_SEND,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"swapcontext",
	"fcntl",
	"__wait",
	"send",
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

	char sa[1024], sb[1024], msg[4096];

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

static long
rnd_long(void)
{
	return ((long)(int32_t)rng_u32());
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
ctx_fill(unsigned char *dst, size_t cap, unsigned char seed)
{
	size_t i;

	for (i = 0; i < cap; i++)
		dst[i] = (unsigned char)(seed + (unsigned char)i);
}

static void
case_swapcontext(const ucontext_t *oucp_in, const ucontext_t *ucp_in,
    long long ret)
{
	unsigned char oucpa[CTX_TOTAL], oucpb[CTX_TOTAL];
	unsigned char ucpa[CTX_TOTAL], ucpb[CTX_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_SWAPCONTEXT]++;

	memset(oucpa, GUARD, sizeof(oucpa));
	memset(oucpb, GUARD, sizeof(oucpb));
	memset(ucpa, GUARD, sizeof(ucpa));
	memset(ucpb, GUARD, sizeof(ucpb));

	if (oucp_in != nullptr) {
		memcpy(oucpa + CTX_OFF, oucp_in, sizeof(ucontext_t));
		memcpy(oucpb + CTX_OFF, oucp_in, sizeof(ucontext_t));
	}
	if (ucp_in != nullptr) {
		memcpy(ucpa + CTX_OFF, ucp_in, sizeof(ucontext_t));
		memcpy(ucpb + CTX_OFF, ucp_in, sizeof(ucontext_t));
	}

	snprintf(ctx, sizeof(ctx), "oucp=%d ucp=%d ret=%lld",
	    oucp_in != nullptr, ucp_in != nullptr, ret);

	base_oucp = oucpa;
	base_ucp = ucpa;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_swapcontext(oucp_in != nullptr ?
	    (ucontext_t *)(oucpa + CTX_OFF) : nullptr,
	    ucp_in != nullptr ? (const ucontext_t *)(ucpa + CTX_OFF) :
	    nullptr);
	snap_a = mock;

	base_oucp = oucpb;
	base_ucp = ucpb;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::swapcontext(oucp_in != nullptr ?
	    (ucontext_t *)(oucpb + CTX_OFF) : nullptr,
	    ucp_in != nullptr ? (const ucontext_t *)(ucpb + CTX_OFF) :
	    nullptr);
	snap_b = mock;

	cmp_snap(FN_SWAPCONTEXT, snap_a, snap_b, ctx);
	cmp_buf(FN_SWAPCONTEXT, "oucp", oucpa, oucpb, sizeof(oucpa), ctx);
	cmp_buf(FN_SWAPCONTEXT, "ucp", ucpa, ucpb, sizeof(ucpa), ctx);
	cmp_ret(FN_SWAPCONTEXT, ra, rb, ctx);
}

static void
case_fcntl(int fd, int cmd, long arg, long long ret)
{
	Snap snap_a, snap_b;
	int ra, rb;
	char ctx[256];

	fn_cases[FN_FCNTL]++;

	snprintf(ctx, sizeof(ctx), "fd=%d cmd=0x%x arg=%ld ret=%lld", fd, cmd,
	    arg, ret);

	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_fcntl(fd, cmd, arg);
	snap_a = mock;

	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::fcntl(fd, cmd, arg);
	snap_b = mock;

	cmp_snap(FN_FCNTL, snap_a, snap_b, ctx);
	cmp_ret(FN_FCNTL, ra, rb, ctx);
}

static void
case_wait(const int *istat_in, long long ret)
{
	unsigned char istata[ISTAT_TOTAL], istatb[ISTAT_TOTAL];
	Snap snap_a, snap_b;
	pid_t ra, rb;
	char ctx[256];

	fn_cases[FN_WAIT]++;

	memset(istata, GUARD, sizeof(istata));
	memset(istatb, GUARD, sizeof(istatb));

	if (istat_in != nullptr) {
		memcpy(istata + ISTAT_OFF, istat_in, sizeof(int));
		memcpy(istatb + ISTAT_OFF, istat_in, sizeof(int));
	}

	snprintf(ctx, sizeof(ctx), "istat=%d ret=%lld", istat_in != nullptr,
	    ret);

	base_istat = istata;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref___wait(istat_in != nullptr ?
	    (int *)(istata + ISTAT_OFF) : nullptr);
	snap_a = mock;

	base_istat = istatb;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::__wait(istat_in != nullptr ?
	    (int *)(istatb + ISTAT_OFF) : nullptr);
	snap_b = mock;

	cmp_snap(FN_WAIT, snap_a, snap_b, ctx);
	cmp_buf(FN_WAIT, "istat", istata, istatb, sizeof(istata), ctx);
	cmp_ret(FN_WAIT, ra, rb, ctx);
}

static void
case_send(int s, const void *msg, size_t len, int flags, long long ret)
{
	unsigned char bufa[BUF_TOTAL], bufb[BUF_TOTAL];
	Snap snap_a, snap_b;
	ssize_t ra, rb;
	char ctx[256];

	fn_cases[FN_SEND]++;

	memset(bufa, GUARD, sizeof(bufa));
	memset(bufb, GUARD, sizeof(bufb));

	if (msg != nullptr) {
		size_t n = len < BUF_CAP ? len : (size_t)BUF_CAP;

		memcpy(bufa + BUF_OFF, msg, n);
		memcpy(bufb + BUF_OFF, msg, n);
	}

	snprintf(ctx, sizeof(ctx), "s=%d len=%zu fl=0x%x buf=%d ret=%lld", s,
	    (size_t)len, flags, msg != nullptr, ret);

	base_buf = bufa;
	install_mocks(ref___libc_interposing);
	mock_reset(ret);
	ra = ref_send(s, msg != nullptr ? (void *)(bufa + BUF_OFF) : nullptr,
	    len, flags);
	snap_a = mock;

	base_buf = bufb;
	install_mocks(port::__libc_interposing);
	mock_reset(ret);
	rb = port::send(s, msg != nullptr ? (void *)(bufb + BUF_OFF) : nullptr,
	    len, flags);
	snap_b = mock;

	cmp_snap(FN_SEND, snap_a, snap_b, ctx);
	cmp_buf(FN_SEND, "buf", bufa, bufb, sizeof(bufa), ctx);
	cmp_ret(FN_SEND, ra, rb, ctx);
}

static void
test_swapcontext(void)
{
	ucontext_t oucp, ucp;
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	memset(&oucp, 0, sizeof(oucp));
	memset(&ucp, 0, sizeof(ucp));

	for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
		for (int m = 0; m < 4; m++)
			case_swapcontext((m & 1) != 0 ? &oucp : nullptr,
			    (m & 2) != 0 ? &ucp : nullptr, rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		ctx_fill((unsigned char *)&oucp, sizeof(oucp),
		    (unsigned char)b);
		ctx_fill((unsigned char *)&ucp, sizeof(ucp),
		    (unsigned char)(~b));
		case_swapcontext(&oucp, &ucp, (long long)(int8_t)b);
	}

	{
		case_swapcontext(nullptr, nullptr, 0);
		case_swapcontext(&oucp, nullptr, -1);
		case_swapcontext(nullptr, &ucp, 1);
	}

	rng_seed(0x7377617063ULL);
	for (int n = 0; n < 50000; n++) {
		int m = (int)(rng_u32() & 3u);

		ctx_fill((unsigned char *)&oucp, sizeof(oucp),
		    (unsigned char)rng_u32());
		ctx_fill((unsigned char *)&ucp, sizeof(ucp),
		    (unsigned char)rng_u32());
		case_swapcontext((m & 1) != 0 ? &oucp : nullptr,
		    (m & 2) != 0 ? &ucp : nullptr, rnd_ret());
	}
}

static void
test_fcntl(void)
{
	static const int fds[] = {
		INT_MIN, -1, 0, 1, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const int cmds[] = {
		F_GETFD, F_SETFD, F_GETFL, F_SETFL, 0, 1, 0x7e, 0x7f, 0x80,
		INT_MAX, INT_MIN,
	};
	static const long args[] = {
		0, 1, -1, LONG_MAX, LONG_MIN, 0x7e, 0x7f, 0x80,
		(long)UINT32_MAX, (long)INT32_MAX, (long)INT32_MIN,
	};
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	for (size_t f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
		for (size_t c = 0; c < sizeof(cmds) / sizeof(cmds[0]); c++)
			for (size_t a = 0; a < sizeof(args) / sizeof(args[0]);
			    a++)
				for (size_t r = 0;
				    r < sizeof(rets) / sizeof(rets[0]); r++)
					case_fcntl(fds[f], cmds[c], args[a],
					    rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++)
		case_fcntl((int)(int8_t)b, (int)(int8_t)~b, (long)(int8_t)b,
		    (long long)(int8_t)b);

	{
		case_fcntl(0, F_GETFD, 0, 0);
		case_fcntl(1, F_SETFD, 1, -1);
		case_fcntl(-1, 0, -1, 1);
	}

	rng_seed(0x66636e746cULL);
	for (int n = 0; n < 50000; n++)
		case_fcntl(rnd_int(), rnd_int(), rnd_long(), rnd_ret());
}

static void
test_wait(void)
{
	int istat;
	static const long long rets[] = {
		0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX,
	};

	istat = 42;

	for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
		for (int m = 0; m < 2; m++)
			case_wait((m & 1) != 0 ? &istat : nullptr, rets[r]);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		istat = (int)(int8_t)b;
		case_wait(&istat, (long long)(int8_t)b);
	}

	{
		case_wait(nullptr, 0);
		case_wait(&istat, -1);
		case_wait(&istat, 1);
	}

	rng_seed(0x7761697400ULL);
	for (int n = 0; n < 50000; n++) {
		int m = (int)(rng_u32() & 1u);

		istat = rnd_int();
		case_wait((m != 0) ? &istat : nullptr, rnd_ret());
	}
}

static void
test_send(void)
{
	unsigned char msg[BUF_CAP];
	static const int socks[] = {
		INT_MIN, -1, 0, 1, 0x7e, 0x7f, 0x80, INT_MAX,
	};
	static const size_t lens[] = {
		0, 1, 2, BUF_CAP - 1, BUF_CAP, BUF_CAP + 1, SIZE_MAX,
	};
	static const int flags_list[] = {
		INT_MIN, -1, 0, 1, MSG_PEEK, MSG_DONTWAIT, 0x7e, 0x7f, 0x80,
		INT_MAX,
	};
	static const long long rets[] = {
		0, 1, -1, SSIZE_MAX, SSIZE_MIN, 0x7f, 0x80,
	};

	for (size_t s = 0; s < sizeof(socks) / sizeof(socks[0]); s++)
		for (size_t l = 0; l < sizeof(lens) / sizeof(lens[0]); l++)
			for (size_t f = 0;
			    f < sizeof(flags_list) / sizeof(flags_list[0]); f++)
				for (size_t r = 0;
				    r < sizeof(rets) / sizeof(rets[0]); r++)
					for (int m = 0; m < 2; m++) {
						fill_pattern(msg, sizeof(msg),
						    (unsigned char)(s + f));
						case_send(socks[s], (m & 1) ?
						    msg : nullptr, lens[l],
						    flags_list[f], rets[r]);
					}

	for (unsigned b = 0x00; b <= 0xff; b++) {
		memset(msg, (int)(char)b, sizeof(msg));
		case_send((int)(int8_t)b, msg, (size_t)(b % 16u),
		    (int)(int8_t)~b, (long long)(int8_t)b);
	}

	{
		unsigned char one = 0;

		case_send(0, &one, 0, 0, 0);
		case_send(1, &one, 1, -1, 1);
		case_send(0, nullptr, 0, 0, -1);
	}

	rng_seed(0x73656e6400ULL);
	for (int n = 0; n < 50000; n++) {
		int m = (int)(rng_u32() & 1u);
		size_t len = rnd_size(BUF_CAP);
		uint64_t wide = rnd_wide();

		fill_pattern(msg, sizeof(msg), (unsigned char)rng_u32());
		if (wide != 0)
			len = (size_t)wide;
		case_send(rnd_int(), (m != 0) ? msg : nullptr, len, rnd_int(),
		    rnd_ret());
	}
}

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_swapcontext();
	test_fcntl();
	test_wait();
	test_send();

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
