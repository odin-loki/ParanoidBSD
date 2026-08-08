// Differential test harness for PBSD batch b0080s2.
//
// carg is compared bit-for-bit against the unmodified C reference in
// oracle.c.  Results are staged through 0x7f-guarded buffers so that any
// stray write past the nominal result window is caught.

#include <cfloat>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0080s2;

namespace port = pbsd::lib_msun_src::b0080s2;

extern "C" {
double ref_carg(double _Complex z);
}

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

static std::uint64_t
dbits(double d)
{
	std::uint64_t u;

	std::memcpy(&u, &d, sizeof(u));
	return u;
}

static double
fromdbits(std::uint64_t u)
{
	double d;

	std::memcpy(&d, &u, sizeof(d));
	return d;
}

static double _Complex
mkdc(double re, double im)
{
	double _Complex z;

	__real__ z = re;
	__imag__ z = im;
	return z;
}

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_carg = { "carg", 0, 0, 0 };

static const unsigned MAX_REPORT = 12;

static void
check_carg(std::uint64_t re, std::uint64_t im, const char *tag)
{
	std::uint64_t p, o;

	st_carg.cases++;

	p = dbits(port::carg(std::complex<double>(fromdbits(re),
	    fromdbits(im))));
	o = dbits(ref_carg(mkdc(fromdbits(re), fromdbits(im))));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_carg.fails++;
	if (st_carg.reported < MAX_REPORT) {
		st_carg.reported++;
		std::printf("  carg FAIL [%s] in=(%016llx,%016llx) "
		    "port=%016llx ref=%016llx\n", tag,
		    (unsigned long long)re, (unsigned long long)im,
		    (unsigned long long)p, (unsigned long long)o);
	}
}

/*
 * Interesting double-precision object representations.  Includes both signed
 * zeroes, both infinities, quiet and signalling NaNs with assorted payloads,
 * the subnormal boundary from either side, the normal boundary, the largest
 * finite value, and a spread of patterns whose bytes are all in the high-bit
 * 0x80-0xff range (or straddle it) so that any sign/byte handling mistake is
 * exercised.
 */
static const std::uint64_t dvec[] = {
	0x0000000000000000ull,	/* +0 */
	0x8000000000000000ull,	/* -0 */
	0x0000000000000001ull,	/* smallest +subnormal */
	0x8000000000000001ull,	/* smallest -subnormal */
	0x0000000000000080ull,
	0x0000000080000000ull,
	0x000fffffffffffffull,	/* largest +subnormal */
	0x800fffffffffffffull,	/* largest -subnormal */
	0x0010000000000000ull,	/* smallest +normal */
	0x8010000000000000ull,	/* smallest -normal */
	0x0010000000000001ull,
	0x3fe0000000000000ull,	/* +0.5 */
	0xbfe0000000000000ull,	/* -0.5 */
	0x3ff0000000000000ull,	/* +1 */
	0xbff0000000000000ull,	/* -1 */
	0x4000000000000000ull,	/* +2 */
	0xc000000000000000ull,	/* -2 */
	0x400921fb54442d18ull,	/* +pi */
	0xc00921fb54442d18ull,	/* -pi */
	0x4330000000000000ull,	/* 2^52 */
	0xc330000000000000ull,	/* -2^52 */
	0x7fefffffffffffffull,	/* DBL_MAX */
	0xffefffffffffffffull,	/* -DBL_MAX */
	0x7feffffffffffffeull,
	0x7ff0000000000000ull,	/* +inf */
	0xfff0000000000000ull,	/* -inf */
	0x7ff0000000000001ull,	/* smallest +sNaN */
	0xfff0000000000001ull,	/* smallest -sNaN */
	0x7ff4000000000000ull,	/* +sNaN */
	0xfff4000000000000ull,	/* -sNaN */
	0x7ff7ffffffffffffull,	/* largest sNaN */
	0x7ff8000000000000ull,	/* +qNaN */
	0xfff8000000000000ull,	/* -qNaN */
	0x7ff8000000000001ull,
	0x7fffffffffffffffull,
	0xffffffffffffffffull,
	0x8080808080808080ull,	/* every byte >= 0x80 */
	0x7f7f7f7f7f7f7f7full,	/* guard-byte pattern */
	0x00ff00ff00ff00ffull,
	0xff00ff00ff00ff00ull,
	0x8000ffffffff0000ull,
	0x3333333333333333ull,
	0xb333333333333333ull,
};
static const std::size_t NDVEC = sizeof(dvec) / sizeof(dvec[0]);

