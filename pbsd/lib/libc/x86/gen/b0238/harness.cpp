/*
 * Differential harness for batch b0238 (getcontextx.c).
 * Every case drives both the C++23 port and the C oracle; return values,
 * structure fields, and full guard buffers are compared.
 */

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.x86.gen.b0238;

namespace P = pbsd::lib_libc_x86_gen::b0238;

extern "C" {
extern P::u_int cpu_feature2;
void ref_b0238_reset_xstate_sz(void);
int ref___getcontextx_size(void);
int ref___fillcontextx2(char *ctx);
int ref___fillcontextx(char *ctx);
P::ucontext_t *ref___getcontextx(void);
void cpuid_count(unsigned int, unsigned int, unsigned int[4]);
int sysarch(int, void *);
int getcontext(P::ucontext_t *);
void *__real_malloc(size_t);
}

#define	CPUID2_OSXSAVE	0x08000000u
#define	_MC_HASFPXSTATE	0x4u
#define	X86_GET_XFPUSTATE	132

/* ------------------------------------------------------------------ */
/* Statistics                                                         */
/* ------------------------------------------------------------------ */

enum {
	F_SIZE,
	F_FILL2,
	F_FILL,
	F_GET,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"__getcontextx_size",
	"__fillcontextx2",
	"__fillcontextx",
	"__getcontextx"
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 16;
static int nreported;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-20s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0xb0238decade5eedULL;

static std::uint64_t
nextrand(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

/* ------------------------------------------------------------------ */
/* Test doubles shared by port and oracle                             */
/* ------------------------------------------------------------------ */

static unsigned int mock_cpuid_xstate_sz;
static int mock_sysarch_fail;
static unsigned char mock_sysarch_fill;
static int mock_getcontext_fail;
static int mock_getcontext_fill;
static int mock_malloc_fail;

static void
reset_mocks(void)
{
	mock_cpuid_xstate_sz = 0;
	mock_sysarch_fail = 0;
	mock_sysarch_fill = 0xab;
	mock_getcontext_fail = 0;
	mock_getcontext_fill = 0xcd;
	mock_malloc_fail = 0;
	cpu_feature2 = 0;
}

static void
reset_xstate(void)
{
	P::b0238_reset_xstate_sz();
	ref_b0238_reset_xstate_sz();
}

extern "C" void *
__wrap_malloc(size_t n)
{

	if (mock_malloc_fail)
		return (nullptr);
	return (__real_malloc(n));
}

extern "C" void
cpuid_count(unsigned int leaf, unsigned int subleaf, unsigned int p[4])
{

	p[0] = 0;
	p[2] = 0;
	p[3] = 0;
	if (leaf == 0xd && subleaf == 0)
		p[1] = mock_cpuid_xstate_sz;
	else
		p[1] = 0;
}

#define	XSTATE_BUF_CAP	4096u

extern "C" int
sysarch(int code, void *arg)
{
	P::x86_get_xfpustate_t *xfpu;
	size_t n;

	if (code != X86_GET_XFPUSTATE || mock_sysarch_fail != 0) {
		errno = EINVAL;
		return (-1);
	}
	xfpu = (P::x86_get_xfpustate_t *)arg;
	if (xfpu->addr != nullptr && xfpu->len > 0) {
		n = (size_t)xfpu->len;
		if (n > XSTATE_BUF_CAP)
			n = XSTATE_BUF_CAP;
		std::memset(xfpu->addr, mock_sysarch_fill, n);
	}
	return (0);
}

extern "C" int
getcontext(P::ucontext_t *ucp)
{

	if (mock_getcontext_fail != 0) {
		errno = EAGAIN;
		return (-1);
	}
	std::memset(ucp, mock_getcontext_fill, sizeof(*ucp));
	return (0);
}

/* ------------------------------------------------------------------ */
/* Context buffer with guards                                         */
/* ------------------------------------------------------------------ */

struct CtxBuf {
	static constexpr size_t PRE = 64;
	static constexpr size_t XTRA = XSTATE_BUF_CAP;
	static constexpr size_t POST = 64;
	unsigned char bytes[PRE + sizeof(P::ucontext_t) + XTRA + POST];

	void init(void)
	{
		std::memset(bytes, 0x7f, sizeof bytes);
	}

	char *
	ctx(void)
	{
		return ((char *)(bytes + PRE));
	}

	P::ucontext_t *
	ucp(void)
	{
		return ((P::ucontext_t *)ctx());
	}

	char *
	xstate(void)
	{
		return (ctx() + sizeof(P::ucontext_t));
	}

	bool
	identical(const CtxBuf &o) const
	{
		return (std::memcmp(bytes, o.bytes, sizeof bytes) == 0);
	}
};

static intptr_t
xfpustate_offset(P::__register_t v, const char *base)
{

	if (v == 0)
		return (0);
	return ((intptr_t)v - (intptr_t)base);
}

static void
normalize_ctxbuf(CtxBuf &buf)
{
	P::ucontext_t *ucp = buf.ucp();

	if (ucp->uc_mcontext.mc_xfpustate != 0)
		ucp->uc_mcontext.mc_xfpustate =
		    (P::__register_t)sizeof(P::ucontext_t);
	else
		ucp->uc_mcontext.mc_xfpustate = 0;
}

static bool
mcontext_fields_match(const P::mcontext_t &a, const P::mcontext_t &b,
    const char *port_ctx, const char *ref_ctx, const char *ctx, int func)
{
	char msg[256];
	intptr_t aoff, boff;

	aoff = xfpustate_offset(a.mc_xfpustate, port_ctx);
	boff = xfpustate_offset(b.mc_xfpustate, ref_ctx);
	if (aoff != boff) {
		std::snprintf(msg, sizeof msg,
		    "mc_xfpustate off port=%td ref=%td",
		    (ptrdiff_t)aoff, (ptrdiff_t)boff);
		report(func, ctx, msg);
		return (false);
	}
	if (a.mc_xfpustate_len != b.mc_xfpustate_len) {
		std::snprintf(msg, sizeof msg,
		    "mc_xfpustate_len port=%lld ref=%lld",
		    (long long)a.mc_xfpustate_len,
		    (long long)b.mc_xfpustate_len);
		report(func, ctx, msg);
		return (false);
	}
	if (a.mc_flags != b.mc_flags) {
		std::snprintf(msg, sizeof msg,
		    "mc_flags port=0x%x ref=0x%x", a.mc_flags, b.mc_flags);
		report(func, ctx, msg);
		return (false);
	}
	return (true);
}

static void
prime_xstate_sz(unsigned int features, unsigned int cpuid_sz)
{
	cpu_feature2 = features;
	mock_cpuid_xstate_sz = cpuid_sz;
	(void)P::__getcontextx_size();
	(void)ref___getcontextx_size();
}

/* ------------------------------------------------------------------ */
/* __getcontextx_size                                                 */
/* ------------------------------------------------------------------ */

static bool
size_ok(unsigned int features, unsigned int cpuid_sz, const char *ctx)
{
	int a, b;

	reset_xstate();
	reset_mocks();
	cpu_feature2 = features;
	mock_cpuid_xstate_sz = cpuid_sz;

	a = P::__getcontextx_size();
	b = ref___getcontextx_size();

	if (a != b) {
		char msg[128];

		std::snprintf(msg, sizeof msg,
		    "ret port=%d ref=%d feat=0x%x cpuid_sz=%u",
		    a, b, features, cpuid_sz);
		report(F_SIZE, ctx, msg);
		return (false);
	}
	return (true);
}

static void
test_size_edges(void)
{
	static const unsigned int feats[] = { 0, CPUID2_OSXSAVE,
	    CPUID2_OSXSAVE | 1u, 0xffffffffu };
	static const unsigned int szs[] = { 0, 1, 512,
	    (unsigned)sizeof(P::savex86_t),
	    (unsigned)sizeof(P::savex86_t) + 1,
	    (unsigned)sizeof(P::savex86_t) + 64,
	    (unsigned)sizeof(P::savex86_t) + 512,
	    8192, 0x80000000u };

	for (unsigned int f : feats) {
		for (unsigned int s : szs) {
			ncases[F_SIZE]++;
			size_ok(f, s, "edge");
		}
	}

	/* Repeated calls must stay stable once xstate_sz is primed. */
	reset_xstate();
	reset_mocks();
	cpu_feature2 = CPUID2_OSXSAVE;
	mock_cpuid_xstate_sz = (unsigned)sizeof(P::savex86_t) + 128;
	for (int i = 0; i < 4; i++) {
		ncases[F_SIZE]++;
		size_ok(cpu_feature2, mock_cpuid_xstate_sz, "repeat");
	}
}

static void
test_size_random(void)
{
	for (int i = 0; i < 200000; i++) {
		unsigned int f = (unsigned int)nextrand();
		unsigned int s = (unsigned int)nextrand();
		char ctx[48];

		if ((nextrand() & 1u) == 0u)
			f &= ~CPUID2_OSXSAVE;
		else
			f |= CPUID2_OSXSAVE;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_SIZE]++;
		size_ok(f, s, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* __fillcontextx2                                                    */
/* ------------------------------------------------------------------ */

static bool
fill2_ok(unsigned int features, unsigned int cpuid_sz, int sysarch_fail,
    unsigned char sys_fill, const char *ctx)
{
	CtxBuf pb, rb;
	int a, b;

	reset_xstate();
	reset_mocks();
	prime_xstate_sz(features, cpuid_sz);

	pb.init();
	rb.init();
	mock_sysarch_fail = sysarch_fail;
	mock_sysarch_fill = sys_fill;

	a = P::__fillcontextx2(pb.ctx());
	b = ref___fillcontextx2(rb.ctx());

	if (a != b) {
		char msg[160];

		std::snprintf(msg, sizeof msg,
		    "ret port=%d ref=%d feat=0x%x cpuid=%u sysfail=%d",
		    a, b, features, cpuid_sz, sysarch_fail);
		report(F_FILL2, ctx, msg);
		return (false);
	}
	if (!mcontext_fields_match(pb.ucp()->uc_mcontext,
	    rb.ucp()->uc_mcontext, pb.ctx(), rb.ctx(), ctx, F_FILL2))
		return (false);
	{
		CtxBuf pc = pb, rc = rb;

		normalize_ctxbuf(pc);
		normalize_ctxbuf(rc);
		if (!pc.identical(rc)) {
			report(F_FILL2, ctx, "full guard buffer mismatch");
			return (false);
		}
	}
	return (true);
}

static unsigned int
safe_cpuid_sz(unsigned int s)
{
	unsigned int min = (unsigned int)sizeof(P::savex86_t);

	if (s < min)
		return (min + (s % 512u));
	return (s);
}

static void
test_fill2_edges(void)
{
	static const unsigned int feats[] = { 0, CPUID2_OSXSAVE };
	static const unsigned int szs[] = { 0,
	    (unsigned)sizeof(P::savex86_t) + 16,
	    (unsigned)sizeof(P::savex86_t) + 256 };
	static const int fails[] = { 0, 1 };
	static const unsigned char fills[] = { 0x00, 0x80, 0xff, 0x7f, 0xab };

	for (unsigned int f : feats) {
		for (unsigned int s : szs) {
			unsigned int cs = (f & CPUID2_OSXSAVE) ? safe_cpuid_sz(s) : s;
			for (int sf : fails) {
				for (unsigned char fl : fills) {
					ncases[F_FILL2]++;
					fill2_ok(f, cs, sf, fl, "edge");
				}
			}
		}
	}

	/* Pre-set mc_flags to exercise OR-assignment. */
	{
		CtxBuf pb, rb;

		reset_xstate();
		reset_mocks();
		prime_xstate_sz(CPUID2_OSXSAVE,
		    (unsigned)sizeof(P::savex86_t) + 32);
		pb.init();
		rb.init();
		pb.ucp()->uc_mcontext.mc_flags = 0xfffffffbu;
		rb.ucp()->uc_mcontext.mc_flags = 0xfffffffbu;
		ncases[F_FILL2]++;
		if (P::__fillcontextx2(pb.ctx()) !=
		    ref___fillcontextx2(rb.ctx()))
			report(F_FILL2, "flags-or", "preset mc_flags case");
		else if (!mcontext_fields_match(pb.ucp()->uc_mcontext,
		    rb.ucp()->uc_mcontext, pb.ctx(), rb.ctx(),
		    "flags-or", F_FILL2)) {
			report(F_FILL2, "flags-or", "preset mc_flags case");
		} else {
			CtxBuf pc = pb, rc = rb;

			normalize_ctxbuf(pc);
			normalize_ctxbuf(rc);
			if (!pc.identical(rc))
				report(F_FILL2, "flags-or", "preset mc_flags case");
		}
	}
}

static void
test_fill2_random(void)
{
	for (int i = 0; i < 200000; i++) {
		unsigned int f = (unsigned int)nextrand();
		unsigned int s = (unsigned int)(nextrand() % 8192u);
		int sf = (int)(nextrand() & 1u);
		unsigned char fl = (unsigned char)(nextrand() & 0xffu);
		char ctx[48];

		if ((nextrand() & 1u) == 0u)
			f &= ~CPUID2_OSXSAVE;
		else {
			f |= CPUID2_OSXSAVE;
			s = safe_cpuid_sz(s);
		}

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_FILL2]++;
		fill2_ok(f, s, sf, fl, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* __fillcontextx                                                     */
/* ------------------------------------------------------------------ */

static bool
fill_ok(unsigned int features, unsigned int cpuid_sz, int gc_fail,
    int sysarch_fail, unsigned char gc_fill, unsigned char sys_fill,
    const char *ctx)
{
	CtxBuf pb, rb;
	int a, b;

	reset_xstate();
	reset_mocks();
	prime_xstate_sz(features, cpuid_sz);

	pb.init();
	rb.init();
	mock_getcontext_fail = gc_fail;
	mock_sysarch_fail = sysarch_fail;
	mock_getcontext_fill = gc_fill;
	mock_sysarch_fill = sys_fill;

	a = P::__fillcontextx(pb.ctx());
	b = ref___fillcontextx(rb.ctx());

	if (a != b) {
		char msg[192];

		std::snprintf(msg, sizeof msg,
		    "ret port=%d ref=%d gc_fail=%d sysfail=%d",
		    a, b, gc_fail, sysarch_fail);
		report(F_FILL, ctx, msg);
		return (false);
	}
	if (!pb.identical(rb)) {
		CtxBuf pc = pb, rc = rb;

		normalize_ctxbuf(pc);
		normalize_ctxbuf(rc);
		if (!pc.identical(rc)) {
			report(F_FILL, ctx, "full guard buffer mismatch");
			return (false);
		}
	}
	if (a == 0 &&
	    !mcontext_fields_match(pb.ucp()->uc_mcontext,
	    rb.ucp()->uc_mcontext, pb.ctx(), rb.ctx(), ctx, F_FILL))
		return (false);
	return (true);
}

static void
test_fill_edges(void)
{
	static const unsigned int feats[] = { 0, CPUID2_OSXSAVE };
	static const int gc_fails[] = { 0, 1 };
	static const int sa_fails[] = { 0, 1 };

	for (unsigned int f : feats) {
		for (int gf : gc_fails) {
			for (int sf : sa_fails) {
				ncases[F_FILL]++;
				fill_ok(f,
				    (unsigned)sizeof(P::savex86_t) + 64,
				    gf, sf, 0xcd, 0x42, "edge");
			}
		}
	}

	ncases[F_FILL]++;
	fill_ok(0, 0, 0, 0, 0x80, 0xff, "noxfpu clean");
	ncases[F_FILL]++;
	fill_ok(CPUID2_OSXSAVE, (unsigned)sizeof(P::savex86_t),
	    0, 0, 0x00, 0xfe, "xfpu exact save size");
}

static void
test_fill_random(void)
{
	for (int i = 0; i < 200000; i++) {
		unsigned int f = (unsigned int)nextrand();
		unsigned int s = (unsigned int)(nextrand() % 8192u);
		int gf = (int)(nextrand() & 1u);
		int sf = (int)(nextrand() & 1u);
		char ctx[48];

		if ((nextrand() & 1u) == 0u)
			f &= ~CPUID2_OSXSAVE;
		else {
			f |= CPUID2_OSXSAVE;
			s = safe_cpuid_sz(s);
		}

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_FILL]++;
		fill_ok(f, s, gf, sf,
		    (unsigned char)(nextrand() & 0xffu),
		    (unsigned char)(nextrand() & 0xffu), ctx);
	}
}

/* ------------------------------------------------------------------ */
/* __getcontextx                                                      */
/* ------------------------------------------------------------------ */

static bool
ucontext_payload_match(const P::ucontext_t *pa, const P::ucontext_t *pb,
    size_t nbytes, const char *ctx)
{
	P::ucontext_t ca = *pa;
	P::ucontext_t cb = *pb;

	if (ca.uc_mcontext.mc_xfpustate != 0)
		ca.uc_mcontext.mc_xfpustate = (P::__register_t)sizeof(P::ucontext_t);
	else
		ca.uc_mcontext.mc_xfpustate = 0;
	if (cb.uc_mcontext.mc_xfpustate != 0)
		cb.uc_mcontext.mc_xfpustate = (P::__register_t)sizeof(P::ucontext_t);
	else
		cb.uc_mcontext.mc_xfpustate = 0;

	if (std::memcmp(&ca, &cb, nbytes) != 0) {
		report(F_GET, ctx, "allocated ucontext payload mismatch");
		return (false);
	}
	return (true);
}

static bool
get_ok(unsigned int features, unsigned int cpuid_sz, int gc_fail,
    int sysarch_fail, int malloc_fail, const char *label)
{
	P::ucontext_t *pa;
	P::ucontext_t *pb;
	size_t expect_sz;
	bool ok = true;

	reset_xstate();
	reset_mocks();
	cpu_feature2 = features;
	mock_cpuid_xstate_sz = cpuid_sz;
	mock_getcontext_fail = gc_fail;
	mock_sysarch_fail = sysarch_fail;
	mock_malloc_fail = malloc_fail;

	expect_sz = (size_t)P::__getcontextx_size();
	if ((int)expect_sz != ref___getcontextx_size()) {
		report(F_GET, label, "size priming mismatch");
		return (false);
	}

	errno = 0;
	pa = P::__getcontextx();
	errno = 0;
	pb = ref___getcontextx();

	if ((pa == nullptr) != (pb == nullptr)) {
		char msg[128];

		std::snprintf(msg, sizeof msg,
		    "null port=%d ref=%d errno=%d",
		    pa == nullptr, pb == nullptr, errno);
		report(F_GET, label, msg);
		ok = false;
	}
	if (pa != nullptr && pb != nullptr) {
		if (!ucontext_payload_match(pa, pb, expect_sz, label))
			ok = false;
	}
	if (pa != nullptr)
		std::free(pa);
	if (pb != nullptr)
		std::free(pb);
	return (ok);
}

static void
test_get_edges(void)
{
	static const unsigned int feats[] = { 0, CPUID2_OSXSAVE };
	static const unsigned int szs[] = { 0,
	    (unsigned)sizeof(P::savex86_t) + 32,
	    (unsigned)sizeof(P::savex86_t) + 512 };

	for (unsigned int f : feats) {
		for (unsigned int s : szs) {
			unsigned int cs = (f & CPUID2_OSXSAVE) ? safe_cpuid_sz(s) : s;

			ncases[F_GET]++;
			get_ok(f, cs, 0, 0, 0, "ok edge");
		}
	}

	ncases[F_GET]++;
	get_ok(CPUID2_OSXSAVE, (unsigned)sizeof(P::savex86_t) + 64,
	    1, 0, 0, "getcontext fail");
	ncases[F_GET]++;
	get_ok(CPUID2_OSXSAVE, (unsigned)sizeof(P::savex86_t) + 64,
	    0, 1, 0, "sysarch fail");
	ncases[F_GET]++;
	get_ok(0, 0, 0, 0, 1, "malloc fail");
}

static void
test_get_random(void)
{
	for (int i = 0; i < 200000; i++) {
		unsigned int f = (unsigned int)nextrand();
		unsigned int s = (unsigned int)(nextrand() % 8192u);
		int gf = (int)(nextrand() & 1u);
		int sf = (int)(nextrand() & 1u);
		char ctx[48];

		if ((nextrand() & 1u) == 0u)
			f &= ~CPUID2_OSXSAVE;
		else {
			f |= CPUID2_OSXSAVE;
			s = safe_cpuid_sz(s);
		}

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_GET]++;
		get_ok(f, s, gf, sf, 0, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	test_size_edges();
	test_size_random();
	test_fill2_edges();
	test_fill2_random();
	test_fill_edges();
	test_fill_random();
	test_get_edges();
	test_get_random();

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NFUNC; i++)
		std::printf("%-22s %12llu %12llu\n", fname[i],
		    ncases[i], nfails[i]);

	unsigned long long total_fail = 0;
	for (int i = 0; i < NFUNC; i++)
		total_fail += nfails[i];

	return (total_fail == 0 ? 0 : 1);
}
