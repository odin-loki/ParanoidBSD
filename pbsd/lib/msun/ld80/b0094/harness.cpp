/*
 * Differential test harness for PBSD batch b0094.
 *
 * Every function in port.cppm is compared, bit for bit, against the
 * unmodified C reference in oracle.c.
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld80.b0094;

namespace port = pbsd::lib_msun_ld80::b0094;

extern "C" {
long double ref___kernel_tanl(long double x, long double y, int iy);
long double ref___exp__D(long double x, long double c);
long double ref_exp2l(long double x);
}

/* ------------------------------------------------------------------ */
/* raw-representation plumbing                                         */
/* ------------------------------------------------------------------ */

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

struct ldrep {
	unsigned char b[sizeof(long double)];
};

static bool
ld_equal(long double p, long double o)
{
	ldrep rp, ro;

	std::memset(rp.b, 0, sizeof(rp.b));
	std::memset(ro.b, 0, sizeof(ro.b));
	std::memcpy(rp.b, &p, LD_SIG);
	std::memcpy(ro.b, &o, LD_SIG);
	return (std::memcmp(rp.b, ro.b, LD_SIG) == 0);
}

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

static void
ldhex(const ldrep &r)
{
	std::size_t i;

	for (i = LD_SIG; i-- > 0;)
		std::printf("%02x", r.b[i]);
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

static stat st_kernel_tanl = { "__kernel_tanl", 0, 0, 0 };
static stat st_exp__D = { "__exp__D", 0, 0, 0 };
static stat st_exp2l = { "exp2l", 0, 0, 0 };

static const unsigned MAX_REPORT = 8;

/* ------------------------------------------------------------------ */
/* per-function checkers                                               */
/* ------------------------------------------------------------------ */

static void
check_kernel_tanl(long double x, long double y, int iy, const char *tag)
{
	long double p, o;

	st_kernel_tanl.cases++;

	p = port::__kernel_tanl(x, y, iy);
	o = ref___kernel_tanl(x, y, iy);

	if (ld_equal(p, o))
		return;

	st_kernel_tanl.fails++;
	if (st_kernel_tanl.reported < MAX_REPORT) {
		st_kernel_tanl.reported++;
		std::printf("  __kernel_tanl FAIL [%s] x=", tag);
		ldhex(ldbits(x));
		std::printf(" y=");
		ldhex(ldbits(y));
		std::printf(" iy=%d port=", iy);
		ldhex(ldbits(p));
		std::printf(" ref=");
		ldhex(ldbits(o));
		std::printf("\n");
	}
}

static void
check_exp__D(long double x, long double c, const char *tag)
{
	long double p, o;

	st_exp__D.cases++;

	p = port::__exp__D(x, c);
	o = ref___exp__D(x, c);

	if (ld_equal(p, o))
		return;

	st_exp__D.fails++;
	if (st_exp__D.reported < MAX_REPORT) {
		st_exp__D.reported++;
		std::printf("  __exp__D FAIL [%s] x=", tag);
		ldhex(ldbits(x));
		std::printf(" c=");
		ldhex(ldbits(c));
		std::printf(" port=");
		ldhex(ldbits(p));
		std::printf(" ref=");
		ldhex(ldbits(o));
		std::printf("\n");
	}
}

static void
check_exp2l(long double x, const char *tag)
{
	long double p, o;

	st_exp2l.cases++;

	p = port::exp2l(x);
	o = ref_exp2l(x);

	if (ld_equal(p, o))
		return;

	st_exp2l.fails++;
	if (st_exp2l.reported < MAX_REPORT) {
		st_exp2l.reported++;
		std::printf("  exp2l FAIL [%s] x=", tag);
		ldhex(ldbits(x));
		std::printf(" port=");
		ldhex(ldbits(p));
		std::printf(" ref=");
		ldhex(ldbits(o));
		std::printf("\n");
	}
}

/* ------------------------------------------------------------------ */
/* hand-written edge vectors                                           */
/* ------------------------------------------------------------------ */

static const long double BOUND = 0.67434L;
static const long double PI4 = 0.78539816339744830962L;

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
	{ 0x0000u, 0x7fffffffffffffffull },
	{ 0x8000u, 0x7fffffffffffffffull },
	{ 0x0001u, 0x8000000000000000ull },
	{ 0x8001u, 0x8000000000000000ull },
	{ 0x3ffeu, 0x8000000000000000ull },
	{ 0xbffeu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000000ull },
	{ 0xbfffu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000001ull },
	{ 0x3ffeu, 0xffffffffffffffffull },
	{ 0x3fffu, 0x8080808080808080ull },
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
};
static const std::size_t NLDVEC = sizeof(ldvec) / sizeof(ldvec[0]);

static const int iyvec[] = {
	0, 1, -1, 2, -2, 3, -3, INT_MAX, INT_MIN, 0x7f, -0x7f, 0x80, -0x80,
};
static const std::size_t NIYVEC = sizeof(iyvec) / sizeof(iyvec[0]);

