/*
 * Differential test harness for PBSD batch b0060.
 *
 * Every function ported in port.cppm is compared against the unmodified C
 * reference in oracle.c.  Integer results are staged through 0x7f-guarded
 * buffers and compared in their entirety.  Floating-point environment state
 * after each llround* call is compared as well so that feraiseexcept paths are
 * observable.  drem() results are compared bit for bit through guarded double
 * buffers.
 */

#include <bit>
#include <cfenv>
#include <climits>
#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0060;

namespace port = pbsd::lib_msun_src::b0060;

extern "C" {
long long ref_llround(double x);
long long ref_llroundf(float x);
long long ref_llroundl(long double x);
double ref_drem(double x, double y);
}

/* ------------------------------------------------------------------ */
/* utilities                                                         */
/* ------------------------------------------------------------------ */

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

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

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

static std::uint64_t
dbits(double d)
{
	return std::bit_cast<std::uint64_t>(d);
}

static double
fromdbits(std::uint64_t u)
{
	return std::bit_cast<double>(u);
}

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_llround = { "llround", 0, 0, 0 };
static stat st_llroundf = { "llroundf", 0, 0, 0 };
static stat st_llroundl = { "llroundl", 0, 0, 0 };
static stat st_drem = { "drem", 0, 0, 0 };

static const unsigned MAX_REPORT = 10;

static void
prep_fenv(int round)
{

	::fesetround(round);
	::feclearexcept(FE_ALL_EXCEPT);
}

/* ------------------------------------------------------------------ */
/* llround / llroundf / llroundl                                     */
/* ------------------------------------------------------------------ */

static void
check_llround(double x, int round, const char *tag)
{
	long long pr, rr;
	int pex, rex;

	st_llround.cases++;

	prep_fenv(round);
	pr = port::llround(x);
	pex = ::fetestexcept(FE_ALL_EXCEPT);

	prep_fenv(round);
	rr = ref_llround(x);
	rex = ::fetestexcept(FE_ALL_EXCEPT);

	if (guarded_equal(&pr, &rr, sizeof(pr)) && pex == rex)
		return;

	st_llround.fails++;
	if (st_llround.reported < MAX_REPORT) {
		st_llround.reported++;
		std::printf("  llround FAIL [%s] round=%d x=%#018llx port=%lld/%#x "
		    "ref=%lld/%#x\n", tag, round, (unsigned long long)dbits(x),
		    pr, pex, rr, rex);
	}
}

static void
check_llroundf(float x, int round, const char *tag)
{
	long long pr, rr;
	int pex, rex;

	st_llroundf.cases++;

	prep_fenv(round);
	pr = port::llroundf(x);
	pex = ::fetestexcept(FE_ALL_EXCEPT);

	prep_fenv(round);
	rr = ref_llroundf(x);
	rex = ::fetestexcept(FE_ALL_EXCEPT);

	if (guarded_equal(&pr, &rr, sizeof(pr)) && pex == rex)
		return;

	st_llroundf.fails++;
	if (st_llroundf.reported < MAX_REPORT) {
		st_llroundf.reported++;
		std::printf("  llroundf FAIL [%s] round=%d x=%#010x port=%lld/%#x "
		    "ref=%lld/%#x\n", tag, round, fbits(x), pr, pex, rr, rex);
	}
}

static void
check_llroundl(long double x, int round, const char *tag)
{
	long long pr, rr;
	int pex, rex;

	st_llroundl.cases++;

	prep_fenv(round);
	pr = port::llroundl(x);
	pex = ::fetestexcept(FE_ALL_EXCEPT);

	prep_fenv(round);
	rr = ref_llroundl(x);
	rex = ::fetestexcept(FE_ALL_EXCEPT);

	if (guarded_equal(&pr, &rr, sizeof(pr)) && pex == rex)
		return;

	st_llroundl.fails++;
	if (st_llroundl.reported < MAX_REPORT) {
		ldrep bx = ldbits(x);

		st_llroundl.reported++;
		std::printf("  llroundl FAIL [%s] round=%d x=", tag, round);
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", bx.b[i]);
		std::printf(" port=%lld/%#x ref=%lld/%#x\n", pr, pex, rr, rex);
	}
}

/* ------------------------------------------------------------------ */
/* drem                                                              */
/* ------------------------------------------------------------------ */

