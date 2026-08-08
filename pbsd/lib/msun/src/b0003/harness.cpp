/*
 * Differential test harness for PBSD batch b0003.
 *
 * Every function ported in port.cppm is compared against the unmodified C
 * reference in oracle.c.  Integer results are staged through 0x7f-guarded
 * buffers and compared in their entirety.  Floating-point environment state
 * after each call is compared as well so that feholdexcept / feupdateenv /
 * feclearexcept paths in the lrint family are observable.
 */

#include <bit>
#include <cerrno>
#include <cfenv>
#include <climits>
#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0003;

namespace port = pbsd::lib_msun_src::b0003;

extern "C" {
long ref_lrintl(long double x);
long long ref_llrintl(long double x);
long ref_lroundf(float x);
long ref_lroundl(long double x);
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

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_lrintl = { "lrintl", 0, 0, 0 };
static stat st_llrintl = { "llrintl", 0, 0, 0 };
static stat st_lroundf = { "lroundf", 0, 0, 0 };
static stat st_lroundl = { "lroundl", 0, 0, 0 };

static const unsigned MAX_REPORT = 10;

static void
prep_fenv(int round, int sticky)
{

	::fesetround(round);
	::feclearexcept(FE_ALL_EXCEPT);
	if (sticky != 0)
		::feraiseexcept(sticky);
}

/* ------------------------------------------------------------------ */
/* lrintl / llrintl                                                  */
/* ------------------------------------------------------------------ */

static void
check_lrintl(long double x, int round, int sticky, const char *tag)
{
	long pr, rr;
	int pex, rex;

	st_lrintl.cases++;

	prep_fenv(round, sticky);
	pr = port::lrintl(x);
	pex = ::fetestexcept(FE_ALL_EXCEPT);

	prep_fenv(round, sticky);
	rr = ref_lrintl(x);
	rex = ::fetestexcept(FE_ALL_EXCEPT);

	if (guarded_equal(&pr, &rr, sizeof(pr)) && pex == rex)
		return;

	st_lrintl.fails++;
	if (st_lrintl.reported < MAX_REPORT) {
		ldrep bx = ldbits(x);

		st_lrintl.reported++;
		std::printf("  lrintl FAIL [%s] round=%d sticky=%#x x=", tag, round,
		    sticky);
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", bx.b[i]);
		std::printf(" port=%ld/%#x ref=%ld/%#x\n", pr, pex, rr, rex);
	}
}

static void
check_llrintl(long double x, int round, int sticky, const char *tag)
{
	long long pr, rr;
	int pex, rex;

	st_llrintl.cases++;

	prep_fenv(round, sticky);
	pr = port::llrintl(x);
	pex = ::fetestexcept(FE_ALL_EXCEPT);

	prep_fenv(round, sticky);
	rr = ref_llrintl(x);
	rex = ::fetestexcept(FE_ALL_EXCEPT);

	if (guarded_equal(&pr, &rr, sizeof(pr)) && pex == rex)
		return;

	st_llrintl.fails++;
	if (st_llrintl.reported < MAX_REPORT) {
		ldrep bx = ldbits(x);

		st_llrintl.reported++;
		std::printf("  llrintl FAIL [%s] round=%d sticky=%#x x=", tag,
		    round, sticky);
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", bx.b[i]);
		std::printf(" port=%lld/%#x ref=%lld/%#x\n", pr, pex, rr, rex);
	}
}

/* ------------------------------------------------------------------ */
/* lroundf / lroundl                                                 */
/* ------------------------------------------------------------------ */

static void
check_lroundf(float x, int round, const char *tag)
{
	long pr, rr;
	int pex, rex;

	st_lroundf.cases++;

	prep_fenv(round, 0);
	pr = port::lroundf(x);
	pex = ::fetestexcept(FE_ALL_EXCEPT);

	prep_fenv(round, 0);
	rr = ref_lroundf(x);
	rex = ::fetestexcept(FE_ALL_EXCEPT);

	if (guarded_equal(&pr, &rr, sizeof(pr)) && pex == rex)
		return;

	st_lroundf.fails++;
	if (st_lroundf.reported < MAX_REPORT) {
		st_lroundf.reported++;
		std::printf("  lroundf FAIL [%s] round=%d x=%#010x port=%ld/%#x "
		    "ref=%ld/%#x\n", tag, round, fbits(x), pr, pex, rr, rex);
	}
}

static void
check_lroundl(long double x, int round, const char *tag)
{
	long pr, rr;
	int pex, rex;

	st_lroundl.cases++;

	prep_fenv(round, 0);
	pr = port::lroundl(x);
	pex = ::fetestexcept(FE_ALL_EXCEPT);

	prep_fenv(round, 0);
	rr = ref_lroundl(x);
	rex = ::fetestexcept(FE_ALL_EXCEPT);

	if (guarded_equal(&pr, &rr, sizeof(pr)) && pex == rex)
		return;

	st_lroundl.fails++;
	if (st_lroundl.reported < MAX_REPORT) {
		ldrep bx = ldbits(x);

		st_lroundl.reported++;
		std::printf("  lroundl FAIL [%s] round=%d x=", tag, round);
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", bx.b[i]);
		std::printf(" port=%ld/%#x ref=%ld/%#x\n", pr, pex, rr, rex);
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

static const int stickies[] = {
	0,
	FE_INEXACT,
	FE_INVALID,
	FE_INEXACT | FE_INVALID,
	FE_DIVBYZERO,
	FE_OVERFLOW,
	FE_UNDERFLOW,
	FE_INEXACT | FE_OVERFLOW,
};

static void
edge_cases(void)
{
	std::size_t i, j, r, s;

	for (r = 0; r < sizeof(rounds) / sizeof(rounds[0]); r++) {
		for (s = 0; s < sizeof(stickies) / sizeof(stickies[0]); s++) {
			for (i = 0; i < NLDVEC; i++) {
				long double x = mkld(ldvec[i].se, ldvec[i].m);

				check_lrintl(x, rounds[r], stickies[s],
				    "ldvec");
				check_llrintl(x, rounds[r], stickies[s],
				    "ldvec");
				check_lroundl(x, rounds[r], "ldvec");
			}

			for (i = 0; i < NFVEC; i++)
				check_lroundf(fromfbits(fvec[i]), rounds[r],
				    "fvec");
		}
	}

	/*
	 * Walk both sides of every INRANGE boundary for lroundl and the
	 * compile-time dtype_max - type_max != 0.5 guard for lroundf.
	 */
	{
		const long double ld_long_min = (long double)LONG_MIN;
		const long double ld_long_max = (long double)LONG_MAX;
		const long double ld_min_b = ld_long_min - 0.5L;
		const long double ld_max_b = ld_long_max + 0.5L;
		static const long double eps[] = {
			0.0L, 0.25L, -0.25L, 0.5L, -0.5L,
			1e-18L, -1e-18L, 1e-9L, -1e-9L,
			LDBL_EPSILON, -LDBL_EPSILON,
		};

		for (r = 0; r < sizeof(rounds) / sizeof(rounds[0]); r++) {
			for (i = 0; i < sizeof(eps) / sizeof(eps[0]); i++) {
				long double d;

				d = ld_min_b + eps[i];
				check_lroundl(d, rounds[r], "long_min_b");
				check_lrintl(d, rounds[r], 0, "long_min_b");
				check_llrintl(d, rounds[r], 0, "long_min_b");

				d = ld_max_b + eps[i];
				check_lroundl(d, rounds[r], "long_max_b");
				check_lrintl(d, rounds[r], 0, "long_max_b");
				check_llrintl(d, rounds[r], 0, "long_max_b");

				d = ld_long_min + eps[i];
				check_lroundl(d, rounds[r], "long_min");
				check_lrintl(d, rounds[r], 0, "long_min");
				check_llrintl(d, rounds[r], 0, "long_min");

				d = ld_long_max + eps[i];
				check_lroundl(d, rounds[r], "long_max");
				check_lrintl(d, rounds[r], 0, "long_max");
				check_llrintl(d, rounds[r], 0, "long_max");
			}
		}
	}

	/*
	 * llrintl / lrintl overflow boundaries at LLONG and LONG limits.
	 */
	{
		const long double ld_llong_min = (long double)LLONG_MIN;
		const long double ld_llong_max = (long double)LLONG_MAX;
		static const long double frac[] = {
			0.0L, 0.5L, -0.5L, 1.0L, -1.0L, 0.25L, -0.25L,
		};

		for (r = 0; r < sizeof(rounds) / sizeof(rounds[0]); r++) {
			for (i = 0; i < sizeof(frac) / sizeof(frac[0]); i++) {
				long double x;

				x = ld_llong_max + frac[i];
				check_llrintl(x, rounds[r], FE_INEXACT,
				    "llong_max");
				check_lrintl(x, rounds[r], FE_INEXACT,
				    "llong_max");

				x = ld_llong_min + frac[i];
				check_llrintl(x, rounds[r], FE_INEXACT,
				    "llong_min");
				check_lrintl(x, rounds[r], FE_INEXACT,
				    "llong_min");
			}
		}
	}

	/*
	 * lroundf: float has less precision than long, so INRANGE is always
	 * true — still exercise roundf on integers, ties, NaNs and extremes.
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
				check_lroundf(specials[i], rounds[r],
				    "special");
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
					check_lroundf(
					    fromfbits(anchors[i] +
						(std::uint32_t)d),
					    rounds[r], "walk");
	}

	/* Cross combinations that mix signs and magnitudes. */
	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++) {
			long double x = mkld(ldvec[i].se, ldvec[i].m);
			long double y = mkld(ldvec[j].se, ldvec[j].m);

			check_lroundl(x + y, FE_TONEAREST, "cross");
			check_lrintl(x - y, FE_TONEAREST, FE_INEXACT,
			    "cross");
			check_llrintl(x * y, FE_TONEAREST, 0, "cross");
		}
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

static const unsigned long long ITERS = 200000ull;

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0xd1ce4e5b91234567ull;
	for (i = 0; i < ITERS; i++) {
		int round = rounds[(unsigned)(rng_next() % 4u)];
		int sticky = stickies[(unsigned)(rng_next() % 8u)];
		long double x;

		switch ((unsigned)(i % 6u)) {
		case 0:
			x = rng_ld(6);
			break;
		case 1:
			x = rng_ld(4000);
			break;
		case 2:
			x = (long double)(LLONG_MIN) +
			    (long double)((rng_next() % 7u) - 3u) * 0.5L;
			break;
		case 3:
			x = (long double)(LLONG_MAX) +
			    (long double)((rng_next() % 7u) - 3u) * 0.5L;
			break;
		case 4:
			x = (long double)(LONG_MIN) +
			    (long double)((rng_next() % 7u) - 3u) * 0.5L;
			break;
		default:
			x = (long double)(LONG_MAX) +
			    (long double)((rng_next() % 7u) - 3u) * 0.5L;
			break;
		}

		check_lrintl(x, round, sticky, "random");
	}

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		int round = rounds[(unsigned)(rng_next() % 4u)];
		int sticky = stickies[(unsigned)(rng_next() % 8u)];
		long double x;

		switch ((unsigned)(i % 6u)) {
		case 0:
			x = rng_ld(8);
			break;
		case 1:
			x = rng_ld(8000);
			break;
		case 2:
			x = (long double)(LLONG_MIN) +
			    (long double)((rng_next() % 11u) - 5u) * 0.25L;
			break;
		case 3:
			x = (long double)(LLONG_MAX) +
			    (long double)((rng_next() % 11u) - 5u) * 0.25L;
			break;
		case 4:
			x = __builtin_nanl("") +
			    (long double)(rng_next() % 1000u) * 1e-300L;
			break;
		default:
			x = __builtin_infl() *
			    (long double)((rng_next() % 3u) + 1u);
			break;
		}

		check_llrintl(x, round, sticky, "random");
	}

	rng_state = 0x13198a2e03707344ull;
	for (i = 0; i < ITERS; i++) {
		int round = rounds[(unsigned)(rng_next() % 4u)];
		float x = fromfbits(rand_fbits());

		if ((i & 15) == 0)
			x = (float)((int)(rng_next() % 4097u) - 2048);
		if ((i & 31) == 0)
			x = (float)(LONG_MAX) * (float)(rng_next() & 1u);

		check_lroundf(x, round, "random");
	}

	rng_state = 0xdeadbeefcafebabeull;
	for (i = 0; i < ITERS; i++) {
		int round = rounds[(unsigned)(rng_next() % 4u)];
		long double x;
		const long double ld_long_min = (long double)LONG_MIN;
		const long double ld_long_max = (long double)LONG_MAX;

		switch ((unsigned)(i % 8u)) {
		case 0:
			x = rng_ld(10);
			break;
		case 1:
			x = rng_ld(5000);
			break;
		case 2:
			x = ld_long_min - 0.5L +
			    (long double)((rng_next() % 9u) - 4u) * 0.125L;
			break;
		case 3:
			x = ld_long_max + 0.5L +
			    (long double)((rng_next() % 9u) - 4u) * 0.125L;
			break;
		case 4:
			x = ld_long_min +
			    (long double)((rng_next() % 9u) - 4u) * 0.5L;
			break;
		case 5:
			x = ld_long_max +
			    (long double)((rng_next() % 9u) - 4u) * 0.5L;
			break;
		case 6:
			x = (long double)(rng_next() % 1000000u) - 0.5L;
			break;
		default:
			x = rng_ld(16000);
			break;
		}

		check_lroundl(x, round, "random");
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

	std::printf("pbsd batch b0003 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-10s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  --------------------------------------------------\n");
	row(st_lrintl);
	row(st_llrintl);
	row(st_lroundf);
	row(st_lroundl);

	fails = st_lrintl.fails + st_llrintl.fails + st_lroundf.fails +
	    st_lroundl.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