static void
edge_kernel_tanl(void)
{
	std::size_t i, j, k;

	/* Cross product of bit-pattern edge values. */
	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++)
			for (k = 0; k < NIYVEC; k++) {
				long double x = mkld(ldvec[i].se, ldvec[i].m);
				long double y = mkld(ldvec[j].se, ldvec[j].m);
				check_kernel_tanl(x, y, iyvec[k], "cross");
			}

	/* Both sides of |x| = 0.67434 boundary. */
	{
		static const long double xvals[] = {
			0.0L, -0.0L,
			BOUND, -BOUND,
			BOUND * 0.999999999999999L,
			-BOUND * 0.999999999999999L,
			BOUND * 1.000000000000001L,
			-BOUND * 1.000000000000001L,
			PI4, -PI4,
			PI4 * 0.5L, -PI4 * 0.5L,
			0.1L, -0.1L, 0.5L, -0.5L, 0.67L, -0.67L,
			1e-4932L, -1e-4932L,
			1e-100L, -1e-100L,
			LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
		};
		static const long double yvals[] = {
			0.0L, -0.0L, 1.0L, -1.0L, PI4, -PI4,
			1e-30L, -1e-30L, 1e30L, -1e30L,
		};
		static const int iys[] = { 0, 1, -1, 42, -42 };
		std::size_t a, b, c;
		const std::size_t nx = sizeof(xvals) / sizeof(xvals[0]);
		const std::size_t ny = sizeof(yvals) / sizeof(yvals[0]);
		const std::size_t ni = sizeof(iys) / sizeof(iys[0]);

		for (a = 0; a < nx; a++)
			for (b = 0; b < ny; b++)
				for (c = 0; c < ni; c++)
					check_kernel_tanl(xvals[a], yvals[b],
					    iys[c], "boundary");
	}

	/* iy==1 vs iy!=1: exercises tan vs cot paths. */
	{
		static const long double pairs[][2] = {
			{ 0.3L, 0.7L },
			{ -0.3L, -0.7L },
			{ BOUND * 0.5L, 1.0L },
			{ -BOUND * 0.5L, -1.0L },
			{ BOUND * 1.1L, 0.01L },
			{ -BOUND * 1.1L, -0.01L },
			{ 1e-20L, 1e-10L },
			{ -1e-20L, -1e-10L },
		};
		std::size_t n;

		for (n = 0; n < sizeof(pairs) / sizeof(pairs[0]); n++) {
			check_kernel_tanl(pairs[n][0], pairs[n][1], 0, "iy0");
			check_kernel_tanl(pairs[n][0], pairs[n][1], 1, "iy1");
			check_kernel_tanl(pairs[n][0], pairs[n][1], -1, "iym1");
		}
	}
}

static void
edge_exp__D(void)
{
	static const long double lnhuge = 1.13627617309191834574e+04L;
	static const long double lntiny = -1.14057368561139000667e+04L;

	/* NaN */
	check_exp__D(0.0L / 0.0L, 0.0L, "nan");

	/* Infinities */
	check_exp__D(1.0L / 0.0L, 0.0L, "pinf");
	check_exp__D(-1.0L / 0.0L, 0.0L, "ninf");

	/* Overflow / underflow thresholds */
	check_exp__D(lnhuge, 0.0L, "lnhuge");
	check_exp__D(lnhuge + 1.0L, 0.0L, "lnhuge+1");
	check_exp__D(lnhuge - 1.0L, 0.0L, "lnhuge-1");
	check_exp__D(lntiny, 0.0L, "lntiny");
	check_exp__D(lntiny - 1.0L, 0.0L, "lntiny-1");
	check_exp__D(lntiny + 1.0L, 0.0L, "lntiny+1");

	/* Normal range with various c magnitudes */
	{
		static const long double xs[] = {
			0.0L, -0.0L, 1.0L, -1.0L, 0.5L, -0.5L,
			10.0L, -10.0L, 100.0L, -100.0L,
			1e-10L, -1e-10L, 1e10L, -1e10L,
			700.0L, -700.0L, 1000.0L, -1000.0L,
		};
		static const long double cs[] = {
			0.0L, -0.0L, 1e-20L, -1e-20L,
			1e-5L, -1e-5L, 0.1L, -0.1L,
		};
		std::size_t a, b;

		for (a = 0; a < sizeof(xs) / sizeof(xs[0]); a++)
			for (b = 0; b < sizeof(cs) / sizeof(cs[0]); b++)
				check_exp__D(xs[a], cs[b], "normal");
	}

	/* Bit-pattern edge values */
	for (std::size_t i = 0; i < NLDVEC; i++)
		for (std::size_t j = 0; j < NLDVEC; j++)
			check_exp__D(mkld(ldvec[i].se, ldvec[i].m),
			    mkld(ldvec[j].se, ldvec[j].m), "bits");
}