static void
check_drem(double x, double y, const char *tag)
{
	double pr, rr;
	int pex, rex;

	st_drem.cases++;

	prep_fenv(FE_TONEAREST);
	pr = port::drem(x, y);
	pex = ::fetestexcept(FE_ALL_EXCEPT);

	prep_fenv(FE_TONEAREST);
	rr = ref_drem(x, y);
	rex = ::fetestexcept(FE_ALL_EXCEPT);

	if (guarded_equal(&pr, &rr, sizeof(pr)) && pex == rex)
		return;

	st_drem.fails++;
	if (st_drem.reported < MAX_REPORT) {
		st_drem.reported++;
		std::printf("  drem FAIL [%s] x=%#018llx y=%#018llx port=%#018llx/%#x "
		    "ref=%#018llx/%#x\n", tag,
		    (unsigned long long)dbits(x),
		    (unsigned long long)dbits(y),
		    (unsigned long long)dbits(pr), pex,
		    (unsigned long long)dbits(rr), rex);
	}
}

/* ------------------------------------------------------------------ */
/* edge vectors                                                      */
/* ------------------------------------------------------------------ */

static const std::uint32_t fvec[] = {
	0x00000000u, 0x80000000u,
	0x00000001u, 0x80000001u,
	0x00000080u, 0x00008000u,
	0x007fffffu, 0x807fffffu,
	0x00800000u, 0x80800000u,
	0x00800001u,
	0x3f000000u, 0xbf000000u,
	0x3f800000u, 0xbf800000u,
	0x40000000u, 0xc0000000u,
	0x40400000u, 0xc0400000u,
	0x4b000000u, 0xcb000000u,
	0x4f000000u, 0xcf000000u,
	0x5f000000u, 0xdf000000u,
	0x7f000000u, 0xff000000u,
	0x7f7ffffeu, 0xff7ffffeu,
	0x7f7fffffu, 0xff7fffffu,
	0x7f800000u, 0xff800000u,
	0x7f800001u, 0xff800001u,
	0x7fa00000u, 0xffa00000u,
	0x7fbfffffu, 0xffbfffffu,
	0x7fc00000u, 0xffc00000u,
	0x7fc00001u, 0xffc00001u,
	0x7fffffffu, 0xffffffffu,
	0x80808080u, 0x7f7f7f7fu,
	0x00ff00ffu, 0xff00ff00u,
	0x8000ffffu, 0x0000ff80u,
};
static const std::size_t NFVEC = sizeof(fvec) / sizeof(fvec[0]);

static const std::uint64_t dvec[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x0000000000000080ull, 0x0000800000000000ull,
	0x000fffffffffffffull, 0x800fffffffffffffull,
	0x0010000000000000ull, 0x8010000000000000ull,
	0x3ff0000000000000ull, 0xbff0000000000000ull,
	0x3ff8000000000000ull, 0xbff8000000000000ull,
	0x4000000000000000ull, 0xc000000000000000ull,
	0x4040000000000000ull, 0xc040000000000000ull,
	0x433fffffffffffffull, 0xc33fffffffffffffull,
	0x43efffffffffffffull, 0xc3efffffffffffffull,
	0x7fefffffffffffffull, 0xffefffffffffffffull,
	0x7ff0000000000000ull, 0xfff0000000000000ull,
	0x7ff8000000000000ull, 0xfff8000000000000ull,
	0x7ff8000000000001ull, 0xfff8000000000001ull,
	0x7fffffffffffffffull, 0xffffffffffffffffull,
	0x8080808080808080ull, 0x7f7f7f7f7f7f7f7full,
};
static const std::size_t NDVEC = sizeof(dvec) / sizeof(dvec[0]);

struct ldcase {
	std::uint16_t se;
	std::uint64_t m;
};

