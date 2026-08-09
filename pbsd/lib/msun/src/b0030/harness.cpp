/*
 * Differential test harness for PBSD batch b0030.
 *
 * Every function ported in port.cppm is compared, bit for bit, against the
 * unmodified C reference in oracle.c.  Results are staged through 0x7f-guarded
 * buffers so that any stray write past the nominal result window is caught.
 */

#include <bit>
#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0030;

namespace port = pbsd::lib_msun_src::b0030;

extern "C" {
long double ref_logbl(long double x);
float ref_rintf(float x);
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
static const std::size_t GUARD_BUF = 64;
static const std::size_t GUARD_OFF = 16;

static bool
guarded_equal(const void *pa, const void *pb, std::size_t n)
{
	unsigned char a[GUARD_BUF], b[GUARD_BUF];

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memcpy(a + GUARD_OFF, pa, n);
	std::memcpy(b + GUARD_OFF, pb, n);
	return (std::memcmp(a, b, sizeof(a)) == 0);
}

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

/* ------------------------------------------------------------------ */
/* bookkeeping                                                         */
/* ------------------------------------------------------------------ */

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_logbl = { "logbl", 0, 0, 0 };
static stat st_rintf = { "rintf", 0, 0, 0 };

static const unsigned MAX_REPORT = 8;

/* ------------------------------------------------------------------ */
/* per-function checkers                                               */
/* ------------------------------------------------------------------ */

static void
check_logbl(long double x, const char *tag)
{
	ldrep p, o;

	st_logbl.cases++;

	p = ldbits(port::logbl(x));
	o = ldbits(ref_logbl(x));

	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;

	st_logbl.fails++;
	if (st_logbl.reported < MAX_REPORT) {
		ldrep bx = ldbits(x);
		std::size_t i;

		st_logbl.reported++;
		std::printf("  logbl FAIL [%s] x=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", bx.b[i]);
		std::printf(" port=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

static void
check_rintf(std::uint32_t u, const char *tag)
{
	std::uint32_t p, o;
	float x = fromfbits(u);

	st_rintf.cases++;

	p = fbits(port::rintf(x));
	o = fbits(ref_rintf(x));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_rintf.fails++;
	if (st_rintf.reported < MAX_REPORT) {
		st_rintf.reported++;
		std::printf("  rintf FAIL [%s] in=%08x port=%08x ref=%08x\n",
		    tag, u, p, o);
	}
}

/* ------------------------------------------------------------------ */
/* hand-written edge vectors                                           */
/* ------------------------------------------------------------------ */

static const std::uint32_t fvec[] = {
	0x00000000u,	/* +0 */
	0x80000000u,	/* -0 */
	0x00000001u,	/* smallest +subnormal */
	0x80000001u,	/* smallest -subnormal */
	0x00000080u,
	0x00008000u,
	0x007fffffu,	/* largest +subnormal */
	0x807fffffu,	/* largest -subnormal */
	0x00800000u,	/* smallest +normal */
	0x80800000u,	/* smallest -normal */
	0x00800001u,
	0x3effffffu,	/* just below 0.5 */
	0x3f000000u,	/* +0.5 */
	0x3f000001u,	/* just above 0.5 */
	0x3f7fffffu,	/* just below 1 */
	0x3f800000u,	/* +1 */
	0x3f800001u,	/* just above 1 */
	0xbf000000u,	/* -0.5 */
	0xbf800000u,	/* -1 */
	0x40000000u,	/* +2 */
	0xc0000000u,	/* -2 */
	0x40400000u,	/* +3 */
	0xc0400000u,	/* -3 */
	0x4affffffu,	/* just below 2^23 */
	0x4b000000u,	/* 2^23 */
	0x4b000001u,	/* just above 2^23 */
	0xcaffffffu,
	0xcb000000u,	/* -2^23 */
	0xcb000001u,
	0x4b7fffffu,	/* 2^23 + 0x7fffff */
	0xcb7fffffu,
	0x4b800000u,	/* 2^24 */
	0xcb800000u,
	0x7f7fffffu,	/* FLT_MAX */
	0xff7fffffu,	/* -FLT_MAX */
	0x7f7ffffeu,
	0x7f800000u,	/* +inf */
	0xff800000u,	/* -inf */
	0x7f800001u,	/* smallest +sNaN */
	0xff800001u,
	0x7fc00000u,	/* +qNaN */
	0xffc00000u,	/* -qNaN */
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
	{ 0x0000u, 0x0000000000000001ull },	/* smallest +subnormal */
	{ 0x8000u, 0x0000000000000001ull },	/* smallest -subnormal */
	{ 0x0000u, 0x0000000000000080ull },
	{ 0x0000u, 0x0000000080000000ull },	/* manh==0, manl nonzero */
	{ 0x8000u, 0x0000000080000000ull },
	{ 0x0000u, 0x7fffffffffffffffull },	/* largest +subnormal */
	{ 0x8000u, 0x7fffffffffffffffull },
	{ 0x0000u, 0x8000000000000000ull },	/* manh!=0 denormal */
	{ 0x8000u, 0x8000000000000000ull },
	{ 0x0000u, 0xc000000000000000ull },
	{ 0x0001u, 0x8000000000000000ull },	/* smallest +normal */
	{ 0x8001u, 0x8000000000000000ull },	/* smallest -normal */
	{ 0x0001u, 0x8000000000000001ull },
	{ 0x3ffeu, 0x8000000000000000ull },	/* +0.5 */
	{ 0xbffeu, 0x8000000000000000ull },	/* -0.5 */
	{ 0x3fffu, 0x8000000000000000ull },	/* +1 */
	{ 0xbfffu, 0x8000000000000000ull },	/* -1 */
	{ 0x4000u, 0x8000000000000000ull },	/* +2 */
	{ 0xc000u, 0x8000000000000000ull },	/* -2 */
	{ 0x403eu, 0x8000000000000000ull },	/* 2^63 */
	{ 0x7ffeu, 0xffffffffffffffffull },	/* LDBL_MAX */
	{ 0xfffeu, 0xffffffffffffffffull },
	{ 0x7fffu, 0x8000000000000000ull },	/* +inf */
	{ 0xffffu, 0x8000000000000000ull },	/* -inf */
	{ 0x7fffu, 0xc000000000000000ull },	/* +qNaN */
	{ 0xffffu, 0xc000000000000000ull },	/* -qNaN */
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

	for (i = 0; i < NFVEC; i++)
		check_rintf(fvec[i], "vector");

	/*
	 * rintf exponent boundaries: j0<0, 0<=j0<23, j0==0x80, j0>=23.
	 * exp = (bits>>23)&0xff, j0 = exp - 0x7f.
	 */
	for (i = 0; i <= 255u; i++) {
		std::uint32_t pos = (i << 23) | 0x400000u;
		std::uint32_t neg = (i << 23) | 0x80000000u | 0x400000u;

		check_rintf(pos, "exp-sweep");
		check_rintf(neg, "exp-sweep");
		check_rintf(pos | 0x3fffffu, "exp-sweep");
		check_rintf(neg | 0x3fffffu, "exp-sweep");
	}

	/* Half-integer and quarter-integer ties around small magnitudes. */
	{
		static const float vals[] = {
			0.0f, -0.0f, 0.25f, -0.25f, 0.5f, -0.5f, 0.75f, -0.75f,
			1.0f, -1.0f, 1.25f, -1.25f, 1.5f, -1.5f, 1.75f, -1.75f,
			2.0f, -2.0f, 3.0f, -3.0f, 16777215.0f, -16777215.0f,
			16777216.0f, -16777216.0f, 16777217.0f, -16777217.0f,
			8388607.5f, -8388607.5f, 8388608.0f, -8388608.0f,
			8388608.5f, -8388608.5f, 1.19209290e-7f, -1.19209290e-7f,
			5.96046448e-8f, -5.96046448e-8f, 1.40129846e-45f,
			-1.40129846e-45f, FLT_MIN, -FLT_MIN, FLT_MAX, -FLT_MAX,
			__builtin_inff(), -__builtin_inff(),
			__builtin_nanf(""), -__builtin_nanf(""),
		};

		for (i = 0; i < sizeof(vals) / sizeof(vals[0]); i++)
			check_rintf(fbits(vals[i]), "rational");
	}

	for (i = 0; i < NLDVEC; i++)
		check_logbl(mkld(ldvec[i].se, ldvec[i].m), "vector");

	/*
	 * logbl: walk both sides of exp==0 vs normal and normal vs inf/nan.
	 * Threshold is (LDBL_MAX_EXP << 1) - 1.
	 */
	{
		const int thresh = (LDBL_MAX_EXP << 1) - 1;
		int e;

		for (e = 0; e <= thresh + 1; e++) {
			std::uint16_t se = (std::uint16_t)e;
			std::uint64_t m = (e == 0) ? 0x0000000080000000ull :
			    0x8000000000000000ull;

			check_logbl(mkld(se, m), "exp-boundary");
			check_logbl(mkld((std::uint16_t)(se | 0x8000u), m),
			    "exp-boundary");
		}
	}

	/* Denormal leading-zero loops: manh==0 walks manl, else walks manh. */
	{
		unsigned b;

		for (b = 0; b < 64u; b++) {
			std::uint64_t m;

			m = (std::uint64_t)1 << b;
			check_logbl(mkld(0x0000u, m), "denorm-manl");
			check_logbl(mkld(0x8000u, m), "denorm-manl");

			if (b >= 32u) {
				m = (std::uint64_t)1 << b;
				check_logbl(mkld(0x0000u, m), "denorm-manh");
				check_logbl(mkld(0x8000u, m), "denorm-manh");
			}
		}
	}

	/* Cross-product of interesting float bit patterns. */
	for (i = 0; i < NFVEC; i++)
		for (j = 0; j < NFVEC; j += (NFVEC > 16 ? NFVEC / 16 : 1))
			check_rintf((fvec[i] & 0xff000000u) |
			    (fvec[j] & 0x00ffffffu), "cross");
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

static const unsigned long long ITERS = 250000ull;

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0xd1ce4e5b91234567ull;
	for (i = 0; i < ITERS; i++) {
		std::uint32_t u = (std::uint32_t)rng_next();

		if ((i & 15) == 0)
			u &= 0x80000000u;
		if ((i & 15) == 1)
			u = (std::uint32_t)((rng_next() & 0xffu) << 23);
		if ((i & 15) == 2)
			u |= 0x7f800000u;
		if ((i & 15) == 3)
			u &= 0x007fffffu;

		check_rintf(u, "random");
	}

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		long double x = rng_ld();

		if ((i & 31) == 0)
			x = 0.0L;
		if ((i & 31) == 1)
			x = -0.0L;
		if ((i & 31) == 2)
			x = __builtin_infl();
		if ((i & 31) == 3)
			x = -__builtin_infl();
		if ((i & 31) == 4)
			x = __builtin_nanl("");

		check_logbl(x, "random");
	}
}

/* ------------------------------------------------------------------ */

static void
row(const stat &s)
{
	std::printf("  %-10s %12llu %10llu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "PASS" : "FAIL");
}

int
main(void)
{
	unsigned long long fails;

	std::printf("pbsd batch b0030 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-10s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  --------------------------------------------------\n");
	row(st_logbl);
	row(st_rintf);

	fails = st_logbl.fails + st_rintf.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
