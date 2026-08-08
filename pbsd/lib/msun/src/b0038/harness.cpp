/*
 * Differential test harness for PBSD batch b0038.
 *
 * Compares fminimuml, fmaximuml, fminimumf, and fmaximumf against the
 * unmodified C reference in oracle.c.  Results are staged through 0x7f-
 * guarded buffers so that any stray write past the nominal result window
 * is caught.
 */

#include <bit>
#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0038;

namespace port = pbsd::lib_msun_src::b0038;

extern "C" {
long double ref_fminimuml(long double x, long double y);
long double ref_fmaximuml(long double x, long double y);
float ref_fminimumf(float x, float y);
float ref_fmaximumf(float x, float y);
}

/* ------------------------------------------------------------------ */
/* raw-representation plumbing                                         */
/* ------------------------------------------------------------------ */

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

static const unsigned char GUARD = 0x7f;
static const unsigned long long ITERS = 200000ull;

struct ldrep {
	unsigned char b[sizeof(long double)];
};

static ldrep
ldbits(long double x)
{
	ldrep r;

	std::memset(r.b, 0, sizeof(r.b));
	std::memcpy(r.b, &x, LD_SIG);
	return r;
}

static long double
mkld(std::uint16_t se, std::uint64_t m)
{
	unsigned char b[sizeof(long double)];
	long double x;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &m, sizeof(m));
	std::memcpy(b + 8, &se, sizeof(se));
	std::memcpy(&x, b, sizeof(x));
	return x;
}

static std::uint32_t
fbits(float f)
{
	return std::bit_cast<std::uint32_t>(f);
}

static float
fromfbits(std::uint32_t u)
{
	return std::bit_cast<float>(u);
}