static const ldcase ldvec[] = {
	{ 0x0000u, 0x0000000000000000ull },
	{ 0x8000u, 0x0000000000000000ull },
	{ 0x0000u, 0x0000000000000001ull },
	{ 0x8000u, 0x0000000000000001ull },
	{ 0x0000u, 0x0000000000000080ull },
	{ 0x0000u, 0x7fffffffffffffffull },
	{ 0x8000u, 0x7fffffffffffffffull },
	{ 0x0001u, 0x8000000000000000ull },
	{ 0x8001u, 0x8000000000000000ull },
	{ 0x0001u, 0x8000000000000001ull },
	{ 0x3ffeu, 0x8000000000000000ull },
	{ 0xbffeu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000000ull },
	{ 0xbfffu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000001ull },
	{ 0x3ffeu, 0xffffffffffffffffull },
	{ 0x4000u, 0x8000000000000000ull },
	{ 0xc000u, 0x8000000000000000ull },
	{ 0x4000u, 0xc000000000000000ull },
	{ 0xc000u, 0xc000000000000000ull },
	{ 0x403eu, 0x8000000000000000ull },
	{ 0x403fu, 0x8000000000000000ull },
	{ 0x407fu, 0x8000000000000000ull },
	{ 0x7ffeu, 0xffffffffffffffffull },
	{ 0xfffeu, 0xffffffffffffffffull },
	{ 0x7fffu, 0x8000000000000000ull },
	{ 0xffffu, 0x8000000000000000ull },
	{ 0x7fffu, 0xc000000000000000ull },
	{ 0xffffu, 0xc000000000000000ull },
	{ 0x7fffu, 0xa000000000000000ull },
	{ 0x7fffu, 0x8000000000000001ull },
	{ 0x7fffu, 0xffffffffffffffffull },
	{ 0x3fffu, 0x8080808080808080ull },
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
};
static const std::size_t NLDVEC = sizeof(ldvec) / sizeof(ldvec[0]);

static const int rounds[] = {
	FE_TONEAREST,
	FE_TOWARDZERO,
	FE_UPWARD,
	FE_DOWNWARD,
};

