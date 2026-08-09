/*
 * Differential test harness for PBSD batch b0080s4 (e_remainderl.c).
 *
 * remainderl is compared bit-for-bit against the unmodified C reference in
 * oracle.c.  Results are staged through 0x7f-guarded buffers so that any
 * stray write past the nominal result window is caught.
 */

#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0080s4;

namespace port = pbsd::lib_msun_src::b0080s4;

extern "C" {
long double ref_remainderl(long double x, long double y);
}

#if LDBL_MANT_DIG == 64
/* x87 80-bit extended: only the first 10 bytes are architectural state, the
 * remaining bytes of the 12/16-byte object are padding and hold garbage. */
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

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_remainderl = { "remainderl", 0, 0, 0 };

static const unsigned MAX_REPORT = 12;

static void
check_remainderl(long double x, long double y, const char *tag)
{
	ldrep p, o;

	st_remainderl.cases++;

	p = ldbits(port::remainderl(x, y));
	o = ldbits(ref_remainderl(x, y));

	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;

	st_remainderl.fails++;
	if (st_remainderl.reported < MAX_REPORT) {
		ldrep bx = ldbits(x), by = ldbits(y);
		std::size_t i;

		st_remainderl.reported++;
		std::printf("  remainderl FAIL [%s] x=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", bx.b[i]);
		std::printf(" y=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", by.b[i]);
		std::printf(" port=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

struct ldcase {
	std::uint16_t se;
	std::uint64_t m;
};

/*
 * Only architecturally valid x87 encodings are listed: for a zero exponent the
 * explicit integer bit must be clear, otherwise it must be set.
 */
static const ldcase ldvec[] = {
	{ 0x0000u, 0x0000000000000000ull },	/* +0 */
	{ 0x8000u, 0x0000000000000000ull },	/* -0 */
	{ 0x0000u, 0x0000000000000001ull },	/* smallest +subnormal */
	{ 0x8000u, 0x0000000000000001ull },	/* smallest -subnormal */
	{ 0x0000u, 0x0000000000000080ull },
	{ 0x0000u, 0x7fffffffffffffffull },	/* largest +subnormal */
	{ 0x8000u, 0x7fffffffffffffffull },	/* largest -subnormal */
	{ 0x0001u, 0x8000000000000000ull },	/* smallest +normal */
	{ 0x8001u, 0x8000000000000000ull },	/* smallest -normal */
	{ 0x0001u, 0x8000000000000001ull },
	{ 0x3ffeu, 0x8000000000000000ull },	/* +0.5 */
	{ 0xbffeu, 0x8000000000000000ull },	/* -0.5 */
	{ 0x3fffu, 0x8000000000000000ull },	/* +1 */
	{ 0xbfffu, 0x8000000000000000ull },	/* -1 */
	{ 0x3fffu, 0x8000000000000001ull },	/* nextafter(1, inf) */
	{ 0x3ffeu, 0xffffffffffffffffull },	/* nextafter(1, 0) */
	{ 0x4000u, 0x8000000000000000ull },	/* +2 */
	{ 0xc000u, 0x8000000000000000ull },	/* -2 */
	{ 0x4000u, 0xc000000000000000ull },	/* +3 */
	{ 0xc000u, 0xc000000000000000ull },	/* -3 */
	{ 0x4001u, 0xe000000000000000ull },	/* +7 */
	{ 0x4000u, 0xc90fdaa22168c235ull },	/* +pi */
	{ 0xc000u, 0xc90fdaa22168c235ull },	/* -pi */
	{ 0x403eu, 0x8000000000000000ull },	/* 2^63 */
	{ 0x403fu, 0x8000000000000000ull },	/* 2^64 */
	{ 0x407fu, 0x8000000000000000ull },	/* 2^128 */
	{ 0x7ffeu, 0xffffffffffffffffull },	/* LDBL_MAX */
	{ 0xfffeu, 0xffffffffffffffffull },	/* -LDBL_MAX */
	{ 0x7fffu, 0x8000000000000000ull },	/* +inf */
	{ 0xffffu, 0x8000000000000000ull },	/* -inf */
	{ 0x7fffu, 0xc000000000000000ull },	/* +qNaN */
	{ 0xffffu, 0xc000000000000000ull },	/* -qNaN */
	{ 0x7fffu, 0xa000000000000000ull },	/* +sNaN */
	{ 0x7fffu, 0x8000000000000001ull },	/* smallest +sNaN */
	{ 0x7fffu, 0xffffffffffffffffull },
	{ 0x3fffu, 0x8080808080808080ull },	/* high-bit byte payload */
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
	{ 0x3fffu, 0xfefefefefefefefeull },
};
static const std::size_t NLDVEC = sizeof(ldvec) / sizeof(ldvec[0]);

static void
edge_cases(void)
{
	std::size_t i, j;

	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++)
			check_remainderl(mkld(ldvec[i].se, ldvec[i].m),
			    mkld(ldvec[j].se, ldvec[j].m), "cross");

	/*
	 * remainderl is defined in terms of the rounded quotient; walk both
	 * sides of every rounding boundary (n, n+1/4, n+1/2, n+3/4 and one ulp
	 * either side of the tie) for a spread of divisors, plus quotients big
	 * enough to overflow the int the reference stores them in.
	 */
	{
		static const long double ys[] = {
			1.0L, -1.0L, 0.5L, -0.5L, 3.0L, -3.0L, 0.1L,
			1e-30L, 1e30L, 7.25L, 1024.0L, 1.0L / 3.0L,
		};
		static const long double frac[] = {
			0.0L, 0.25L, 0.5L, 0.75L, 1.0L,
			0.5L - 1e-18L, 0.5L + 1e-18L,
			-0.25L, -0.5L, -0.75L,
		};
		static const long double qs[] = {
			0.0L, 1.0L, 2.0L, 3.0L, 4.0L, 5.0L, 17.0L,
			1e9L, 2147483647.0L, 2147483648.0L, 4294967296.0L,
			1e18L, 1e25L,
		};
		std::size_t a, b, c;

		for (a = 0; a < sizeof(ys) / sizeof(ys[0]); a++)
			for (b = 0; b < sizeof(qs) / sizeof(qs[0]); b++)
				for (c = 0; c < sizeof(frac) / sizeof(frac[0]);
				    c++) {
					long double y = ys[a];
					long double x = (qs[b] + frac[c]) * y;

					check_remainderl(x, y, "quotient");
					check_remainderl(-x, y, "quotient");
					check_remainderl(x, -y, "quotient");
					check_remainderl(y, x, "quotient");
				}
	}

	/* Degenerate divisors and dividends. */
	{
		static const long double specials[] = {
			0.0L, -0.0L, 1.0L, -1.0L,
			__builtin_infl(), -__builtin_infl(),
			__builtin_nanl(""), -__builtin_nanl(""),
			LDBL_MIN, -LDBL_MIN, LDBL_MAX, -LDBL_MAX,
			LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
		};
		std::size_t a, b;
		const std::size_t n = sizeof(specials) / sizeof(specials[0]);

		for (a = 0; a < n; a++)
			for (b = 0; b < n; b++)
				check_remainderl(specials[a], specials[b],
				    "special");
	}

	/* Walk both sides of every long-double boundary in the vector. */
	for (std::size_t k = 0; k < NLDVEC; k++) {
		long double base = mkld(ldvec[k].se, ldvec[k].m);

		for (int d = -8; d <= 8; d++) {
			std::uint64_t m = ldvec[k].m + (std::uint64_t)d;
			long double x = mkld(ldvec[k].se, m);

			check_remainderl(x, base, "walk");
			check_remainderl(base, x, "walk");
			check_remainderl(-x, base, "walk");
			check_remainderl(x, -base, "walk");
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

/*
 * A valid x87 extended value with the exponent drawn from a window around 1.0
 * (plus a slice of zeroes, subnormals, infinities and NaNs).
 */
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

static const unsigned long long ITERS = 250000ull;

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0x13198a2e03707344ull;
	for (i = 0; i < ITERS; i++) {
		long double x, y;
		unsigned mode = (unsigned)(i % 4u);

		switch (mode) {
		case 0:
			/* Similar magnitudes: small, well-defined quotients. */
			x = rng_ld(6);
			y = rng_ld(6);
			break;
		case 1:
			/* Wildly differing magnitudes. */
			x = rng_ld(60);
			y = rng_ld(60);
			break;
		case 2: {
			/* Deliberately near a rounding tie of the quotient. */
			long double q = (long double)(rng_next() % 1000000ull);
			long double eps;

			y = rng_ld(8);
			switch ((unsigned)(rng_next() % 5u)) {
			case 0: eps = 0.0L; break;
			case 1: eps = 0.5L; break;
			case 2: eps = -0.5L; break;
			case 3: eps = 0.5L - 1e-17L; break;
			default: eps = 0.5L + 1e-17L; break;
			}
			if (rng_next() & 1)
				q = -q;
			x = (q + eps) * y;
			break;
		}
		default:
			/* Full exponent range, including the extremes. */
			x = rng_ld(16000);
			y = rng_ld(16000);
			break;
		}

		check_remainderl(x, y, "random");
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

	std::printf("pbsd batch b0080s4 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-14s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ----------------------------------------------------\n");
	row(st_remainderl);

	fails = st_remainderl.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	if (st_remainderl.cases == 0) {
		std::printf("remainderl: no cases were run\n");
		return 1;
	}

	return (fails == 0 ? 0 : 1);
}
