/*
 * Differential test harness for PBSD batch b0095.
 *
 * powl, logl, log1pl, log10l, log2l — port vs ref_ oracle, bit for bit.
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld80.b0095;

namespace port = pbsd::lib_msun_ld80::b0095;

extern "C" {
long double ref_powl(long double x, long double y);
long double ref_logl(long double x);
long double ref_log1pl(long double x);
long double ref_log10l(long double x);
long double ref_log2l(long double x);
}

/* ------------------------------------------------------------------ */
/* raw long-double representation (ld80: 10 significant bytes)         */
/* ------------------------------------------------------------------ */

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

static bool
ld_equal(long double a, long double b)
{
	ldrep ra = ldbits(a);
	ldrep rb = ldbits(b);

	return std::memcmp(ra.b, rb.b, LD_SIG) == 0;
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

static const unsigned MAX_REPORT = 8;
static const unsigned long long ITERS = 200000ull;

static stat st_powl = { "powl", 0, 0, 0 };
static stat st_logl = { "logl", 0, 0, 0 };
static stat st_log1pl = { "log1pl", 0, 0, 0 };
static stat st_log10l = { "log10l", 0, 0, 0 };
static stat st_log2l = { "log2l", 0, 0, 0 };

static void
report_fail(stat &s, const char *tag, long double p, long double o)
{
	if (s.reported >= MAX_REPORT)
		return;
	s.reported++;
	std::printf("  %s FAIL [%s] port=", s.name, tag);
	ldhex(ldbits(p));
	std::printf(" ref=");
	ldhex(ldbits(o));
	std::printf("\n");
}

static void
check_unary(stat &s, long double x, const char *tag,
    long double (*pf)(long double), long double (*of)(long double))
{
	long double p = pf(x);
	long double o = of(x);

	s.cases++;
	if (ld_equal(p, o))
		return;
	s.fails++;
	report_fail(s, tag, p, o);
}

static void
check_binary(stat &s, long double x, long double y, const char *tag,
    long double (*pf)(long double, long double),
    long double (*of)(long double, long double))
{
	long double p = pf(x, y);
	long double o = of(x, y);

	s.cases++;
	if (ld_equal(p, o))
		return;
	s.fails++;
	report_fail(s, tag, p, o);
}

/* ------------------------------------------------------------------ */
/* hand-written edge vectors                                           */
/* ------------------------------------------------------------------ */

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
	{ 0x0000u, 0x7fffffffffffffffull },	/* largest +subnormal */
	{ 0x8000u, 0x7fffffffffffffffull },	/* largest -subnormal */
	{ 0x0001u, 0x8000000000000000ull },	/* smallest +normal */
	{ 0x8001u, 0x8000000000000000ull },	/* smallest -normal */
	{ 0x3ffeu, 0x8000000000000000ull },	/* +0.5 */
	{ 0xbffeu, 0x8000000000000000ull },	/* -0.5 */
	{ 0x3fffu, 0x8000000000000000ull },	/* +1 */
	{ 0xbfffu, 0x8000000000000000ull },	/* -1 */
	{ 0x3fffu, 0x8000000000000001ull },
	{ 0x3ffeu, 0xffffffffffffffffull },
	{ 0x3fffu, 0x8080808080808080ull },
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
	{ 0x7fffu, 0x8000000000000000ull },	/* +Inf */
	{ 0xffffu, 0x8000000000000000ull },	/* -Inf */
	{ 0x7fffu, 0xc000000000000000ull },	/* qNaN */
	{ 0xffffu, 0xc000000000000000ull },	/* -qNaN */
	{ 0x7fffu, 0x8000000000000001ull },	/* sNaN-ish */
	{ 0x3ffeu, 0xc90fdaa22168c235ull },	/* ~pi/2 */
	{ 0x4000u, 0x8000000000000000ull },	/* 2 */
	{ 0x4001u, 0x8000000000000000ull },	/* 4 */
	{ 0x3ffdu, 0x8000000000000000ull },	/* 0.25 */
	{ 0x4004u, 0x8000000000000000ull },	/* 16 */
	{ 0x400eu, 0x8000000000000000ull },	/* 2^15 */
	{ 0x401eu, 0x8000000000000000ull },	/* 2^30 */
};
static const std::size_t NLDVEC = sizeof(ldvec) / sizeof(ldvec[0]);