static void
edge_cases(void)
{
	std::size_t i, j, r;

	for (r = 0; r < sizeof(rounds) / sizeof(rounds[0]); r++) {
		for (i = 0; i < NLDVEC; i++) {
			long double x = mkld(ldvec[i].se, ldvec[i].m);

			check_llroundl(x, rounds[r], "ldvec");
		}

		for (i = 0; i < NFVEC; i++) {
			check_llroundf(fromfbits(fvec[i]), rounds[r], "fvec");
			check_llround((double)fromfbits(fvec[i]), rounds[r],
			    "fvec");
		}

		for (i = 0; i < NDVEC; i++)
			check_llround(fromdbits(dvec[i]), rounds[r], "dvec");
	}

	/*
	 * Walk both sides of every INRANGE boundary for llroundl and the
	 * compile-time dtype_max - type_max != 0.5 guard for llroundf.
	 */
	{
		const long double ld_llong_min = (long double)LLONG_MIN;
		const long double ld_llong_max = (long double)LLONG_MAX;
		const long double ld_min_b = ld_llong_min - 0.5L;
		const long double ld_max_b = ld_llong_max + 0.5L;
		static const long double eps[] = {
			0.0L, 0.25L, -0.25L, 0.5L, -0.5L,
			1e-18L, -1e-18L, 1e-9L, -1e-9L,
			LDBL_EPSILON, -LDBL_EPSILON,
		};

		for (r = 0; r < sizeof(rounds) / sizeof(rounds[0]); r++) {
			for (i = 0; i < sizeof(eps) / sizeof(eps[0]); i++) {
				long double d;

				d = ld_min_b + eps[i];
				check_llroundl(d, rounds[r], "llong_min_b");

				d = ld_max_b + eps[i];
				check_llroundl(d, rounds[r], "llong_max_b");

				d = ld_llong_min + eps[i];
				check_llroundl(d, rounds[r], "llong_min");

				d = ld_llong_max + eps[i];
				check_llroundl(d, rounds[r], "llong_max");
			}
		}
	}

	/*
	 * llroundf: float has less precision than long long, so INRANGE is
	 * always true — still exercise roundf on integers, ties, NaNs and
	 * extremes.
	 */
	{
		static const float specials[] = {
			0.0f, -0.0f, 0.5f, -0.5f, 1.0f, -1.0f,
			1.5f, -1.5f, 2.0f, -2.0f, 3.0f, -3.0f,
			2147483520.0f, -2147483520.0f,
			4294967296.0f, -4294967296.0f,
			__builtin_inff(), -__builtin_inff(),
			__builtin_nanf(""), -__builtin_nanf("1"),
			FLT_MIN, -FLT_MIN, FLT_MAX, -FLT_MAX,
			FLT_TRUE_MIN, -FLT_TRUE_MIN,
		};

		for (r = 0; r < sizeof(rounds) / sizeof(rounds[0]); r++)
			for (i = 0; i < sizeof(specials) / sizeof(specials[0]);
			    i++)
				check_llroundf(specials[i], rounds[r], "special");
	}

	/*
	 * llround: double has less precision than long long, so INRANGE is
	 * always true — exercise round on integers, ties, NaNs and extremes.
	 */
	{
		static const double dspecials[] = {
			0.0, -0.0, 0.5, -0.5, 1.0, -1.0,
			1.5, -1.5, 2.0, -2.0,
			9007199254740992.0, -9007199254740992.0,
			9007199254740993.0, -9007199254740993.0,
			__builtin_inf(), -__builtin_inf(),
			__builtin_nan(""), -__builtin_nan("1"),
			DBL_MIN, -DBL_MIN, DBL_MAX, -DBL_MAX,
			DBL_TRUE_MIN, -DBL_TRUE_MIN,
		};

		for (r = 0; r < sizeof(rounds) / sizeof(rounds[0]); r++)
			for (i = 0; i < sizeof(dspecials) / sizeof(dspecials[0]);
			    i++)
				check_llround(dspecials[i], rounds[r], "special");
	}

	/* Dense ulp walks around float bit-pattern boundaries. */
	{
		static const std::uint32_t anchors[] = {
			0x00000000u, 0x00800000u, 0x007fffffu,
			0x7f800000u, 0x7f7fffffu, 0x7fffffffu,
			0x80000000u, 0x80800000u, 0xff800000u,
			0xffffffffu, 0x3f800000u, 0xbf800000u,
			0x4f000000u, 0xcf000000u,
		};

		for (r = 0; r < sizeof(rounds) / sizeof(rounds[0]); r++)
			for (i = 0; i < sizeof(anchors) / sizeof(anchors[0]);
			    i++)
				for (int d = -48; d <= 48; d++)
					check_llroundf(
					    fromfbits(anchors[i] +
						(std::uint32_t)d),
					    rounds[r], "walk");
	}

	/* Dense ulp walks around double bit-pattern boundaries. */
	{
		static const std::uint64_t anchors[] = {
			0x0000000000000000ull, 0x0010000000000000ull,
			0x000fffffffffffffull, 0x7ff0000000000000ull,
			0x7fefffffffffffffull, 0x7fffffffffffffffull,
			0x8000000000000000ull, 0x8010000000000000ull,
			0xfff0000000000000ull, 0xffffffffffffffffull,
			0x3ff0000000000000ull, 0xbff0000000000000ull,
			0x43efffffffffffffull, 0xc3efffffffffffffull,
		};

		for (r = 0; r < sizeof(rounds) / sizeof(rounds[0]); r++)
			for (i = 0; i < sizeof(anchors) / sizeof(anchors[0]);
			    i++)
				for (int d = -48; d <= 48; d++)
					check_llround(
					    fromdbits(anchors[i] +
						(std::uint64_t)d),
					    rounds[r], "walk");
	}

	/* Cross combinations that mix signs and magnitudes. */
	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++) {
			long double x = mkld(ldvec[i].se, ldvec[i].m);
			long double y = mkld(ldvec[j].se, ldvec[j].m);

			check_llroundl(x + y, FE_TONEAREST, "cross");
			check_llroundl(x - y, FE_TONEAREST, "cross");
			check_llroundl(x * y, FE_TONEAREST, "cross");
		}

	/* drem edge cases: zeros, infinities, NaNs, sign combinations. */
	{
		static const double xspecial[] = {
			0.0, -0.0, 1.0, -1.0, 3.0, -3.0, 7.0, -7.0,
			0.5, -0.5, 1.5, -1.5,
			__builtin_inf(), -__builtin_inf(),
			__builtin_nan(""), -__builtin_nan("1"),
			DBL_MIN, -DBL_MIN, DBL_MAX, -DBL_MAX,
			1e100, -1e100, 1e-100, -1e-100,
		};
		static const double yspecial[] = {
			0.0, -0.0, 1.0, -1.0, 2.0, -2.0, 3.0, -3.0,
			0.5, -0.5, 1.5, -1.5,
			__builtin_inf(), -__builtin_inf(),
			__builtin_nan(""), -__builtin_nan("2"),
			DBL_MIN, -DBL_MIN, DBL_MAX, -DBL_MAX,
			1e50, -1e50, 1e-50, -1e-50,
		};

		for (i = 0; i < sizeof(xspecial) / sizeof(xspecial[0]); i++)
			for (j = 0; j < sizeof(yspecial) / sizeof(yspecial[0]);
			    j++)
				check_drem(xspecial[i], yspecial[j], "special");
	}

	for (i = 0; i < NDVEC; i++)
		for (j = 0; j < NDVEC; j++)
			check_drem(fromdbits(dvec[i]), fromdbits(dvec[j]),
			    "dvec");
}