static void
edge_exp2l(void)
{
	/* Special exponents: +Inf, -Inf, NaN, -0 */
	check_exp2l(1.0L / 0.0L, "pinf");
	check_exp2l(-1.0L / 0.0L, "ninf");
	check_exp2l(0.0L / 0.0L, "nan");
	check_exp2l(-0.0L, "negzero");

	/* Overflow / underflow boundaries */
	check_exp2l(16384.0L, "ovf");
	check_exp2l(16383.999999999999L, "ovf-");
	check_exp2l(16384.000000000001L, "ovf+");
	check_exp2l(-16446.0L, "unf");
	check_exp2l(-16445.999999999999L, "unf-");
	check_exp2l(-16446.000000000001L, "unf+");

	/* Tiny argument path: |x| < 0x1p-65 */
	check_exp2l(0x1p-66L, "tiny");
	check_exp2l(-0x1p-66L, "tiny-");
	check_exp2l(0x1p-65L, "tiny65");
	check_exp2l(-0x1p-65L, "tiny65-");
	check_exp2l(0x1p-100L, "tiny100");
	check_exp2l(0.0L, "zero");

	/* Subnormal scaling: k < LDBL_MIN_EXP */
	check_exp2l(-16380.0L, "subnorm");
	check_exp2l(-16381.0L, "subnorm2");

	/* k == LDBL_MAX_EXP special case */
	check_exp2l(16383.0L, "kmax");

	/* Representative interior values */
	{
		static const long double xs[] = {
			1.0L, -1.0L, 0.5L, -0.5L, 2.0L, -2.0L,
			10.0L, -10.0L, 100.0L, -100.0L,
			1000.0L, -1000.0L, 5000.0L, -5000.0L,
			0.00390625L, -0.00390625L,
			1.0L / 128.0L, -1.0L / 128.0L,
			1.0L / 64.0L, -1.0L / 64.0L,
			PI4, -PI4,
		};
		std::size_t i;

		for (i = 0; i < sizeof(xs) / sizeof(xs[0]); i++)
			check_exp2l(xs[i], "interior");
	}

	/* Bit-pattern edge values */
	for (std::size_t i = 0; i < NLDVEC; i++)
		check_exp2l(mkld(ldvec[i].se, ldvec[i].m), "bits");
}

static void
edge_cases(void)
{
	edge_kernel_tanl();
	edge_exp__D();
	edge_exp2l();
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
rng_ld80(void)
{
	std::uint64_t r = rng_next();
	std::uint16_t se = (std::uint16_t)(r & 0xffffu);
	std::uint64_t m = rng_next();

	if ((r >> 16) & 1)
		se |= 0x8000u;
	return mkld(se, m);
}

static long double
rng_tan_x(void)
{
	std::uint64_t r = rng_next();
	unsigned kind = (unsigned)(r % 100u);
	long double s;

	if (kind < 5)
		return (r & 1) ? -0.0L : 0.0L;
	if (kind < 12)
		return (r & 1) ? -BOUND : BOUND;
	if (kind < 20)
		return (r & 1) ? -BOUND * 1.1L : BOUND * 1.1L;
	if (kind < 28)
		return (r & 1) ? -PI4 : PI4;

	s = (long double)(rng_next() % 1000000u) / 1000000.0L;
	if (r & 1)
		s = -s;
	return s * BOUND * (1.0L + (long double)(rng_next() % 100u) / 1e12L);
}

static long double
rng_exp_x(void)
{
	std::uint64_t r = rng_next();
	unsigned kind = (unsigned)(r % 100u);
	int expn;

	if (kind < 8)
		return (r & 1) ? -0.0L : 0.0L;
	if (kind < 14)
		return rng_ld80();
	if (kind < 22) {
		expn = (int)(rng_next() % 20000u) - 10000;
		return ldexpl(1.0L + (long double)(rng_next() % 1000u) / 1000.0L,
		    expn);
	}
	expn = (int)(rng_next() % 33000u) - 16500;
	return ldexpl((long double)(rng_next() % 10000u) / 1000.0L, expn);
}

static const unsigned long long ITERS = 250000ull;

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		long double x, y, c;
		int iy;

		x = rng_tan_x();
		y = rng_exp_x() * 0.01L;
		iy = (int)((i & 1) ? 0 : (1 + (rng_next() % 255u)));
		if ((i % 97) == 0)
			iy = 0;
		if ((i % 89) == 0)
			iy = -1;
		if ((i % 83) == 0)
			iy = 1;
		if ((i % 79) == 0)
			iy = (int)rng_next();
		check_kernel_tanl(x, y, iy, "random");

		x = rng_exp_x();
		c = rng_exp_x() * 1e-8L;
		check_exp__D(x, c, "random");

		x = rng_exp_x();
		if ((i % 17) == 0)
			x = (long double)((int)(rng_next() % 33000u) - 16500);
		if ((i % 23) == 0)
			x = ldexpl(1.0L, (int)(rng_next() % 40u) - 80);
		check_exp2l(x, "random");
	}
}

/* ------------------------------------------------------------------ */

static void
row(const stat &s)
{
	std::printf("  %-18s %12llu %10llu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "PASS" : "FAIL");
}

int
main(void)
{
	unsigned long long fails;

	std::printf("pbsd batch b0094 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-18s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ------------------------------------------------------\n");
	row(st_kernel_tanl);
	row(st_exp__D);
	row(st_exp2l);

	fails = st_kernel_tanl.fails + st_exp__D.fails + st_exp2l.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