static void
edge_cases(void)
{
	std::size_t i, j;

	for (i = 0; i < NDVEC; i++)
		for (j = 0; j < NDVEC; j++)
			check_carg(dvec[i], dvec[j], "cross");

	/*
	 * carg is atan2 on (imag, real): make sure every quadrant, both axes
	 * and every signed-zero/infinity combination is hit explicitly.
	 */
	{
		static const double dq[] = {
			0.0, -0.0, 1.0, -1.0, 2.0, -2.0,
			__builtin_inf(), -__builtin_inf(),
			__builtin_nan(""), -__builtin_nan(""),
			DBL_MIN, -DBL_MIN, DBL_MAX, -DBL_MAX,
			5e-324, -5e-324,
		};
		std::size_t a, b;
		const std::size_t nd = sizeof(dq) / sizeof(dq[0]);

		for (a = 0; a < nd; a++)
			for (b = 0; b < nd; b++)
				check_carg(dbits(dq[a]), dbits(dq[b]),
				    "quadrant");
	}

	/* Walk both sides of every double boundary that affects atan2. */
	static const std::uint64_t kWalk[] = {
		0x0000000000000000ull, 0x8000000000000000ull,
		0x0010000000000000ull, 0x000fffffffffffffull,
		0x7ff0000000000000ull, 0x7fefffffffffffffull,
		0x7fffffffffffffffull, 0x3ff0000000000000ull,
		0xbff0000000000000ull, 0x8010000000000000ull,
		0xfff0000000000000ull, 0xffffffffffffffffull,
	};
	for (std::size_t k = 0; k < sizeof kWalk / sizeof kWalk[0]; k++) {
		for (int d = -32; d <= 32; d++) {
			std::uint64_t re = kWalk[k] + (std::uint64_t)d;
			std::uint64_t im = kWalk[k] + (std::uint64_t)d;

			check_carg(re, im, "walk");
			check_carg(re, kWalk[k], "walk");
			check_carg(kWalk[k], im, "walk");
			check_carg(re, re ^ 0x8000000000000000ull, "walk");
		}
	}

	/* Single-component cases: real-only and imag-only. */
	for (i = 0; i < NDVEC; i++) {
		check_carg(dvec[i], 0ull, "real-only");
		check_carg(0ull, dvec[i], "imag-only");
		check_carg(dvec[i], 0x8000000000000000ull, "imag-negzero");
		check_carg(0x8000000000000000ull, dvec[i], "real-negzero");
	}

	/*
	 * atan2 branch boundaries: one component exactly zero (both signs),
	 * the other positive/negative finite, and both components equal.
	 */
	{
		static const double unit[] = {
			1.0, -1.0, 0.5, -0.5, 2.0, -2.0,
			DBL_MIN, -DBL_MIN, 1e-100, -1e-100,
		};
		std::size_t a;
		const std::size_t nu = sizeof(unit) / sizeof(unit[0]);

		for (a = 0; a < nu; a++) {
			check_carg(dbits(unit[a]), 0.0, "axis");
			check_carg(dbits(unit[a]), -0.0, "axis");
			check_carg(0.0, dbits(unit[a]), "axis");
			check_carg(-0.0, dbits(unit[a]), "axis");
			check_carg(dbits(unit[a]), dbits(unit[a]), "diag");
			check_carg(dbits(unit[a]), dbits(-unit[a]), "diag");
		}
	}
}

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

static const unsigned long long ITERS = 250000ull;

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		std::uint64_t re = rng_next();
		std::uint64_t im = rng_next();

		if ((i & 7) == 0)
			im = (i & 8) ? 0x8000000000000000ull : 0ull;
		if ((i & 7) == 1)
			re = (i & 8) ? 0x8000000000000000ull : 0ull;

		check_carg(re, im, "random");
	}
}

static void
row(const stat &s)
{
	std::printf("  %-14s %12llu %10llu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "PASS" : "FAIL");
}

int
main(void)
{
	unsigned long long fails;

	std::printf("pbsd batch b0080s2 differential test\n\n");

	edge_cases();
	random_sweep();

	std::printf("\n  %-14s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ----------------------------------------------------\n");
	row(st_carg);

	fails = st_carg.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	if (st_carg.cases == 0) {
		std::printf("carg: no cases were run\n");
		return 1;
	}

	return (fails == 0 ? 0 : 1);
}