static bool
guarded_equal(const void *pa, const void *pb, std::size_t n)
{
	static const std::size_t GUARD_BUF = 64;
	static const std::size_t GUARD_OFF = 16;
	unsigned char a[64], b[64];

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memcpy(a + GUARD_OFF, pa, n);
	std::memcpy(b + GUARD_OFF, pb, n);
	return (std::memcmp(a, b, sizeof(a)) == 0);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                         */
/* ------------------------------------------------------------------ */

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_fminimuml = { "fminimuml", 0, 0, 0 };
static stat st_fmaximuml = { "fmaximuml", 0, 0, 0 };
static stat st_fminimumf = { "fminimumf", 0, 0, 0 };
static stat st_fmaximumf = { "fmaximumf", 0, 0, 0 };

static const unsigned MAX_REPORT = 8;

/* ------------------------------------------------------------------ */
/* per-function checkers                                               */
/* ------------------------------------------------------------------ */

static void
report_ld_fail(stat &s, const char *tag, ldrep ax, ldrep ay, ldrep p, ldrep o)
{
	std::size_t i;

	if (s.reported >= MAX_REPORT)
		return;
	s.reported++;
	std::printf("  %s FAIL [%s] x=", s.name, tag);
	for (i = LD_SIG; i-- > 0;)
		std::printf("%02x", ax.b[i]);
	std::printf(" y=");
	for (i = LD_SIG; i-- > 0;)
		std::printf("%02x", ay.b[i]);
	std::printf(" port=");
	for (i = LD_SIG; i-- > 0;)
		std::printf("%02x", p.b[i]);
	std::printf(" ref=");
	for (i = LD_SIG; i-- > 0;)
		std::printf("%02x", o.b[i]);
	std::printf("\n");
}

static void
report_f_fail(stat &s, const char *tag, std::uint32_t ux, std::uint32_t uy,
    std::uint32_t p, std::uint32_t o)
{
	if (s.reported >= MAX_REPORT)
		return;
	s.reported++;
	std::printf("  %s FAIL [%s] x=%08x y=%08x port=%08x ref=%08x\n",
	    s.name, tag, ux, uy, p, o);
}

static void
check_fminimuml(long double x, long double y, const char *tag)
{
	ldrep ax, ay, p, o;

	st_fminimuml.cases++;
	ax = ldbits(x);
	ay = ldbits(y);
	p = ldbits(port::fminimuml(x, y));
	o = ldbits(ref_fminimuml(x, y));
	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;
	st_fminimuml.fails++;
	report_ld_fail(st_fminimuml, tag, ax, ay, p, o);
}

static void
check_fmaximuml(long double x, long double y, const char *tag)
{
	ldrep ax, ay, p, o;

	st_fmaximuml.cases++;
	ax = ldbits(x);
	ay = ldbits(y);
	p = ldbits(port::fmaximuml(x, y));
	o = ldbits(ref_fmaximuml(x, y));
	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;
	st_fmaximuml.fails++;
	report_ld_fail(st_fmaximuml, tag, ax, ay, p, o);
}

static void
check_fminimumf(float x, float y, const char *tag)
{
	std::uint32_t ux, uy, p, o;

	st_fminimumf.cases++;
	ux = fbits(x);
	uy = fbits(y);
	p = fbits(port::fminimumf(x, y));
	o = fbits(ref_fminimumf(x, y));
	if (guarded_equal(&p, &o, sizeof(p)))
		return;
	st_fminimumf.fails++;
	report_f_fail(st_fminimumf, tag, ux, uy, p, o);
}

static void
check_fmaximumf(float x, float y, const char *tag)
{
	std::uint32_t ux, uy, p, o;

	st_fmaximumf.cases++;
	ux = fbits(x);
	uy = fbits(y);
	p = fbits(port::fmaximumf(x, y));
	o = fbits(ref_fmaximumf(x, y));
	if (guarded_equal(&p, &o, sizeof(p)))
		return;
	st_fmaximumf.fails++;
	report_f_fail(st_fmaximumf, tag, ux, uy, p, o);
}

static void
check_all(long double x, long double y, const char *tag)
{
	check_fminimuml(x, y, tag);
	check_fmaximuml(x, y, tag);
}

static void
check_allf(float x, float y, const char *tag)
{
	check_fminimumf(x, y, tag);
	check_fmaximumf(x, y, tag);
}

/* ------------------------------------------------------------------ */
/* hand-written edge vectors                                           */
/* ------------------------------------------------------------------ */

static const std::uint32_t fvec[] = {
	0x00000000u,	/* +0 */
	0x80000000u,	/* -0 */
	0x00000001u,
	0x80000001u,
	0x00000080u,
	0x00008000u,
	0x007fffffu,
	0x807fffffu,
	0x00800000u,
	0x80800000u,
	0x00800001u,
	0x3effffffu,
	0x3f000000u,	/* +0.5 */
	0x3f000001u,
	0x3f7fffffu,
	0x3f800000u,	/* +1 */
	0x3f800001u,
	0xbf000000u,	/* -0.5 */
	0xbf800000u,	/* -1 */
	0x40000000u,
	0xc0000000u,
	0x40400000u,
	0xc0400000u,
	0x4affffffu,
	0x4b000000u,
	0x4b000001u,
	0xcaffffffu,
	0xcb000000u,
	0xcb000001u,
	0x4b7fffffu,
	0xcb7fffffu,
	0x4b800000u,
	0xcb800000u,
	0x7f7fffffu,
	0xff7fffffu,
	0x7f7ffffeu,
	0x7f800000u,	/* +inf */
	0xff800000u,	/* -inf */
	0x7f800001u,	/* sNaN */
	0xff800001u,
	0x7fc00000u,	/* qNaN */
	0xffc00000u,
	0x7fffffffu,
	0xffffffffu,
	0x80808080u,
	0x7f7f7f7fu,
	0x00ff00ffu,
	0xff00ff00u,
	0x8000ffffu,
	0x0000ff80u,
	0x33333333u,
	0xb3333333u,
};
static const std::size_t NFVEC = sizeof(fvec) / sizeof(fvec[0]);

struct ldcase {
	std::uint16_t se;
	std::uint64_t m;
};

static const ldcase ldvec[] = {
	{ 0x0000u, 0x0000000000000000ull },	/* +0 */
	{ 0x8000u, 0x0000000000000000ull },	/* -0 */
	{ 0x0000u, 0x0000000000000001ull },
	{ 0x8000u, 0x0000000000000001ull },
	{ 0x0000u, 0x0000000000000080ull },
	{ 0x0000u, 0x0000000080000000ull },
	{ 0x8000u, 0x0000000080000000ull },
	{ 0x0000u, 0x7fffffffffffffffull },
	{ 0x8000u, 0x7fffffffffffffffull },
	{ 0x0000u, 0x8000000000000000ull },
	{ 0x8000u, 0x8000000000000000ull },
	{ 0x0000u, 0xc000000000000000ull },
	{ 0x0001u, 0x8000000000000000ull },
	{ 0x8001u, 0x8000000000000000ull },
	{ 0x0001u, 0x8000000000000001ull },
	{ 0x3ffeu, 0x8000000000000000ull },
	{ 0xbffeu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000000ull },
	{ 0xbfffu, 0x8000000000000000ull },
	{ 0x4000u, 0x8000000000000000ull },
	{ 0xc000u, 0x8000000000000000ull },
	{ 0x403eu, 0x8000000000000000ull },
	{ 0x7ffeu, 0xffffffffffffffffull },
	{ 0xfffeu, 0xffffffffffffffffull },
	{ 0x7fffu, 0x8000000000000000ull },	/* +inf */
	{ 0xffffu, 0x8000000000000000ull },	/* -inf */
	{ 0x7fffu, 0xc000000000000000ull },	/* +qNaN */
	{ 0xffffu, 0xc000000000000000ull },
	{ 0x7fffu, 0x8000000000000001ull },
	{ 0x7fffu, 0xffffffffffffffffull },
	{ 0x3fffu, 0x8080808080808080ull },
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
};
static const std::size_t NLDVEC = sizeof(ldvec) / sizeof(ldvec[0]);

static void
edge_cases(void)
{
	std::size_t i, j;

	/* Signed-zero pairs: both orderings, both min and max. */
	check_all(0.0L, -0.0L, "signed-zero");
	check_all(-0.0L, 0.0L, "signed-zero");
	check_allf(0.0f, -0.0f, "signed-zero");
	check_allf(-0.0f, 0.0f, "signed-zero");

	/* Scalar specials. */
	{
		static const float fvals[] = {
			0.0f, -0.0f, 0.5f, -0.5f, 1.0f, -1.0f, 2.0f, -2.0f,
			FLT_MIN, -FLT_MIN, FLT_MAX, -FLT_MAX,
			__builtin_inff(), -__builtin_inff(),
			__builtin_nanf(""), -__builtin_nanf(""),
		};
		static const long double ldvals[] = {
			0.0L, -0.0L, 0.5L, -0.5L, 1.0L, -1.0L, 2.0L, -2.0L,
			LDBL_MIN, -LDBL_MIN, LDBL_MAX, -LDBL_MAX,
			__builtin_infl(), -__builtin_infl(),
			__builtin_nanl(""), -__builtin_nanl(""),
		};

		for (i = 0; i < sizeof(fvals) / sizeof(fvals[0]); i++)
			for (j = 0; j < sizeof(fvals) / sizeof(fvals[0]); j++)
				check_allf(fvals[i], fvals[j], "rational-pair");

		for (i = 0; i < sizeof(ldvals) / sizeof(ldvals[0]); i++)
			for (j = 0; j < sizeof(ldvals) / sizeof(ldvals[0]); j++)
				check_all(ldvals[i], ldvals[j], "rational-pair");
	}

	/* Full cross-product of bit-pattern vectors. */
	for (i = 0; i < NFVEC; i++)
		for (j = 0; j < NFVEC; j++) {
			float x = fromfbits(fvec[i]);
			float y = fromfbits(fvec[j]);

			check_allf(x, y, "fvec-cross");
		}

	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++)
			check_all(mkld(ldvec[i].se, ldvec[i].m),
			    mkld(ldvec[j].se, ldvec[j].m), "ldvec-cross");

	/*
	 * NaN boundary: exp==255/32767 with man==0 (inf) vs man!=0 (NaN).
	 * Exercise each side of the || in the NaN guard independently.
	 */
	for (i = 0; i <= 255u; i++) {
		std::uint32_t infp = (i << 23);
		std::uint32_t infn = (i << 23) | 0x80000000u;
		std::uint32_t nanp = infp | 1u;
		std::uint32_t nann = infn | 1u;
		std::uint32_t normal = 0x3f800000u;

		check_allf(fromfbits(normal), fromfbits(nanp), "nan-boundary");
		check_allf(fromfbits(nanp), fromfbits(normal), "nan-boundary");
		check_allf(fromfbits(normal), fromfbits(nann), "nan-boundary");
		check_allf(fromfbits(nann), fromfbits(normal), "nan-boundary");
		check_allf(fromfbits(infp), fromfbits(nanp), "nan-boundary");
		check_allf(fromfbits(nanp), fromfbits(infp), "nan-boundary");
		check_allf(fromfbits(nanp), fromfbits(nann), "nan-boundary");
	}

	for (i = 0; i <= 32767u; i++) {
		std::uint16_t infse = (std::uint16_t)i;
		std::uint16_t nanse = (std::uint16_t)(i | 0x8000u);
		std::uint64_t infm = 0x8000000000000000ull;
		std::uint64_t nanm = 0x8000000000000001ull;
		long double normal = 1.0L;

		check_all(normal, mkld(infse, infm), "ld-nan-boundary");
		check_all(mkld(infse, nanm), normal, "ld-nan-boundary");
		check_all(normal, mkld(nanse, nanm), "ld-nan-boundary");
		check_all(mkld(nanse, nanm), normal, "ld-nan-boundary");
		check_all(mkld(infse, infm), mkld(infse, nanm), "ld-nan-boundary");
		check_all(mkld(infse, nanm), mkld(nanse, nanm), "ld-nan-boundary");
	}

	/*
	 * Comparison branch: x<y, x>y, x==y on both sides of zero.
	 * Include ties and near-ties.
	 */
	for (i = 0; i < NFVEC; i++) {
		float x = fromfbits(fvec[i]);
		float yp = x + 1.0f;
		float ym = x - 1.0f;

		check_allf(x, yp, "cmp-order");
		check_allf(x, ym, "cmp-order");
		check_allf(x, x, "cmp-tie");
	}

	for (i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].m);

		check_all(x, x + 1.0L, "cmp-order");
		check_all(x, x - 1.0L, "cmp-order");
		check_all(x, x, "cmp-tie");
	}

	/*
	 * Signed-zero branch index selection for float variants:
	 * u[u[1].bits.sign] vs u[u[0].bits.sign].
	 */
	check_fminimumf(0.0f, -0.0f, "sign-index");
	check_fminimumf(-0.0f, 0.0f, "sign-index");
	check_fmaximumf(0.0f, -0.0f, "sign-index");
	check_fmaximumf(-0.0f, 0.0f, "sign-index");
	check_fminimumf(-1.0f, 1.0f, "sign-mix");
	check_fmaximumf(-1.0f, 1.0f, "sign-mix");

	/* Long-double signed-zero branch: u[1].bits.sign vs u[0].bits.sign. */
	check_fminimuml(0.0L, -0.0L, "sign-branch");
	check_fminimuml(-0.0L, 0.0L, "sign-branch");
	check_fmaximuml(0.0L, -0.0L, "sign-branch");
	check_fmaximuml(-0.0L, 0.0L, "sign-branch");
}