static void
edge_powl(void)
{
	std::size_t i, j;

	/* cross product of raw bit patterns */
	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++) {
			long double x = mkld(ldvec[i].se, ldvec[i].m);
			long double y = mkld(ldvec[j].se, ldvec[j].m);

			check_binary(st_powl, x, y, "cross", port::powl, ref_powl);
		}

	/* explicit algebraic edge cases from powl() control flow */
	{
		static const long double xs[] = {
			0.0L, -0.0L, 1.0L, -1.0L, 2.0L, -2.0L, 0.5L, -0.5L,
			1.000000000000001L, 0.999999999999999L,
			-1.000000000000001L, -0.999999999999999L,
			LDBL_MAX, -LDBL_MAX, LDBL_MIN, -LDBL_MIN,
			LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
			1e-4932L, -1e-4932L, 1e100L, -1e100L,
			3.0L, -3.0L, 10.0L, -10.0L,
			32767.0L, -32767.0L, 32768.0L, -32768.0L,
			0x1p10000L, -0x1p10000L,
			0.001L, 1000.0L, 0.99L, 1.01L,
			8700.0L, -8700.0L,
		};
		static const long double ys[] = {
			0.0L, -0.0L, 1.0L, -1.0L, 2.0L, -2.0L, 0.5L, -0.5L,
			3.0L, -3.0L, 0.25L, 4.0L, 16.0L,
			LDBL_MAX, -LDBL_MAX,
			1e-4932L, -1e-4932L, 1e100L, -1e100L,
			32767.0L, -32767.0L, 32768.0L, -32768.0L,
			8700.0L, -8700.0L,
			1.5L, -1.5L, 2.5L, -2.5L,
		};
		std::size_t a, b;

		for (a = 0; a < sizeof(xs) / sizeof(xs[0]); a++)
			for (b = 0; b < sizeof(ys) / sizeof(ys[0]); b++)
				check_binary(st_powl, xs[a], ys[b], "algebraic",
				    port::powl, ref_powl);
	}

	/* integer-exponent fast path: x and y both integers, |y| < 32768 */
	{
		static const long double ixs[] = {
			-32767.0L, -256.0L, -17.0L, -2.0L, -1.0L, 0.0L,
			1.0L, 2.0L, 3.0L, 7.0L, 17.0L, 256.0L, 32767.0L,
		};
		static const long double iys[] = {
			-32767.0L, -1024.0L, -2.0L, -1.0L, 0.0L, 1.0L,
			2.0L, 3.0L, 7.0L, 1024.0L, 32767.0L,
		};
		std::size_t a, b;

		for (a = 0; a < sizeof(ixs) / sizeof(ixs[0]); a++)
			for (b = 0; b < sizeof(iys) / sizeof(iys[0]); b++)
				check_binary(st_powl, ixs[a], iys[b], "intpow",
				    port::powl, ref_powl);
	}

	/* overflow / underflow boundaries: y * log2(x) near MEXP/MNEXP */
	{
		static const long double bx[] = {
			0x1.ffffp-1L, 0x1.0001p0L, 0x1.fffp0L, 0x1.001p0L,
			2.0L, 0.5L, 1e-4L, 1e4L,
		};
		static const long double by[] = {
			16384.0L * 32.0L, -(16384.0L * 32.0L + 64.0L),
			1000.0L, -1000.0L, 8700.0L,
		};
		std::size_t a, b;

		for (a = 0; a < sizeof(bx) / sizeof(bx[0]); a++)
			for (b = 0; b < sizeof(by) / sizeof(by[0]); b++)
				check_binary(st_powl, bx[a], by[b], "ovfl",
				    port::powl, ref_powl);
	}
}

static void
edge_log_unary(stat &s, long double (*pf)(long double),
    long double (*of)(long double), const char *pfx)
{
	std::size_t i;
	char tag[32];

	for (i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].m);

		std::snprintf(tag, sizeof(tag), "%s_raw", pfx);
		check_unary(s, x, tag, pf, of);
	}

	{
		static const long double xs[] = {
			0.0L, -0.0L, 1.0L, -1.0L, 2.0L, -2.0L,
			0.5L, -0.5L, 10.0L, -10.0L,
			LDBL_MAX, LDBL_MIN, LDBL_TRUE_MIN,
			1e-4932L, 1e-4000L, 1e-100L, 1e100L,
			0x1.0000000000001p0L, 0x1.fffffffffffffp0L,
			0x1.0000000000001p-1L, 0x1.fffffffffffffp-1L,
			0x1.008p0L, 0x1.ff8p0L,
			0x1p-65L, 0x1p65L, 0x1p128L,
			0x1.0000000000001p-64L,
		};
		std::size_t a;

		for (a = 0; a < sizeof(xs) / sizeof(xs[0]); a++) {
			std::snprintf(tag, sizeof(tag), "%s_alg", pfx);
			check_unary(s, xs[a], tag, pf, of);
		}
	}
}