/* ------------------------------------------------------------------ */
/* fixed-seed randomised sweep                                       */
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
rng_ld(int spread)
{
	std::uint64_t m = rng_next();
	std::uint64_t r = rng_next();
	std::uint16_t sign = (r & 1) ? 0x8000u : 0x0000u;
	unsigned kind = (unsigned)((r >> 1) % 100u);
	std::uint16_t e;

	if (kind < 4)
		e = 0x0000u;
	else if (kind < 7)
		e = 0x7fffu;
	else
		e = (std::uint16_t)(0x3fff +
		    (int)(rng_next() % (std::uint64_t)(2 * spread + 1)) -
		    spread);

	if (e == 0x0000u)
		m &= ~(std::uint64_t)1 << 63;
	else
		m |= (std::uint64_t)1 << 63;

	return mkld((std::uint16_t)(sign | e), m);
}

static std::uint32_t
rand_fbits(void)
{
	std::uint32_t r = (std::uint32_t)rng_next();

	switch ((unsigned)(rng_next() % 12u)) {
	case 0:
		return r;
	case 1:
		return r & 0x807fffffu;
	case 2:
		return (r & 0x807fffffu) | 0x7f800000u;
	case 3: {
		std::uint32_t d = (std::uint32_t)(rng_next() % 11u) - 5u;
		return (r & 0x80000000u) |
		    ((0x7f800000u + d) & 0x7fffffffu);
	}
	case 4: {
		std::uint32_t d = (std::uint32_t)(rng_next() % 11u) - 5u;
		return (r & 0x80000000u) |
		    ((0x00800000u + d) & 0x7fffffffu);
	}
	case 5: {
		std::uint32_t e = 97u + (std::uint32_t)(rng_next() % 61u);
		return (r & 0x80000000u) | (e << 23) | (r & 0x007fffffu);
	}
	case 6: {
		int k = (int)(rng_next() % 2049u) - 1024;
		return fbits((float)k / 4.0f);
	}
	case 7: {
		int k = (int)(rng_next() % 401u) - 200;
		return fbits((float)k);
	}
	case 8: {
		std::uint32_t e = (rng_next() & 1u) ?
		    (std::uint32_t)(rng_next() % 8u) :
		    248u + (std::uint32_t)(rng_next() % 8u);
		return (r & 0x80000000u) | (e << 23) | (r & 0x007fffffu);
	}
	default:
		return r ^ (std::uint32_t)rng_next();
	}
}

static std::uint64_t
rand_dbits(void)
{
	std::uint64_t r = rng_next();

	switch ((unsigned)(rng_next() % 12u)) {
	case 0:
		return r;
	case 1:
		return r & 0x800fffffffffffffull;
	case 2:
		return (r & 0x800fffffffffffffull) | 0x7ff0000000000000ull;
	case 3: {
		std::uint64_t d = (std::uint64_t)(rng_next() % 11u) - 5u;
		return (r & 0x8000000000000000ull) |
		    ((0x7ff0000000000000ull + d) & 0x7fffffffffffffffull);
	}
	case 4: {
		std::uint64_t d = (std::uint64_t)(rng_next() % 11u) - 5u;
		return (r & 0x8000000000000000ull) |
		    ((0x0010000000000000ull + d) & 0x7fffffffffffffffull);
	}
	case 5: {
		std::uint64_t e = 970ull + (rng_next() % 61ull);
		return (r & 0x8000000000000000ull) |
		    ((e << 52) | (r & 0x000fffffffffffffull));
	}
	case 6: {
		int k = (int)(rng_next() % 4097u) - 2048;
		return dbits((double)k / 4.0);
	}
	case 7: {
		int k = (int)(rng_next() % 401u) - 200;
		return dbits((double)k);
	}
	case 8: {
		std::uint64_t e = (rng_next() & 1u) ?
		    (rng_next() % 8ull) :
		    2040ull + (rng_next() % 8ull);
		return (r & 0x8000000000000000ull) |
		    ((e << 52) | (r & 0x000fffffffffffffull));
	}
	default:
		return r ^ rng_next();
	}
}