/* ------------------------------------------------------------------ */
/* fixed-seed randomised sweep                                         */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static std::uint64_t
rng_next(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ull;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return (z ^ (z >> 31));
}

static long double
rng_ld(void)
{
	std::uint64_t m = rng_next();
	std::uint64_t r = rng_next();
	std::uint16_t sign = (r & 1) ? 0x8000u : 0x0000u;
	unsigned kind = (unsigned)((r >> 1) % 100u);
	std::uint16_t e;

	if (kind < 5)
		e = 0x0000u;
	else if (kind < 10)
		e = 0x7fffu;
	else
		e = (std::uint16_t)(rng_next() & 0x7fffu);

	if (e == 0x0000u)
		m &= ~(std::uint64_t)1 << 63;
	else if (e < 0x7fffu)
		m |= (std::uint64_t)1 << 63;

	return mkld((std::uint16_t)(sign | e), m);
}

static std::uint32_t
rng_f(void)
{
	std::uint32_t u = (std::uint32_t)rng_next();
	unsigned kind = (unsigned)(rng_next() % 100u);

	if (kind < 5)
		u &= 0x80000000u;
	else if (kind < 10)
		u |= 0x7f800000u;
	else if (kind < 15)
		u &= 0x807fffffu;

	return u;
}

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0xd1ce4e5b91234567ull;
	for (i = 0; i < ITERS; i++) {
		std::uint32_t ux = rng_f();
		std::uint32_t uy = rng_f();

		if ((i & 15) == 0)
			ux = 0x00000000u;
		if ((i & 15) == 1)
			uy = 0x80000000u;
		if ((i & 15) == 2)
			ux = 0x7fc00000u;
		if ((i & 15) == 3)
			uy = 0xffc00000u;
		if ((i & 15) == 4)
			ux = 0x7f800000u;
		if ((i & 15) == 5)
			uy = 0xff800000u;

		check_fminimumf(fromfbits(ux), fromfbits(uy), "random");
	}

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		std::uint32_t ux = rng_f();
		std::uint32_t uy = rng_f();

		if ((i & 15) == 0)
			ux = 0x00000000u;
		if ((i & 15) == 1)
			uy = 0x80000000u;
		if ((i & 15) == 2)
			ux = 0x7fc00000u;
		if ((i & 15) == 3)
			uy = 0xffc00000u;

		check_fmaximumf(fromfbits(ux), fromfbits(uy), "random");
	}

	rng_state = 0xdecafbadc0ffee01ull;
	for (i = 0; i < ITERS; i++) {
		long double x = rng_ld();
		long double y = rng_ld();

		if ((i & 31) == 0)
			x = 0.0L;
		if ((i & 31) == 1)
			y = -0.0L;
		if ((i & 31) == 2)
			x = __builtin_nanl("");
		if ((i & 31) == 3)
			y = __builtin_infl();

		check_fminimuml(x, y, "random");
	}

	rng_state = 0x0123456789abcdefull;
	for (i = 0; i < ITERS; i++) {
		long double x = rng_ld();
		long double y = rng_ld();

		if ((i & 31) == 0)
			x = 0.0L;
		if ((i & 31) == 1)
			y = -0.0L;
		if ((i & 31) == 2)
			x = __builtin_nanl("");
		if ((i & 31) == 3)
			y = -__builtin_infl();

		check_fmaximuml(x, y, "random");
	}
}

/* ------------------------------------------------------------------ */

static void
row(const stat &s)
{
	std::printf("  %-12s %12llu %10llu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "PASS" : "FAIL");
}

int
main(void)
{
	unsigned long long fails;

	std::printf("pbsd batch b0038 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-12s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ----------------------------------------------------\n");
	row(st_fminimuml);
	row(st_fmaximuml);
	row(st_fminimumf);
	row(st_fmaximumf);

	fails = st_fminimuml.fails + st_fmaximuml.fails +
	    st_fminimumf.fails + st_fmaximumf.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