static void
edge_log1pl(void)
{
	std::size_t i;
	char tag[32];

	for (i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].m);

		check_unary(st_log1pl, x, "raw", port::log1pl, ref_log1pl);
	}

	{
		static const long double xs[] = {
			0.0L, -0.0L,
			-1.0L + LDBL_EPSILON, -1.0L - LDBL_EPSILON,
			-1.0L, -0.999999999999999L, -0.5L,
			-1e-4000L, -1e-100L, -1e-20L, -1e-64L,
			1e-4000L, 1e-100L, 1e-20L, 1e-64L,
			1.0L, 2.0L, 10.0L, 1e30L, 1e100L,
			0x1p127L, 0x1p128L, 0x1.fffp127L,
			LDBL_MAX, LDBL_MIN, LDBL_TRUE_MIN,
		};
		std::size_t a;

		for (a = 0; a < sizeof(xs) / sizeof(xs[0]); a++) {
			std::snprintf(tag, sizeof(tag), "alg[%zu]", a);
			check_unary(st_log1pl, xs[a], tag, port::log1pl,
			    ref_log1pl);
		}
	}
}

static void
edge_cases(void)
{
	edge_powl();
	edge_log_unary(st_logl, port::logl, ref_logl, "logl");
	edge_log_unary(st_log10l, port::log10l, ref_log10l, "log10");
	edge_log_unary(st_log2l, port::log2l, ref_log2l, "log2");
	edge_log1pl();

	/* log table interval boundaries (128 intervals) */
	for (std::size_t i = 0; i <= 128; i++) {
		long double c = 1.0L + (long double)i / 128.0L;
		long double e = (long double)i / 128.0L - 0.5L / 128.0L;

		check_unary(st_logl, c, "interval", port::logl, ref_logl);
		check_unary(st_log10l, c, "interval", port::log10l, ref_log10l);
		check_unary(st_log2l, c, "interval", port::log2l, ref_log2l);
		check_unary(st_log1pl, e, "interval", port::log1pl, ref_log1pl);
		check_unary(st_log1pl, -e, "interval_neg", port::log1pl,
		    ref_log1pl);
	}
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
	std::uint64_t r = rng_next();
	std::uint16_t se;
	std::uint64_t m;
	unsigned kind = (unsigned)(r % 100u);

	if (kind < 4)
		return (r & 1) ? -0.0L : 0.0L;
	if (kind < 8)
		return (r & 1) ? -1.0L : 1.0L;
	if (kind < 14) {
		m = rng_next() | ((std::uint64_t)1 << 63);
		se = (std::uint16_t)((r & 1) ? 0x8000u : 0x0000u);
		return mkld(se, m & 0x7fffffffffffffffull);
	}
	if (kind < 20) {
		se = (std::uint16_t)(0x3fffu + (rng_next() % 64u));
		if (r & 1)
			se |= 0x8000u;
		m = rng_next() | 0x8000000000000000ull;
		return mkld(se, m);
	}
	if (kind < 26)
		return (long double)((int64_t)(rng_next() % 65535u) - 32767);

	se = (std::uint16_t)(0x3ffc + (rng_next() % 8));
	if (r & 2)
		se |= 0x8000u;
	m = (rng_next() & 0xfffffffffffffull) | 0x8000000000000000ull;
	return mkld(se, m);
}

static long double
rng_y(void)
{
	std::uint64_t r = rng_next();
	unsigned kind = (unsigned)(r % 100u);

	if (kind < 10)
		return (long double)((int)(rng_next() % 65535u) - 32767);
	if (kind < 20)
		return (long double)(rng_next() % 10000u) / 100.0L;
	if (kind < 30)
		return (r & 1) ? LDBL_MAX : -LDBL_MAX;
	if (kind < 40)
		return (r & 1) ? 1e100L : -1e100L;

	return rng_ld();
}

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		long double x = rng_ld();
		long double y = rng_y();

		check_binary(st_powl, x, y, "random", port::powl, ref_powl);

		if ((i % 3) == 0)
			check_unary(st_logl, x, "random", port::logl,
			    ref_logl);
		if ((i % 3) == 1)
			check_unary(st_log10l, x, "random", port::log10l,
			    ref_log10l);
		if ((i % 3) == 2)
			check_unary(st_log2l, x, "random", port::log2l,
			    ref_log2l);

		if ((i % 5) == 0) {
			long double t = (long double)(rng_next() % 1000000u) /
			    500000.0L - 1.0L;

			check_unary(st_log1pl, t, "random", port::log1pl,
			    ref_log1pl);
		}
		if ((i % 17) == 0) {
			long double t = rng_ld() * 1e30L;

			check_unary(st_log1pl, t, "random_big", port::log1pl,
			    ref_log1pl);
		}
		if ((i % 23) == 0) {
			long double t = -0.5L +
			    (long double)(rng_next() % 1000000u) / 1e15L;

			check_unary(st_log1pl, t, "random_near0", port::log1pl,
			    ref_log1pl);
		}
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

	std::printf("pbsd batch b0095 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-10s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ----------------------------------------------\n");
	row(st_powl);
	row(st_logl);
	row(st_log1pl);
	row(st_log10l);
	row(st_log2l);

	fails = st_powl.fails + st_logl.fails + st_log1pl.fails +
	    st_log10l.fails + st_log2l.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