static const unsigned long long ITERS = 200000ull;

static void
random_sweep(void)
{
	unsigned long long k;

	rng_state = 0x0060b0060b0060ULL;
	for (k = 0; k < ITERS; k++) {
		int round = rounds[(unsigned)(rng_next() % 4u)];
		long double x;
		const long double ld_llong_min = (long double)LLONG_MIN;
		const long double ld_llong_max = (long double)LLONG_MAX;

		switch ((unsigned)(k % 8u)) {
		case 0:
			x = rng_ld(10);
			break;
		case 1:
			x = rng_ld(5000);
			break;
		case 2:
			x = ld_llong_min - 0.5L +
			    (long double)((rng_next() % 9u) - 4u) * 0.125L;
			break;
		case 3:
			x = ld_llong_max + 0.5L +
			    (long double)((rng_next() % 9u) - 4u) * 0.125L;
			break;
		case 4:
			x = ld_llong_min +
			    (long double)((rng_next() % 9u) - 4u) * 0.5L;
			break;
		case 5:
			x = ld_llong_max +
			    (long double)((rng_next() % 9u) - 4u) * 0.5L;
			break;
		case 6:
			x = (long double)(rng_next() % 1000000u) - 0.5L;
			break;
		default:
			x = rng_ld(16000);
			break;
		}

		check_llroundl(x, round, "random");
	}

	rng_state = 0x13198a2e03707344ull;
	for (k = 0; k < ITERS; k++) {
		int round = rounds[(unsigned)(rng_next() % 4u)];
		float x = fromfbits(rand_fbits());

		if ((k & 15) == 0)
			x = (float)((int)(rng_next() % 4097u) - 2048);
		if ((k & 31) == 0)
			x = (float)(LLONG_MAX) * (float)(rng_next() & 1u);

		check_llroundf(x, round, "random");
	}

	rng_state = 0xd1ce4e5b91234567ull;
	for (k = 0; k < ITERS; k++) {
		int round = rounds[(unsigned)(rng_next() % 4u)];
		double x = fromdbits(rand_dbits());

		if ((k & 15) == 0)
			x = (double)((int)(rng_next() % 4097u) - 2048);
		if ((k & 31) == 0)
			x = (double)(LLONG_MAX) * (double)(rng_next() & 1u);
		if ((k & 63) == 0)
			x = (double)(LLONG_MIN) +
			    (double)((rng_next() % 7u) - 3u) * 0.5;

		check_llround(x, round, "random");
	}

	rng_state = 0xdeadbeefcafebabeull;
	for (k = 0; k < ITERS; k++) {
		double x, y;

		switch ((unsigned)(k % 6u)) {
		case 0:
			x = fromdbits(rand_dbits());
			y = fromdbits(rand_dbits());
			break;
		case 1:
			x = (double)((int)(rng_next() % 10001u) - 5000);
			y = (double)((int)(rng_next() % 97u) + 1);
			break;
		case 2:
			x = (double)(rng_next() % 1000u) * 0.1;
			y = (double)(rng_next() % 100u) * 0.3 + 0.1;
			break;
		case 3:
			x = __builtin_nan("");
			y = fromdbits(rand_dbits());
			break;
		case 4:
			x = fromdbits(rand_dbits());
			y = __builtin_inf() * ((rng_next() & 1u) ? 1.0 : -1.0);
			break;
		default:
			x = (double)(1ull << (rng_next() % 60u));
			y = (double)(1ull << (rng_next() % 60u));
			break;
		}

		check_drem(x, y, "random");
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

	std::printf("pbsd batch b0060 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-10s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  --------------------------------------------------\n");
	row(st_llround);
	row(st_llroundf);
	row(st_llroundl);
	row(st_drem);

	fails = st_llround.fails + st_llroundf.fails + st_llroundl.fails +
	    st_drem.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
